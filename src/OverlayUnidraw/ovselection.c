/*
 * Copyright (c) 1994 Vectaport Inc.
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
 * Implementation of OverlaySelection class.
 */

#include <OverlayUnidraw/ovdamage.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>

/*****************************************************************************/

OverlaySelection::OverlaySelection (OverlaySelection* s) : Selection(s) {
    _clear_to_repair = false;
    _handles_disabled = false;
}

OverlaySelection::OverlaySelection (Selection* s) : Selection(s) {
    _clear_to_repair = false;
    _handles_disabled = false;
}

void OverlaySelection::Show (Viewer* viewer) {
    if (!viewer) return;
    if (ShowHighlights(viewer)) 
	viewer->GetDamage()->Repair();
    if (HandlesEnabled()) 
	ShowHandles(viewer);
}

void OverlaySelection::Hide (Viewer* viewer) {
    if (!viewer) return;
    if (HandlesEnabled()) 
	HideHandles(viewer);
    if (HideHighlights(viewer)) 
	viewer->GetDamage()->Repair();
}

void OverlaySelection::Update (Viewer* viewer) {
    if (!viewer) return;
    if (HandlesEnabled()) 
	HideHandles(viewer);
    ShowHighlights(viewer);
    viewer->GetDamage()->Repair();
    if (HandlesEnabled())
	ShowHandles(viewer);
}

void OverlaySelection::Clear (Viewer* viewer) {

    if (HandlesEnabled())
	HideHandles(viewer);
    _clear_to_repair = HideHighlights(viewer) != nil;

    Iterator i;
    First(i);
    while (!Done(i)) 
        Remove(i);

}

void OverlaySelection::RepairClear(Viewer* viewer, boolean flag) {

    if (_clear_to_repair && flag) 
	viewer->GetDamage()->Repair();
    _clear_to_repair = false;
}

void OverlaySelection::Merge (Selection* s) {
    Iterator i;
    OverlayView* ov = nil;

    for (s->First(i); !s->Done(i); s->Next(i)) {
        ov = GetView(i);
	if (!Includes(ov)) {
	    Append(ov);
	}
    }
    if (ov) 
	Update(ov->GetViewer());
}

void OverlaySelection::Exclusive (Selection* s) {
    Iterator i;
    OverlayView* ov = nil;

    for (s->First(i); !s->Done(i); s->Next(i)) {
        ov = GetView(i);

	if (Includes(ov)) {
	    if (ov->Highlightable())
		ov->Unhighlight();
	    else {
		if (HandlesEnabled())
		    ov->EraseHandles();
	    }
	    Remove(ov);
	} else {
	    Append(ov);
	}
    }
    if (ov) 
	Update(ov->GetViewer());
}

void OverlaySelection::ShowHandles (Viewer* viewer) {
    Iterator i;

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = GetView(i);

	if (view->Highlightable())
	    continue;

        if (viewer == nil || view->GetViewer() == viewer) 
	    if (HandlesEnabled())
		view->RedrawHandles();
    }
}

void OverlaySelection::HideHandles (Viewer* viewer) {
    Iterator i;

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = GetView(i);

	if (view->Highlightable())
	    continue;

        if (viewer == nil || view->GetViewer() == viewer) 
	    if (HandlesEnabled())
		view->EraseHandles();
    }
}

OverlayViewer* OverlaySelection::ShowHighlights (Viewer* viewer) {
    Iterator i;
    OverlayViewer* ovviewer = nil;

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = GetView(i);

	if (!view->Highlightable())
	    continue;

	ovviewer = (OverlayViewer*)view->GetViewer();
        if (viewer == nil || ovviewer == viewer)
            view->Highlight();
    }
    return ovviewer;
}

OverlayViewer* OverlaySelection::HideHighlights (Viewer* viewer) {
    Iterator i;
    OverlayViewer* ovviewer = nil;

    for (First(i); !Done(i); Next(i)) {
        OverlayView* view = GetView(i);

	if (!view->Highlightable())
	    continue;

	ovviewer = (OverlayViewer*)view->GetViewer();
        if (viewer == nil || ovviewer == viewer) 
            view->Unhighlight();
    }
    return ovviewer;
}


void OverlaySelection::EnableHandles() { 
    _handles_disabled = false; 
    ShowHandles();
}

void OverlaySelection::DisableHandles() { 
    HideHandles();
    _handles_disabled = true; 
}

boolean OverlaySelection::HandlesEnabled() { return !_handles_disabled; }
boolean OverlaySelection::HandlesDisabled() { return _handles_disabled; }

OverlayView* OverlaySelection::GetView (Iterator i) { return (OverlayView*) View(Elem(i)); }

void OverlaySelection::SetView (OverlayView* ov, Iterator& i) {
    i.SetValue(_ulist->Find(ov));
}

OverlaySelection* OverlaySelection::ViewsWithin(IntCoord l, IntCoord b, IntCoord r, IntCoord t) {
    OverlaySelection* newSel = new OverlaySelection;
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	OverlayView* view = GetView(i);
	newSel->Merge(view->ViewsWithin(l, b, r, t));
    }
    return newSel;
}
