/*
 * Copyright (c) 2004 Scott E. Johnston
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Implementation of DrawServ class.
 */

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <DrawServ/ackback-handler.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawcomps.h>
#include <DrawServ/drawclasses.h>
#include <DrawServ/drawkit.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/linkcmd.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>
#include <DrawServ/linkselection.h>
#include <DrawServ/sid.h>

#include <FrameUnidraw/framecomps.h>

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/ulist.h>
#include <Unidraw/viewer.h>

#include <ComTerp/comterpserv.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <fstream.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <uuid/uuid.h>
#if !defined(__APPLE__) && !defined(IV_UUID_STRING_T_DEFINED)
#define IV_UUID_STRING_T_DEFINED
typedef char uuid_string_t[37];  /* Apple-only type; Linux libuuid lacks it */
#endif

using std::cout;
using std::cerr;

implementTable(GraphicIdTable,uint32_t,void*)
implementTable(SessionIdTable,uint32_t,void*)
implementTable(CompIdTable,void*,void*)

static int seed=0;

// utility function for grabbing key from uuid_t.
// Only needed until tables can be keyed on all 64 bits of the UUID.
// You win a lollipop if UUID8 fails you in the meantime.
extern uint32_t uuid_key(const uuid_t u)
{
    uint32_t v;
    memcpy(&v, u, sizeof(v));
    return ntohl(v);
}

/*****************************************************************************/

DrawServ::DrawServ (Catalog* c, int& argc, char** argv, 
		    OptionDesc* od, PropertyData* pd) 
: OverlayUnidraw(c, argc, argv, od, pd) {
  Init();
}

DrawServ::DrawServ (Catalog* c, World* w) 
: OverlayUnidraw(c, w) {
  Init();
}

void DrawServ::Init() {
  _linklist = new DrawLinkList;

  _gridtable = new GraphicIdTable(1024);
  _sessionidtable = new SessionIdTable(256);
  _compidtable = new CompIdTable(1024);

  _freeze_active = false;
  _freeze_qid = 0;
  _freeze_parent = nil;
  _freeze_sent_to = nil;
  _freeze_acks_pending = 0;
  _freeze_done = false;

  create_unique_sessionid();
  char hostbuf[HOST_NAME_MAX];
  gethostname(hostbuf, HOST_NAME_MAX);
  hostbuf[HOST_NAME_MAX-1] = '\0';  // gethostname needn't NUL-terminate on truncation
  const char* username = getlogin();
  if (!username) username = "";  // getlogin() is NULL with no login session (e.g. a CI runner)
  int pid = getpid();
  int hostid = gethostid();
  SessionId* sid = new SessionId(_sessionid, pid, username, hostbuf, hostid);
  _sessionidtable->insert(uuid_key(_sessionid), sid);

  _comdraw_port = atoi(unidraw->GetCatalog()->GetAttribute("comdraw"));
}

DrawServ::~DrawServ () 
{
  Iterator it;
  _linklist->First(it);
  while(_linklist->GetDrawLink(it) && !_linklist->Done(it)) {
    DrawLink* link = _linklist->GetDrawLink(it);
    _linklist->Next(it);
    linkdown(link);
  }
  delete _linklist;
  delete _gridtable;
  delete _sessionidtable;
  delete _compidtable;
}

DrawLink* DrawServ::linkup(const char* hostname, int portnum, 
		     int state, uuid_t link_id,  ComTerp* comterp,
		     int interactive) {

  if (comterp!=NULL) comterp->handler()->alt_fd(portnum);
  
  if (state == DrawLink::new_link || state == DrawLink::one_way) {
    
    DrawLink* link = new DrawLink(hostname, portnum, state);
    link->interactive(interactive);
    if (state==DrawLink::one_way && comterp && comterp->handler()) {
      ((DrawServHandler*)comterp->handler())->drawlink(link);
      link->comhandler((DrawServHandler*)comterp->handler());
    }
    if (link_id == NULL) {
      uuid_generate(link->linkid());
    } else {
      uuid_copy(link->linkid(), link_id);
    }
    if (link->open(link->linkid())==0 && link->ok()) {
      _linklist->add_drawlink(link);
      return link;
    } else {
      delete link;
      return nil;
    }
  } else if (state == DrawLink::two_way) {

    // search for the link this leg answers: one we opened and are still
    // waiting on, since an already-up link is not awaiting a leg.
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i) &&
	  (uuid_compare(_linklist->GetDrawLink(i)->linkid(), link_id)!=0 ||
	   _linklist->GetDrawLink(i)->state() != DrawLink::new_link))
      _linklist->Next(i);

    /* if found, finalize linkup */
    if (!_linklist->Done(i)) {
      DrawLink* curlink = _linklist->GetDrawLink(i);
      curlink->linkid(link_id);
      curlink->althostname(hostname);
      curlink->state(DrawLink::two_way);
      if (comterp && comterp->handler()) {
	((DrawServHandler*)comterp->handler())->drawlink(curlink);
	curlink->comhandler((DrawServHandler*)comterp->handler());
      }
      fprintf(stderr, "link up with %s(%s) via port %d\n",
	      curlink->hostname(), curlink->althostname(), portnum);
      // fprintf(stderr, "link id %.8s\n", curlink->linkid_str());

      /* register all sessionid's with other DrawServ */
      sessionid_register(curlink);
      SendCmdString(curlink, "sid(:all)");
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "drawlink(:linkid \"%s\" :state 2)\n", curlink->linkid_str());
      SendCmdString(curlink, buf);

      return curlink;
    } else {
      fprintf(stderr, "confirmation of two-way link\n");
      return nil;
    }
  } else {
    fprintf(stderr, "unexpected state of %d, nothing done\n", state);
    abort();
  }
}

int DrawServ::linkdown(DrawLink* link) {
  if (link && _linklist->Includes(link)) {
    if (link->has_pending_freeze()) {
      unfreeze_fragment(link->pending_freeze_qid());
      link->clear_pending_freeze();
    }
    freeze_link_down(link);
    Resource::ref(link);  // stops _linklist->Remove from deleting it right away
    _linklist->Remove(link);
    link->close();
    remove_sids(link);
    Resource::unref(link);
    return 0;
  } else
    return -1;
}

DrawLink* DrawServ::linkget(const char* hostname, int portnum) {
  DrawLink* link = nil;
  if (_linklist) {
    Iterator(i);
    _linklist->First(i);
    while (!_linklist->Done(i) && !link) {
      DrawLink* l = _linklist->GetDrawLink(i);
      if (strcmp(l->hostname(),hostname)==0 && l->portnum()==portnum)
	link = l;
      _linklist->Next(i);
    }
  }
  return link;
}

DrawLink* DrawServ::linkget(uuid_t sessionid) {
  void* ptr = nil;
  sessionidtable()->find(ptr, uuid_key(sessionid));
  return ptr ? ((SessionId*)ptr)->drawlink() : nil;
}

void DrawServ::linkdump(FILE* fptr) {
  fprintf(fptr, "Host                            Alt.                            Port    LID       State\n");
  fprintf(fptr, "------------------------------  ------------------------------  ------  --------  -----\n");
  if (_linklist) {
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i)) {
      DrawLink* link = _linklist->GetDrawLink(i);
      fprintf(fptr, "%-30.30s  %-30.30s  %-6d  %.8s  %-3d\n", 
	      link->hostname(), link->althostname(), link->portnum(),
	      link->linkid_str(), link->state());
      _linklist->Next(i);
    }
  }
}

void DrawServ::ExecuteCmd(Command* cmd) {
  uuid_t sid;
  uuid_t grid;
  uuid_clear(sid);
  uuid_clear(grid);
  
  boolean original = false;
  
  if(!_linklist || _linklist->Number()==0) 
    
    /* normal Unidraw command execution */
    Unidraw::ExecuteCmd(cmd);
  
  else {
    
    /* indirect command execution, all by script */
    std::ostringstream sbuf;
    boolean oldflag = OverlayScript::ptlist_parens();
    OverlayScript::ptlist_parens(false);
    switch (cmd->GetClassId()) {
    case PASTE_CMD:
      {
	boolean scripted = false;
	Clipboard* cb = cmd->GetClipboard();
	if (cb) {
	  Iterator it;
	  for (cb->First(it); !cb->Done(it); cb->Next(it)) {
	    OverlayComp* comp = (OverlayComp*)cb->GetComp(it);
	    
	    original = add_grid(comp, grid, sid);

	    if (comp && (original || linklist()->Number()>1)) {
	      Creator* creator = unidraw->GetCatalog()->GetCreator();
	      OverlayScript* scripter = (OverlayScript*)
		creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
	      if (scripter) {
		scripter->SetSubject(comp);
		if (comp->IsA(OVRASTER_COMP))
		  ((RasterScript*)scripter)->SetCommandSerialize(true);
		if (scripted) 
		  sbuf << ';';
		else 
		  scripted = true;
		boolean status = scripter->Definition(sbuf);
		delete scripter;
	      }
	    }
	  }
	}
	if (original || linklist()->Number()>1) {
	  if (!scripted)
	    fprintf(stderr, "Failed attempt to generate script for a PASTE_CMD|OV_IMPORT_CMD\n");
	}

	Iterator it;
	First(it);
	Editor* ed = GetEditor(it); // only 1 Editor per Unidraw
	Selection* sel = ed ? ed->GetSelection() : nil;
	if (sel) ((LinkSelection*)sel)->paste_in_progress_flag() = true;
	cmd->Execute();
	if (sel) ((LinkSelection*)sel)->paste_in_progress_flag() = false;
	break;
      }
      
      case LINK_BRUSH_CMD:
      {
	const char* script = ((LinkBrushCmd*)cmd)->dist_script();
	if (script && *script) sbuf << script;
	/* exclude the link back toward the change's owner so a relayed brush
	   flows onward along a chain instead of echoing to its origin. */
	uuid_copy(sid, ((LinkBrushCmd*)cmd)->dist_owner_sid());
	cmd->Execute();
	break;
      }

      case LINK_TRANSFORM_CMD:
      {
	const char* script = ((LinkTransformCmd*)cmd)->dist_script();
	if (script && *script) sbuf << script;
	uuid_copy(sid, ((LinkTransformCmd*)cmd)->dist_owner_sid());
	cmd->Execute();
	break;
      }

      case LINK_FONT_CMD:
      {
	const char* script = ((LinkFontCmd*)cmd)->dist_script();
	if (script && *script) sbuf << script;
	uuid_copy(sid, ((LinkFontCmd*)cmd)->dist_owner_sid());
	cmd->Execute();
	break;
      }

      case LINK_PATTERN_CMD:
      {
	const char* script = ((LinkPatternCmd*)cmd)->dist_script();
	if (script && *script) sbuf << script;
	uuid_copy(sid, ((LinkPatternCmd*)cmd)->dist_owner_sid());
	cmd->Execute();
	break;
      }

      case LINK_COLOR_CMD:
      {
	const char* script = ((LinkColorCmd*)cmd)->dist_script();
	if (script && *script) sbuf << script;
	uuid_copy(sid, ((LinkColorCmd*)cmd)->dist_owner_sid());
	cmd->Execute();
	break;
      }
      
      default:
	cmd->Execute();
	break;
      
    }
    
    /* then send everywhere else */
    if (original || linklist()->Number()>0) 
      DistributeCmdString(sbuf.str().c_str(), linkget(sid));
    
    if (cmd->Reversible()) {
      cmd->Log();
    } else {
      delete cmd;
    }

    OverlayScript::ptlist_parens(oldflag);
  }
}

boolean DrawServ::write_full(int fd, const char* buf, size_t len) {
  size_t sent = 0;
  static const int max_wait_usec = 2000000;
  static const int slice_usec    =    5000;
  int waited = 0;

  boolean ok = true;
  while (sent < len) {
    ssize_t n = write(fd, buf+sent, len-sent);
    if (n > 0) {
      sent += n;
    } else if (n < 0 && errno == EINTR) {
      continue;
    } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      if (waited >= max_wait_usec) { ok = false; break; }
      /* pumps other links/timers instead of sleeping dead -- same
	 primitive as update() (ctrlfunc.c) and resolve_requests() below. */
      ACE_Time_Value timeout(0, slice_usec);
      ComterpHandler::reactor_singleton()->handle_events(timeout);
      waited += slice_usec;
    } else {
      ok = false;
      break;
    }
  }
  return ok;
}

void DrawServ::DistributeCmdString(const char* cmdstring, DrawLink* orglink) {

  if (cmdstring==NULL || *cmdstring=='\0') return;

  /* targets are ref'd via Append(); write_full() pumps the reactor, so
     Includes() below re-validates each one before it's touched again. */
  DrawLinkList* targets = new DrawLinkList;
  Iterator i;
  _linklist->First(i);
  while (!_linklist->Done(i)) {
    DrawLink* link = _linklist->GetDrawLink(i);
    if (link && link != orglink && link->state()==DrawLink::two_way)
      targets->Append(link);
    _linklist->Next(i);
  }

  Iterator ti;
  targets->First(ti);
  while (!targets->Done(ti)) {
    DrawLink* link = targets->GetDrawLink(ti);
    if (_linklist->Includes(link)) {
      int fd = link->handle();
      if (fd>=0) {
	link->log_outgoing_command(cmdstring);
	if (!write_full(fd, cmdstring, strlen(cmdstring)) || !write_full(fd, "\n", 1))
	  fprintf(stderr, "drawserv: failed to send command to %s:%d (lid=%.8s): %s\n",
		  link->hostname(), link->portnum(), link->linkid_str(), strerror(errno));
	if (_linklist->Includes(link))
	  link->ackhandler()->start_timer();
      }
    }
    targets->Next(ti);
  }
  delete targets;
}

void DrawServ::SendCmdString(DrawLink* link, const char* cmdstring) {

  if (cmdstring==NULL || *cmdstring=='\0') return;

  if (link) {
    /* ref'd since write_full() pumps the reactor; Includes() below
       re-validates before start_timer() touches link again. */
    Resource::ref(link);
    int fd = link->handle();
    if (fd>=0) {
      link->log_outgoing_command(cmdstring);
      if (!write_full(fd, cmdstring, strlen(cmdstring)) || !write_full(fd, "\n", 1))
	fprintf(stderr, "drawserv: failed to send command to %s:%d (lid=%.8s): %s\n",
		link->hostname(), link->portnum(), link->linkid_str(), strerror(errno));
      if (_linklist->Includes(link))
	link->ackhandler()->start_timer();
    }
    Resource::unref(link);
  }
}

// generate request to register each locally unique session id
void DrawServ::sessionid_register(DrawLink* link) {
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more()) {
    if(it.cur_value()) {
      SessionId* sessionid = (SessionId*)it.cur_value();
      if (sessionid && sessionid->drawlink() != link) {
	char buf[BUFSIZ];
	uuid_string_t sid_str;
	uuid_unparse(sessionid->sid(), sid_str);
	snprintf(buf, BUFSIZ, "sid(\"%s\" :pid %d :user \"%s\" :host \"%s\" :hostid 0x%08x)%c", 
		 sid_str, sessionid->pid(), sessionid->username(),
		 sessionid->hostname(), sessionid->hostid(), '\0');
	SendCmdString(link, buf);
      }
    }
    it.next();
  }
}

// handle request to register session id
void DrawServ::sessionid_register_handle
(DrawLink* link, uuid_t sid, int pid,
 const char* username, const char* hostname, int hostid)
{
  if (link == NULL) return;

  /* a sid already on record via a different link marks that link
     redundant with this one -- bench it instead of re-propagating. */
  SessionIdTable* sidtable = ((DrawServ*)unidraw)->sessionidtable();
  void* ptr = nil;
  sidtable->find(ptr, uuid_key(sid));
  SessionId* known = (SessionId*)ptr;
  if (known) {
    DrawLink* via = known->drawlink();
    if (via == link) return;   // already recorded via this same link

    DrawLink* other = via;
    if (other == nil) {
      /* our own sid echoed back is expected, not a cycle; it's redundant
	 only if link's peer is already reached some other way -- find it. */
      Iterator it;
      _linklist->First(it);
      while (!_linklist->Done(it)) {
	DrawLink* o = _linklist->GetDrawLink(it);
	if (o != link && o->same_peer(link)) { other = o; break; }
	_linklist->Next(it);
      }
      if (!other) return;
    }

    /* compare linkid's, not arrival order -- a linkid is minted once by
       its initiator and copied unchanged to the far end, so every node
       comparing this pair reaches the same answer regardless of order. */
    DrawLink* loser = uuid_compare(link->linkid(), other->linkid()) < 0 ? other : link;

    /* never bench a node's only active link: redundant traffic once is
       cheap, losing the sole path out is a real disconnection. */
    if (sole_active_link(loser)) return;

    bench(loser);
    return;
  }

  SessionId* session_id = new SessionId(sid, pid, username, hostname, hostid, link);
  sidtable->insert(uuid_key(sid), session_id);

  /* propagate */
  sessionid_register_propagate(link, sid, pid, username,
			       hostname, hostid);
}

// propagate request to register session id
void DrawServ::sessionid_register_propagate
(DrawLink* link, uuid_t sid, int pid, 
 const char* username, const char* hostname, int hostid)
{
  Iterator it;
  _linklist->First(it);

  uuid_string_t sid_str;
  uuid_unparse(sid, sid_str);
  
  while (!_linklist->Done(it)) {
    char buf[BUFSIZ];
    DrawLink* otherlink = _linklist->GetDrawLink(it);
    if (otherlink != link && otherlink->state() == DrawLink::two_way) {
      snprintf(buf, BUFSIZ, "sid(\"%s\" :pid %d :user \"%s\" :host \"%s\" :hostid 0x%08x)%c", sid_str, pid, username, hostname, hostid, '\0');
      SendCmdString(otherlink, buf);
    }
    _linklist->Next(it);
  }
}

void DrawServ::unique_grid(uuid_t uuid) {
  uuid_generate(uuid);
}

int DrawServ::test_grid(uuid_t id) {
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  void* ptr = nil;
  table->find(ptr, uuid_key(id));
  if (ptr) 
    return 0;
  else
    return 1;
}

void DrawServ::create_unique_sessionid() {
  uuid_generate(_sessionid);
  uuid_unparse(_sessionid, _sessionid_str);
}

int DrawServ::test_sessionid(uuid_t id) {
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  void* ptr = nil;
  table->find(ptr, uuid_key(id));
  if (ptr) 
    return 0;
  else
    return 1;
}

void DrawServ::grid_message(GraphicId* grid) {
  char buf[BUFSIZ];
  if (grid->selected()==LinkSelection::LocallySelected ||
      (uuid_compare(grid->selector(), sessionid())==0 && grid->selected()==LinkSelection::NotSelected)) {
    snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :state %d :class \"%s\")%c", grid->idstr(), grid->selectorstr(), 
	     grid->selected()==LinkSelection::LocallySelected ? 
	     LinkSelection::RemotelySelected : LinkSelection::NotSelected,
	     grid->compclass(), '\0');
    DistributeCmdString(buf);
  } else {
    
    /* find link on which current selector lives */
    DrawLink* link = _linklist->find_drawlink(grid);
    
    if (link) {
      /* a fresh generation each time we ask, so a delayed answer can be
	 told apart from one for a request that replaced it. */
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :gen %d :class \"%s\")%c",
	       grid->idstr(), grid->selectorstr(), sessionidstr(),
	       grid->next_reqgen(), grid->compclass(), '\0');
      SendCmdString(link, buf);
    }
  }
}
  
// handle reserve request from remote DrawLink.
void DrawServ::grid_message_handle(DrawLink* link, uuid_t id, uuid_t selector, 
				   int state, uuid_t newselector, int gen)
{
  void* ptr = nil;
  gridtable()->find(ptr, uuid_key(id));
  uuid_string_t selector_str;
  selector_str[0] = '\0';
  if (selector != NULL && !uuid_is_null(selector))
    uuid_unparse(selector, selector_str);
  uuid_string_t newselector_str;
  newselector_str[0] = '\0';
  if ((newselector!= NULL) && !uuid_is_null(newselector))
    uuid_unparse(newselector, newselector_str);
  
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;

    /* if this request is aimed here */
    if (uuid_compare(selector,sessionid())==0 &&
	newselector != NULL && !uuid_is_null(newselector)) {

      /* if graphic is still locally owned */
	if (uuid_compare(grid->selector(), sessionid())==0) {

	/* if graphic is not actually selected */
	if ((grid->selected()==LinkSelection::NotSelected || 
	     grid->selected()==LinkSelection::WaitingToBeSelected)) {
	  grid->selected(LinkSelection::NotSelected);
	  grid->selector(newselector);
	  grid->grantgen(gen);
	  char buf[BUFSIZ];
	  snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :gen %d :class \"%s\")%c",
		   grid->idstr(), newselector_str, sessionidstr(),
		   gen, grid->compclass(), '\0');
	  SendCmdString(link, buf);
	  fprintf(stderr, "grid: request granted\n");
	} 

	  /* else deny it, because it is selected */
	else {
	  char buf[BUFSIZ];
	  /* asker in the selector field, us as the value, the same shape a
	     grant has, so a refusal can be relayed the same way a grant is. */
	  snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :deny \"%s\" :gen %d :class \"%s\")%c",
		   grid->idstr(), newselector_str, sessionidstr(),
		   gen, grid->compclass(), '\0');
	  SendCmdString(link, buf);
	  fprintf(stderr, "grid: request denied, graphic locally selected\n");
	}	
      } 
      
      /* else reformulate this request and pass it along */
      else if (linkget(grid->selector()) != link) {
	fprintf(stderr, "grid: request passed along to current selector\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :gen %d :class \"%s\")%c",
		 grid->idstr(), grid->selectorstr(), newselector_str,
		 gen, grid->compclass(), '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }

      /* our record points back where the request came from, so passing it
	 on would just return it; refuse instead so the asker gets an answer. */
      else {
	fprintf(stderr, "grid: request would go back where it came from, refused\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :deny \"%s\" :gen %d :class \"%s\")%c",
		 grid->idstr(), newselector_str, sessionidstr(),
		 gen, grid->compclass(), '\0');
	SendCmdString(link, buf);
      }
    }

    /* else this request and/or simple state update should be passed along */
    else {
      /* if simple state, set the values here, and pass it on to everyone else */
      if (((const char *)newselector)==NULL || uuid_is_null(newselector)) {
	if (linklist()->Number()>1)
	  fprintf(stderr, "grid: state change passed along to everyone else\n");

	/* "free, still held by the node we asked" is what our request is
	   waiting on, not news that voids it; every other change does. */
	if (!(grid->selected()==LinkSelection::WaitingToBeSelected &&
	      state==LinkSelection::NotSelected &&
	      selector != NULL &&
	      uuid_compare(grid->selector(), selector)==0)) {
	  if (grid->selected()==LinkSelection::WaitingToBeSelected) {
	    /* ownership moved while we were asking; nobody refused us, but
	       resolve it the way a denial resolves, since it reads the same. */
	    LinkSelection* lsel =
	      (LinkSelection*)DrawKit::Instance()->GetEditor()->GetSelection();
	    if (lsel) lsel->request_resolved_check(false, FILELINE);
	  }
	  grid->selector(selector);
	  grid->selected(state);
	}

	/* relay what was announced, not what we hold */
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :state %d :class \"%s\")%c",
		 grid->idstr(), selector_str, state,
		 grid->compclass(), '\0');
	DistributeCmdString(buf, link);
      } 

      /* else pass the request on to the target selector */
      else if (linkget(grid->selector()) != link) {
	fprintf(stderr, "grid:  request passed along to targeted selector\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :gen %d :class \"%s\")%c",
	  grid->idstr(), selector_str, newselector_str,
	  gen, grid->compclass(), '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }

      /* as above: back the way it came is not onward */
      else {
	fprintf(stderr, "grid:  request would go back where it came from, refused\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :deny \"%s\" :gen %d :class \"%s\")%c",
	  grid->idstr(), newselector_str, sessionidstr(),
	  gen, grid->compclass(), '\0');
	SendCmdString(link, buf);
      }
    }
  }
}

/* a refusal on its way back to whoever asked.  A request carries the asker and
   so does a grant, so both can be relayed; a refusal used to carry only the node
   that refused, which in a hub-and-spoke table is not where it has to go -- it
   was applied at the hub and dropped, and the spoke that asked waited for ever
   in WaitingToBeSelected. */
void DrawServ::grid_deny(DrawLink* link, uuid_t id, uuid_t requester,
			 uuid_t denier, int gen)
{
  void* ptr = nil;
  gridtable()->find(ptr, uuid_key(id));
  if (!ptr) return;
  GraphicId* grid = (GraphicId*)ptr;

  if (requester != NULL && !uuid_is_null(requester) &&
      uuid_compare(requester, sessionid())) {
    DrawLink* rlink = linkget(requester);
    if (rlink && rlink != link) {
      uuid_string_t requester_str;
      uuid_unparse(requester, requester_str);
      uuid_string_t denier_str;
      denier_str[0] = '\0';
      if (denier != NULL && !uuid_is_null(denier))
	uuid_unparse(denier, denier_str);
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :deny \"%s\" :gen %d :class \"%s\")%c",
	       grid->idstr(), requester_str, denier_str,
	       gen, grid->compclass(), '\0');
      SendCmdString(rlink, buf);
      fprintf(stderr, "grid: denial passed along to the node that asked\n");
    } else
      fprintf(stderr, "grid: denial undeliverable, dropped\n");
    return;
  }

  /* only while the question is still outstanding: applying a refusal
     after withdrawal or an ownership change would overwrite newer state. */
  if (grid->selected() != LinkSelection::WaitingToBeSelected) {
    fprintf(stderr, "grid: refusal for a request no longer outstanding, ignored\n");
    return;
  }

  /* and only for the asking it answers, so a delayed refusal for a
     withdrawn-then-repeated request is not applied to its successor. */
  if (gen != grid->reqgen()) {
    fprintf(stderr, "grid: refusal for asking %d, we are on %d now, ignored\n",
	    gen, grid->reqgen());
    return;
  }

  if (denier != NULL && !uuid_is_null(denier)) {
    grid->selected(LinkSelection::RemotelySelected);
    grid->selector(denier);
  }
  fprintf(stderr, "grid: request denied\n");
  LinkSelection* lsel =
    (LinkSelection*)DrawKit::Instance()->GetEditor()->GetSelection();
  if (lsel) lsel->request_resolved_check(false, FILELINE);
}

/* a grant of ours that the responder could not take.  the grant is identified,
   so a response that has been overtaken -- we have since handed the graphic to
   someone else -- is recognisable and ignored. */
void DrawServ::grid_notaken(DrawLink* link, uuid_t id, uuid_t responder,
			    uuid_t granter, int gen)
{
  void* ptr = nil;
  gridtable()->find(ptr, uuid_key(id));
  if (!ptr) return;
  GraphicId* grid = (GraphicId*)ptr;

  if (granter==NULL || uuid_is_null(granter)) return;

  /* not our grant: a grant reaches its recipient through linkget(selector),
     so the response travels back the same way, not stopping at this relay. */
  if (uuid_compare(granter, sessionid())) {
    DrawLink* glink = linkget(granter);
    if (glink && glink != link) {
      uuid_string_t responder_str;
      responder_str[0] = '\0';
      if (responder != NULL && !uuid_is_null(responder))
	uuid_unparse(responder, responder_str);
      uuid_string_t granter_str;
      uuid_unparse(granter, granter_str);
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :gen %d :notaken :class \"%s\")%c",
	       grid->idstr(), responder_str, granter_str,
	       gen, grid->compclass(), '\0');
      SendCmdString(glink, buf);
      fprintf(stderr, "grid: grant-not-taken passed along to granter\n");
    } else
      fprintf(stderr, "grid: grant-not-taken undeliverable, dropped\n");
    return;
  }

  /* the same asker can be granted the same graphic twice, so naming the
     responder alone can't tell which grant a not-taken answers; match gen too. */
  if (responder != NULL && uuid_compare(grid->selector(), responder)==0 &&
      gen==grid->grantgen()) {
    grid->selector(sessionid());
    fprintf(stderr, "grid: grant not taken, ownership restored here\n");
  } else
    fprintf(stderr, "grid: grant-not-taken overtaken, ignored\n");
}

// handle callback from remote DrawLink.
void DrawServ::grid_message_callback(DrawLink* link, uuid_t id, uuid_t selector, 
				     int state, uuid_t oldselector, int gen)
{
  void* ptr = nil;
  gridtable()->find(ptr, uuid_key(id));
  uuid_string_t selector_str;
  selector_str[0] = '\0';
  if (selector!= NULL) 
    uuid_unparse(selector, selector_str);
  uuid_string_t oldselector_str;
  oldselector_str[0] = '\0';
  if (oldselector!= NULL) 
    uuid_unparse(oldselector, oldselector_str);
  
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;

    /* if request is granted, add to selection */
    if (grid->selected()==LinkSelection::WaitingToBeSelected && selector != NULL &&
	uuid_compare(selector, sessionid())==0 &&
	gen==grid->reqgen()) {
      grid->selector(selector);
      grid->selected(LinkSelection::LocallySelected);

      
      fprintf(stderr, "grid:  request granted, add to selection now\n");
      LinkSelection* sel = (LinkSelection*)DrawKit::Instance()->GetEditor()->GetSelection();
      if (sel) {
	sel->request_resolved_check(true, FILELINE); // kaching upon 
	OverlayComp* comp = (OverlayComp*)grid->grcomp();
	sel->AddComp(comp);
	grid_message(grid);
      }
    }

    /* a grant addressed to us that we could not take: say so rather than
       forwarding it back to the granter, which would just send it here again. */
    else if (selector != NULL && uuid_compare(selector, sessionid())==0) {
      fprintf(stderr, "grid:  grant for us arrived on a graphic in %s, dropped\n",
	      LinkSelection::selected_string(grid->selected()));
      /* answer the grant, not a state message, which asserts unconditionally
	 and could undo a later handoff if stale; the granter needs to know. */
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :gen %d :notaken :class \"%s\")%c",
	       grid->idstr(), sessionidstr(), oldselector_str,
	       gen, grid->compclass(), '\0');
      SendCmdString(link, buf);
    }

    /* otherwise, pass the granting message along */
    else {
      fprintf(stderr, "grid:  pass grant request along\n");
      char buf[BUFSIZ];
      /* forward the generation, so a requester two hops away sees the real
	 value rather than zero, which would match every grant here. */
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :gen %d :class \"%s\")%c",
	       grid->idstr(), selector_str, oldselector_str,
	       gen, grid->compclass(), '\0');
      SendCmdString(linkget(selector), buf);
    }
  }
}

void DrawServ::print_gridtable() {
  GraphicIdTable* table = gridtable();
  GraphicIdTable_Iterator it(*table);
  printf("grid     comptype              selector  selected             unlocked\n");
  printf("-------- --------------------  --------  -------------------  --------\n");
  while(it.more()) {
    GraphicId* grid = (GraphicId*)it.cur_value();
    OverlayComp* comp = (OverlayComp*)grid->grcomp();
    const char* comptype = comp ? comp->GetClassName() : "nil";
    uuid_string_t idstr;
    uuid_unparse(grid->id(), idstr);
    printf("%.8s %-20s  %.8s  %-19s  %s\n",
 	   idstr, comptype,
 	   grid->selectorstr(), LinkSelection::selected_string(grid->selected()),
 	   grid->unlocked() ? "unlocked" : "locked");
    it.next();
  }
}

void DrawServ::print_sidtable() {
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  printf("key       sid       linkid   pid    hostid user             host            \n");
  printf("--------- --------  -------- ------ ------ ----             ----            \n");
  while(it.more()) {
    SessionId* sid = (SessionId*)it.cur_value();
    DrawLink* link = sid->drawlink();
    
    printf("%8x %.8s  %.8s %6d %6d %-16s %-16s\n", 
	   it.cur_key(), sid->sidstr(), link ? link->linkid_str() : "00000000", 
	   sid->pid(), sid->hostid(), sid->username(), sid->hostname());
    it.next();
  }
}

void DrawServ::remove_sids(DrawLink* link) {
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more()) {
    SessionId* sid = (SessionId*)it.cur_value();
    void* vsid = (void*)sid;
    DrawLink* testlink = sid->drawlink();
    int altid = it.cur_key();
    it.next();
    if (testlink==link)
      if (!table->find_and_remove(vsid, altid))
	fprintf(stderr, "unable to remove SessionId's associated with DrawLink\n");
  }
}

void DrawServ::bench(DrawLink* link) {
  if (link->state() == DrawLink::redundant) return;
  link->state(DrawLink::redundant);
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, "drawlink(:linkid \"%s\" :state %d)", link->linkid_str(), (int)DrawLink::redundant);
  SendCmdString(link, buf);
  char detail[BUFSIZ];
  snprintf(detail, BUFSIZ, "%s:%d", link->hostname() ? link->hostname() : "", link->portnum());
  link->report("Benched redundant connection", detail);
}

boolean DrawServ::sole_active_link(DrawLink* link) {
  Iterator it;
  _linklist->First(it);
  while (!_linklist->Done(it)) {
    DrawLink* l = _linklist->GetDrawLink(it);
    if (l != link && l->state() == DrawLink::two_way) return false;
    _linklist->Next(it);
  }
  return true;
}

void DrawServ::freeze_clear() {
  _freeze_active = false;
  _freeze_qid = 0;
  _freeze_parent = nil;
  delete _freeze_sent_to;
  _freeze_sent_to = nil;
  _freeze_acks_pending = 0;
  _freeze_done = false;
}

void DrawServ::freeze_request_handle(DrawLink* fromlink, freezeid_t qid) {
  // a request for a different qid than the one already held here can't be
  // serviced until that hold is released -- drop it rather than queue it,
  // so its sender times out and retries once this hold clears
  if (_freeze_active) return;

  _freeze_active = true;
  _freeze_qid = qid;
  _freeze_parent = fromlink;
  _freeze_done = false;
  _freeze_sent_to = new DrawLinkList;

  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, "drawlink(:frzid \"%08X\" :req)", qid);

  Iterator it;
  _linklist->First(it);
  while (!_linklist->Done(it)) {
    DrawLink* l = _linklist->GetDrawLink(it);
    if (l != fromlink && l->state() == DrawLink::two_way) {
      _freeze_sent_to->add_drawlink(l);
      SendCmdString(l, buf);
    }
    _linklist->Next(it);
  }

  // no other established links to relay to: this hold is already
  // complete, so ack back (or, if self-originated, finish) right away
  _freeze_acks_pending = _freeze_sent_to->Number();
  if (_freeze_acks_pending == 0) {
    if (_freeze_parent != nil) {
      snprintf(buf, BUFSIZ, "drawlink(:frzid \"%08X\" :ack)", qid);
      SendCmdString(_freeze_parent, buf);
    } else {
      _freeze_done = true;
    }
  }
}

void DrawServ::freeze_ack_handle(DrawLink* fromlink, freezeid_t qid) {
  if (!_freeze_active || _freeze_qid != qid) return; // stale or mismatched
  if (_freeze_acks_pending > 0) _freeze_acks_pending--;
  if (_freeze_acks_pending > 0) return;

  if (_freeze_parent != nil) {
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "drawlink(:frzid \"%08X\" :ack)", _freeze_qid);
    SendCmdString(_freeze_parent, buf);
  } else {
    _freeze_done = true; // originator: unblocks freeze_fragment()'s wait loop
  }
}

void DrawServ::freeze_release_handle(DrawLink* fromlink, freezeid_t qid) {
  if (!_freeze_active || _freeze_qid != qid) return; // stale or mismatched
  if (fromlink != nil && fromlink != _freeze_parent) return; // release only ever arrives from the parent direction

  if (_freeze_sent_to) {
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, "drawlink(:frzid \"%08X\" :thaw)", qid);
    Iterator it;
    _freeze_sent_to->First(it);
    while (!_freeze_sent_to->Done(it)) {
      DrawLink* l = _freeze_sent_to->GetDrawLink(it);
      if (l->state() == DrawLink::two_way) SendCmdString(l, buf);
      _freeze_sent_to->Next(it);
    }
  }
  freeze_clear();
}

boolean DrawServ::freeze_fragment(freezeid_t& qid_out, const uuid_t linkid) {
  // a hold already active here belongs to some other freeze -- waiting it
  // out would risk a deadlock symmetric with a peer doing the same (each
  // side blocked on the other's release, when each release depends on the
  // other side answering first). Decline on the spot instead: if this
  // node is already frozen for something else, no linkup happens this
  // round, whether or not it would have been a cycle -- the caller can
  // simply try again once whatever holds the freeze now has cleared.
  if (_freeze_active) return false;

  qid_out = (linkid[0]<<24) | (linkid[1]<<16) | (linkid[2]<<8) | linkid[3];
  freeze_request_handle(nil, qid_out);
  if (_freeze_done) return true; // no established links to flood to: trivially frozen

  static const int max_wait_usec = 5000000;  // 5 second overall timeout, matching linkup()'s handshake wait
  static const int slice_usec    =    5000;
  int elapsed = 0;

  long oldsec, oldusec;
  get_timeout(oldsec, oldusec);
  while (!_freeze_done && elapsed < max_wait_usec) {
    set_timeout(0, slice_usec);
    Run();
    elapsed += slice_usec;
  }
  set_timeout(oldsec, oldusec);

  if (!_freeze_done) {
    freeze_release_handle(nil, _freeze_qid); // release whatever partial hold accumulated, and clear it
    return false;
  }
  return true;
}

void DrawServ::unfreeze_fragment(freezeid_t qid) {
  if (_freeze_active && _freeze_qid == qid)
    freeze_release_handle(nil, qid);
}

void DrawServ::freeze_link_down(DrawLink* link) {
  if (!_freeze_active) return;

  if (link == _freeze_parent) {
    /* the direction the held freeze would eventually ack back to, or
       receive its release from, is gone -- treat it exactly like a
       release arriving from that direction: thaw whatever children this
       node has relayed to and drop the hold, rather than leave a stale
       pointer for a later ack or release to dereference. */
    freeze_release_handle(link, _freeze_qid);
    return;
  }

  if (_freeze_sent_to && _freeze_sent_to->Includes(link)) {
    /* a relay target this node is still waiting on died before acking --
       its loss can't block the hold forever, so count it as answered and
       drop it before a later release flood can reach it. */
    _freeze_sent_to->Remove(link);
    if (_freeze_acks_pending > 0) _freeze_acks_pending--;
    if (_freeze_acks_pending > 0) return;
    if (_freeze_parent != nil) {
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "drawlink(:frzid \"%08X\" :ack)", _freeze_qid);
      SendCmdString(_freeze_parent, buf);
    } else {
      _freeze_done = true;
    }
  }
}

boolean DrawServ::cycletest(uuid_t sid, const char* host, const char* user, int pid)
{
  boolean found = false;
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more() && !found) {
    SessionId* sessionid = (SessionId*)it.cur_value();
    uuid_t& ssid = sessionid->sid();
    if (uuid_compare(ssid, sid)==0) {
      if (strcmp(host, sessionid->hostname())==0 && 
	  strcmp(user, sessionid->username())==0 &&
	  pid==sessionid->pid())
	found = true;
    }
    it.next();
  }
  return found;
}

boolean DrawServ::selftest(const char* host, unsigned int portnum) 
{
  if (portnum==comdraw_port()) {
    if (strcmp(host, "localhost")==0 ||
	strcmp(host, "127.0.0.1")==0)
      return 1;
    else {
      char hostbuf[HOST_NAME_MAX];
      gethostname(hostbuf, HOST_NAME_MAX);
      hostbuf[HOST_NAME_MAX-1] = '\0';  // gethostname needn't NUL-terminate on truncation
      if (strcmp(host, hostbuf)==0)
	return 1;
    }
  }
  return 0;
}

boolean DrawServ::PrintAttributeList(ostream& out, AttributeList* attrlist) {
  static int grid_sym = symbol_add("grid");
  static int sid_sym = symbol_add("sid");

  ALIterator i;
  for (attrlist->First(i); !attrlist->Done(i); attrlist->Next(i)) {
    Attribute* attr = attrlist->GetAttr(i);
    out << " :" << attr->Name() << " ";
    AttributeValue* attrval = attr->Value();
    boolean special = attr->SymbolId()==grid_sym || attr->SymbolId()==sid_sym;
    out << *attrval;
  }
  return true;
}

void DrawServ::SendAllToBackgroundEditor(DrawLink* link, DrawEditor* fged) {

    boolean original = false;
    uuid_t grid; uuid_clear(grid);
    uuid_t sid; uuid_clear(sid);

    // fged->GetSelection()->Clear();
    
    std::ostringstream sbuf;
    boolean oldflag = OverlayScript::ptlist_parens();
    OverlayScript::ptlist_parens(false);
    boolean scripted = false;
    DrawIdrawComp* idrawcomp = (DrawIdrawComp*)(fged->GetComponent()->IsA(DRAW_IDRAW_COMP) ? fged->GetComponent() : nil);
    
    if (idrawcomp) {
	Iterator it;
	idrawcomp->First(it);
	FrameComp* bgfcomp = (FrameComp*)idrawcomp->GetComp(it);
	if (bgfcomp) {
	    for (bgfcomp->First(it); !bgfcomp->Done(it); bgfcomp->Next(it)) {
		OverlayComp* comp = (OverlayComp*)bgfcomp->GetComp(it);
		original = add_grid(comp, grid, sid);

		if (comp && (original || linklist()->Number()>1)) {
		    Creator* creator = unidraw->GetCatalog()->GetCreator();
		    OverlayScript* scripter = (OverlayScript*)
			creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
		    if (scripter) {
			scripter->SetSubject(comp);
			if (comp->IsA(OVRASTER_COMP))
			  ((RasterScript*)scripter)->SetCommandSerialize(true);
			if (scripted) 
			    sbuf << ';';
			else 
			    scripted = true;
			boolean status = scripter->Definition(sbuf);
			delete scripter;
		    }
		}
	    }
	}
    }

    /* then send to new background connection pasted in front */
    if (original || linklist()->Number()>1) {
	sbuf << "\n";
	SendCmdString(link, sbuf.str().c_str());
    }
    
}
    
void DrawServ::SendAllToForegroundEditor(DrawLink* link, DrawEditor* bged) {
  
    boolean original = false;
    uuid_t grid; uuid_clear(grid);
    uuid_t sid; uuid_clear(sid);
    
    std::ostringstream sbuf;
    boolean oldflag = OverlayScript::ptlist_parens();
    OverlayScript::ptlist_parens(false);
    boolean scripted = false;
    DrawIdrawComp* idrawcomp = (DrawIdrawComp*)(bged->GetComponent()->IsA(DRAW_IDRAW_COMP) ? bged->GetComponent() : nil);
    
    if (idrawcomp) {
	Iterator it;
	idrawcomp->First(it);
	FrameComp* bgfcomp = (FrameComp*)idrawcomp->GetComp(it);
	if (bgfcomp) {
	    for (bgfcomp->Last(it); !bgfcomp->Done(it); bgfcomp->Prev(it)) {
		OverlayComp* comp = (OverlayComp*)bgfcomp->GetComp(it);
		original = add_grid(comp, grid, sid);
		
		if (comp && (original || linklist()->Number()>1)) {
		    Creator* creator = unidraw->GetCatalog()->GetCreator();
		    OverlayScript* scripter = (OverlayScript*)
			creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
		    if (scripter) {
			scripter->SetSubject(comp);
			if (comp->IsA(OVRASTER_COMP))
			    ((RasterScript*)scripter)->SetCommandSerialize(true);
			if (scripted) 
			    sbuf << ';';
			else 
			    scripted = true;
			sbuf << "_comp=";
			boolean status = scripter->Definition(sbuf);
			sbuf << ";back(_comp)";
			delete scripter;
		    }
		}
	    }
	}
    }
    
    /* then send to new foreground connection pasted in front and moved to back */
    if (original || linklist()->Number()>1) {
	sbuf << "\n";
	SendCmdString(link, sbuf.str().c_str());
    }
    
}

boolean DrawServ::add_grid(OverlayComp* comp, uuid_t grid, uuid_t sid) {
    boolean original = false;
    static int grid_sym = symbol_add("grid");
    static int sid_sym = symbol_add("sid");
    
    AttributeList* al = comp->GetAttributeList();
    if (al!=NULL) {
	
	AttributeValue *gridv = al->find(grid_sym);
	if (gridv!=NULL && gridv->is_string()) {
	  uuid_parse(gridv->string_ptr(), grid);
	}
	
	AttributeValue *sidv = al->find(sid_sym);
	if (sidv!=NULL && sidv->is_string()) {
	  uuid_parse(sidv->string_ptr(), sid);
	}
	
	
    }
    
    /* unique id already assigned */
    if (!uuid_is_null(grid) && !uuid_is_null(sid)) {
	void *ptr = nil;
	if (gridtable()->find(ptr, uuid_key(grid))) {
	    GraphicId* graphicid = (GraphicId*)ptr;
	    // graphicid->selected(LinkSelection::WaitingToBeSelected);
	} else {
	    GraphicId* graphicid = new GraphicId(sid);
	    graphicid->grcomp(comp);
	    graphicid->set_id(grid);
	    graphicid->selector(sid);
	    graphicid->selected(LinkSelection::NotSelected);
	}
    } 
    
    /* generate unique id and add as attribute */
    /* also mark with selector id */
    else {
	original = true;
	GraphicId* graphicid = new GraphicId(((DrawServ*)unidraw)->sessionid());
	grid = graphicid->generate_id();
	graphicid->grcomp(comp);
	graphicid->selector(((DrawServ*)unidraw)->sessionid());

	uuid_copy(grid, graphicid->id());
	uuid_string_t grid_str;
	uuid_unparse(grid, grid_str);
	AttributeValue* gridv = new AttributeValue(grid_str);
	al->add_attr(grid_sym, gridv);
	
	uuid_copy(sid, graphicid->selector());
	uuid_string_t sid_str;
	uuid_unparse(sid, sid_str);
	AttributeValue* sidv = new AttributeValue(sid_str);
	al->add_attr(sid_sym, sidv);

	#if 0
	Editor* ed = DrawKit::Instance()->GetEditor();
	OverlaySelection* sel = (OverlaySelection*)ed->GetViewer()->GetSelection();
	Iterator it;
	boolean is_selected = false;
	for (sel->First(it); !sel->Done(it); sel->Next(it)) {
	    OverlayView* view = (OverlayView*)sel->GetView(it);
	    if (view && view->GetOverlayComp() == comp) {
		is_selected = true;
		break;
	    }
	}
	graphicid->selected(is_selected ? 
			    (uuid_compare(graphicid->selector(),sessionid())==0 ? LinkSelection::LocallySelected : LinkSelection::WaitingToBeSelected) : 
			    LinkSelection::NotSelected);
	#endif
	graphicid->selected(LinkSelection::LocallySelected);  // this assumes an initial paste that leaves the graphic in the clipboard
    }
    return original;
}
    
