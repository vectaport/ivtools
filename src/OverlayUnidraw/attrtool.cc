/*
 * Copyright (c) 1996 Vectaport Inc.
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

#include <OverlayUnidraw/attrtool.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>

#include <IVGlyph/observables.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubrect.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

ClassId AttributeTool::GetClassId () { return ATTRIBUTE_TOOL; }

boolean AttributeTool::IsA (ClassId id) {
    return ATTRIBUTE_TOOL == id || Tool::IsA(id);
}

AttributeTool::AttributeTool (ControlInfo* m) 
: Tool(m)
{
}

Tool* AttributeTool::Copy () { return new AttributeTool(CopyControlInfo()); }

Manipulator* AttributeTool::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel
) {
    Manipulator* m = nil;
    GraphicView* views = v->GetGraphicView();
    Selection* s = v->GetSelection(), *newSel;

    newSel = views->ViewIntersecting(e.x-SLOP, e.y-SLOP, e.x+SLOP, e.y+SLOP);
    if (e.shift) {
        Localize(s, v);
    } else {
	s->Clear();
    }
    if (newSel->IsEmpty()) {		// do nothing
    } else {				// else user selected object directly
	s->Exclusive(newSel);
    }
    delete newSel;

    if (s->Number() == 1) {
	Iterator i;
	s->First(i);
	GraphicView* view = s->GetView(i);
	if (view->IsA(OVERLAY_VIEW)) {
            ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue("");
	    ((OverlayEditor*)v->GetEditor())->AttrEdit(((OverlayView*)view)->GetOverlayComp());
            ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_attr);
        }
    }
    return m;
}

Command* AttributeTool::InterpretManipulator (Manipulator* m) {
    return nil;
}

void AttributeTool::Localize (Selection* s, Viewer* v) {
    Iterator i;

    for (s->First(i); !s->Done(i);) {
        GraphicView* view = s->GetView(i);

        if (view->GetViewer() != v) {
            s->Remove(i);
            view->EraseHandles();

        } else {
            s->Next(i);
        }
    }
}
