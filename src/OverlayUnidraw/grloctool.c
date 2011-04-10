/*
 * Copyright 1998 Vectaport Inc.
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

#include <OverlayUnidraw/grloctool.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>

#include <IVGlyph/gdialogs.h>

#include <InterViews/event.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>

#include <iostream.h>
#include <stdio.h>

/*****************************************************************************/

ClassId GrLocTool::GetClassId () { return GRLOC_TOOL; }

boolean GrLocTool::IsA (ClassId id) {
    return GRLOC_TOOL == id || Tool::IsA(id);
}

GrLocTool::GrLocTool (ControlInfo* m) : Tool(m)
{
}

Tool* GrLocTool::Copy () { return new GrLocTool(CopyControlInfo()); }

Manipulator* GrLocTool::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel
) {
  OverlayViewer* viewer = (OverlayViewer*)v;
  float xgr, ygr;
  OverlaysView* views = ((OverlayEditor*)viewer->GetEditor())->GetFrame();
  Selection* sel = views->ViewContaining(e.x, e.y);
  if (sel) {
    Iterator i; sel->First(i); 
    OverlayView* view = (OverlayView*) sel->GetView(i);
    Graphic* gr;
    if (view && (gr = view->GetGraphic())) {
      viewer->ScreenToGraphic(e.x, e.y, gr, xgr, ygr);
      char buffer[BUFSIZ];
      sprintf( buffer, "x,y:  %.2f %.2f", xgr, ygr);
      GAcknowledgeDialog* dialog = new GAcknowledgeDialog(buffer);
      Resource::ref(dialog);
      dialog->post_for(v->GetEditor()->GetWindow());
      Resource::unref(dialog);
    }
  }
  Manipulator* m = nil;
  return m;

}
