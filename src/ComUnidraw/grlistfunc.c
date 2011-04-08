/*
 * Copyright (c) 2000 IET Inc.
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

#include <ComUnidraw/grlistfunc.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Components/compview.h>
#include <Unidraw/iterator.h>
#include <ComTerp/listfunc.h>

#define TITLE "GrListFunc"

/*****************************************************************************/

GrListAtFunc::GrListAtFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GrListAtFunc::execute() {
  ComValue listv(stack_arg(0));
  ComValue nv(stack_arg(1));

  if (listv.object_compview()) {
    reset_stack();
    ComponentView* compview = (ComponentView*)listv.obj_val();
    OverlayComp* comp = (OverlayComp*)compview->GetSubject();
    OverlaysComp* comps = (OverlaysComp*) (comp->IsA(OVERLAYS_COMP) ? comp : nil);
    if (comps && nv.int_val()>=0) {
      Iterator i;
      int count = 0;
      comps->First(i);
      while (!comps->Done(i)) {
	if (count==nv.int_val()) {
	  OverlayComp* retcomp = (OverlayComp*)comps->GetComp(i);
	  if (retcomp) {
	    ComValue retval(retcomp->classid(), new ComponentView(retcomp));
	    retval.object_compview(true);
	    push_stack(retval);
	    return;
	  }
	}
	comps->Next(i);
	count++;
      }
    }
  } else {
    ListAtFunc atfunc(comterp());
    atfunc.exec(funcstate()->nargs(), funcstate()->nkeys(), pedepth());
    return;
  }
  push_stack(ComValue::nullval());
}

/*****************************************************************************/

GrListSizeFunc::GrListSizeFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GrListSizeFunc::execute() {
  ComValue listv(stack_arg(0));

  if (listv.object_compview()) {
    reset_stack();
    ComponentView* compview = (ComponentView*)listv.obj_val();
    OverlayComp* comp = (OverlayComp*)compview->GetSubject();
    OverlaysComp* comps = (OverlaysComp*) (comp->IsA(OVERLAYS_COMP) ? comp : nil);
    if (comps) {
      Iterator i;
      int count = 0;
      comps->First(i);
      while (!comps->Done(i)) {
	count++;
	comps->Next(i);
      }
      ComValue retval (count, ComValue::IntType);
      push_stack(retval);
      return;
    }
  } else {
    ListSizeFunc atfunc(comterp());
    atfunc.exec(funcstate()->nargs(), funcstate()->nkeys(), pedepth());
    return;
  }
  push_stack(ComValue::nullval());
}


