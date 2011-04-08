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

#include <OverlayUnidraw/clipline.h>
#include <OverlayUnidraw/cliplinepoly.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovclip.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovviewer.h>
#include <IVGlyph/gdialogs.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/macro.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/manips.h>
#include <Unidraw/selection.h>
#include <Unidraw/unidraw.h>
#include <IV-2_6/InterViews/rubrect.h>
#include <IV-2_6/InterViews/rubverts.h>
#include <InterViews/cursor.h>
#include <InterViews/event.h>
#include <InterViews/transformer.h>
#include <InterViews/window.h>
#include <OS/math.h>


#ifdef CLIPPOLY

#include <OverlayUnidraw/clippoly.h>
#include <OverlayUnidraw/ovpolygon.h>

#include <Unidraw/Components/polygon.h>
#include <Unidraw/Graphic/polygons.h>

#endif

/*****************************************************************************/

ClipRectCmd::ClipRectCmd(Editor* ed, Selection* sel,
				   Coord l, Coord b, Coord r, Coord t)
: MacroCmd(ed)
{
  _sel = sel;
  _l = l;
  _b = b;
  _r = r;
  _t = t;
}

Command* ClipRectCmd::Copy() {
  Command* copy = new ClipRectCmd(GetEditor(), _sel, _l, _b, _r, _t);
  InitCopy(copy);
  return copy;
}

ClassId ClipRectCmd::GetClassId() { return CLIPRECT_CMD; }

boolean ClipRectCmd::IsA (ClassId id) {
    return CLIPRECT_CMD == id || MacroCmd::IsA(id);
}

void ClipRectCmd::Execute() {

    
    Iterator k;
    First(k);
    if (!Done(k)) {
	MacroCmd::Execute();
	return;
    }
    
    GetEditor()->GetWindow()->cursor(hourglass);

    Coord* x;
    Coord* y;
    Coord lx[2], ly[2];
    int n;
    Transformer tn;
    Clipboard* cutcb = new Clipboard();
    Clipboard* pastecb = new Clipboard();
    Iterator i;
    for (_sel->First(i); !_sel->Done(i); _sel->Next(i)) {
	GraphicView* view = _sel->GetView(i);
	Graphic* graphic = view->GetGraphic();
	if (view->IsA(OVPOLYGON_VIEW)) {
#ifdef CLIPPOLY
	    Coord *tx1, *ty1;
	    float *ftx1, *fty1;
	    float *x1, *y1, *x2, *y2;
	    SF_Polygon *g1;
	    int n1, n2;
	    Transformer tn1, tn2;
	    g1 = (SF_Polygon*)((PolygonOvView*)view)->GetGraphic();
	    n1 = g1->GetOriginal(tx1, ty1);
	    ftx1 = new float[n1];
	    fty1 = new float[n1];
	    for (int j = 0; j < n1; j++) {
	      ftx1[j] = tx1[j];
	      fty1[j] = ty1[j];
	    }
	    n2 = 4;
	    x1 = new float[n1];
	    y1 = new float[n1];
	    x2 = new float[n2];
	    y2 = new float[n2];
	    x2[0] = _l; x2[1] = _l; x2[2] = _r; x2[3] = _r;
	    y2[0] = _b; y2[1] = _t; y2[2] = _t; y2[3] = _b;
	    g1->TotalTransformation(tn1);
	    for (int j = 0; j < n1; j++)
	      tn1.transform(ftx1[j], fty1[j], x1[j], y1[j]);
	    
	    
	    ClipOperation op = A_AND_B;
	    int npolys;
	    int* ni;
	    float** cx;
	    float** cy;
	    clippoly(op, n1, x1, y1, n2, x2, y2, npolys, ni, cx, cy);
	    if (npolys > 0) {
		int j;
		FullGraphic gs(g1);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (j = 0; j < npolys; j++) {
		    Coord *icx, *icy;
		    icx = new Coord[ni[j]];
		    icy = new Coord[ni[j]];
		    for (int ii = 0; ii < ni[j]; ii++) {
		      icx[ii] = Math::round(cx[j][ii]);
		      icy[ii] = Math::round(cy[j][ii]);
		    }
		    SF_Polygon* newpoly = new SF_Polygon(icx, icy, ni[j], &gs);
		    PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
		clippoly_delete(npolys, ni, cx, cy);
	    }
#endif
	}
	else if (view->IsA(OVRECT_VIEW)) {
#ifdef CLIPPOLY
	    float tx1[4], ty1[4];
	    float *x1, *y1, *x2, *y2;
	    SF_Rect *g1;
	    int n1, n2;
	    Transformer tn1, tn2;
	    g1 = (SF_Rect*)((RectOvView*)view)->GetGraphic();
	    n1 = 4;
	    Coord rx0, ry0, rx1, ry1;
	    g1->GetOriginal(rx0, ry0, rx1, ry1);
	    tx1[0] = Math::min(rx0, rx1); ty1[0] = Math::min(ry0, ry1);
	    tx1[1] = Math::min(rx0, rx1); ty1[1] = Math::max(ry0, ry1);
	    tx1[2] = Math::max(rx0, rx1); ty1[2] = Math::max(ry0, ry1);
	    tx1[3] = Math::max(rx0, rx1); ty1[3] = Math::min(ry0, ry1);
	    n2 = 4;
	    x1 = new float[n1];
	    y1 = new float[n1];
	    x2 = new float[n2];
	    y2 = new float[n2];
	    x2[0] = _l; x2[1] = _l; x2[2] = _r; x2[3] = _r;
	    y2[0] = _b; y2[1] = _t; y2[2] = _t; y2[3] = _b;
	    g1->TotalTransformation(tn1);
	    for (int ii = 0; ii < n1; ii++)
	      tn.transform(tx1[ii], ty1[ii], x1[ii], y1[ii]);
	    
	    
	    ClipOperation op = A_AND_B;
	    int npolys;
	    int* ni;
	    float** cx;
	    float** cy;
	    clippoly(op, n1, x1, y1, n2, x2, y2, npolys, ni, cx, cy);
	    if (npolys > 0) {
		int j;
		FullGraphic gs(g1);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (j = 0; j < npolys; j++) {
		    SF_Rect* newrect = new SF_Rect
		      (Math::round(cx[j][0]), Math::round(cy[j][0]),
		       Math::round(cx[j][2]), Math::round(cy[j][2]), &gs);
		    RectOvComp* newcomp = new RectOvComp(newrect);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
		clippoly_delete(npolys, ni, cx, cy);
	    }
#endif
	}
	else {
	    if (view->IsA(OVMULTILINE_VIEW)) {
		MultiLine* ml = (MultiLine*)graphic;
		n = ml->GetOriginal(x, y);
	    }
	    else if (view->IsA(OVLINE_VIEW)) {
		Line* line = (Line*)graphic;
		line->GetOriginal(lx[0], ly[0], lx[1], ly[1]);
		n = 2;
		x = lx;
		y = ly;
	    }
	    Coord *tx = new Coord[n];
	    Coord *ty = new Coord[n];
	    graphic->TotalTransformation(tn);
	    tn.TransformList(x, y, n, tx, ty);
	    int nlines;
	    int* ni;
	    int** cx;
	    int** cy;
	    clipmultiline(n, tx, ty, _l, _b, _r, _t, nlines, ni, cx, cy);
	    if (nlines > 0) {
		FullGraphic gs(graphic);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (int j = 0; j < nlines; j++) {
		    SF_MultiLine* newline = new SF_MultiLine(cx[j], cy[j], ni[j], &gs);
		    MultiLineOvComp* newcomp = new MultiLineOvComp(newline);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
		clipmultiline_delete(nlines, ni, cx, cy);
	    }
	    delete [] tx;
	    delete [] ty;
	}
    }
    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
    Append(cutcmd, pastecmd);
    MacroCmd::Execute();
}

/*****************************************************************************/

ClipRectTool::ClipRectTool(ControlInfo* ci) : Tool(ci) {}

Tool* ClipRectTool::Copy() {
  return new ClipRectTool(CopyControlInfo());
}

ClassId ClipRectTool::GetClassId() { return CLIPRECT_TOOL; }

boolean ClipRectTool::IsA(ClassId id) {
  return CLIPRECT_TOOL == id || Tool::IsA(id);
}

Manipulator* ClipRectTool::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel
) {
  Manipulator* m =
    new DragManip(v, new RubberRect(nil,nil, e.x,e.y,e.x,e.y), rel);
  return m;
}

Command* ClipRectTool::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Viewer* viewer = dm->GetViewer();
    GraphicView* views = viewer->GetGraphicView();
    RubberRect* rr = (RubberRect*) dm->GetRubberband();
    Selection* newSel;
    Selection* mlines = new Selection();
    Coord l, b, r, t;

    rr->GetCurrent(l, b, r, t);
    newSel = views->ViewsIntersecting(l, b, r, t);
    Iterator i;
    for (newSel->First(i); !newSel->Done(i); newSel->Next(i)) {
	GraphicView* view = newSel->GetView(i);
	if (!view->IsA(OVMULTILINE_VIEW) && !view->IsA(OVLINE_VIEW)
#ifdef CLIPPOLY
	    && !view->IsA(OVPOLYGON_VIEW) && !view->IsA(OVRECT_VIEW)
#endif
	) {
#ifdef CLIPPOLY
	    const char * message = "Only polygons, rectangles, multilines, and lines supported by this command";
#else
	    const char * message = "Only lines and multilines supported by this command without clippoly";
#endif
	    GAcknowledgeDialog::post
	      (viewer->GetEditor()->GetWindow(), message, 
	       "(groups are not supported yet either)", "clippoly exception");
	    return nil;
	}
    }
    
    for (newSel->First(i); !newSel->Done(i); newSel->Next(i)) {
	GraphicView* view = newSel->GetView(i);
	if (view->IsA(OVMULTILINE_VIEW) || view->IsA(OVLINE_VIEW) 
#ifdef CLIPPOLY
	    || view->IsA(OVPOLYGON_VIEW) || view->IsA(OVRECT_VIEW)
#endif
	    )
	  mlines->Append(view);
    }
    return new ClipRectCmd(viewer->GetEditor(), mlines, l, b, r, t);
}

/*****************************************************************************/

ClipPolyCmd::ClipPolyCmd(Editor* ed, Selection* sel,
			 float* x, float* y, int n)
: MacroCmd(ed)
{
  _sel = sel;
  _x = x;
  _y = y;
  _n = n;
}

Command* ClipPolyCmd::Copy() {
  Command* copy = new ClipPolyCmd(GetEditor(), _sel, _x, _y, _n);
  InitCopy(copy);
  return copy;
}

ClassId ClipPolyCmd::GetClassId() { return CLIPPOLY_CMD; }

boolean ClipPolyCmd::IsA (ClassId id) {
    return CLIPPOLY_CMD == id || MacroCmd::IsA(id);
}

void ClipPolyCmd::Execute() {
    Iterator k;
    First(k);
    if (!Done(k)) {
	MacroCmd::Execute();
	return;
    }
    
    if (_n >= 3) {
    GetEditor()->GetWindow()->cursor(hourglass);

    Coord* x;
    Coord* y;
    Coord lx[2], ly[2];
    int n;
    Transformer tn;
    Clipboard* cutcb = new Clipboard();
    Clipboard* pastecb = new Clipboard();
    Iterator i;
    for (_sel->First(i); !_sel->Done(i); _sel->Next(i)) {
	GraphicView* view = _sel->GetView(i);
	Graphic* graphic = view->GetGraphic();
	if (view->IsA(OVPOLYGON_VIEW)) {
#ifdef CLIPPOLY
	    Coord *tx1, *ty1;
	    float *ftx1, *fty1;
	    float *x1, *y1, *x2, *y2;
	    SF_Polygon *g1;
	    int n1, n2;
	    Transformer tn1, tn2;
	    g1 = (SF_Polygon*)((PolygonOvView*)view)->GetGraphic();
	    n1 = g1->GetOriginal(tx1, ty1);
	    ftx1 = new float[n1];
	    fty1 = new float[n1];
	    for (int ii = 0; ii < n1; ii++) {
	      ftx1[ii] = tx1[ii];
	      fty1[ii] = ty1[ii];
	    }
	    x1 = new float[n1];
	    y1 = new float[n1];
	    g1->TotalTransformation(tn1);
	    for (int ii = 0; ii < n1; ii++)
	      tn1.transform(ftx1[ii], fty1[ii], x1[ii], y1[ii]);

	    float *fx, *fy;
	    fx = new float[_n];
	    fy = new float[_n];
	    for (int ii = 0; ii < _n; ii++) {
	      fx[ii] = _x[ii];
	      fy[ii] = _y[ii];
	    }
	    
	    ClipOperation op = A_AND_B;
	    int npolys;
	    int* ni;
	    float** cx;
	    float** cy;
	    clippoly(op, n1, x1, y1, _n, fx, fy, npolys, ni, cx, cy);
	    if (npolys > 0) {
		int j;
		FullGraphic gs(g1);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (j = 0; j < npolys; j++) {
		    Coord *icx, *icy;
		    icx = new Coord[ni[j]];
		    icy = new Coord[ni[j]];
		    for (int ii = 0; ii < ni[j]; ii++) {
		      icx[ii] = Math::round(cx[j][ii]);
		      icy[ii] = Math::round(cy[j][ii]);
		    }
		    SF_Polygon* newpoly = new SF_Polygon(icx, icy, ni[j], &gs);
		    PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
		clippoly_delete(npolys, ni, cx, cy);
	    }
#endif
	}
	else if (view->IsA(OVRECT_VIEW)) {
#ifdef CLIPPOLY
	    float tx1[4], ty1[4];
	    float *x1, *y1, *x2, *y2;
	    SF_Rect *g1;
	    int n1, n2;
	    Transformer tn1, tn2;
	    g1 = (SF_Rect*)((RectOvView*)view)->GetGraphic();
	    n1 = 4;
	    Coord rx0, ry0, rx1, ry1;
	    g1->GetOriginal(rx0, ry0, rx1, ry1);
	    tx1[0] = Math::min(rx0, rx1); ty1[0] = Math::min(ry0, ry1);
	    tx1[1] = Math::min(rx0, rx1); ty1[1] = Math::max(ry0, ry1);
	    tx1[2] = Math::max(rx0, rx1); ty1[2] = Math::max(ry0, ry1);
	    tx1[3] = Math::max(rx0, rx1); ty1[3] = Math::min(ry0, ry1);
	    x1 = new float[n1];
	    y1 = new float[n1];
	    g1->TotalTransformation(tn1);
	    for (int ii = 0; ii < n1; ii++)
	      tn1.transform(tx1[ii], ty1[ii], x1[ii], y1[ii]);

	    float *fx, *fy;
	    fx = new float[_n];
	    fy = new float[_n];
	    for (int ii = 0; ii < _n; ii++) {
	      fx[ii] = _x[ii];
	      fy[ii] = _y[ii];
	    }
	    
	    ClipOperation op = A_AND_B;
	    int npolys;
	    int* ni;
	    float** cx;
	    float** cy;
	    clippoly(op, n1, x1, y1, _n, fx, fy, npolys, ni, cx, cy);
	    if (npolys > 0) {
		int j;
		FullGraphic gs(g1);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (j = 0; j < npolys; j++) {
		    SF_Rect* newrect = new SF_Rect
		      (Math::round(cx[j][0]), Math::round(cy[j][0]),
		       Math::round(cx[j][2]), Math::round(cy[j][2]), &gs);
		    RectOvComp* newcomp = new RectOvComp(newrect);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
		clippoly_delete(npolys, ni, cx, cy);
	    }
#endif
	}
	else {
	    if (view->IsA(OVMULTILINE_VIEW)) {
		MultiLine* ml = (MultiLine*)graphic;
		n = ml->GetOriginal(x, y);
	    }
	    else if (view->IsA(OVLINE_VIEW)) {
		Line* line = (Line*)graphic;
		line->GetOriginal(lx[0], ly[0], lx[1], ly[1]);
		n = 2;
		x = lx;
		y = ly;
	    }
	    float *tx = new float[n];
	    float *ty = new float[n];
	    graphic->TotalTransformation(tn);
	    //tn.TransformList(x, y, n, tx, ty);
	    for (int ii = 0; ii < n; ii++)
	      tn.transform(x[ii], y[ii], tx[ii], ty[ii]);

	    int in_nlines;
	    int* in_ni;
	    float** in_cx;
	    float** in_cy;
	    int out_nlines;
	    int* out_ni;
	    float** out_cx;
	    float** out_cy;
	    cliplinepoly(n, tx, ty, _n, _x, _y,
			  in_nlines, in_ni, in_cx, in_cy,
			  out_nlines, out_ni, out_cx, out_cy);
	    if (in_nlines > 0) {
		FullGraphic gs(graphic);
		Transformer* rel = new Transformer
		    (((OverlayViewer*)view->GetViewer())->GetRel());
		rel->Invert();
		gs.SetTransformer(rel);
		rel->unref();
		for (int j = 0; j < in_nlines; j++) {
		    Coord *icx, *icy;
		    icx = new Coord[in_ni[j]];
		    icy = new Coord[in_ni[j]];
		    for (int ii = 0; ii < in_ni[j]; ii++) {
		      icx[ii] = Math::round(in_cx[j][ii]);
		      icy[ii] = Math::round(in_cy[j][ii]);
		    }
		    SF_MultiLine* newline =
		      new SF_MultiLine(icx, icy, in_ni[j], &gs);
		    MultiLineOvComp* newcomp = new MultiLineOvComp(newline);
		    pastecb->Append(newcomp);
		}
		cutcb->Append(view->GetGraphicComp());
	    }
	    delete [] tx;
	    delete [] ty;
	}
    }
    // paste clipping polygon to aid debug
    FullGraphic gs(stdgraphic);
    gs.SetPattern(psnonepat);
    Transformer* rel = new Transformer
      (((OverlayViewer*)GetEditor()->GetViewer())->GetRel());
    rel->Invert();
    gs.SetTransformer(rel);
    rel->unref();
    Coord *ix, *iy;
    ix = new Coord[_n];
    iy = new Coord[_n];
    for (int ii = 0; ii < _n; ii++) {
      ix[ii] = Math::round(_x[ii]);
      iy[ii] = Math::round(_y[ii]);
    }
    SF_Polygon* cpoly = new SF_Polygon(ix, iy, _n, &gs);
    PolygonOvComp* ccomp = new PolygonOvComp(cpoly);
    pastecb->Append(ccomp);

    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
    Append(cutcmd, pastecmd);
    MacroCmd::Execute();
    }
}

/*****************************************************************************/

ClipPolyTool::ClipPolyTool(ControlInfo* ci) : Tool(ci) {}

Tool* ClipPolyTool::Copy() {
  return new ClipPolyTool(CopyControlInfo());
}

ClassId ClipPolyTool::GetClassId() { return CLIPPOLY_TOOL; }

boolean ClipPolyTool::IsA(ClassId id) {
  return CLIPPOLY_TOOL == id || Tool::IsA(id);
}

Manipulator* ClipPolyTool::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel
) {
  Coord x[1], y[1];
  x[0] = e.x;
  y[0] = e.y;
  GrowingVertices* rub = new GrowingPolygon(nil, nil, x, y, 1, -1, HANDLE_SIZE
        );
  Manipulator* m =
    new VertexManip(v, rub, rel, this);
  return m;
}

Command* ClipPolyTool::InterpretManipulator (Manipulator* m) {
    VertexManip* vm = (VertexManip*) m;
    Viewer* viewer = vm->GetViewer();
    GraphicView* views = viewer->GetGraphicView();
    GrowingVertices* gv = vm->GetGrowingVertices();
    Selection* newSel;
    Selection* mlines = new Selection();
    Coord* x;
    Coord* y;
    int n;

    gv->GetCurrent(x, y, n);
    FillPolygonObj po(x, y, n);
    BoxObj bbox;
    po.GetBox(bbox);
    newSel = views->ViewsIntersecting(bbox._left, bbox._bottom,
				      bbox._right, bbox._top);
    Iterator i;
    for (newSel->First(i); !newSel->Done(i); newSel->Next(i)) {
	GraphicView* view = newSel->GetView(i);
	if (!view->IsA(OVMULTILINE_VIEW) && !view->IsA(OVLINE_VIEW)
#ifdef CLIPPOLY
	    && !view->IsA(OVPOLYGON_VIEW) && !view->IsA(OVRECT_VIEW)
#endif
	) {
#ifdef CLIPPOLY
	    const char * message = "Only polygons, rectangles, multilines, and lines supported by this command";
#else
	    const char * message = "Only lines and multilines supported by this command without clippoly";
#endif
	    GAcknowledgeDialog::post
	      (viewer->GetEditor()->GetWindow(), message, 
	       "(groups are not supported yet either)", "clippoly exception");
	    return nil;
	}
    }
    
    for (newSel->First(i); !newSel->Done(i); newSel->Next(i)) {
	GraphicView* view = newSel->GetView(i);
	if (view->IsA(OVMULTILINE_VIEW) || view->IsA(OVLINE_VIEW) 
#ifdef CLIPPOLY
	    || view->IsA(OVPOLYGON_VIEW) || view->IsA(OVRECT_VIEW)
#endif
	    )
	  mlines->Append(view);
    }
    float* fx = new float[n];
    float* fy = new float[n];
    for (int ii = 0; ii < n; ii++) {
      fx[ii] = x[ii];
      fy[ii] = y[ii];
    }
    return new ClipPolyCmd(viewer->GetEditor(), mlines, fx, fy, n);
}

/*****************************************************************************/
#ifdef CLIPPOLY

ClassId ClipPolyAMinusBCmd::GetClassId () { return CLIPPOLY_AMINB_CMD; }

boolean ClipPolyAMinusBCmd::IsA (ClassId id) {
    return CLIPPOLY_AMINB_CMD == id || MacroCmd::IsA(id);
}

ClipPolyAMinusBCmd::ClipPolyAMinusBCmd (ControlInfo* c) : MacroCmd(c) {
}

ClipPolyAMinusBCmd::ClipPolyAMinusBCmd (Editor* ed) : MacroCmd(ed) {
}

Command* ClipPolyAMinusBCmd::Copy () {
    Command* copy = new ClipPolyAMinusBCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void ClipPolyAMinusBCmd::Execute () {
    Iterator j;
    First(j);
    if (!Done(j)) {
        MacroCmd::Execute();
        return;
    }

    GetEditor()->GetWindow()->cursor(hourglass);
    Selection* sel = GetEditor()->GetSelection();
    if (sel && !sel->IsEmpty()) {
	Iterator i;
	sel->First(i);
	GraphicView* view1 = sel->GetView(i);
	sel->Next(i);
	if (sel->Done(i))
	    return;
	GraphicView* view2 = sel->GetView(i);
	sel->Next(i);

	if (view1 && view1->IsA(OVPOLYGON_VIEW) && 
	    view2 && view2->IsA(OVPOLYGON_VIEW)) {
	Coord *tx1, *ty1, *tx2, *ty2;
	float *x1, *y1, *x2, *y2;
	SF_Polygon *g1, *g2;
	int n1, n2;
	Transformer tn1, tn2;
	g1 = (SF_Polygon*)((PolygonOvView*)view1)->GetGraphic();
	g2 = (SF_Polygon*)((PolygonOvView*)view2)->GetGraphic();
	n1 = g1->GetOriginal(tx1, ty1);
	n2 = g2->GetOriginal(tx2, ty2);
	x1 = new float[n1];
	y1 = new float[n1];
	x2 = new float[n2];
	y2 = new float[n2];
	g1->TotalTransformation(tn1);
	g2->TotalTransformation(tn2);
	for (int ii = 0; ii < n1; ii++)
	  tn1.transform(tx1[ii], ty1[ii], x1[ii], y1[ii]);
	for (int ii = 0; ii < n2; ii++)
	  tn1.transform(tx2[ii], ty2[ii], x2[ii], y2[ii]);

	ClipOperation op = A_MIN_B;
	int npolys;
	int* ni;
	float** x;
	float** y;
	clippoly(op, n1, x1, y1, n2, x2, y2, npolys, ni, x, y);

	if (npolys > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    int j;
	    FullGraphic gs(g1);
	    Transformer* rel = new Transformer
		(((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (j = 0; j < npolys; j++) {
	        Coord *icx, *icy;
		icx = new Coord[ni[j]];
		icy = new Coord[ni[j]];
		for (int ii = 0; ii < ni[j]; ii++) {
		  icx[ii] = Math::round(x[j][ii]);
		  icy[ii] = Math::round(y[j][ii]);
		}
		SF_Polygon* newpoly = new SF_Polygon(icx, icy, ni[j], &gs);
		PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
		pastecb->Append(newcomp);
	    }

            Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
            globalcb->DeleteComps();
            globalcb->Clear();
            globalcb->Append((GraphicComp*)view2->GetGraphicComp()->Copy());

	    cutcb->Append(view1->GetGraphicComp());
	    cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();

	    clippoly_delete(npolys, ni, x, y);
	}
	delete [] x1;
	delete [] y1;
	delete [] x2;
	delete [] y2;
	}
	else if (view1 && (view1->IsA(OVLINE_VIEW) || view1->IsA(OVMULTILINE_VIEW)) &&
		 view2 && view2->IsA(OVPOLYGON_VIEW)) {
	  Transformer tn1, tn2;
	  Coord* x;
	  Coord* y;
	  Coord lx[2], ly[2];
	  int n;
	  Graphic* graphic1 = view1->GetGraphic();
	  if (view1->IsA(OVMULTILINE_VIEW)) {
	    MultiLine* ml = (MultiLine*)graphic1;
	    n = ml->GetOriginal(x, y);
	  }
	  else if (view1->IsA(OVLINE_VIEW)) {
	    Line* line = (Line*)graphic1;
	    line->GetOriginal(lx[0], ly[0], lx[1], ly[1]);
	    n = 2;
	    x = lx;
	    y = ly;
	  }
	  float *tx = new float[n];
	  float *ty = new float[n];
	  graphic1->TotalTransformation(tn1);
	  // tn1.TransformList(x, y, n, tx, ty);
	  for (int ii = 0; ii < n; ii++)
	    tn1.transform(x[ii], y[ii], tx[ii], ty[ii]);

	  SF_Polygon* graphic2 = (SF_Polygon*)view2->GetGraphic();
	  Coord* px;
	  Coord* py;
	  int pn;
	  pn = graphic2->GetOriginal(px, py);
	  graphic2->TotalTransformation(tn2);
	  float* tpx = new float[pn];
	  float* tpy = new float[pn];
	  // tn2.TransformList(px, py, pn, tpx, tpy);
	  for (int ii = 0; ii < pn; ii++)
	    tn2.transform(px[ii], py[ii], tpx[ii], tpy[ii]);

	  int in_nlines;
	  int* in_ni;
	  float** in_cx;
	  float** in_cy;
	  int out_nlines;
	  int* out_ni;
	  float** out_cx;
	  float** out_cy;
	  cliplinepoly(n, tx, ty, pn, tpx, tpy,
		       in_nlines, in_ni, in_cx, in_cy,
		       out_nlines, out_ni, out_cx, out_cy);
	  if (out_nlines > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    FullGraphic gs(graphic1);
	    Transformer* rel = new Transformer
	      (((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (int k = 0; k < out_nlines; k++) {
	      Coord *icx, *icy;
	      icx = new Coord[out_ni[k]];
	      icy = new Coord[out_ni[k]];
	      for (int ii = 0; ii < out_ni[k]; ii++) {
		icx[ii] = Math::round(out_cx[k][ii]);
		icy[ii] = Math::round(out_cy[k][ii]);
	      }
	      SF_MultiLine* newline =
		new SF_MultiLine(icx, icy, out_ni[k], &gs);
	      MultiLineOvComp* newcomp = new MultiLineOvComp(newline);
	      pastecb->Append(newcomp);
	    }
	    cutcb->Append(view1->GetGraphicComp());
	    // cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();
	  }
	}
    }
}

/*****************************************************************************/

ClassId ClipPolyBMinusACmd::GetClassId () { return CLIPPOLY_BMINA_CMD; }

boolean ClipPolyBMinusACmd::IsA (ClassId id) {
    return CLIPPOLY_BMINA_CMD == id || MacroCmd::IsA(id);
}

ClipPolyBMinusACmd::ClipPolyBMinusACmd (ControlInfo* c) : MacroCmd(c) {
}

ClipPolyBMinusACmd::ClipPolyBMinusACmd (Editor* ed) : MacroCmd(ed) {
}

Command* ClipPolyBMinusACmd::Copy () {
    Command* copy = new ClipPolyBMinusACmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void ClipPolyBMinusACmd::Execute () {
    Iterator j;
    First(j);
    if (!Done(j)) {
        MacroCmd::Execute();
        return;
    }

    GetEditor()->GetWindow()->cursor(hourglass);
    Selection* sel = GetEditor()->GetSelection();
    if (sel && !sel->IsEmpty()) {
	Iterator i;
	sel->First(i);
	GraphicView* view1 = sel->GetView(i);
	sel->Next(i);
	if (sel->Done(i))
	    return;
	GraphicView* view2 = sel->GetView(i);
	sel->Next(i);
	if (view1 && view1->IsA(OVPOLYGON_VIEW) &&
	    view2 && view2->IsA(OVPOLYGON_VIEW)) {

	Coord *tx1, *ty1, *tx2, *ty2;
	float *x1, *y1, *x2, *y2;
	SF_Polygon *g1, *g2;
	int n1, n2;
	Transformer tn1, tn2;
	g1 = (SF_Polygon*)((PolygonOvView*)view1)->GetGraphic();
	g2 = (SF_Polygon*)((PolygonOvView*)view2)->GetGraphic();
	n1 = g1->GetOriginal(tx1, ty1);
	n2 = g2->GetOriginal(tx2, ty2);
	x1 = new float[n1];
	y1 = new float[n1];
	x2 = new float[n2];
	y2 = new float[n2];
	g1->TotalTransformation(tn1);
	g2->TotalTransformation(tn2);
	for (int ii = 0; ii < n1; ii++)
	  tn1.transform(tx1[ii], ty1[ii], x1[ii], y1[ii]);
	for (int ii = 0; ii < n2; ii++)
	  tn1.transform(tx2[ii], ty2[ii], x2[ii], y2[ii]);

	ClipOperation op = B_MIN_A;
	int npolys;
	int* ni;
	float** x;
	float** y;
	clippoly(op, n1, x1, y1, n2, x2, y2, npolys, ni, x, y);

	if (npolys > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    int j;
	    FullGraphic gs(g2);
	    Transformer* rel = new Transformer
		(((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (j = 0; j < npolys; j++) {
	        Coord *icx, *icy;
		icx = new Coord[ni[j]];
		icy = new Coord[ni[j]];
		for (int ii = 0; ii < ni[j]; ii++) {
		  icx[ii] = Math::round(x[j][ii]);
		  icy[ii] = Math::round(y[j][ii]);
		}
		SF_Polygon* newpoly = new SF_Polygon(icx, icy, ni[j], &gs);
		PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
		pastecb->Append(newcomp);
	    }

            Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
            globalcb->DeleteComps();
            globalcb->Clear();
            globalcb->Append((GraphicComp*)view1->GetGraphicComp()->Copy());

	    cutcb->Append(view1->GetGraphicComp());
	    cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();

	    clippoly_delete(npolys, ni, x, y);
	}
	delete [] x1;
	delete [] y1;
	delete [] x2;
	delete [] y2;
	}
	else if (view2 && (view2->IsA(OVLINE_VIEW) || view2->IsA(OVMULTILINE_VIEW)) &&
		 view1 && view1->IsA(OVPOLYGON_VIEW)) {
	  Transformer tn1, tn2;
	  Coord* x;
	  Coord* y;
	  Coord lx[2], ly[2];
	  int n;
	  Graphic* graphic1 = view2->GetGraphic();
	  if (view2->IsA(OVMULTILINE_VIEW)) {
	    MultiLine* ml = (MultiLine*)graphic1;
	    n = ml->GetOriginal(x, y);
	  }
	  else if (view2->IsA(OVLINE_VIEW)) {
	    Line* line = (Line*)graphic1;
	    line->GetOriginal(lx[0], ly[0], lx[1], ly[1]);
	    n = 2;
	    x = lx;
	    y = ly;
	  }
	  float *tx = new float[n];
	  float *ty = new float[n];
	  graphic1->TotalTransformation(tn1);
	  // tn1.TransformList(x, y, n, tx, ty);
	  for (int ii = 0; ii < n; ii++)
	    tn1.transform(x[ii], y[ii], tx[ii], ty[ii]);

	  SF_Polygon* graphic2 = (SF_Polygon*)view1->GetGraphic();
	  Coord* px;
	  Coord* py;
	  int pn;
	  pn = graphic2->GetOriginal(px, py);
	  graphic2->TotalTransformation(tn2);
	  float* tpx = new float[pn];
	  float* tpy = new float[pn];
	  // tn2.TransformList(px, py, pn, tpx, tpy);
	  for (int ii = 0; ii < pn; ii++)
	    tn2.transform(px[ii], py[ii], tpx[ii], tpy[ii]);

	  int in_nlines;
	  int* in_ni;
	  float** in_cx;
	  float** in_cy;
	  int out_nlines;
	  int* out_ni;
	  float** out_cx;
	  float** out_cy;
	  cliplinepoly(n, tx, ty, pn, tpx, tpy,
		       in_nlines, in_ni, in_cx, in_cy,
		       out_nlines, out_ni, out_cx, out_cy);
	  if (out_nlines > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    FullGraphic gs(graphic1);
	    Transformer* rel = new Transformer
	      (((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (int k = 0; k < out_nlines; k++) {
	      Coord *icx, *icy;
	      icx = new Coord[out_ni[k]];
	      icy = new Coord[out_ni[k]];
	      for (int ii = 0; ii < out_ni[k]; ii++) {
		icx[ii] = Math::round(out_cx[k][ii]);
		icy[ii] = Math::round(out_cy[k][ii]);
	      }
	      SF_MultiLine* newline =
		new SF_MultiLine(icx, icy, out_ni[k], &gs);
	      MultiLineOvComp* newcomp = new MultiLineOvComp(newline);
	      pastecb->Append(newcomp);
	    }
	    // cutcb->Append(view1->GetGraphicComp());
	    cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();
	  }
	}
    }
}

/*****************************************************************************/

ClassId ClipPolyAAndBCmd::GetClassId () { return CLIPPOLY_AANDB_CMD; }

boolean ClipPolyAAndBCmd::IsA (ClassId id) {
    return CLIPPOLY_AANDB_CMD == id || MacroCmd::IsA(id);
}

ClipPolyAAndBCmd::ClipPolyAAndBCmd (ControlInfo* c) : MacroCmd(c) {
}

ClipPolyAAndBCmd::ClipPolyAAndBCmd (Editor* ed) : MacroCmd(ed) {
}

Command* ClipPolyAAndBCmd::Copy () {
    Command* copy = new ClipPolyAAndBCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void ClipPolyAAndBCmd::Execute () {
    Iterator j;
    First(j);
    if (!Done(j)) {
        MacroCmd::Execute();
        return;
    }

    GetEditor()->GetWindow()->cursor(hourglass);
    Selection* sel = GetEditor()->GetSelection();
    if (sel && !sel->IsEmpty()) {
	Iterator i;
	sel->First(i);
	GraphicView* view1 = sel->GetView(i);
	sel->Next(i);
	if (sel->Done(i))
	    return;
	GraphicView* view2 = sel->GetView(i);
	sel->Next(i);
	if (view1 && view1->IsA(OVPOLYGON_VIEW) &&
	    view2 && view2->IsA(OVPOLYGON_VIEW)) {

	Coord *tx1, *ty1, *tx2, *ty2;
	float *x1, *y1, *x2, *y2;
	SF_Polygon *g1, *g2;
	int n1, n2;
	Transformer tn1, tn2;
	g1 = (SF_Polygon*)((PolygonOvView*)view1)->GetGraphic();
	g2 = (SF_Polygon*)((PolygonOvView*)view2)->GetGraphic();
	n1 = g1->GetOriginal(tx1, ty1);
	n2 = g2->GetOriginal(tx2, ty2);
	x1 = new float[n1];
	y1 = new float[n1];
	x2 = new float[n2];
	y2 = new float[n2];
	g1->TotalTransformation(tn1);
	g2->TotalTransformation(tn2);
	for (int ii = 0; ii < n1; ii++)
	  tn1.transform(tx1[ii], ty1[ii], x1[ii], y1[ii]);
	for (int ii = 0; ii < n2; ii++)
	  tn1.transform(tx2[ii], ty2[ii], x2[ii], y2[ii]);

	ClipOperation op = A_AND_B;
	int npolys;
	int* ni;
	float** x;
	float** y;
	clippoly(op, n1, x1, y1, n2, x2, y2, npolys, ni, x, y);

	if (npolys > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    int j;
	    FullGraphic gs(g1);
	    Transformer* rel = new Transformer
		(((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (j = 0; j < npolys; j++) {
	        Coord *icx, *icy;
		icx = new Coord[ni[j]];
		icy = new Coord[ni[j]];
		for (int ii = 0; ii < ni[j]; ii++) {
		  icx[ii] = Math::round(x[j][ii]);
		  icy[ii] = Math::round(y[j][ii]);
		}
		SF_Polygon* newpoly = new SF_Polygon(icx, icy, ni[j], &gs);
		PolygonOvComp* newcomp = new PolygonOvComp(newpoly);
		pastecb->Append(newcomp);
	    }

            Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
            globalcb->DeleteComps();
            globalcb->Clear();
            globalcb->Append((GraphicComp*)view1->GetGraphicComp()->Copy());
            globalcb->Append((GraphicComp*)view2->GetGraphicComp()->Copy());

	    cutcb->Append(view1->GetGraphicComp());
	    cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();

	    clippoly_delete(npolys, ni, x, y);
	}
	delete [] x1;
	delete [] y1;
	delete [] x2;
	delete [] y2;
	}
	else if (view1 && (view1->IsA(OVLINE_VIEW) || view1->IsA(OVMULTILINE_VIEW)) &&
		 view2 && view2->IsA(OVPOLYGON_VIEW)) {
	  Transformer tn1, tn2;
	  Coord* x;
	  Coord* y;
	  Coord lx[2], ly[2];
	  int n;
	  Graphic* graphic1 = view1->GetGraphic();
	  if (view1->IsA(OVMULTILINE_VIEW)) {
	    MultiLine* ml = (MultiLine*)graphic1;
	    n = ml->GetOriginal(x, y);
	  }
	  else if (view1->IsA(OVLINE_VIEW)) {
	    Line* line = (Line*)graphic1;
	    line->GetOriginal(lx[0], ly[0], lx[1], ly[1]);
	    n = 2;
	    x = lx;
	    y = ly;
	  }
	  float *tx = new float[n];
	  float *ty = new float[n];
	  graphic1->TotalTransformation(tn1);
	  // tn1.TransformList(x, y, n, tx, ty);
	  for (int ii = 0; ii < n; ii++)
	    tn1.transform(x[ii], y[ii], tx[ii], ty[ii]);

	  SF_Polygon* graphic2 = (SF_Polygon*)view2->GetGraphic();
	  Coord* px;
	  Coord* py;
	  int pn;
	  pn = graphic2->GetOriginal(px, py);
	  graphic2->TotalTransformation(tn2);
	  float* tpx = new float[pn];
	  float* tpy = new float[pn];
	  // tn2.TransformList(px, py, pn, tpx, tpy);
	  for (int ii = 0; ii < pn; ii++)
	    tn2.transform(px[ii], py[ii], tpx[ii], tpy[ii]);

	  int in_nlines;
	  int* in_ni;
	  float** in_cx;
	  float** in_cy;
	  int out_nlines;
	  int* out_ni;
	  float** out_cx;
	  float** out_cy;
	  cliplinepoly(n, tx, ty, pn, tpx, tpy,
		       in_nlines, in_ni, in_cx, in_cy,
		       out_nlines, out_ni, out_cx, out_cy);
	  if (in_nlines > 0) {
	    Clipboard* cutcb = new Clipboard();
	    Clipboard* pastecb = new Clipboard();
	    FullGraphic gs(graphic1);
	    Transformer* rel = new Transformer
	      (((OverlayViewer*)view1->GetViewer())->GetRel());
	    rel->Invert();
	    gs.SetTransformer(rel);
	    rel->unref();
	    for (int k = 0; k < in_nlines; k++) {
	      Coord *icx, *icy;
	      icx = new Coord[in_ni[k]];
	      icy = new Coord[in_ni[k]];
	      for (int ii = 0; ii < in_ni[k]; ii++) {
		icx[ii] = Math::round(in_cx[k][ii]);
		icy[ii] = Math::round(in_cy[k][ii]);
	      }
	      SF_MultiLine* newline =
		new SF_MultiLine(icx, icy, in_ni[k], &gs);
	      MultiLineOvComp* newcomp = new MultiLineOvComp(newline);
	      pastecb->Append(newcomp);
	    }
	    cutcb->Append(view1->GetGraphicComp());
	    // cutcb->Append(view2->GetGraphicComp());
	    CutCmd* cutcmd = new CutCmd(GetEditor(), cutcb);
	    PasteCmd* pastecmd = new PasteCmd(GetEditor(), pastecb);
	    Append(cutcmd, pastecmd);
	    MacroCmd::Execute();
	  }
	}

    }
}
#endif
