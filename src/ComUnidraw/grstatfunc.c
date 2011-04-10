/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

#include <ComUnidraw/grstatfunc.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/Components/grview.h>
#include <Unidraw/Components/grcomp.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/verts.h>

#include <ComTerp/comvalue.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>

#define TITLE "GrStatFunc"

static int scrn_symid = symbol_add("scrn");

/*****************************************************************************/

CenterFunc::CenterFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CenterFunc::execute() {
    
    static int xy_symval = symbol_add("xy");
    static int yx_symval = symbol_add("yx");
    static int x_symval = symbol_add("x");
    static int y_symval = symbol_add("y");
    boolean xy_flag = stack_key(xy_symval).is_true();
    boolean yx_flag = stack_key(yx_symval).is_true();
    boolean x_flag = stack_key(x_symval).is_true();
    boolean y_flag = stack_key(y_symval).is_true();
    if (!yx_flag && !x_flag && !y_flag) xy_flag = true;
    boolean return_an_array = xy_flag || yx_flag;

    boolean scrn_flag = stack_key(scrn_symid).is_true();

    Viewer* viewer = _ed->GetViewer();
    ComValue obj(stack_arg(0));
    reset_stack();
    if (obj.object_compview()) {
      ComponentView* compview = (ComponentView*)obj.obj_val();
      if (compview && compview->GetSubject()) {
	Graphic* gr = ((GraphicComp*)compview->GetSubject())->GetGraphic();
	if (gr) {
	  float cx, cy;
	  gr->GetCenter(cx, cy);

	  if (scrn_flag)
	    ((OverlayViewer*)viewer)->DrawingToScreen(cx, cy, cx, cy);

	  if (return_an_array) {
	    AttributeValueList* avl = new AttributeValueList();
	    ComValue* v1 = new ComValue(xy_flag ? cx : cy);
	    ComValue* v2 = new ComValue(xy_flag ? cy : cx);
	    avl->Append(v1);
	    avl->Append(v2);
	    ComValue retval(avl);
	    push_stack(retval);
	  } else {
	    ComValue retval(x_flag ? cx : cy);
	    push_stack(retval);
	  }
	}
      } 	
    }
}

/*****************************************************************************/

MbrFunc::MbrFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void MbrFunc::execute() {
    
    static int lbrt_symval = symbol_add("lbrt");
    static int lrbt_symval = symbol_add("lrbt");
    boolean lbrt_flag = stack_key(lbrt_symval).is_true();
    boolean lrbt_flag = stack_key(lrbt_symval).is_true();

    boolean scrn_flag = stack_key(scrn_symid).is_true();

    Viewer* viewer = _ed->GetViewer();
    ComValue obj(stack_arg(0));
    reset_stack();
    if (obj.object_compview()) {
      ComponentView* compview = (ComponentView*)obj.obj_val();
      if (compview && compview->GetSubject()) {
	Graphic* gr = ((GraphicComp*)compview->GetSubject())->GetGraphic();
	if (gr) {
	  float l, b, r, t;
	  gr->GetBounds(l, b, r, t);

	  if (scrn_flag) {
	    ((OverlayViewer*)viewer)->DrawingToScreen(l, b, l, b);
	    ((OverlayViewer*)viewer)->DrawingToScreen(r, t, r, t);
	  }

	  AttributeValueList* avl = new AttributeValueList();
	  ComValue* lval = new ComValue(l);
	  ComValue* bval = new ComValue(b);
	  ComValue* rval = new ComValue(r);
	  ComValue* tval = new ComValue(t);
	  avl->Append(lval);
	  avl->Append(!lrbt_flag ? bval : rval);
	  avl->Append(!lrbt_flag ? rval : bval);
	  avl->Append(tval);
	  ComValue retval(avl);
	  push_stack(retval);
	}
      } 	
    }
}


/*****************************************************************************/

PointsFunc::PointsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PointsFunc::execute() {
    
    Viewer* viewer = _ed->GetViewer();
    ComValue obj(stack_arg(0));
    reset_stack();
    if (obj.object_compview()) {
      ComponentView* compview = (ComponentView*)obj.obj_val();
      if (compview && compview->GetSubject()) {
	GraphicComp* comp = (GraphicComp*)compview->GetSubject();
	Graphic* gr = comp ? comp->GetGraphic() : nil;
	AttributeValueList* avl = new AttributeValueList();
	if (gr && comp->IsA(OVVERTICES_COMP)) {
	  VerticesOvComp* vertcomp = (VerticesOvComp*)comp;
	  Vertices* vertgr = vertcomp->GetVertices();
	  for(int i=0; i<vertgr->count(); i++) {
	    ComValue* val = new ComValue(vertgr->x()[i]);
	    avl->Append(val);
	    val = new ComValue(vertgr->y()[i]);	    
	    avl->Append(val);
	  }

	} else if (gr && comp->IsA(OVLINE_COMP)) {
	  LineOvComp* linecomp = (LineOvComp*)comp;
	  Coord x0, y0, x1, y1;
	  linecomp->GetLine()->GetOriginal(x0, y0, x1, y1);
	  avl->Append(new ComValue(x0));
	  avl->Append(new ComValue(y0));
	  avl->Append(new ComValue(x1));
	  avl->Append(new ComValue(y1));
	}

	ComValue retval(avl);
	push_stack(retval);
      } 	
    }
}


