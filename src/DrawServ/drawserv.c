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
#include <uuid/uuid.h>

using std::cout;
using std::cerr;

#ifdef HAVE_ACE
implementTable(GraphicIdTable,uint32_t,void*)
implementTable(SessionIdTable,uint32_t,void*)
implementTable(CompIdTable,void*,void*)

static int seed=0;
#endif /* HAVE_ACE */

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
#ifdef HAVE_ACE
  _linklist = new DrawLinkList;

  _gridtable = new GraphicIdTable(1024);
  _sessionidtable = new SessionIdTable(256);
  _compidtable = new CompIdTable(1024);

  create_unique_sessionid();
  char hostbuf[HOST_NAME_MAX];
  gethostname(hostbuf, HOST_NAME_MAX);
  char* username = getlogin();
  int pid = getpid();
  int hostid = gethostid();
  SessionId* sid = new SessionId(_sessionid, pid, username, hostbuf, hostid);
  _sessionidtable->insert(uuid_key(_sessionid), sid);

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

DrawLink* DrawServ::linkup(const char* hostname, int portnum, 
		     int state, uuid_t link_id,  ComTerp* comterp) {

  if (comterp!=NULL) comterp->handler()->alt_fd(portnum);
  
  if (state == DrawLink::new_link || state == DrawLink::one_way) {
    
    DrawLink* link = new DrawLink(hostname, portnum, state);
    if (state==DrawLink::one_way && comterp && comterp->handler()) {
      ((DrawServHandler*)comterp->handler())->drawlink(link);
      link->comhandler((DrawServHandler*)comterp->handler());
    }
    if (state == DrawLink::new_link) {
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

    // search for existing link with matching local_id
    Iterator i;
    _linklist->First(i);
    while(!_linklist->Done(i) && uuid_compare(_linklist->GetDrawLink(i)->linkid(), link_id)!=0)
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
    _linklist->Remove(link);
    link->close();
    remove_sids(link);
    delete link;
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
      
      case BRUSH_CMD:
      {
	boolean scripted = false;
	Iterator it;
	First(it);
	Selection* s = GetEditor(it)->GetSelection(); // only 1 Editor per Unidraw
	if (s) {
	  Iterator it;
	  for (s->First(it); !s->Done(it); s->Next(it)) {
	    OverlayView* compview = (OverlayView*)s->GetView(it);
	    if (compview && linklist()->Number()>0) {
	      if (scripted) 
		sbuf << ';';
	      else 
		scripted = true;
	      sbuf << "print(\"BRUSH_CMD\")";  // change to select();brush() call
	    }
	  }
	}
	if (!scripted)
	  fprintf(stderr, "Failed attempt to generate script for a BRUSH_CMD\n");

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

void DrawServ::DistributeCmdString(const char* cmdstring, DrawLink* orglink) {

  if (cmdstring==NULL || *cmdstring=='\0') return;

  Iterator i;
  _linklist->First(i);
  while (!_linklist->Done(i)) {
    DrawLink* link = _linklist->GetDrawLink(i);
    if (link && link != orglink && link->state()==DrawLink::two_way) {
      int fd = link->handle();
      if (fd>=0) {
	link->log_outgoing_command(cmdstring);
	FILE* fp=fdopen(dup(fd), "w");
	fputs(cmdstring, fp);
	fputs("\n", fp);
	fclose(fp);
	link->ackhandler()->start_timer();
      }
    }
    _linklist->Next(i);
  }
  
}

void DrawServ::SendCmdString(DrawLink* link, const char* cmdstring) {

  if (cmdstring==NULL || *cmdstring=='\0') return;
  
  if (link) {
    int fd = link->handle();
    if (fd>=0) {
      link->log_outgoing_command(cmdstring);
      FILE* fp=fdopen(dup(fd), "w");
      fputs(cmdstring, fp);
      fputs("\n", fp);
      fclose(fp);
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
  if (link != NULL) {
    SessionIdTable* sidtable = ((DrawServ*)unidraw)->sessionidtable();
    SessionId* session_id = new SessionId(sid, pid, username, hostname, hostid, link);
    sidtable->insert(uuid_key(sid), session_id);

    /* propagate */
    sessionid_register_propagate(link, sid, pid, username,
				 hostname, hostid);
  }
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
    if (otherlink != link) {
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
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :class \"%s\")%c", grid->idstr(), 
	       grid->selectorstr(), sessionidstr(), grid->compclass(), '\0');
      SendCmdString(link, buf);
    }
  }
}
  
// handle reserve request from remote DrawLink.
void DrawServ::grid_message_handle(DrawLink* link, uuid_t id, uuid_t selector, 
				   int state, uuid_t newselector)
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
	  char buf[BUFSIZ];
	  snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :class \"%s\")%c",
		   grid->idstr(), newselector_str, sessionidstr(),
		   grid->compclass(), '\0');
	  SendCmdString(link, buf);
	  fprintf(stderr, "grid: request granted\n");
	} 

	  /* else deny it, because it is selected */
	else {
	  char buf[BUFSIZ];
	  snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :deny :class \"%s\")%c",
		   grid->idstr(), sessionidstr(), grid->compclass(), '\0');
	  SendCmdString(link, buf);
	  fprintf(stderr, "grid: request denied, graphic locally selected\n");
	}	
      } 
      
      /* else reformulate this request and pass it along */
      else {
	fprintf(stderr, "grid: request passed along to current selector\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :class \"%s\")%c",
		 grid->idstr(), grid->selectorstr(), newselector_str,
		 grid->compclass(), '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }
    }

    /* else this request and/or simple state update should be passed along */
    else {
      /* if simple state, set the values here, and pass it on to everyone else */
      if (((const char *)newselector)==NULL || uuid_is_null(newselector)) {
	if (linklist()->Number()>1)
	  fprintf(stderr, "grid: state change passed along to everyone else\n");
	grid->selector(selector);
	grid->selected(state);
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :state %d :class \"%s\")%c",
		 grid->idstr(), grid->selectorstr(), grid->selected(),
		 grid->compclass(), '\0');
	DistributeCmdString(buf, link);
      } 

      /* else pass the request on to the target selector */
      else {
	fprintf(stderr, "grid:  request passed along to targeted selector\n");
	char buf[BUFSIZ];
	snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :request \"%s\" :class \"%s\")%c",
	  grid->idstr(), selector_str, newselector_str,
	  grid->compclass(), '\0');
	SendCmdString(linkget(grid->selector()), buf);
      }
    }
  }
}

// handle callback from remote DrawLink.
void DrawServ::grid_message_callback(DrawLink* link, uuid_t id, uuid_t selector, 
				     int state, uuid_t oldselector)
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
    if (grid->selected()==LinkSelection::WaitingToBeSelected && selector != NULL && uuid_compare(selector, sessionid())==0) {
      grid->selector(selector);
      grid->selected(LinkSelection::LocallySelected);

      
      fprintf(stderr, "grid:  request granted, add to selection now\n");
      LinkSelection* sel = (LinkSelection*)DrawKit::Instance()->GetEditor()->GetSelection();
      sel->request_resolved_check(true, FILELINE); // kaching upon 
      OverlayComp* comp = (OverlayComp*)grid->grcomp();
      sel->AddComp(comp);
      grid_message(grid);
    }

    /* otherwise, pass the granting message along */
    else {
      fprintf(stderr, "grid:  pass grant request along\n");
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :grant \"%s\" :class \"%s\")%c",
	       grid->idstr(), selector_str, oldselector_str,
	       grid->compclass(), '\0');
      SendCmdString(linkget(selector), buf);
    }
  }
}

void DrawServ::print_gridtable() {
  GraphicIdTable* table = gridtable();
  GraphicIdTable_Iterator it(*table);
  printf("grid     comptype              selector  selected\n");
  printf("-------- --------------------  --------  --------\n");
  while(it.more()) {
    GraphicId* grid = (GraphicId*)it.cur_value();
    OverlayComp* comp = (OverlayComp*)grid->grcomp();
    const char* comptype = comp ? comp->GetClassName() : "nil";
    uuid_string_t idstr;
    uuid_unparse(grid->id(), idstr);
    printf("%.8s %-20s  %.8s  %s\n",
 	   idstr, comptype,
 	   grid->selectorstr(), LinkSelection::selected_string(grid->selected()));
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
    
