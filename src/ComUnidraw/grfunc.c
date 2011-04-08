/*
 * Copyright (c) 2001-2007 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
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
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovtext.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/selection.h>
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
#include <IV-2_6/InterViews/world.h>
#include <IV-X11/Xlib.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xfont.h>
#include <X11/Xatom.h>

#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

#define TITLE "GrFunc"

/*****************************************************************************/

CreateGraphicFunc::CreateGraphicFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

Transformer* CreateGraphicFunc::get_transformer(AttributeList* al) {
  static int transform_symid = symbol_add("transform");

  AttributeValue* transformv = nil;
  Transformer* rel = nil;
  AttributeValueList* avl = nil;
  if (al && 
      (transformv=al->find(transform_symid)) && 
      transformv->is_array() && 
      (avl=transformv->array_val()) &&
      avl->Number()==6) {
    float a00, a01, a10, a11, a20, a21;
    Iterator it;
    avl->First(it); a00=avl->GetAttrVal(it)->float_val();
    avl->Next(it); a01=avl->GetAttrVal(it)->float_val();
    avl->Next(it); a10=avl->GetAttrVal(it)->float_val();
    avl->Next(it); a11=avl->GetAttrVal(it)->float_val();
    avl->Next(it); a20=avl->GetAttrVal(it)->float_val();
    avl->Next(it); a21=avl->GetAttrVal(it)->float_val();
    rel = new Transformer(a00, a01, a10, a11, a20, a21);
  } else {
    rel = ((OverlayViewer*)_ed->GetViewer())->GetRel();
    if (rel != nil) {
      rel = new Transformer(rel);
      rel->Invert();
    }
  }
  return rel;
  
}

/*****************************************************************************/

CreateRectFunc::CreateRectFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (coords[x0] != coords[x1] || coords[y0] != coords[y1]) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	SF_Rect* rect = new SF_Rect(coords[x0], coords[y0], coords[x1], coords[y1], stdgraphic);

	if (brVar != nil) rect->SetBrush(brVar->GetBrush());
	if (patVar != nil) rect->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    rect->FillBg(!colVar->GetBgColor()->None());
	    rect->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	rect->SetTransformer(rel);
	Unref(rel);
	RectOvComp* comp = new RectOvComp(rect);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("RectComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateLineFunc::CreateLineFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (coords[x0] != coords[x1] || coords[y0] != coords[y1]) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowLine* line = new ArrowLine(coords[x0], coords[y0], coords[x1], coords[y1], aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) line->SetBrush(brVar->GetBrush());

	if (colVar != nil) {
	    line->FillBg(!colVar->GetBgColor()->None());
	    line->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	line->SetTransformer(rel);
	Unref(rel);
	ArrowLineOvComp* comp = new ArrowLineOvComp(line);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("ArrowLineComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateEllipseFunc::CreateEllipseFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (args[r1] > 0 && args[r2] > 0) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

	Transformer* rel = get_transformer(al);
	
	SF_Ellipse* ellipse = new SF_Ellipse(args[x0], args[y0], args[r1], args[r2], stdgraphic);

	if (brVar != nil) ellipse->SetBrush(brVar->GetBrush());
	if (patVar != nil) ellipse->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    ellipse->FillBg(!colVar->GetBgColor()->None());
	    ellipse->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	ellipse->SetTransformer(rel);
	Unref(rel);
	EllipseOvComp* comp = new EllipseOvComp(ellipse);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("EllipseComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

// this one needs to get the string value, plus x,y location

CreateTextFunc::CreateTextFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();
   
    PasteCmd* cmd = nil;
    
    if (txt) {
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");
	FontVar* fntVar = (FontVar*) _ed->GetState("FontVar");
	
        Transformer* rel = get_transformer(al);
	
	TextGraphic* text = new TextGraphic(txt, stdgraphic);

	if (colVar != nil) {
	    text->FillBg(!colVar->GetBgColor()->None());
	    text->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	if (fntVar != nil) text->SetFont(fntVar->GetFont());
	text->SetTransformer(new Transformer());
	text->Translate(args[x0], args[y0]);
	text->GetTransformer()->postmultiply(rel);
	Unref(rel);
	TextOvComp* comp = new TextOvComp(text);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("TextComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else
        push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateMultiLineFunc::CreateMultiLineFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowMultiLine* multiline = new ArrowMultiLine(x, y, npts, aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) multiline->SetBrush(brVar->GetBrush());
	if (patVar != nil) multiline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    multiline->FillBg(!colVar->GetBgColor()->None());
	    multiline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	multiline->SetTransformer(rel);
	Unref(rel);
	ArrowMultiLineOvComp* comp = new ArrowMultiLineOvComp(multiline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("ArrowMultiLineComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateOpenSplineFunc::CreateOpenSplineFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	ArrowOpenBSpline* openspline = new ArrowOpenBSpline(x, y, npts, aVar->Head(), aVar->Tail(), 
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) openspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) openspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    openspline->FillBg(!colVar->GetBgColor()->None());
	    openspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	openspline->SetTransformer(rel);
	Unref(rel);
	ArrowSplineOvComp* comp = new ArrowSplineOvComp(openspline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("ArrowSplineComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreatePolygonFunc::CreatePolygonFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	SF_Polygon* polygon = new SF_Polygon(x, y, npts, stdgraphic);

	if (brVar != nil) polygon->SetBrush(brVar->GetBrush());
	if (patVar != nil) polygon->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    polygon->FillBg(!colVar->GetBgColor()->None());
	    polygon->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	polygon->SetTransformer(rel);
	Unref(rel);
	PolygonOvComp* comp = new PolygonOvComp(polygon);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("PolygonComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateClosedSplineFunc::CreateClosedSplineFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (npts) {
	BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
	PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
	ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");

        Transformer* rel = get_transformer(al);

	ArrowVar* aVar = (ArrowVar*) _ed->GetState("ArrowVar");
	SFH_ClosedBSpline* closedspline = new SFH_ClosedBSpline(x, y, npts, stdgraphic);

	if (brVar != nil) closedspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) closedspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    closedspline->FillBg(!colVar->GetBgColor()->None());
	    closedspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	closedspline->SetTransformer(rel);
	Unref(rel);
	ClosedSplineOvComp* comp = new ClosedSplineOvComp(closedspline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("ClosedSplineComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

CreateRasterFunc::CreateRasterFunc(ComTerp* comterp, Editor* ed) : CreateGraphicFunc(comterp, ed) {
}

void CreateRasterFunc::execute() {
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

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (coords[x0] != coords[x1] || coords[y0] != coords[y1]) {

	float dcoords[n];
	((OverlayViewer*)GetEditor()->GetViewer())->ScreenToDrawing
	  (coords[x0], coords[y0], dcoords[x0], dcoords[y0]);
	((OverlayViewer*)GetEditor()->GetViewer())->ScreenToDrawing
	  (coords[x1], coords[y1], dcoords[x1], dcoords[y1]);
	
	OverlayRaster* raster = 
	  new OverlayRaster((int)(dcoords[x1]-dcoords[x0]+1), 
			    (int)(dcoords[y1]-dcoords[y0]+1), 
			    2 /* initialize with border of 2 */);

	OverlayRasterRect* rasterrect = new OverlayRasterRect(raster, stdgraphic);

#if 1
	Transformer* t = new Transformer();
	t->Translate(dcoords[x0], dcoords[y0]);
	rasterrect->SetTransformer(t);
	Unref(t);
#else
        Transformer* rel = get_transformer(al);
#endif

	RasterOvComp* comp = new RasterOvComp(rasterrect);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(symbol_add("RasterComp"), new ComponentView(comp));
	compval.object_compview(true);
	push_stack(compval);
	execute_log(cmd);
    } else 
	push_stack(ComValue::nullval());

    Unref(al);
}

/*****************************************************************************/

FontFunc::FontFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FontFunc::execute() {
    ComValue fnum(stack_arg(0));
    int fn = fnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSFont* font = catalog->ReadFont("font", fn);

    FontCmd* cmd = nil;

    if (font) {
	cmd = new FontCmd(_ed, font);
	execute_log(cmd);
    }

}

/*****************************************************************************/
FontByNameFunc::FontByNameFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

static char  *psfonttoxfont(char* f)
{
  /* convert a PS name to a X... */
  char type[10];
  int size=0;
  static char copy[256];
  static char name[256];
  static char *wght[] = { "bold","demi","light","demibold","book",0 };
  char *s;
  
  if (*f=='-')
    return f;
  
  strcpy(copy,f);
  s = copy;
  while (*s) {
    *s = tolower(*s);
    s++;
  }
  f = copy+strlen(copy);
  
  s = strchr(copy,'-');
  if (!s) {
    strcpy(type,"medium-r");
  } else {
    *s=0;
    s++;
    for (size=0;wght[size];size++)
      if (!strncmp(s,wght[size],strlen(wght[size]))) {
	strcpy(type,wght[size]);
	strcat(type,"-");
	s+=strlen(wght[size]);
	break;
      }
    if (!wght[size])
      strcpy(type,"medium-");
    switch (*s) {
    case 'i':
      strcat(type,"i");
      break;
    case 'o':
      strcat(type,"o");
      break;
    default:
      strcat(type,"r");
      break;
    }
  }
  
  size = 11;
  while (f[-1]>='0' && f[-1]<='9')
    f--;
  
  if (*f)
    size = atoi(f);
  f[0] = 0;
  if (f[-1]=='-')
    f[-1] = 0;
  sprintf(name,"-*-%s-%s-normal-*-%d-*",
	  copy, type, size );
  return name;
}
 /*****************************************************************************/
void FontByNameFunc::execute() {
  ComValue& fontarg = stack_arg(0);
  const char*  fontval = fontarg.string_ptr();
  reset_stack();
  
  char* fontvaldup=strdup(fontval);
  Catalog* catalog = unidraw->GetCatalog();
  XDisplay* dpy =World::current()->display()->rep()->display_;
  XFontStruct* xfs = XLoadQueryFont(dpy, fontvaldup);
  PSFont* font = nil;
  
  if (!xfs){
    char* xfontval=psfonttoxfont(fontvaldup);
    fontvaldup = strdup(xfontval);
    xfs = XLoadQueryFont(dpy,xfontval);
    if (!xfs){
      fprintf(stderr, "Can not load font:  %s, \n", fontval);
      fprintf(stderr, "Keeping the current font.\n");
    }
  }
  if (xfs){
    unsigned long value;
    char fontname[CHARBUFSIZE];
    char fontsizeptr[CHARBUFSIZE];
    char fontfullname[CHARBUFSIZE];
    
    XGetFontProperty(xfs, XA_FULL_NAME, &value);
    strcpy(fontfullname, XGetAtomName(dpy, (Atom)value));
    
    XGetFontProperty(xfs, XA_FONT_NAME, &value);
    strcpy(fontname, XGetAtomName(dpy, (Atom)value));
    
    XGetFontProperty(xfs,XA_POINT_SIZE, &value);
    sprintf(fontsizeptr,"%d",(unsigned int)(value/10));
    
    font = catalog->FindFont(fontvaldup,fontname,fontsizeptr);
    delete fontvaldup;
  }
  FontCmd* cmd = nil;
  
  if (font) {
    cmd = new FontCmd(_ed, font);
    execute_log(cmd);
  }
  
}
/*****************************************************************************/
ColorRgbFunc::ColorRgbFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
 }

void ColorRgbFunc::execute() {
  ComValue& fgarg = stack_arg(0);
  ComValue& bgarg = stack_arg(1);
  const char* fgname = fgarg.string_ptr();
  const char* bgname = bgarg.string_ptr();
  reset_stack();
  PSColor* fgcolor=nil;
  PSColor* bgcolor=nil;
  Catalog* catalog = unidraw->GetCatalog();
  fgcolor = catalog->FindColor(fgname);
  //This comparison is made because the user can set only the foreground color by calling
  //colorsrgb with one argument.
  if (bgname && strcmp(bgname,"sym")!=0){
    bgcolor = catalog->FindColor(bgname);
  }
  ColorCmd* cmd = new ColorCmd(_ed, fgcolor, bgcolor);
  execute_log(cmd);
}
/*****************************************************************************/

BrushFunc::BrushFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void BrushFunc::execute() {
    ComValue& bnum =stack_arg(0);
    int bn = bnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSBrush* brush = catalog->ReadBrush("brush", bn);

    BrushCmd* cmd = nil;

    if (brush) {
	cmd = new BrushCmd(_ed, brush);
	execute_log(cmd);
    }

}

/*****************************************************************************/

PatternFunc::PatternFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PatternFunc::execute() {
    ComValue pnum(stack_arg(0));
    int pn = pnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSPattern* pattern = catalog->ReadPattern("pattern", pn);

    PatternCmd* cmd = nil;

    if (pattern) {
	cmd = new PatternCmd(_ed, pattern);
	execute_log(cmd);
    }

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
    static int all_symid = symbol_add("all");
    ComValue all_flagv(stack_key(all_symid));
    boolean all_flag = all_flagv.is_true();
    static int clear_symid = symbol_add("clear");
    ComValue clear_flagv(stack_key(clear_symid));
    boolean clear_flag = clear_flagv.is_true();

    Selection* sel = _ed->GetViewer()->GetSelection();
    if (clear_flag) {
      sel->Clear();
      reset_stack();
      return;
    }
      
    OverlaySelection* newSel = ((OverlayEditor*)_ed)->overlay_kit()->MakeSelection();
    
    Viewer* viewer = _ed->GetViewer();
    AttributeValueList* avl = new AttributeValueList();
    if (all_flag) {

      GraphicView* gv = ((OverlayEditor*)_ed)->GetFrame();
      Iterator i;
      int count=0;
      for (gv->First(i); !gv->Done(i); gv->Next(i)) {
	GraphicView* subgv = gv->GetView(i);
	newSel->Append(subgv);
	OverlayComp* comp = (OverlayComp*)subgv->GetGraphicComp();
	ComValue* compval = new ComValue(comp->classid(), new ComponentView(comp));
	compval->object_compview(true);
	avl->Append(compval);
      }

    } else if (nargs()==0) {
      Iterator i;
      int count=0;
      for (sel->First(i); !sel->Done(i); sel->Next(i)) {
	GraphicView* grview = sel->GetView(i);
	OverlayComp* comp = grview ? (OverlayComp*)grview->GetSubject() : nil;
	ComValue* compval = comp ? new ComValue(comp->classid(), new ComponentView(comp)) : nil;

	if (compval) {
	  compval->object_compview(true);
	  avl->Append(compval);
	}
	delete newSel;
        newSel = nil;
      }

    } else {

      for (int i=0; i<nargsfixed(); i++) {
        ComValue& obj = stack_arg(i);
	if (obj.object_compview()) {
	  ComponentView* comview = (ComponentView*)obj.obj_val();
	  OverlayComp* comp = (OverlayComp*)comview->GetSubject();
	  if (comp) {
	    newSel->Append(comp->FindView(viewer));
	    ComValue* compval = new ComValue(comp->classid(), new ComponentView(comp));
	    compval->object_compview(true);
	    avl->Append(compval);
	  }
	}
      }
    }

    if (newSel){
      sel->Clear();
      delete sel;
      _ed->SetSelection(newSel);
      newSel->Update(viewer);
      unidraw->Update();
    }
    reset_stack();
    ComValue retval(avl);
    push_stack(retval);
}

/*****************************************************************************/

DeleteFunc::DeleteFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void DeleteFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  int nf=nargsfixed();
  if (nf==0) {
    reset_stack();
    return;
  }

  Clipboard* delcb = new Clipboard();

  for (int i=0; i<nf; i++) {
    ComValue& obj = stack_arg(i);
    if (obj.object_compview()) {
      ComponentView* comview = (ComponentView*)obj.obj_val();
      OverlayComp* comp = (OverlayComp*)comview->GetSubject();
      if (comp) delcb->Append(comp);
    }
  }

  DeleteCmd* delcmd = new DeleteCmd(GetEditor(), delcb);
  delcmd->Execute();
  unidraw->Update();
  delete delcmd;

  reset_stack();
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
	execute_log(cmd);
    }

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
	execute_log(cmd);
    }

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
	execute_log(cmd);
    }

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
    ComValue zoomv(pop_stack());
    double zoom = zoomv.double_val();
    reset_stack();
    
    
    ZoomCmd* cmd = nil;
    
    if (zoom > 0.0) {
	cmd = new ZoomCmd(_ed, zoom);
	execute_log(cmd);
    }
    
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
    ComValue ifilev(stack_arg(0));
    ComValue ofilev(stack_arg(1));
    ComValue five12(512);
    ComValue twidthv(stack_arg(2, false, five12));
    ComValue theightv(stack_arg(3, false, five12));
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

/*****************************************************************************/

TransformerFunc::TransformerFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void TransformerFunc::execute() {
    
    ComValue objv(stack_arg(0));
    ComValue transv(stack_arg(0));
    reset_stack();
    if (objv.object_compview()) {
      ComponentView* compview = (ComponentView*)objv.obj_val();
      if (compview && compview->GetSubject()) {
	OverlayComp* comp = (OverlayComp*)compview->GetSubject();
	Graphic* gr = comp->GetGraphic();
	if (gr) {
	  Transformer* trans = gr->GetTransformer();
	  if (transv.is_unknown() || !transv.is_array() || transv.array_val()->Number()!=6) {
	    AttributeValueList* avl = new AttributeValueList();
	    float a00, a01, a10, a11, a20, a21;
	    trans->matrix(a00, a01, a10, a11, a20, a21);
	    avl->Append(new AttributeValue(a00));
	    avl->Append(new AttributeValue(a01));
	    avl->Append(new AttributeValue(a10));
	    avl->Append(new AttributeValue(a11));
	    avl->Append(new AttributeValue(a20));
	    avl->Append(new AttributeValue(a21));
	    ComValue retval(avl);
	    push_stack(retval);

	  } else {
	    float a00, a01, a10, a11, a20, a21;
	    AttributeValueList* avl = transv.array_val();
	    Iterator it;
	    AttributeValue* av;

	    avl->First(it);
	    av = avl->GetAttrVal(it);
	    a00 = av->float_val();
	    avl->Next(it);
	    av = avl->GetAttrVal(it);
	    a01 = av->float_val();
	    avl->Next(it);
	    av = avl->GetAttrVal(it);
	    a10 = av->float_val();
	    avl->Next(it);
	    av = avl->GetAttrVal(it);
	    a11 = av->float_val();
	    avl->Next(it);
	    av = avl->GetAttrVal(it);
	    a20 = av->float_val();
	    avl->Next(it);
	    av = avl->GetAttrVal(it);
	    a21 = av->float_val();

	    Transformer t(a00, a01, a10, a11, a20, a21);
	    *gr->GetTransformer()=t;

	    ComValue compval(comp->class_symid(), new ComponentView(comp));
	    compval.object_compview(true);
	    push_stack(compval);
	  }
	}
      } 	
    }
}

