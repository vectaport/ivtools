/*
 * Copyright (c) 1998 Vectaport Inc.
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
 * Graph tool definitions.
 */

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphtools.h>
#include <GraphUnidraw/nodecomp.h>
#include <OverlayUnidraw/ovviews.h>
#include <TopoFace/topoedge.h>
#include <TopoFace/toponode.h>
#include <Unidraw/iterator.h>
#include <Unidraw/manip.h>
#include <Unidraw/selection.h>
#include <Unidraw/viewer.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

ClassId GraphMoveTool::GetClassId () { return GRAPH_MOVE_TOOL; }

boolean GraphMoveTool::IsA (ClassId id) {
    return GRAPH_MOVE_TOOL == id || MoveTool::IsA(id);
}

GraphMoveTool::GraphMoveTool (ControlInfo* m) : MoveTool(m) { }
Tool* GraphMoveTool::Copy () { return new GraphMoveTool(CopyControlInfo()); }

Command* GraphMoveTool::InterpretManipulator (Manipulator* m) {
  Selection* s;
  Command* cmd = nil;
  Iterator i;
  GraphicView* gv;
  
  if (m != nil) {
    s = m->GetViewer()->GetSelection();
    s->First(i);
    gv = s->GetView(i);
    
    if (s->Number() > 1) {
      /* add all unselected nodes that lie between two selected */
      /* edges to the selection */
      Selection ns;
      while (!s->Done(i)) {
	OverlayView* e1view = (OverlayView*)s->GetView(i);
	if (e1view->IsA(EDGE_VIEW)) {
	  EdgeComp* e1comp = (EdgeComp*) e1view->GetSubject();
	  TopoEdge* edge1 = e1comp->Edge();
	  Iterator j;
	  for (s->First(j); !s->Done(j); s->Next(j)) {
	    OverlayView* e2view = (OverlayView*) s->GetView(j);
	    if (e2view != e1view && e2view->IsA(EDGE_VIEW)) {
	      EdgeComp* e2comp = (EdgeComp*) e2view->GetSubject();
	      TopoEdge* edge2 = e2comp->Edge();
	      if (edge1->start_node() == edge2->end_node() ||
		  edge1->start_node() == edge2->start_node()) {
		NodeComp* ncomp = (NodeComp*) e1comp->NodeStart();
		NodeView* nview = (NodeView*)ncomp->FindView(m->GetViewer());
		ns.Append(nview);
	      } else if (edge1->end_node() == edge2->end_node() ||
			 edge1->end_node() == edge2->start_node()) {
		  NodeComp* ncomp = (NodeComp*) e1comp->NodeEnd();
		  NodeView* nview = (NodeView*)ncomp->FindView(m->GetViewer());
		  ns.Append(nview);
		}
	    }
	  }
	}
	s->Next(i);
      }
      s->Merge(&ns);
      cmd = gv->GraphicView::InterpretManipulator(m);
    } else {
      cmd = gv->InterpretManipulator(m);
    }
  }
  return cmd;
}
