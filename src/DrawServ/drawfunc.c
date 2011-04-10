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

#include <DrawServ/draweditor.h>
#include <DrawServ/drawfunc.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawlinkcomp.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <OverlayUnidraw/ovviews.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <fstream.h>
#include <cstdio>

#define TITLE "DrawLinkFunc"

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
  static int port_sym = symbol_add("port");
  ComValue default_port(20002);
  ComValue portv(stack_key(port_sym, false, default_port, true));
  static int state_sym = symbol_add("state");
  ComValue default_state(0);
  ComValue statev(stack_key(state_sym, false, default_state, true));
  static int lid_sym = symbol_add("lid");
  ComValue lidv(stack_key(lid_sym));
  static int rid_sym = symbol_add("rid");
  ComValue ridv(stack_key(rid_sym));
  static int close_sym = symbol_add("close");
  ComValue closev(stack_key(close_sym));
  static int dump_sym = symbol_add("dump");
  ComValue dumpv(stack_key(dump_sym));
  static int pid_sym = symbol_add("pid");
  ComValue pidv(stack_key(pid_sym));
  static int sid_sym = symbol_add("sid");
  ComValue sidv(stack_key(sid_sym));
  static int user_sym = symbol_add("user");
  ComValue userv(stack_key(user_sym));
  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  DrawLink* link = nil;

  /* creating a new link to remote drawserv */
  if (hostv.is_string() && portv.is_known() && statev.is_known()) {

    /* cast of this link if it is a duplicate or cycle */
    if (statev.int_val()==DrawLink::one_way && 
	((DrawServ*)unidraw)->cycletest
	(sidv.uint_val(), hostv.string_ptr(), userv.string_ptr(), pidv.int_val())) {
#if 1
#if __GNUC__<4
      fileptr_filebuf obuf(comterp()->handler()->get_handle(), ios_base::out, false, static_cast<size_t>(BUFSIZ));
#else
      fileptr_filebuf obuf(comterp()->handler()->get_handle(), ios_base::out, static_cast<size_t>(BUFSIZ));
#endif
      ostream out(&obuf);
      out << "ackback(cycle)\n";
      out.flush();
      comterp()->quit();
#else
      comterp()->handler()->destroy();
#endif
      return;
    }
    
    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    u_short statenum = statev.ushort_val();
    int lidnum = lidv.is_known() ? lidv.int_val() : -1;
    int ridnum = ridv.is_known() ? ridv.int_val() : -1;

    link = 
      ((DrawServ*)unidraw)->linkup(hoststr, portnum, statenum, 
				   lidnum, ridnum, this->comterp());
  
     
  } 

  /* return pointer to existing link */
  else if (ridv.is_known() || lidv.is_known()) {
    link = ((DrawServ*)unidraw)->linkget
      (lidv.is_known() ? lidv.int_val() : -1, 
       ridv.is_known() ? ridv.int_val() : -1);
    
    /* close if that flag is set. */
    if (link && closev.is_true()) {
      ((DrawServ*)unidraw)->linkdown(link);
      link = nil;
    }
    
    else if (link && dumpv.is_true()) {
      link->dump(stderr);
    }
    
  }

  /* set state to complete linkup */
  else if (statev.int_val()==DrawLink::two_way) {
    DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
    DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;
    link->state(DrawLink::two_way);
  }

  /* dump DrawLink table to stderr */
  else 
    ((DrawServ*)unidraw)->linkdump(stderr);
  
  if (link) {
    DrawLinkComp* linkcomp = new DrawLinkComp(link);
    ComValue result(DrawLinkComp::class_symid(), new OverlayView(linkcomp));
    result.object_compview(true);
    push_stack(result);
  }
  else
    push_stack(ComValue::nullval());

  return;

#endif

}


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
  static int remap_sym = symbol_add("remap");
  ComValue remapv(stack_key(remap_sym));

  static int pid_sym = symbol_add("pid");
  ComValue pidv(stack_key(pid_sym));
  static int user_sym = symbol_add("user");
  ComValue userv(stack_key(user_sym));
  static int host_sym = symbol_add("host");
  ComValue hostv(stack_key(host_sym));
  static int hostid_sym = symbol_add("hostid");
  ComValue hostidv(stack_key(hostid_sym));

  ComValue sidv(stack_arg(0));
  ComValue osidv(stack_arg(1));

  reset_stack();

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif
  DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
  DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;
  
  if (allv.is_true()) {
    ((DrawServ*)unidraw)->sessionid_register(link);
    return;
  }

  if (sidv.is_known() && osidv.is_known()) {
    if (remapv.is_false()) 
      ((DrawServ*)unidraw)->sessionid_register_handle
	(link, sidv.uint_val(), osidv.uint_val(), pidv.int_val(), 
	 userv.string_ptr(), hostv.string_ptr(), 
	 hostidv.int_val());
    else
      link->sid_insert(osidv.uint_val(), sidv.uint_val());
  } else {
    ((DrawServ*)unidraw)->print_sidtable();
  }
#endif
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

  ComValue idv(stack_arg(0));
  ComValue selectorv(stack_arg(1));

  reset_stack();

  DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
  DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;

  if (idv.is_known() && selectorv.is_known()) {
    if (grantv.is_unknown()) {
      if (requestv.is_unknown()) 
	((DrawServ*)unidraw)->grid_message_handle
	  (link, idv.uint_val(), selectorv.uint_val(), statev.int_val());
      else
	((DrawServ*)unidraw)->grid_message_handle
	  (link, idv.uint_val(), selectorv.uint_val(), statev.int_val(), 
	   requestv.uint_val());
    } else 
    ((DrawServ*)unidraw)->grid_message_callback
      (link, idv.uint_val(), selectorv.uint_val(), statev.int_val(), 
       grantv.uint_val());
  } else if (idv.is_unknown()) {
    ((DrawServ*)unidraw)->print_gridtable();
  }
}

/*****************************************************************************/

ChangeIdFunc::ChangeIdFunc(ComTerp* comterp, DrawEditor* ed) : UnidrawFunc(comterp, ed) {
}

void ChangeIdFunc::execute() {

  ComValue idv(stack_arg(0));
  reset_stack();

  DrawServHandler* handler = comterp() ? (DrawServHandler*)comterp()->handler() : nil;
  DrawLink* link = handler ? (DrawLink*)handler->drawlink() : nil;
  
  if (idv.is_known()) {
    unsigned int id = idv.uint_val();
    link->sid_change(id);
    ComValue result(id, ComValue::UIntType);
    result.state(AttributeValue::HexState);
    push_stack(result);
  }
}


