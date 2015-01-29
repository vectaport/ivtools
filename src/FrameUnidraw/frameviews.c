/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/frameviews.h>

#include <Unidraw/iterator.h>
#include <Unidraw/viewer.h>

/*****************************************************************************/

FrameOverlaysView::FrameOverlaysView() : OverlaysView() {}

ClassId FrameOverlaysView::GetClassId() { return FRAME_OVERLAYS_VIEW; }

boolean FrameOverlaysView::IsA(ClassId id) {
    return id == FRAME_OVERLAYS_VIEW || OverlaysView::IsA(id);
}

/*****************************************************************************/

FrameView::FrameView(FrameComp* comp) : OverlaysView(comp) {}

ClassId FrameView::GetClassId() { return FRAME_VIEW; }

boolean FrameView::IsA(ClassId id) {
    return id == FRAME_VIEW || OverlaysView::IsA(id);
}

/*****************************************************************************/


FramesView::FramesView(FramesComp* comp) : FrameView(comp) {}

ClassId FramesView::GetClassId() { return FRAMES_VIEW; }

boolean FramesView::IsA(ClassId id) {
    return id == FRAMES_VIEW || FrameView::IsA(id);
}

void FramesView::UpdateFrame(FrameView* curr, FrameView* prev,
			     int* curr_others, int num_curr_others,
			     int* prev_others, int num_prev_others) {
  Iterator i;
  First(i);
  FrameView* background = (FrameView*)GetView(i);
  
  if (curr != prev) {
    if (prev) {		
      if (prev != background) prev->Hide();
      prev->Desensitize();
      if (prev_others) {
	for (int np=0; np<num_prev_others; np++) {
	  SetView(prev, i);
	  if (prev_others[np]>0) 
	    for (int ii=0; ii<prev_others[np]; ii++) Next(i);
	  else 
	    for (int ii=0; ii>prev_others[np]; ii--) Prev(i);
	  if (!Done(i)) {
	    FrameView* frame = (FrameView*)GetView(i);
	    if (frame != background) {
	      frame->Hide();
	      frame->Sensitize();
	    }
	  }
	}
      }
    }
    if (curr) {
      if (curr != background) curr->Show();
      curr->Sensitize();
      if (curr_others) {
	for (int np=0; np<num_curr_others; np++) {
	  SetView(curr, i);
	  if (curr_others[np]>0) 
	    for (int ii=0; ii<curr_others[np]; ii++) Next(i);
	  else 
	    for (int ii=0; ii>curr_others[np]; ii--) Prev(i);
	  if (!Done(i)) {
	    FrameView* frame = (FrameView*)GetView(i);
	    if (frame != background) {
	      frame->Show();
	      frame->Desensitize();
	    }
	  }
	}
      }
    }
  }
}

/*****************************************************************************/

FrameIdrawView::FrameIdrawView(FrameIdrawComp* comp) : FramesView(comp) {}

ClassId FrameIdrawView::GetClassId() { return FRAME_IDRAW_VIEW; }

boolean FrameIdrawView::IsA(ClassId id) {
    return id == FRAME_IDRAW_VIEW || FramesView::IsA(id);
}

GraphicView* FrameIdrawView::GetGraphicView (Component* c) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame 
      ? frame->GetGraphicView(c) 
      : OverlaysView::GetGraphicView(c);
}

Selection* FrameIdrawView::SelectAll() {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame 
      ? frame->SelectAll()
      : OverlaysView::SelectAll();
}
Selection* FrameIdrawView::ViewContaining(Coord x, Coord y) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ViewContaining(x, y)
      : OverlaysView::ViewContaining(x, y);
}

Selection* FrameIdrawView::ViewsContaining(Coord x, Coord y) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ViewsContaining(x, y) 
      : OverlaysView::ViewsContaining(x, y);
}

Selection* FrameIdrawView::ViewIntersecting(Coord x0, Coord y0, Coord x1, Coord y1) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ViewIntersecting(x0, y0, x1, y1)
      : OverlaysView::ViewIntersecting(x0, y0, x1, y1);
}

Selection* FrameIdrawView::ViewsIntersecting(Coord x0, Coord y0, Coord x1, Coord y1) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ViewsIntersecting(x0, y0, x1, y1)
      : OverlaysView::ViewsIntersecting(x0, y0, x1, y1);
}
Selection* FrameIdrawView::ViewsWithin(Coord x0, Coord y0, Coord x1, Coord y1) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ViewsWithin(x0, y0, x1, y1)
      : OverlaysView::ViewsWithin(x0, y0, x1, y1);
}
ConnectorView* FrameIdrawView::ConnectorIntersecting(Coord x0, Coord y0, Coord x1, Coord y1) {
    OverlaysView* frame = ((FrameEditor*)GetViewer()->GetEditor())->GetFrame();
    return frame
      ? frame->ConnectorIntersecting(x0, y0, x1, y1)
      : OverlaysView::ConnectorIntersecting(x0, y0, x1, y1);
}


