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

#include <DrawServ/ackback-handler.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawkit.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>
#include <DrawServ/linkselection.h>
#include <DrawServ/sid.h>

#include <OverlayUnidraw/ovclasses.h>
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
#include <strstream>
#include <unistd.h>

#ifdef HAVE_ACE
implementTable(GraphicIdTable,int,void*)
implementTable(SessionIdTable,int,void*)
implementTable(CompIdTable,void*,void*)

unsigned int DrawServ::GraphicIdMask = 0x000fffff;
unsigned int DrawServ::SessionIdMask = 0xfff00000;

static int seed=0;
#endif /* HAVE_ACE */

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
#ifdef HAVE_ACE
  _linklist = new DrawLinkList;

  _gridtable = new GraphicIdTable(1024);
  _sessionidtable = new SessionIdTable(256);
  _compidtable = new CompIdTable(1024);

  _sessionid = unique_sessionid();
#if 0
  _sessionid = 0xfff00000;  // for testing purposes
#endif
  char hostbuf[HOST_NAME_MAX];
  gethostname(hostbuf, HOST_NAME_MAX);
  char* username = getlogin();
  int pid = getpid();
  int hostid = gethostid();
  SessionId* sid = new SessionId(_sessionid, _sessionid, pid, username, hostbuf, hostid);
  _sessionidtable->insert(_sessionid, sid);

  _comdraw_port = atoi(unidraw->GetCatalog()->GetAttribute("comdraw"));
#endif /* HAVE_ACE */
}

DrawServ::~DrawServ () 
{
#ifdef HAVE_ACE
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
#endif /* HAVE_ACE */
}

#ifdef HAVE_ACE

DrawLink* DrawServ::linkup(const char* hostname, int portnum, 
		     int state, int local_id, int remote_id,
		     ComTerp* comterp) {
  if (state == DrawLink::new_link || state == DrawLink::one_way) {
    DrawLink* link = new DrawLink(hostname, portnum, state);
    link->remote_linkid(remote_id);
    if (state==DrawLink::one_way && comterp && comterp->handler()) {
      ((DrawServHandler*)comterp->handler())->drawlink(link);
      link->comhandler((DrawServHandler*)comterp->handler());
    }
    if (link->open()==0 && link->ok()) {
      _linklist->add_drawlink(link);
      return link;
    } else {
      delete link;
      return nil;
    }
  } else {

    // search for existing link with matching local_id
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i) && _linklist->GetDrawLink(i)->local_linkid()!=local_id)
      _linklist->Next(i);

    /* if found, finalize linkup */
    if (!_linklist->Done(i)) {
      DrawLink* curlink = _linklist->GetDrawLink(i);
      curlink->remote_linkid(remote_id);
      curlink->althostname(hostname);
      curlink->state(DrawLink::two_way);
      if (comterp && comterp->handler()) {
	((DrawServHandler*)comterp->handler())->drawlink(curlink);
	curlink->comhandler((DrawServHandler*)comterp->handler());
      }
      fprintf(stderr, "link up with %s(%s) via port %d\n", 
	      curlink->hostname(), curlink->althostname(), portnum);
      fprintf(stderr, "local id %d, remote id %d\n", curlink->local_linkid(), 
	      curlink->remote_linkid());

      /* register all sessionid's with other DrawServ */
      sessionid_register(curlink);
      SendCmdString(curlink, "sid(:all)");
      SendCmdString(curlink, "drawlink(:state 2)");

      return curlink;
    } else {
      fprintf(stderr, "unable to complete two-way link\n");
      return nil;
    }
  }
}

int DrawServ::linkdown(DrawLink* link) {
  if (link && _linklist->Includes(link)) {
    _linklist->Remove(link);
    link->close();
    remove_sids(link);
    delete link;
    return 0;
  } else
    return -1;
}

DrawLink* DrawServ::linkget(int local_id, int remote_id) {
  DrawLink* link = nil;
  if (_linklist) {
    Iterator(i);
    _linklist->First(i);
    while (!_linklist->Done(i) && !link) {
      DrawLink* l = _linklist->GetDrawLink(i);
      if (l->local_linkid()==local_id && remote_id==-1 ||
	  local_id==-1 && l->remote_linkid()==remote_id ||
	  l->local_linkid()==local_id && l->remote_linkid()==remote_id)
	link = l;
      _linklist->Next(i);
    }
  }
  return link;
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

DrawLink* DrawServ::linkget(unsigned int sessionid) {
  void* ptr = nil;
  if (sessionid>0) sessionidtable()->find(ptr, sessionid);
  return ptr ? ((SessionId*)ptr)->drawlink() : nil;
}

void DrawServ::linkdump(FILE* fptr) {
  fprintf(fptr, "Host                            Alt.                            Port    LID  RID  State\n");
  fprintf(fptr, "------------------------------  ------------------------------  ------  ---  ---  -----\n");
  if (_linklist) {
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i)) {
      DrawLink* link = _linklist->GetDrawLink(i);
      fprintf(fptr, "%-30.30s  %-30.30s  %-6d  %-3d  %-3d  %-3d\n", 
	      link->hostname(), link->althostname(), link->portnum(),
	      link->local_linkid(), link->remote_linkid(), link->state());
      _linklist->Next(i);
    }
  }
}

void DrawServ::ExecuteCmd(Command* cmd) {
  static int grid_sym = symbol_add("grid");
  static int sid_sym = symbol_add("sid");
  boolean original = false;
  unsigned int from_sid = 0;

  if(!_linklist || _linklist->Number()==0) 

    /* normal Unidraw command execution */
    Unidraw::ExecuteCmd(cmd);

  else {

    /* indirect command execution, all by script */
    std::ostrstream sbuf;
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
	  AttributeList* al = comp->GetAttributeList();
	  AttributeValue* idv = al->find(grid_sym);
	  AttributeValue* sidv = al->find(sid_sym);
	  from_sid = sidv ? sidv->uint_val() : 0;
	  
	  /* unique id already remotely assigned */
	  if (idv && idv->uint_val() !=0 && sidv && sidv->uint_val() !=0) {
	    GraphicId* graphicid = new GraphicId();
	    graphicid->grcomp(comp);
	    graphicid->id(idv->uint_val());
	    graphicid->selector(sidv->uint_val());
	    graphicid->selected(LinkSelection::RemotelySelected);
	  } 
	  
	  /* generate unique id and add as attribute */
	  /* also mark with selector id */
	  else {
	    GraphicId* graphicid = new GraphicId(sessionid());
	    graphicid->grcomp(comp);
	    graphicid->selector(((DrawServ*)unidraw)->sessionid());
	    AttributeValue* gridv = new AttributeValue(graphicid->id(), AttributeValue::UIntType);
	    gridv->state(AttributeValue::HexState);
	    al->add_attr(grid_sym, gridv);
	    AttributeValue* sidv = new AttributeValue(graphicid->selector(), AttributeValue::UIntType);
	    sidv->state(AttributeValue::HexState);
	    al->add_attr(sid_sym, sidv);
	    original = true;
	  }
	    
	  if (comp && (original || linklist()->Number()>1)) {
	    Creator* creator = unidraw->GetCatalog()->GetCreator();
	    OverlayScript* scripter = (OverlayScript*)
	      creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
	    if (scripter) {
	      scripter->SetSubject(comp);
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
	  sbuf << "print(\"Failed attempt to generate script for a PASTE_CMD\\n\" :err)";
	sbuf.put('\0');
	cout << sbuf.str() << "\n";
	cout.flush();
      }

      /* first execute here */
#if 0
      ((ComEditor*)cmd->GetEditor())->GetComTerp()->run(sbuf.str());
      ((PasteCmd*)cmd)->executed(true);
#else
      cmd->Execute();
#endif

      /* then send everywhere else */
      if (original || linklist()->Number()>1) 
	DistributeCmdString(sbuf.str(), linkget(from_sid));
      
      }
      break;
    default:
      sbuf << "print(\"Attempt to convert unknown command (id == %d) to interpretable script\\n\" " << cmd->GetClassId() << " :err)";
      cmd->Execute();
      break;
    }

    if (cmd->Reversible()) {
      cmd->Log();
    } else {
      delete cmd;
    }

    OverlayScript::ptlist_parens(oldflag);
  }
}

void DrawServ::DistributeCmdString(const char* cmdstring, DrawLink* orglink) {

  Iterator i;
  _linklist->First(i);
  while (!_linklist->Done(i)) {
    DrawLink* link = _linklist->GetDrawLink(i);
    if (link && link != orglink && link->state()==DrawLink::two_way) {
      int fd = link->handle();
      if (fd>=0) {
	fileptr_filebuf fbuf(fd, ios_base::out, false, static_cast<size_t>(BUFSIZ));
	ostream out(&fbuf);
	out << cmdstring;
	out << "\n";
	out.flush();
	link->ackhandler()->start_timer();
      }
    }
    _linklist->Next(i);
  }

}

void DrawServ::SendCmdString(DrawLink* link, const char* cmdstring) {

  if (link) {
    int fd = link->handle();
    if (fd>=0) {
      fileptr_filebuf fbuf(fd, ios_base::out, false, static_cast<size_t>(BUFSIZ));
      ostream out(&fbuf);
      out << cmdstring;
      out << "\n";
      out.flush();
      link->ackhandler()->start_timer();
    }
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
	snprintf(buf, BUFSIZ, "sid(0x%08x 0x%08x :pid %d :user \"%s\" :host \"%s\" :hostid 0x%08x)%c", 
		 sessionid->sid(), sessionid->osid(), 
		 sessionid->pid(), sessionid->username(),
		 sessionid->hostname(), sessionid->hostid(), '\0');
	SendCmdString(link, buf);
      }
    }
    it.next();
  }
}

// handle request to register session id
void DrawServ::sessionid_register_handle
(DrawLink* link, unsigned int sid, unsigned osid, int pid, 
 const char* username, const char* hostname, int hostid) 
{
  if (link) {
    SessionIdTable* sidtable = ((DrawServ*)unidraw)->sessionidtable();
    unsigned int alt_id = sid;
    if (!DrawServ::test_sessionid(sid)) 
      alt_id = DrawServ::unique_sessionid();
    SessionId* session_id = new SessionId(alt_id, osid, pid, username, hostname, hostid, link);
    sidtable->insert(alt_id, session_id);
    link->sid_insert(sid, alt_id);
    if (sid != alt_id) {
      char buffer[BUFSIZ];
      snprintf(buffer, BUFSIZ, "sid(0x%08x 0x%08x :remap)%c", sid, alt_id, '\0');
      SendCmdString(link, buffer);
    }

    /* propagate */
    sessionid_register_propagate(link, alt_id, osid, pid, username,
				 hostname, hostid);
  }
}

// propagate request to register session id
void DrawServ::sessionid_register_propagate
(DrawLink* link, unsigned int sid, unsigned int osid, int pid, 
 const char* username, const char* hostname, int hostid)
{
  Iterator it;
  _linklist->First(it);
  while (!_linklist->Done(it)) {
    char buf[BUFSIZ];
    DrawLink* otherlink = _linklist->GetDrawLink(it);
    if (otherlink != link) {
      snprintf(buf, BUFSIZ, "sid(0x%08x 0x%08x :pid %d :user \"%s\" :host \"%s\" :hostid 0x%08x)%c", sid, osid, pid, username, hostname, hostid, '\0');
      SendCmdString(otherlink, buf);
    }
    _linklist->Next(it);
  }
}

unsigned int DrawServ::unique_grid() {
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  unsigned int retval;
  do {
    static int flip=0;
    while ((retval=rand()&GraphicIdMask)<=1);

  } while (!test_grid(retval));
  return retval;
}

int DrawServ::test_grid(unsigned int id) {
  GraphicIdTable* table = ((DrawServ*)unidraw)->gridtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

unsigned int DrawServ::unique_sessionid() {
  if (!seed) {
    seed = time(nil) & (time(nil) << 16);
    srand(seed);
  }
  int retval;
  do {
    while ((retval=rand()&SessionIdMask)==0);

  } while (!test_sessionid(retval));
  return retval;
}

int DrawServ::test_sessionid(unsigned int id) {
  SessionIdTable* table = ((DrawServ*)unidraw)->sessionidtable();
  void* ptr = nil;
  table->find(ptr, id);
  if (ptr) 
    return 0;
  else
    return 1;
}

void DrawServ::grid_message(GraphicId* grid) {
  char buf[BUFSIZ];
  if (grid->selected()==LinkSelection::LocallySelected ||
      (grid->selector()==sessionid() && grid->selected()==LinkSelection::NotSelected)) {
    snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :state %d )%c", grid->id(), grid->selector(), 
	     grid->selected()==LinkSelection::LocallySelected ? 
	     LinkSelection::RemotelySelected : LinkSelection::NotSelected, '\0');
    DistributeCmdString(buf);
  } else {
    
    /* find link on which current selector lives */
    DrawLink* link = _linklist->find_drawlink(grid);
    
    if (link) {
      snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :request chgid(0x%08x))%c", grid->id(), 
	       grid->selector(), sessionid(), '\0');
      SendCmdString(link, buf);
    }
  }
}
  
// handle reserve request from remote DrawLink.
void DrawServ::grid_message_handle(DrawLink* link, unsigned int id, unsigned int selector, 
				   int state, unsigned int newselector)
{
  void* ptr = nil;
  gridtable()->find(ptr, id);
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;

    /* if this request is aimed here */
    if (selector==sessionid() && newselector!=0) {

      /* if graphic is still locally owned */
      if (grid->selector()==sessionid()) {

	/* if graphic is not actually selected */
	if ((grid->selected()==LinkSelection::NotSelected || 
	     grid->selected()==LinkSelection::WaitingToBeSelected)) {
	  grid->selected(LinkSelection::NotSelected);
	  grid->selector(newselector);
	  char buf[BUFSIZ];
	  snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :grant chgid(0x%08x))%c",
		   grid->id(), newselector, sessionid(), '\0');
	  SendCmdString(link, buf);
	  fprintf(stderr, "grid: request granted\n");
	} 

	/* else do nothing, because it is selected */
	else {
	  fprintf(stderr, "grid: request ignored, graphic locally selected\n");
	}
      } 
      
      /* else reformulate this request and pass it along */
      else {
	fprintf(stderr, "grid: request passed along to current selector\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :request chgid(0x%08x))%c",
		 grid->id(), grid->selector(), newselector, '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }
    }

    /* else this request and/or simple state update should be passed along */
    else {
      /* if simple state, set the values here, and pass it on to everyone else */
      if (newselector==0) {
	if (linklist()->Number()>1)
	  fprintf(stderr, "grid: state change passed along to everyone else\n");
	grid->selector(selector);
	grid->selected(state);
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :state %d)%c",
		 grid->id(), grid->selector(), grid->selected(), '\0');
	DistributeCmdString(buf, link);
      } 

      /* else pass the request on to the target selector */
      else {
	fprintf(stderr, "grid:  request passed along to targeted selector");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :request chgid(0x%08x))%c",
		 grid->id(), selector, newselector, '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }
    }
  }
}

// handle callback from remote DrawLink.
void DrawServ::grid_message_callback(DrawLink* link, unsigned int id, unsigned int selector, 
				     int state, unsigned int oldselector)
{
  void* ptr = nil;
  gridtable()->find(ptr, id);
  if (ptr) {
    GraphicId* grid = (GraphicId*)ptr;

    /* if request is granted, add to selection */
    if (grid->selected()==LinkSelection::WaitingToBeSelected && selector==sessionid()) {
      grid->selector(selector);
      fprintf(stderr, "grid:  request granted, add to selection now\n");
      OverlayComp* comp = (OverlayComp*)grid->grcomp();
      LinkSelection* sel = (LinkSelection*)DrawKit::Instance()->GetEditor()->GetSelection();
      sel->AddComp(comp);
    }

    /* otherwise, pass the granting message along */
    else {
      fprintf(stderr, "grid:  pass grant request along\n");
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :grant chgid(0x%08x))%c",
	       grid->id(), selector, oldselector, '\0');
      SendCmdString(linkget(selector), buf);
    }
  }
}

void DrawServ::print_gridtable() {
  GraphicIdTable* table = gridtable();
  GraphicIdTable_Iterator it(*table);
  printf("id          &grid       &grcomp     selector    selected\n");
  printf("----------  ----------  ----------  ----------  --------\n");
  while(it.more()) {
    GraphicId* grid = (GraphicId*)it.cur_value();
    printf("0x%08x  0x%08x  0x%08x  0x%08x  %s\n", 
	   (unsigned int)it.cur_key(), grid, grid->grcomp(),
	   grid->selector(), LinkSelection::selected_string(grid->selected()));
    it.next();
  }
}

void DrawServ::print_sidtable() {
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  printf("sid         osid        &DrawLink   lid   rid   pid     hostid  user              host            \n");
  printf("----------  ----------  ----------  ----  ----  ------  ------  ----              ----            \n");
  while(it.more()) {
    SessionId* sid = (SessionId*)it.cur_value();
    DrawLink* link = sid->drawlink();
    printf("0x%08x  0x%08x  0x%08x  %4d  %4d  %6d  %6d  %16s  %16s\n", 
	   sid->sid(), sid->osid(), link, 
	   link ? link->local_linkid() : 9999, link ? link->remote_linkid() : 9999, 
	   sid->pid(), sid->hostid(), sid->username(), sid->hostname());
    it.next();
  }
}

void DrawServ::remove_sids(DrawLink* link) {
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more()) {
    SessionId* sid = (SessionId*)it.cur_value();
    DrawLink* testlink = sid->drawlink();
    unsigned int altid = it.cur_key();
    it.next();
    if (testlink==link) 
      if (!table->find_and_remove((void*)sid, altid)) 
	fprintf(stderr, "unable to remove SessionId's associated with DrawLink\n");
  }
}

boolean DrawServ::cycletest(unsigned int sid, const char* host,
			    const char* user, int pid) 
{
  boolean found = false;
  SessionIdTable* table = sessionidtable();
  SessionIdTable_Iterator it(*table);
  while(it.more() && !found) {
    SessionId* sessionid = (SessionId*)it.cur_value();
    if (sessionid->osid()==sid) {
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
    if (special) out << "chgid(";
    out << *attrval;
    if (special) out << ")";
  }
  return true;
}

#endif /* HAVE_ACE */
