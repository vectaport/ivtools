/*
 * Copyright (c) 2020 Scott E. Johnston
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

#include <ComUnidraw/comclasses.h>
#include <ComUnidraw/pixelcmds.h>
#include <ComUnidraw/pixelfunc.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovvertices.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

/*-----------------------------------------------------------------*/

PolyClipRasterCmd::PolyClipRasterCmd(ControlInfo* ci) : Command(ci) {
}

Command* PolyClipRasterCmd::Copy() {
    Command* copy = new PolyClipRasterCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

ClassId PolyClipRasterCmd::GetClassId () { return POLYCLIPRASTER_CMD; }

boolean PolyClipRasterCmd::IsA (ClassId id) {
    return POLYCLIPRASTER_CMD == id || Command::IsA(id);
}

boolean PolyClipRasterCmd::Reversible() {
    return true;
}

void PolyClipRasterCmd::Execute() {

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlaySelection* sel = (OverlaySelection*) ed->GetSelection();
  Iterator i;
  RasterOvComp* rastcomp = nil;
  VerticesOvComp* vertcomp = nil;

  for (sel->First(i); !sel->Done(i); sel->Next(i)) {
    GraphicView* view = sel->GetView(i);

    if (view->IsA(OVRASTER_VIEW)) {
      RasterOvView *rastview = (RasterOvView*)view;
      if (rastview != nil) { rastcomp = (RasterOvComp*) (rastview->GetSubject()); }
      if (vertcomp != nil ) { break; }
      continue;
    }

    if (view->IsA(OVVERTICES_VIEW)) {
      VerticesOvView* vertview = (VerticesOvView*)view;
      if (vertview != nil) { vertcomp = (VerticesOvComp*) (vertview->GetSubject()); }
      if (rastcomp != nil ) { break; }
      continue;
    }
    
  }

  if (rastcomp != nil && vertcomp != nil) {
    ComTerpServ *comterp = ((OverlayUnidraw*)unidraw)->comterp();
    ComValue rastval(new OverlayViewRef(rastcomp), rastcomp->classid());
    ComValue vertval(new OverlayViewRef(vertcomp), vertcomp->classid());
    comterp->push_stack(rastval);
    comterp->push_stack(vertval);
    PixelClipFunc clipfunc(comterp, ed);
    clipfunc.exec(2, 0);
  }
}

void PolyClipRasterCmd::Unexecute() {

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlaySelection* sel = (OverlaySelection*) ed->GetSelection();
  Iterator i;
  for (sel->First(i); !sel->Done(i); sel->Next(i)) {
    GraphicView* view = sel->GetView(i);
    if (view->IsA(OVRASTER_VIEW)) {
      RasterOvView* rastview = (RasterOvView*)view;
      if (rastview) {
	RasterOvComp* rastcomp = (RasterOvComp*)rastview->GetSubject();
	OverlayRasterRect* rr = rastcomp->GetOverlayRasterRect();
	if (rr) {
	  rastcomp->Notify();
	  unidraw->Update();
	}
      }
    }
  }
}

