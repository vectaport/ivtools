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

#include <ComUnidraw/grfunc.h>

#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovtext.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/statevars.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Commands/brushcmd.h>
#include <Unidraw/Commands/colorcmd.h>
#include <Unidraw/Commands/patcmd.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/font.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Components/text.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/ellipses.h>

#include <InterViews/transformer.h>

#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

#define TITLE "GrFunc"

/*****************************************************************************/

CreateRectFunc::CreateRectFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateRectFunc::execute() {
    const int x0 = 0;  
    const int y0 = 1;  
    const int x1 = 2;  
    const int y1 = 3;  
    const int n = 4;
    int coords[n];
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len() != n) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<n && !avl->Done(i); j++) {
        coords[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (coords[x0] != coords[x1] || coords[y0] != coords[y1]) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	SF_Rect* rect = new SF_Rect(coords[x0], coords[y0], coords[x1], coords[y1], stdgraphic);

	if (brVar != nil) rect->SetBrush(brVar->GetBrush());
	if (patVar != nil) rect->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    rect->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	rect->SetTransformer(rel);
	Unref(rel);
	RectOvComp* comp = new RectOvComp(rect);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreateLineFunc::CreateLineFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateLineFunc::execute() {
    const int x0 = 0;  
    const int y0 = 1;  
    const int x1 = 2;  
    const int y1 = 3;  
    const int n = 4;
    int coords[n];
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len() != n) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<n && !avl->Done(i); j++) {
        coords[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (coords[x0] != coords[x1] || coords[y0] != coords[y1]) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowLine* line = new ArrowLine(coords[x0], coords[y0], coords[x1], coords[y1], aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) line->SetBrush(brVar->GetBrush());

	if (colVar != nil) {
	    line->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	line->SetTransformer(rel);
	Unref(rel);
	ArrowLineOvComp* comp = new ArrowLineOvComp(line);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreateEllipseFunc::CreateEllipseFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateEllipseFunc::execute() {
    const int x0 = 0;  
    const int y0 = 1;  
    const int r1 = 2;  
    const int r2 = 3;  
    const int n = 4;
    int args[n];
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) ||  vect.array_len() != n) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<n && !avl->Done(i); j++) {
        args[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (args[r1] > 0 && args[r2] > 0) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	SF_Ellipse* ellipse = new SF_Ellipse(args[x0], args[y0], args[r1], args[r2], stdgraphic);

	if (brVar != nil) ellipse->SetBrush(brVar->GetBrush());
	if (patVar != nil) ellipse->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    ellipse->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	ellipse->SetTransformer(rel);
	Unref(rel);
	EllipseOvComp* comp = new EllipseOvComp(ellipse);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

// this one needs to get the string value, plus x,y location

CreateTextFunc::CreateTextFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateTextFunc::execute() {
    const int x0 = 0;  
    const int y0 = 1;  
    const int n = 2;
    int args[n];
    ComValue& vect = stack_arg(0);
    ComValue& txtv = stack_arg(1);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len() != n) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<n && !avl->Done(i); j++) {
        args[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }

    char* txt = symbol_pntr( txtv.symbol_ref() );
    reset_stack();
   
    PasteCmd* cmd = nil;
    
    if (txt) {
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");
	FontVar* fntVar = (FontVar*) _ed->GetState("FontVar");
	
        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}
	
	TextGraphic* text = new TextGraphic(txt, stdgraphic);

	if (colVar != nil) {
	    text->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	if (fntVar != nil) text->SetFont(fntVar->GetFont());
	text->SetTransformer(rel);
	Unref(rel);
	text->Translate(args[x0], args[y0]);
	TextOvComp* comp = new TextOvComp(text);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else
        push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreateMultiLineFunc::CreateMultiLineFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateMultiLineFunc::execute() {
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len()==0) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    const int len = vect.array_len();
    const int npts = len/2;
    int x[npts];
    int y[npts];
    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<npts && !avl->Done(i); j++) {
        x[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
        y[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowMultiLine* multiline = new ArrowMultiLine(x, y, npts, aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) multiline->SetBrush(brVar->GetBrush());
	if (patVar != nil) multiline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    multiline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	multiline->SetTransformer(rel);
	Unref(rel);
	ArrowMultiLineOvComp* comp = new ArrowMultiLineOvComp(multiline);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreateOpenSplineFunc::CreateOpenSplineFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateOpenSplineFunc::execute() {
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len()==0) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    const int len = vect.array_len();
    const int npts = len/2;
    int x[npts];
    int y[npts];
    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<npts && !avl->Done(i); j++) {
        x[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
        y[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowOpenBSpline* openspline = new ArrowOpenBSpline(x, y, npts, aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) openspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) openspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    openspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	openspline->SetTransformer(rel);
	Unref(rel);
	ArrowSplineOvComp* comp = new ArrowSplineOvComp(openspline);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreatePolygonFunc::CreatePolygonFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreatePolygonFunc::execute() {
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len()==0) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    const int len = vect.array_len();
    const int npts = len/2;
    int x[npts];
    int y[npts];
    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<npts && !avl->Done(i); j++) {
        x[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
        y[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	SF_Polygon* polygon = new SF_Polygon(x, y, npts, stdgraphic);

	if (brVar != nil) polygon->SetBrush(brVar->GetBrush());
	if (patVar != nil) polygon->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    polygon->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	polygon->SetTransformer(rel);
	Unref(rel);
	PolygonOvComp* comp = new PolygonOvComp(polygon);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

CreateClosedSplineFunc::CreateClosedSplineFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateClosedSplineFunc::execute() {
    ComValue& vect = stack_arg(0);
    if (!vect.is_type(ComValue::ArrayType) || vect.array_len()==0) {
        reset_stack();
	push_stack(ComValue::nullval());
	return;
    }

    const int len = vect.array_len();
    const int npts = len/2;
    int x[npts];
    int y[npts];
    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<npts && !avl->Done(i); j++) {
        x[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
        y[j] = avl->GetAttrVal(i)->int_val();
	avl->Next(i);
    }
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
	if (rel != nil) {
	    rel = new Transformer(rel);
	    rel->Invert();
	}

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	SFH_ClosedBSpline* closedspline = new SFH_ClosedBSpline(x, y, npts, stdgraphic);

	if (brVar != nil) closedspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) closedspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    closedspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	closedspline->SetTransformer(rel);
	Unref(rel);
	ClosedSplineOvComp* comp = new ClosedSplineOvComp(closedspline);
	cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(_compview_id, new ComponentView(comp));
	push_stack(compval);
    } else 
	push_stack(ComValue::nullval());

    execute_log(cmd);
}

/*****************************************************************************/

FontFunc::FontFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FontFunc::execute() {
    ComValue& fnum = stack_arg(0);
    int fn = fnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSFont* font = catalog->ReadFont("font", fn);

    FontCmd* cmd = nil;

    if (font) {
	cmd = new FontCmd(_ed, font);
    }

    execute_log(cmd);
}

/*****************************************************************************/

BrushFunc::BrushFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void BrushFunc::execute() {
    ComValue& bnum = stack_arg(0);
    int bn = bnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSBrush* brush = catalog->ReadBrush("brush", bn);

    BrushCmd* cmd = nil;

    if (brush) {
	cmd = new BrushCmd(_ed, brush);
    }

    execute_log(cmd);
}

/*****************************************************************************/

PatternFunc::PatternFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PatternFunc::execute() {
    ComValue& pnum = stack_arg(0);
    int pn = pnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSPattern* pattern = catalog->ReadPattern("pattern", pn);

    PatternCmd* cmd = nil;

    if (pattern) {
	cmd = new PatternCmd(_ed, pattern);
    }

    execute_log(cmd);
}

/*****************************************************************************/

ColorFunc::ColorFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ColorFunc::execute() {
    ComValue& fgv = stack_arg(0);
    ComValue& bgv = stack_arg(1);
    int fgn = fgv.int_val();
    int bgn = bgv.int_val();
    reset_stack();


    Catalog* catalog = unidraw->GetCatalog();
    PSColor* fgcolor = catalog->ReadColor("fgcolor", fgn);
    PSColor* bgcolor = catalog->ReadColor("bgcolor", bgn);

    ColorCmd* cmd = new ColorCmd(_ed, fgcolor, bgcolor);

    execute_log(cmd);
}

/*****************************************************************************/

SelectFunc::SelectFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void SelectFunc::execute() {
    Selection* s = _ed->GetSelection();
    delete s;
    OverlaySelection* newSel = new OverlaySelection();
    
    Viewer* viewer = _ed->GetViewer();
    AttributeValueList* avl = new AttributeValueList();
    if (nargs()==0) {

      GraphicView* gv = viewer->GetGraphicView();
      Iterator i;
      int count=0;
      for (gv->First(i); !gv->Done(i); gv->Next(i)) {
	GraphicView* subgv = gv->GetView(i);
	newSel->Append(subgv);
	GraphicComp* comp = subgv->GetGraphicComp();
	ComValue* compval = new ComValue(_compview_id, new ComponentView(comp));
	avl->Append(compval);
      }

    } else {

      for (int i=0; i<nargsfixed(); i++) {
        ComValue& obj = stack_arg(i);
	if (obj.obj_type_val() == _compview_id) {
	  ComponentView* comview = (ComponentView*)obj.obj_val();
	  OverlayComp* comp = (OverlayComp*)comview->GetSubject();
	  if (comp) {
	    newSel->Append(comp->FindView(viewer));
	    ComValue* compval = new ComValue(_compview_id, new ComponentView(comp));
	    avl->Append(compval);
	  }
	}
      }
    }

    _ed->SetSelection(newSel);
    newSel->Update();
    unidraw->Update();
    reset_stack();
    ComValue retval(avl);
    push_stack(retval);
}

/*****************************************************************************/

MoveFunc::MoveFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void MoveFunc::execute() {
    ComValue& delxv = stack_arg(0);
    ComValue& delyv = stack_arg(1);
    int delx = delxv.int_val();
    int dely = delyv.int_val();
    reset_stack();


    MoveCmd* cmd = nil;

    if (delx != 0  || dely != 0) {
	cmd = new MoveCmd(_ed, delx, dely);
    }

    execute_log(cmd);
}

/*****************************************************************************/

ScaleFunc::ScaleFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ScaleFunc::execute() {
    ComValue& svx = stack_arg(0);
    ComValue& svy = stack_arg(1);
    double fx = svx.double_val();
    double fy = svy.double_val();
    reset_stack();

    ScaleCmd* cmd = nil;

    if (fx > 0.0  || fy > 0.0) {
	cmd = new ScaleCmd(_ed, fx, fy);
    }

    execute_log(cmd);
}

/*****************************************************************************/

RotateFunc::RotateFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void RotateFunc::execute() {
    ComValue& rfv = stack_arg(0);
    double rf = rfv.double_val();
    reset_stack();

    RotateCmd* cmd = nil;

    cmd = new RotateCmd(_ed, rf);

    execute_log(cmd);
}

/*****************************************************************************/

PanFunc::PanFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanFunc::execute() {
    ComValue& delxv = stack_arg(0);
    ComValue& delyv = stack_arg(1);
    int delx = delxv.int_val();
    int dely = delyv.int_val();
    reset_stack();

    PanCmd* cmd = nil;

    if (delx != 0  || dely != 0) {
	cmd = new PanCmd(_ed, delx, dely);
    }

    execute_log(cmd);
}

/*****************************************************************************/

PanUpSmallFunc::PanUpSmallFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanUpSmallFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, NO_PAN, PLUS_SMALL_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanDownSmallFunc::PanDownSmallFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanDownSmallFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, NO_PAN, MINUS_SMALL_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanLeftSmallFunc::PanLeftSmallFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanLeftSmallFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, MINUS_SMALL_PAN, NO_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanRightSmallFunc::PanRightSmallFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanRightSmallFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, PLUS_SMALL_PAN, NO_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanUpLargeFunc::PanUpLargeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanUpLargeFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, NO_PAN, PLUS_LARGE_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanDownLargeFunc::PanDownLargeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanDownLargeFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, NO_PAN, MINUS_LARGE_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanLeftLargeFunc::PanLeftLargeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanLeftLargeFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, MINUS_LARGE_PAN, NO_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

PanRightLargeFunc::PanRightLargeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PanRightLargeFunc::execute() {
    reset_stack();
    FixedPanCmd* cmd = new FixedPanCmd(_ed, PLUS_LARGE_PAN, NO_PAN);
    execute_log(cmd);
}

/*****************************************************************************/

ZoomFunc::ZoomFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ZoomFunc::execute() {
    ComValue& zoomv = pop_stack();
    double zoom = zoomv.double_val();
    reset_stack();
    
    
    ZoomCmd* cmd = nil;
    
    if (zoom > 0.0) {
	cmd = new ZoomCmd(_ed, zoom);
    }
    
    execute_log(cmd);
}

/*****************************************************************************/

ZoomInFunc::ZoomInFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ZoomInFunc::execute() {
    reset_stack();
    ZoomCmd* cmd = new ZoomCmd(_ed, 2.0);
    execute_log(cmd);
}

/*****************************************************************************/

ZoomOutFunc::ZoomOutFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ZoomOutFunc::execute() {
    reset_stack();
    ZoomCmd* cmd = new ZoomCmd(_ed, 0.5);
    execute_log(cmd);
}


/*****************************************************************************/

#ifndef NDEBUG
#include <iostream.h>
#endif

TileFileFunc::TileFileFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void TileFileFunc::execute() {
    ComValue& ifilev = stack_arg(0);
    ComValue& ofilev = stack_arg(1);
    ComValue five12(512);
    ComValue& twidthv = stack_arg(2, false, five12);
    ComValue& theightv = stack_arg(3, false, five12);
    reset_stack();

    char* ifile = symbol_pntr(ifilev.symbol_ref());
    char* ofile = symbol_pntr(ofilev.symbol_ref());

#ifndef NDEBUG
    cerr << "tilefile args - ifn: " << ifile << "ofn: " << ofile 
        << ", twidth: " << twidthv.int_val() << ", theight: " 
        << theightv.int_val() << "\n";
#endif

    if (
        ifile && ofile &&
        (twidthv.type() == ComValue::IntType) &&
        (theightv.type() == ComValue::IntType)
    ) {
        int twidth = twidthv.int_val(); 
        int theight = theightv.int_val(); 

        Command* cmd = new TileFileCmd(_ed, ifile, ofile, twidth, theight);

        execute_log(cmd);
    }
    else {
	push_stack(ComValue::nullval());
    }
}




