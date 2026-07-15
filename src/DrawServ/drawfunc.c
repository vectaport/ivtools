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

#include <DrawServ/ackback-handler.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawclasses.h>
#include <DrawServ/drawfunc.h>
#include <DrawServ/drawfunc.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinkcomp.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/linkselection.h>
#include <DrawServ/sid.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovvertices.h>
#include <UniIdraw/idarrows.h>
#include <ComTerp/comterp.h>
#include <ComTerp/socket.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/verts.h>
#include <IV-2_6/InterViews/world.h>

#include <fstream.h>
#include <cstdio>

#define TITLE "DrawLinkFunc"

#if 1
/*****************************************************************************/

DrawLinkFunc::DrawLinkFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawLinkFunc::execute() {
  ComValue hostv(stack_arg(0, true));
  ComValue linkv(stack_arg(0, false));
  static int port_sym = symbol_add("port");
  ComValue default_port(20002);
  ComValue portfixed(stack_arg(1, true, default_port));  // positional form, already defaults to 20002
  ComValue portv(portfixed);
  ComValue portkeyv(stack_key(port_sym));                 // :port val overrides; bare/absent leaves portv as portfixed
  if (portkeyv.is_known()) portv = portkeyv;
  static int state_sym = symbol_add("state");
  ComValue default_state(0);
  ComValue statev(default_state);
  ComValue statekeyv(stack_key(state_sym));
  if (statekeyv.is_known()) statev = statekeyv;
  static int linkid_sym = symbol_add("linkid");
  ComValue linkidv(stack_key(linkid_sym));
  static int close_sym = symbol_add("close");
  ComValue closev(stack_key(close_sym));
  static int pid_sym = symbol_add("pid");
  ComValue pidv(stack_key(pid_sym));
  static int sid_sym = symbol_add("sid");
  ComValue sidv(stack_key(sid_sym));
  static int user_sym = symbol_add("user");
  ComValue userv(stack_key(user_sym));
  static int socket_sym = symbol_add("socket");
  ComValue socketv(stack_key(socket_sym));
  static int timer_sym = symbol_add("timer");
  ComValue default_timer(5);
  ComValue timerv(default_timer);
  ComValue timerkeyv(stack_key(timer_sym));
  if (timerkeyv.is_known()) timerv = timerkeyv;
  static int table_sym = symbol_add("table");
  ComValue tablev(stack_key(table_sym));
  reset_stack();

  DrawLink* link = nil;

  /* introspect existing link from DrawLinkComp argument */
  if (linkv.is_compview()) {
    ComponentView* compview = (ComponentView*)linkv.obj_val();
    if (compview && compview->GetSubject() && 
        ((GraphicComp*)compview->GetSubject())->IsA(DRAWLINK_COMP)) {
      DrawLinkComp* dlcomp = (DrawLinkComp*)compview->GetSubject();
      link = dlcomp ? dlcomp->drawlink() : nil;
      if (link) {
        if (closev.is_true()) {
          ((DrawServ*)unidraw)->linkdown(link);
	  push_stack(ComValue::nullval());
	}
	else if (socketv.is_true()) {
	  SocketObj* sockobj = new SocketObj();
	  sockobj->socket(link->socket());
	  ComValue result(SocketObj::class_symid(), sockobj);
	  push_stack(result);
	} else if (timerv.is_known()) {
	  int sec = timerv.int_val();
	  link->ackhandler()->start_timer(sec);
	  push_stack(ComValue::nullval());
	} else {
          link->dump(stderr);
	  push_stack(ComValue::nullval());
        }
        return;
      }
    }
    push_stack(ComValue::nullval());
    return;
  }
  

  /* creating a new link to remote drawserv */
  if (hostv.is_string() && portv.is_known() && statev.is_known()) {
    
    /* cast off this link if it is a duplicate or cycle */
    uuid_t sid;
    uuid_parse(sidv.string_ptr(), sid);
    if (statev.int_val()==DrawLink::one_way && 
	((DrawServ*)unidraw)->cycletest
	(sid, hostv.string_ptr(), userv.string_ptr(), pidv.int_val())) {
      fputs("ackback(cycle)\n", comterp()->handler()->wrfptr());
      fflush(comterp()->handler()->wrfptr());
      comterp()->quit();
      return;
    }
    
    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    u_short statenum = statev.ushort_val();
    
    uuid_t linkid; uuid_clear(linkid);
    if (linkidv.is_string()) {
	uuid_parse(linkidv.string_ptr(),linkid);
    }
	
    link = ((DrawServ*)unidraw)->linkup(hoststr, portnum, statenum, linkid, this->comterp());
    Resource::ref(link); // reference here if calling Run makes linkdown()

    /* wait for two_way handshake to complete */
    if (link && statenum == DrawLink::new_link) {
      static const int max_wait_usec = 5000000;  // 5 second overall timeout
      static const int slice_usec    =    5000;  // 5ms per slice
      int elapsed = 0;
      
      long oldsec, oldusec;
      ((OverlayUnidraw*)unidraw)->get_timeout(oldsec, oldusec);
      
      while (link->state() != DrawLink::two_way && elapsed < max_wait_usec) {
        ((OverlayUnidraw*)unidraw)->set_timeout(0, slice_usec);
        ((OverlayUnidraw*)unidraw)->Run();
        elapsed += slice_usec;
      }
      
      ((OverlayUnidraw*)unidraw)->set_timeout(oldsec, oldusec);
      
      if (link->state() != DrawLink::two_way) {
        fprintf(stderr, "drawlink: timed out waiting for two_way handshake\n");
        ((DrawServ*)unidraw)->linkdown(link);
        push_stack(ComValue::nullval());
	Resource::unref(link); // unreference here because Run calls are done
        return;
      }
    }
    Resource::unref(link); // unreference here because Run calls are done
  } 
  
  /* set state to complete linkup */
  if (statev.int_val()==DrawLink::two_way) {
    DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
    if (handler) {
      if (link==NULL) link  = (DrawLink*)handler->drawlink();
      if (link != NULL) {
	link->state(DrawLink::two_way);
	
	// at this point paste all graphics to new connection
	DrawServ* drawserv = (DrawServ*)unidraw;
	    if (hostv.is_string())
	      drawserv->SendAllToBackgroundEditor(link, (DrawEditor*)GetEditor());
	    else 
	      drawserv->SendAllToForegroundEditor(link, (DrawEditor*)GetEditor());
      }
    }
  }
  
  /* dump DrawLink table to stderr, or return as list of attrlists */
  else if(nargs()==0) {
    if (tablev.is_true()) {
      static int host_row_sym   = symbol_add("host");
      static int alt_row_sym    = symbol_add("alt");
      static int port_row_sym   = symbol_add("port");
      static int lid_row_sym    = symbol_add("lid");
      static int state_row_sym  = symbol_add("state");
      DrawServ* drawserv = (DrawServ*)unidraw;
      AttributeValueList* avl = new AttributeValueList();
      if (drawserv->linklist()) {
        Iterator i;
        drawserv->linklist()->First(i);
        while (!drawserv->linklist()->Done(i)) {
          DrawLink* link = drawserv->linklist()->GetDrawLink(i);
          AttributeList* row = new AttributeList();
          row->add_attr(host_row_sym,  new AttributeValue(link->hostname()    ? link->hostname()    : ""));
          row->add_attr(alt_row_sym,   new AttributeValue(link->althostname() ? link->althostname() : ""));
          row->add_attr(port_row_sym,  new AttributeValue((int)link->portnum(), AttributeValue::IntType));
          row->add_attr(lid_row_sym,   new AttributeValue(link->linkid_str()  ? link->linkid_str()  : ""));
          row->add_attr(state_row_sym, new AttributeValue((int)link->state(),  AttributeValue::IntType));
          avl->Append(new AttributeValue(AttributeList::class_symid(), (void*)row));
          drawserv->linklist()->Next(i);
        }
      }
      ComValue result(avl);
      push_stack(result);
      return;
    } else {
      ((DrawServ*)unidraw)->linkdump(stderr);
      push_stack(ComValue::nullval());
      return;
    }
  }
  
  if (link) {
    DrawLinkComp* linkcomp = new DrawLinkComp(link);
    ComValue result(new OverlayViewRef(linkcomp), DrawLinkComp::class_symid());
    push_stack(result);
  }
  else
    push_stack(ComValue::nullval());
  
  return;

}

#endif

/*****************************************************************************/

SessionIdFunc::SessionIdFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void SessionIdFunc::execute() {
  static int all_sym = symbol_add("all");
  ComValue allv(stack_key(all_sym));
  static int pid_sym = symbol_add("pid");
  ComValue pidv(stack_key(pid_sym));
  static int user_sym = symbol_add("user");
  ComValue userv(stack_key(user_sym));
  static int host_sym = symbol_add("host");
  ComValue hostv(stack_key(host_sym));
  static int hostid_sym = symbol_add("hostid");
  ComValue hostidv(stack_key(hostid_sym));
  static int table_sym2 = symbol_add("table");
  ComValue tablev(stack_key(table_sym2));
 
  ComValue sidv(stack_arg(0));

  reset_stack();

  DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
  DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;
  
  if (allv.is_true()) {
    ((DrawServ*)unidraw)->sessionid_register(link);
    return;
  }

  if (sidv.is_known()) {

    uuid_t sid;
    uuid_parse(sidv.string_ptr(), sid);
    
    ((DrawServ*)unidraw)->sessionid_register_handle
      (link, sid, pidv.int_val(), 
       userv.string_ptr(), hostv.string_ptr(), 
       hostidv.int_val());
    
  } else {
    if (tablev.is_true()) {
      static int key_row_sym    = symbol_add("key");
      static int sid_row_sym    = symbol_add("sid");
      static int linkid_row_sym = symbol_add("linkid");
      static int pid_row_sym    = symbol_add("pid");
      static int hostid_row_sym = symbol_add("hostid");
      static int user_row_sym   = symbol_add("user");
      static int host_row_sym   = symbol_add("host");
      DrawServ* drawserv = (DrawServ*)unidraw;
      AttributeValueList* avl = new AttributeValueList();
      SessionIdTable* table = drawserv->sessionidtable();
      SessionIdTable_Iterator it(*table);
      while (it.more()) {
        SessionId* sid = (SessionId*)it.cur_value();
        DrawLink* lnk = sid->drawlink();
        AttributeList* row = new AttributeList();
        row->add_attr(key_row_sym,    new AttributeValue((int)it.cur_key(),   AttributeValue::IntType));
        row->add_attr(sid_row_sym,    new AttributeValue(sid->sidstr()        ? sid->sidstr()           : ""));
        row->add_attr(linkid_row_sym, new AttributeValue(lnk                  ? lnk->linkid_str()       : "00000000"));
        row->add_attr(pid_row_sym,    new AttributeValue((int)sid->pid(),     AttributeValue::IntType));
        row->add_attr(hostid_row_sym, new AttributeValue((int)sid->hostid(),  AttributeValue::IntType));
        row->add_attr(user_row_sym,   new AttributeValue(sid->username()      ? sid->username()         : ""));
        row->add_attr(host_row_sym,   new AttributeValue(sid->hostname()      ? sid->hostname()         : ""));
        avl->Append(new AttributeValue(AttributeList::class_symid(), (void*)row));
        it.next();
      }
      ComValue result(avl);
      push_stack(result);
    } else {
      ((DrawServ*)unidraw)->print_sidtable();
      push_stack(ComValue::nullval());
    }
  }
}


/*****************************************************************************/

GraphicIdFunc::GraphicIdFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void GraphicIdFunc::execute() {
  static int request_sym = symbol_add("request");
  ComValue requestv(stack_key(request_sym));
  static int grant_sym = symbol_add("grant");
  ComValue grantv(stack_key(grant_sym));
  static int state_sym = symbol_add("state");
  ComValue statev(stack_key(state_sym));
  static int deny_sym = symbol_add("deny");
  ComValue denyv(stack_key(deny_sym));
  static int table_sym = symbol_add("table");
  ComValue tablev(stack_key(table_sym));

  ComValue idv(stack_arg(0));
  ComValue selectorv(stack_arg(1));

  reset_stack();

  uuid_t id;
  if (idv.is_string()) uuid_parse(idv.string_ptr(), id); else uuid_clear(id);
  uuid_t selector;
  if (selectorv.is_string()) uuid_parse(selectorv.string_ptr(), selector); else uuid_clear(selector);

  LinkSelection* sel = (LinkSelection*)_ed->GetSelection();
  
 if (denyv.is_true()) {
    void* ptr = nil;
    ((DrawServ*)unidraw)->gridtable()->find(ptr, uuid_key(id));
    if (ptr) {
      GraphicId* grid = (GraphicId*)ptr;
      grid->selected(LinkSelection::RemotelySelected);
      grid->selector(selector);
      fprintf(stderr, "grid: request denied\n");
      if (sel) sel->request_resolved_check(false, FILELINE);
    }
    return;
  }

  DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
  DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;

  if (idv.is_known() && selectorv.is_known()) {
    
    if (grantv.is_unknown()) {
      
      if (requestv.is_unknown())  {
	((DrawServ*)unidraw)->grid_message_handle
	  (link, id, selector, statev.int_val());
      }
      
      else {
	uuid_t rid;
	uuid_parse(requestv.string_ptr(), rid);
	((DrawServ*)unidraw)->grid_message_handle
	  (link, id, selector, statev.int_val(), rid);
      }
      
    } else {
      uuid_t gid;
      uuid_parse(grantv.string_ptr(), gid);
      
      ((DrawServ*)unidraw)->grid_message_callback
	(link, id, selector, statev.int_val(), gid);
    }
    
  } else if (idv.is_known() && selectorv.is_unknown()) {
    /* single id arg -- look up and return the compview.  accept either a full
       uuid or its 8-char index: inside a linkup the distributed traffic carries
       only the 8-char, and the gridtable is keyed on it (uuid_key = first 4
       bytes).  uuid_parse can't parse the 8-char prefix, so for an 8-char id
       parse it straight to the key. */
    void* ptr = nil;
    const char* idstr = idv.is_string() ? idv.string_ptr() : nil;
    uint32_t key = (idstr && strlen(idstr)==8) ? (uint32_t)strtoul(idstr, nil, 16) : uuid_key(id);
    ((DrawServ*)unidraw)->gridtable()->find(ptr, key);
    if (ptr) {
        GraphicId* grid = (GraphicId*)ptr;
        OverlayComp* comp = grid->grcomp();
        if (comp) {
            ComValue result(new OverlayViewRef(comp), comp->classid());
            push_stack(result);
            return;
        }
    }
    push_stack(ComValue::nullval());

  } else if (idv.is_unknown()) {
    if (tablev.is_true()) {
      /* return the gridtable as a list of rows, mirroring the columns of
         print_gridtable(): grid (8-char uuid prefix), comptype, selector,
         selected.  same idiom as sid(:table). */
      static int grid_row_sym     = symbol_add("grid");
      static int comptype_row_sym = symbol_add("comptype");
      static int selector_row_sym = symbol_add("selector");
      static int selected_row_sym = symbol_add("selected");
      DrawServ* drawserv = (DrawServ*)unidraw;
      AttributeValueList* avl = new AttributeValueList();
      GraphicIdTable* table = drawserv->gridtable();
      GraphicIdTable_Iterator it(*table);
      while (it.more()) {
        GraphicId* grid = (GraphicId*)it.cur_value();
        OverlayComp* comp = (OverlayComp*)grid->grcomp();
        const char* comptype = comp ? comp->GetClassName() : "nil";
        /* first 8 chars of the uuid, matching print_gridtable() and the
           uuid_key granularity the gridtable itself is keyed on.  a prefix
           collision in the small set of linked drawservs is expected and
           simply triggers the normal merge, where the full uuid surfaces. */
        char id8[9];
        const char* idfull = grid->idstr() ? grid->idstr() : "";
        strncpy(id8, idfull, 8); id8[8] = '\0';
        char sel8[9];
        const char* selfull = grid->selectorstr() ? grid->selectorstr() : "";
        strncpy(sel8, selfull, 8); sel8[8] = '\0';
        AttributeList* row = new AttributeList();
        row->add_attr(grid_row_sym,     new AttributeValue(id8));
        row->add_attr(comptype_row_sym, new AttributeValue(comptype));
        row->add_attr(selector_row_sym, new AttributeValue(sel8));
        row->add_attr(selected_row_sym, new AttributeValue(LinkSelection::selected_string(grid->selected())));
        avl->Append(new AttributeValue(AttributeList::class_symid(), (void*)row));
        it.next();
      }
      ComValue result(avl);
      push_stack(result);
    } else {
      ((DrawServ*)unidraw)->print_gridtable();
    }
  }

}

/*****************************************************************************/

DrawPointsFunc::DrawPointsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void DrawPointsFunc::execute() {
    
    Viewer* viewer = _ed->GetViewer();
    ComValue obj(stack_arg(0));
    reset_stack();
    if (obj.object_compview()) {
      ComponentView* compview = (ComponentView*)obj.obj_val();
      if (compview && compview->GetSubject()) {
	GraphicComp* comp = (GraphicComp*)compview->GetSubject();
	Graphic* gr = comp ? comp->GetGraphic() : nil;
	AttributeValueList* avl = new AttributeValueList();
	if (gr && comp->IsA(OVVERTICES_COMP)) {
	  VerticesOvComp* vertcomp = (VerticesOvComp*)comp;
	  Vertices* vertgr = vertcomp->GetVertices();
	  for(int i=0; i<vertgr->count(); i++) {
	    ComValue* val = new ComValue(vertgr->x()[i]);
	    avl->Append(val);
	    val = new ComValue(vertgr->y()[i]);	    
	    avl->Append(val);
	  }

	} else if (gr && comp->IsA(OVLINE_COMP)) {
	  LineOvComp* linecomp = (LineOvComp*)comp;
	  Coord x0, y0, x1, y1;
	  linecomp->GetLine()->GetOriginal(x0, y0, x1, y1);
	  avl->Append(new ComValue(x0));
	  avl->Append(new ComValue(y0));
	  avl->Append(new ComValue(x1));
	  avl->Append(new ComValue(y1));

	} else if (gr && comp->IsA(EDGE_COMP)) {
	  EdgeComp* edgecomp = (EdgeComp*)comp;
	  Coord x0, y0, x1, y1;
	  edgecomp->GetArrowLine()->GetOriginal(x0, y0, x1, y1);
	  avl->Append(new ComValue(x0));
	  avl->Append(new ComValue(y0));
	  avl->Append(new ComValue(x1));
	  avl->Append(new ComValue(y1));
	}

	ComValue retval(avl);
	push_stack(retval);
      } 	
    }
}



