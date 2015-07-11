/*
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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
 * FrameViewer implementation.
 */

#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/frameviewer.h>
#include <FrameUnidraw/frameviews.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovdamage.h>
#include <OverlayUnidraw/ovselection.h>

#include <UniIdraw/idclasses.h>

#include <Unidraw/iterator.h>

#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/perspective.h>
#include <InterViews/window.h>

/*****************************************************************************/

FrameViewer::FrameViewer (
    Editor* ed, GraphicView* gv, UPage* page, Grid* grid, 
    Coord w, Coord h, Orientation orientation,
    Alignment align, Zooming zoom
) : OverlayViewer(ed, gv, page, grid, w, h, orientation) {
}

void FrameViewer::Update () {
    if (_needs_resize)
      return;

    OverlaySelection* s = (OverlaySelection*)GetSelection();
    OverlayView* view = GetOverlayView();
    Component* viewComp = view->GetOverlayComp();
    OverlayComp* edComp = (OverlayComp*)_editor->GetComponent();
    boolean glyph_repair = _damage->Incurred();

    if (viewComp != edComp) {
        FrameIdrawView* newView = (FrameIdrawView*)edComp->Create(ViewCategory());

        if (newView->IsA(GRAPHIC_VIEW)) {
            edComp->Attach(newView);
            newView->Update();
            SetGraphicView(newView);

	    FrameEditor* ed = (FrameEditor*)GetEditor();

	    Iterator last;
	    newView->Last(last);
            int nframes = newView->Index(last);
	    if (ed->frameliststate())
	      ed->frameliststate()->framenumber(nframes, true);
	    
	    if (ed->framenumstate())
	      ed->framenumstate()->framenumber(nframes ? 1 : 0, true);
	    Iterator first;
	    newView->First(first);
	    newView->Next(first);
	    if (newView->Done(first))
		newView->First(first);
	    ed->InitFrame();
#if 0
	    ed->SetFrame((FrameView*)newView->GetView(first));
#endif
	    ed->UpdateFrame(true);

	    Draw();

        } else 
            delete newView;
	glyph_repair = true;
    } else {
      if (_damage->Incurred()) {
	s->HideHandles(this);
        _viewerView->Update();
        GraphicBlock::UpdatePerspective();
	s->ShowHighlights();
	_damage->Repair();
        s->ShowHandles(this);
      }
    }
    if (glyph_repair) {
      GetEditor()->GetWindow()->repair();
      GetEditor()->GetWindow()->display()->flush();
    }
    GetEditor()->GetWindow()->cursor(arrow);
}


void FrameViewer::SetGraphicView (GraphicView* gv) {
    Perspective np = *perspective;

    GetEditor()->GetSelection()->Clear();
    delete _viewerView;
    delete _gview;

    _gview = gv;
    _viewerView = new ViewerView(_gview, _page, _grid, this);
    _graphic = _viewerView->GetGraphic();
    _damage->SetGraphic(_graphic);
    _damage->Incur(0, 0, 0, 0);                 // for detecting Draw in Adjust

    Reorient();
    GraphicBlock::Init();

    register Perspective* p = perspective;
    Perspective ptmp;
    
    if (canvas == nil) {
        *p = np;
    } else if (_graphic != nil && *p != np) {
	Normalize(np);
	ptmp = *p;
	if (np.curwidth != canvas->Width() || np.curheight!=canvas->Height()) {
	    Zoom(np);
	} else {
	    Scroll(np);
	}
	p->Update();
    }
    UpdateMagnifVar();

}

OverlayView* FrameViewer::GetCurrent() 
{ 
  return GetFrameEditor()->GetFrame(); 
}

GraphicView* FrameViewer::GetCurrentGraphicView()
{
    OverlaysView* frame = ((FrameEditor*)GetEditor())->GetFrame();
    return frame ? frame : GetGraphicView();
}
