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

#include <ComUnidraw/highlightfunc.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Components/grcomp.h>

#define TITLE "HighlightFunc"

/*****************************************************************************/

HighlightFunc::HighlightFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void HighlightFunc::execute() {
    
    ComValue grv = stack_arg(0);
    ComValue gsv = stack_arg(1);
    reset_stack();
    if (grv.object_compview() && gsv.object_compview()) {
      ComponentView* grcompview = (ComponentView*)grv.obj_val();
      ComponentView* gscompview = (ComponentView*)gsv.obj_val();
      if (grcompview && grcompview->GetSubject() && gscompview && gscompview->GetSubject()) {
	
	Graphic* grgs = ((GraphicComp*)gscompview->GetSubject())->GetGraphic();
	if (grgs) {
	  OverlayComp* grcomp = ((OverlayView*)grcompview)->GetOverlayComp();
	  OverlayView* grview = grcomp ? grcomp->FindView(_ed->GetViewer()) : nil;
	  if (grview) grview->HighlightGraphic(grgs);
	}
      }
    }
    push_stack(grv);
}

