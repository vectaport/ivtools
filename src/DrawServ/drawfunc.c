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
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/linkselection.h>
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
#ifndef HAVE_ACE

  reset_stack();
  fprintf(stderr, "rebuild ivtools with ACE support to get full drawserv functionality\n");
  push_stack(ComValue::nullval());

#else

  ComValue hostv(stack_arg(0, true));
  ComValue linkv(stack_arg(0, false));
  static int port_sym = symbol_add("port");
  ComValue default_port(20002);
  ComValue portfixed(stack_arg(1, true, default_port));
  ComValue portv(stack_key(port_sym, false, portfixed, true));
  static int state_sym = symbol_add("state");
  ComValue default_state(0);
  ComValue statev(stack_key(state_sym, false, default_state, true));
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
  ComValue timerv(stack_key(timer_sym, false, default_timer, true));
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
  
  /* dump DrawLink table to stderr */
  else if(nargs()==0) 
    ((DrawServ*)unidraw)->linkdump(stderr);
  
  if (link) {
    DrawLinkComp* linkcomp = new DrawLinkComp(link);
    ComValue result(new OverlayViewRef(linkcomp), DrawLinkComp::class_symid());
    push_stack(result);
  }
  else
    push_stack(ComValue::nullval());
  
  return;
  
#endif
  
}

#endif
  
/*****************************************************************************/

SessionIdFunc::SessionIdFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void SessionIdFunc::execute() {
#ifndef HAVE_ACE

  reset_stack();
  fprintf(stderr, "rebuild ivtools with ACE support to get full drawserv functionality\n");
  push_stack(ComValue::nullval());

#else
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
    ((DrawServ*)unidraw)->print_sidtable();
  }
#endif
}


/*****************************************************************************/

#ifdef HAVE_ACE
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
    /* single uuid arg -- lookup and return compview */
    void* ptr = nil;
    uint32_t key = uuid_key(id);
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
    ((DrawServ*)unidraw)->print_gridtable();
  }

}

#endif /* defined(HAVE_ACE) */

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



