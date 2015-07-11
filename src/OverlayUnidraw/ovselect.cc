/*
 * Copyright (c) 1997 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * OverlaySelect tool definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovselect.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>
#include <Unidraw/manips.h>
#include <Unidraw/viewer.h>
#include <Unidraw/Components/grview.h>
#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubrect.h>

/*****************************************************************************/

ClassId OverlaySelectTool::GetClassId () { return OVSELECT_TOOL; }

boolean OverlaySelectTool::IsA (ClassId id) {
    return OVSELECT_TOOL == id || SelectTool::IsA(id);
}

OverlaySelectTool::OverlaySelectTool 
(ControlInfo* m, const int* ignore_ids, int num_ignores) : SelectTool(m) {
  _nignores = num_ignores;
  _ignores = new int[_nignores];
  for (int i=0; i<_nignores; i++)
    _ignores[i] = ignore_ids[i];
}

OverlaySelectTool::~OverlaySelectTool() {
  delete _ignores;
}

Tool* OverlaySelectTool::Copy () { 
  return new OverlaySelectTool(CopyControlInfo(), _ignores, _nignores); 
}

Manipulator* OverlaySelectTool::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel
) {
    Manipulator* m = nil;
    GraphicView* views = v->GetGraphicView();
    Selection* s = v->GetSelection();
    OverlaySelection *newSel = (OverlaySelection*) 
      views->ViewIntersecting(e.x-SLOP, e.y-SLOP, e.x+SLOP, e.y+SLOP);
    if (e.shift) {
        Localize(s, v);
    } else {
	s->Clear();
	Iterator i;
	newSel->First(i);
	while (!newSel->Done(i)) {
	  OverlayView* view = newSel->GetView(i);
	  if (ignored(view))
	    newSel->Remove(i);
	  else
	    newSel->Next(i);
	}
    }

    if (newSel->IsEmpty()) {		// select w/RubberRect if nothing hit
	m = new DragManip(v, new RubberRect(nil,nil, e.x,e.y,e.x,e.y), rel);
    } else {				// else user selected object directly
	s->Exclusive(newSel);
    }
    delete newSel;
    return m;
}

Command* OverlaySelectTool::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Viewer* viewer = dm->GetViewer();
    GraphicView* views = viewer->GetGraphicView();
    Selection* s = viewer->GetSelection();
    RubberRect* rr = (RubberRect*) dm->GetRubberband();
    Selection* newSel;
    Coord l, b, r, t;

    rr->GetCurrent(l, b, r, t);
    newSel = views->ViewsWithin(l, b, r, t);
    s->Exclusive(newSel);
    delete newSel;

    return nil;
}

boolean OverlaySelectTool::ignored(OverlayView* view) {
  boolean is_ignored = false;
  for (int j=0; j<_nignores; j++) 
    is_ignored = is_ignored || view->IsA(_ignores[j]);
  return is_ignored;
}
