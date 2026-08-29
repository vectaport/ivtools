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
#include <OverlayUnidraw/ovcmds.h>
#include <vector>

#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovkit.h>
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
#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>

#include <sstream>
#include <string>

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

/* remove an attribute by symbol id; Remove(Attribute*) unrefs it for us */
static void remove_key(AttributeList* al, int symid) {
    Attribute* a = al->GetAttr(symid);
    if (a) al->Remove(a);
}

/* build a PSColor from a [name,r,g,b] keyword array; FindColor resolves by name
   first (catalog, then X11 -- which parses "#RRGGBB"), falling back to the rgb
   intensities (0..1, scaled to 0..0xffff as FindColor expects) */
static PSColor* color_from_attrval(Catalog* catalog, AttributeValue* v) {
    if (!v || !v->is_array()) return nil;
    AttributeValueList* avl = v->array_val();
    if (!avl) return nil;
    Iterator it; avl->First(it);
    if (avl->Done(it)) return nil;
    const char* name = avl->GetAttrVal(it)->string_ptr();
    int ir=0, ig=0, ib=0;
    if (avl->Number() >= 4) {
	avl->Next(it); ir = (int)(avl->GetAttrVal(it)->float_val()*0xffff);
	avl->Next(it); ig = (int)(avl->GetAttrVal(it)->float_val()*0xffff);
	avl->Next(it); ib = (int)(avl->GetAttrVal(it)->float_val()*0xffff);
    }
    return name ? catalog->FindColor(name, ir, ig, ib) : nil;
}

/* build a PSFont from the [name,printfont,printsize] triple OverlayScript::Font
   writes.  The print size goes out unquoted, so it comes back as a number
   rather than a string; FindFont wants text either way.  A bare name (no
   commas) is accepted too -- FindFont defaults the other two. */
static PSFont* font_from_attrval(Catalog* catalog, AttributeValue* v) {
    if (!v) return nil;

    if (v->is_string()) {
	const char* name = v->string_ptr();
	return name ? catalog->FindFont(name) : nil;
    }
    if (!v->is_array()) return nil;

    AttributeValueList* avl = v->array_val();
    if (!avl) return nil;
    Iterator it; avl->First(it);
    if (avl->Done(it)) return nil;
    const char* name = avl->GetAttrVal(it)->string_ptr();
    if (!name) return nil;

    const char* pf = "";
    const char* ps = "";
    char sizebuf[32];
    if (avl->Number() >= 2) {
	avl->Next(it);
	const char* p = avl->GetAttrVal(it)->string_ptr();
	if (p) pf = p;
    }
    if (avl->Number() >= 3) {
	avl->Next(it);
	AttributeValue* sv = avl->GetAttrVal(it);
	if (sv->is_string()) {
	    const char* s = sv->string_ptr();
	    if (s) ps = s;
	} else {
	    snprintf(sizebuf, sizeof(sizebuf), "%d", sv->int_val());
	    ps = sizebuf;
	}
    }
    return catalog->FindFont(name, pf, ps);
}

void CreateGraphicFunc::set_graphic_gs(AttributeList* al, Graphic* gr) {
    if (!al || !gr) return;
    Catalog* catalog = unidraw->GetCatalog();

    static int brush_sym   = symbol_add("brush");
    static int nonebr_sym  = symbol_add("nonebr");
    static int fgcolor_sym = symbol_add("fgcolor");
    static int bgcolor_sym = symbol_add("bgcolor");
    static int fillbg_sym  = symbol_add("fillbg");
    static int pattern_sym = symbol_add("pattern");
    static int graypat_sym = symbol_add("graypat");
    static int nonepat_sym = symbol_add("nonepat");
    static int font_sym     = symbol_add("font");

    AttributeValue* v;

    /* brush: :nonebr  or  :brush linepat,width */
    if (al->find(nonebr_sym)) {
	gr->SetBrush(new PSBrush());
	remove_key(al, nonebr_sym);
    } else if ((v = al->find(brush_sym)) && v->is_array()) {
	AttributeValueList* avl = v->array_val();
	if (avl && avl->Number() >= 2) {
	    Iterator it; avl->First(it);
	    int linepat = avl->GetAttrVal(it)->int_val();
	    avl->Next(it);
	    float width = avl->GetAttrVal(it)->float_val();
	    gr->SetBrush(new PSBrush(linepat, width));
	}
	remove_key(al, brush_sym);
    }

    /* colors: :fgcolor name,r,g,b  :bgcolor name,r,g,b -- fall back to the
       graphic's existing color whenever a key is absent OR its lookup fails, so a
       failed fg lookup doesn't also drop a good bg (or vice versa) */
    AttributeValue* fgv = al->find(fgcolor_sym);
    AttributeValue* bgv = al->find(bgcolor_sym);
    if (fgv || bgv) {
	PSColor* fg_resolved = fgv ? color_from_attrval(catalog, fgv) : nil;
	PSColor* bg_resolved = bgv ? color_from_attrval(catalog, bgv) : nil;
	PSColor* fg = fg_resolved ? fg_resolved : gr->GetFgColor();
	PSColor* bg = bg_resolved ? bg_resolved : gr->GetBgColor();
	if (fg && bg) gr->SetColors(fg, bg);
	remove_key(al, fgcolor_sym);
	remove_key(al, bgcolor_sym);
    }

    /* font: :font "name","printfont",printsize -- keep the graphic's existing
       font when the key is absent, or when the literal is malformed and
       font_from_attrval hands back nil.  A well-formed literal always yields a
       font: Catalog::FindFont substitutes "fixed" for a name this display does
       not have, which is what the rest of Unidraw does.  Note that is not the
       colors' behavior -- FindColor keeps the requested name and defaults only
       the rgb, so a color survives the trip and a missing font does not. */
    if ((v = al->find(font_sym))) {
	PSFont* font = font_from_attrval(catalog, v);
	if (font) gr->SetFont(font);
	remove_key(al, font_sym);
    }

    /* :fillbg flag */
    if ((v = al->find(fillbg_sym))) {
	gr->FillBg(v->int_val());
	remove_key(al, fillbg_sym);
    }

    /* :transform was already consumed by get_transformer and applied to the
       graphic -- strip it here too so it doesn't re-serialize on export */
    remove_key(al, symbol_add("transform"));

    /* pattern: :nonepat  or  :graypat level  or  :pattern bits... */
    if (al->find(nonepat_sym)) {
	gr->SetPattern(new PSPattern());
	remove_key(al, nonepat_sym);
    } else if ((v = al->find(graypat_sym))) {
	gr->SetPattern(catalog->FindGrayLevel(v->float_val()));
	remove_key(al, graypat_sym);
    } else if ((v = al->find(pattern_sym)) && v->is_array()) {
	AttributeValueList* avl = v->array_val();
	if (avl && avl->Number()==16) {
	    int mask[16]; int i=0; Iterator it;
	    for (avl->First(it); !avl->Done(it) && i<16; avl->Next(it))
		mask[i++] = avl->GetAttrVal(it)->int_val();
	    gr->SetPattern(new PSPattern(mask, 16));
	} else if (avl && avl->Number()>=1) {
	    Iterator it; avl->First(it);
	    gr->SetPattern(new PSPattern(avl->GetAttrVal(it)->int_val(), -1));
	}
	remove_key(al, pattern_sym);
    }
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
	/* command-supplied gs keywords win over editor state, and are stripped
	   from al so they round-trip via the graphic, not as leftover attributes */
	set_graphic_gs(al, rect);
	RectOvComp* comp = new RectOvComp(rect);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("RectComp"));
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
	set_graphic_gs(al, line);
	ArrowLineOvComp* comp = new ArrowLineOvComp(line);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("ArrowLineComp"));
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
	set_graphic_gs(al, ellipse);
	EllipseOvComp* comp = new EllipseOvComp(ellipse);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval( new OverlayViewRef(comp), symbol_add("EllipseComp"));
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

    /* Two shapes arrive here.  The command form is text(x0,y0 "str"), which is
       what a person writes.  The serialized form is text(lineheight,"str") --
       what export() emits, what a saved drawing holds, and what DrawServ sends
       across a link -- where the position rides in :transform rather than in
       coordinates, and the string is folded into the first array so nothing is
       left in argument 1.  Read as the command form it was, that string landed
       in the y coordinate and the text came out empty, so a text graphic did
       not survive being exported and re-created, nor reach a far node intact.

       They are told apart by whether a text argument was supplied at all. */
    /* the serialized form supplies no text argument, so nothing string-like
       is sitting in argument 1.  (nargs() does not distinguish the two: it
       reports 2 either way.) */
    boolean serialized_form = !txtv.is_string();

    const char* txt = nil;
    args[x0] = args[y0] = 0;

    if (serialized_form) {
	avl->First(i);
	if (!avl->Done(i)) avl->Next(i);      /* step over the line height */
	if (!avl->Done(i)) {
	    AttributeValue* sv = avl->GetAttrVal(i);
	    txt = sv->is_string() ? symbol_pntr(sv->symbol_val()) : nil;
	}

    } else {
	avl->First(i);
	for (int j=0; j<n && !avl->Done(i); j++) {
	    args[j] = avl->GetAttrVal(i)->int_val();
	    avl->Next(i);
	}
	txt = symbol_pntr( txtv.symbol_ref() );
    }

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
	/* command-supplied gs keywords win over editor state, and are stripped
	   from al so they round-trip via the graphic, not as leftover attributes.
	   Text is where :font actually appears -- TextGS is the only gs emitter
	   that writes one for a graphic any create command builds. */
	set_graphic_gs(al, text);

	/* TextScript::Definition writes a transform corrected by lineHeight-1,
	   to account for the vertical shift between where a text graphic sits
	   and where its baseline is.  TextOvComp's reading constructor takes
	   that correction back out when a saved drawing is loaded; reaching the
	   same serialized form through this command has to do the same, or the
	   text creeps down the canvas by a line every time it is exported and
	   re-created -- and once per hop when it crosses a link. */
	if (serialized_form) {
	    /* the emitter corrects by the GRAPHIC's line height, not the
	       font's, so the inverse has to use the same one */
	    float sep = 1 - text->GetLineHeight();
	    Transformer* tt = text->GetTransformer();
	    float dx = 0., dy = sep;
	    if (tt != nil) {
		float xa, ya, xb, yb;
		tt->Transform(0., 0., xa, ya);
		tt->Transform(0., sep, xb, yb);
		dx = xb - xa;
		dy = yb - ya;
	    }
	    text->Translate(dx, dy);
	}

	TextOvComp* comp = new TextOvComp(text);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("TextComp"));
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
    std::vector<int> x(npts);
    std::vector<int> y(npts);
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
	ArrowMultiLine* multiline = new ArrowMultiLine(&x[0], &y[0], npts, aVar->Head(), aVar->Tail(),
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) multiline->SetBrush(brVar->GetBrush());
	if (patVar != nil) multiline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    multiline->FillBg(!colVar->GetBgColor()->None());
	    multiline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	multiline->SetTransformer(rel);
	Unref(rel);
	set_graphic_gs(al, multiline);
	ArrowMultiLineOvComp* comp = new ArrowMultiLineOvComp(multiline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("ArrowMultiLineComp"));
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
    std::vector<int> x(npts);
    std::vector<int> y(npts);
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
	ArrowOpenBSpline* openspline = new ArrowOpenBSpline(&x[0], &y[0], npts, aVar->Head(), aVar->Tail(),
					_ed->GetViewer()->GetMagnification(), stdgraphic);

	if (brVar != nil) openspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) openspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    openspline->FillBg(!colVar->GetBgColor()->None());
	    openspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	openspline->SetTransformer(rel);
	Unref(rel);
	set_graphic_gs(al, openspline);
	ArrowSplineOvComp* comp = new ArrowSplineOvComp(openspline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("ArrowSplineComp"));
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
    std::vector<int> x(npts);
    std::vector<int> y(npts);
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

	SF_Polygon* polygon = new SF_Polygon(&x[0], &y[0], npts, stdgraphic);

	if (brVar != nil) polygon->SetBrush(brVar->GetBrush());
	if (patVar != nil) polygon->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    polygon->FillBg(!colVar->GetBgColor()->None());
	    polygon->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	polygon->SetTransformer(rel);
	Unref(rel);
	set_graphic_gs(al, polygon);
	PolygonOvComp* comp = new PolygonOvComp(polygon);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("PolygonComp"));
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
    std::vector<int> x(npts);
    std::vector<int> y(npts);
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
	SFH_ClosedBSpline* closedspline = new SFH_ClosedBSpline(&x[0], &y[0], npts, stdgraphic);

	if (brVar != nil) closedspline->SetBrush(brVar->GetBrush());
	if (patVar != nil) closedspline->SetPattern(patVar->GetPattern());

	if (colVar != nil) {
	    closedspline->FillBg(!colVar->GetBgColor()->None());
	    closedspline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
	closedspline->SetTransformer(rel);
	Unref(rel);
	set_graphic_gs(al, closedspline);
	ClosedSplineOvComp* comp = new ClosedSplineOvComp(closedspline);
	comp->SetAttributeList(al);
	if (PasteModeFunc::paste_mode()==0)
	  cmd = new PasteCmd(_ed, new Clipboard(comp));
	ComValue compval(new OverlayViewRef(comp), symbol_add("ClosedSplineComp"));
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
    ComValue vect = stack_arg(0);
    ComValue rgbv(stack_key(symbol_add("rgb")));

    AttributeList* al = stack_keys();
    Resource::ref(al);
    reset_stack();

    PasteCmd* cmd = nil;

    if (rgbv.is_type(ComValue::ArrayType)) {
      
      RasterOvComp* comp = create_from_rgb(rgbv, al);
      if (PasteModeFunc::paste_mode() == 0)
        cmd = new PasteCmd(_ed, new Clipboard(comp));
      ComValue compval(new OverlayViewRef(comp), symbol_add("RasterComp"));
      push_stack(compval);
      execute_log(cmd);
      Unref(al);
      return;
    }

    if (!vect.is_type(ComValue::ArrayType) || vect.array_len() != n) {
      reset_stack();
      push_stack(ComValue::nullval());
      Unref(al);
      return;
    }
    ALIterator i;
    AttributeValueList* avl = vect.array_val();
    avl->First(i);
    for (int j=0; j<n && !avl->Done(i); j++) {
      coords[j] = avl->GetAttrVal(i)->int_val();
      avl->Next(i);
    }
    
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
      
      Transformer* t = new Transformer();
      t->Translate(dcoords[x0], dcoords[y0]);
      rasterrect->SetTransformer(t);
      Unref(t);
      /* the screen coords imply the translate above, so the viewer-relative
         transformer the other create commands start from would double it --
         but an explicit :transform off a re-created command still has to win */
      if (al && al->find(symbol_add("transform"))) {
	Transformer* rel = get_transformer(al);
	rasterrect->SetTransformer(rel);
	Unref(rel);
      }
      set_graphic_gs(al, rasterrect);
      
      RasterOvComp* comp = new RasterOvComp(rasterrect);
      comp->SetAttributeList(al);
      if (PasteModeFunc::paste_mode()==0)
	cmd = new PasteCmd(_ed, new Clipboard(comp));
      ComValue compval(new OverlayViewRef(comp), symbol_add("RasterComp"));
      push_stack(compval);
      execute_log(cmd);
    } else 
      push_stack(ComValue::nullval());
    
    Unref(al);
}

RasterOvComp* CreateRasterFunc::create_from_rgb(ComValue& rgbv, AttributeList* al) {
    AttributeValueList* avl = rgbv.array_val();
    ALIterator i;
    avl->First(i);

    int w = avl->GetAttrVal(i)->int_val(); avl->Next(i);
    int h = avl->GetAttrVal(i)->int_val(); avl->Next(i);
    int npix = w*h;
    int nval = avl->Number()-2;

    OverlayRaster* raster = new OverlayRaster(w, h, 0);
    OverlayRasterRect* rasterrect = new OverlayRasterRect(raster, stdgraphic);

    /* pixel data comes in one of three forms, told apart purely by count
       against w*h: flat r,g,b (three scalars per pixel), nested (r,g,b)
       triples (one array per pixel), or legacy packed 0xRRGGBB ints (one
       scalar per pixel) -- see doc/APPENDIX-B-COMTERP-EXAMPLES.md */
    if (nval == npix*3) {
      for (int row = 0; row < h && !avl->Done(i); row++) {
        for (int col = 0; col < w && !avl->Done(i); col++) {
          float r = avl->GetAttrVal(i)->int_val()/255.; avl->Next(i);
          float g = avl->GetAttrVal(i)->int_val()/255.; avl->Next(i);
          float b = avl->GetAttrVal(i)->int_val()/255.; avl->Next(i);
          raster->poke(col, row, r, g, b, 1.0);
        }
      }
    } else {
      for (int row = 0; row < h && !avl->Done(i); row++) {
        for (int col = 0; col < w && !avl->Done(i); col++) {
          AttributeValue* elem = avl->GetAttrVal(i); avl->Next(i);
          float r, g, b;
          if (elem->is_array()) {
            AttributeValueList* rgb = elem->array_val();
            ALIterator j;
            rgb->First(j);
            r = rgb->GetAttrVal(j)->int_val()/255.; rgb->Next(j);
            g = rgb->GetAttrVal(j)->int_val()/255.; rgb->Next(j);
            b = rgb->GetAttrVal(j)->int_val()/255.; rgb->Next(j);
	    raster->poke(col, row, r, g, b, 1.0);
          } else {
            char colorname[8];
            snprintf(colorname, sizeof(colorname), "#%06x", elem->int_val());
            if (Color::find(World::current()->display(), colorname, r, g, b)) 
	      raster->poke(col, row, r, g, b, 1.0);
          }
        }
      }
    }
    raster->flush();

    Transformer* rel = get_transformer(al);
    if (rel) rasterrect->SetTransformer(rel);
    Unref(rel);
    set_graphic_gs(al, rasterrect);
    /* the pixels now live in the raster, so drop the keyword that carried them
       -- left in the list it re-serializes as a trailing attribute alongside
       the raster's own emitted pixel data */
    remove_key(al, symbol_add("rgb"));

    RasterOvComp* comp = new RasterOvComp(rasterrect);
    comp->SetAttributeList(al);
    return comp;
}

/*****************************************************************************/

static FontByNameFunc* _font_by_name_func = NULL;

FontFunc::FontFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FontFunc::execute() {
    if (nargs()==0 && nkeys()==0) {
        /* font() -- return the current editor font by X name
           (valid input to fontbyname(fontname)) */
        reset_stack();
        FontVar* fntVar = (FontVar*) _ed->GetState("FontVar");
        PSFont* fnt = fntVar ? fntVar->GetFont() : nil;
        if (!fnt) {
            push_stack(ComValue::nullval());
        } else {
            ComValue retval(fnt->GetName());
            push_stack(retval);
        }
        return;
    }

    ComValue& fv = stack_arg(0, true);
    if (fv.is_string()) {
        if (_font_by_name_func == NULL) {
            _font_by_name_func = new FontByNameFunc(comterp(), editor());
        } else {
            _font_by_name_func->comterp(comterp());
            _font_by_name_func->editor(editor());
        }
        _font_by_name_func->execute();
        return;
    }

    ComValue fnum(stack_arg(0));
    int fn = fnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSFont* font = catalog->ReadFont("font", fn);

    FontCmd* cmd = nil;

    if (font) {
        OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
	cmd = kit->make_font_cmd(_ed, font, fn);
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
  const static char *wght[] = { "bold","demi","light","demibold","book",0 };
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
  snprintf(name, sizeof(name),"-*-%s-%s-normal-*-%d-*",
	  copy, type, size );
  return name;
}
 /*****************************************************************************/
void FontByNameFunc::execute() {
  if (nargs()==0 && nkeys()==0) {
      /* fontbyname() -- return the current editor font by X name,
         same getter as bare font() */
      reset_stack();
      FontVar* fntVar = (FontVar*) _ed->GetState("FontVar");
      PSFont* fnt = fntVar ? fntVar->GetFont() : nil;
      if (!fnt) {
          push_stack(ComValue::nullval());
      } else {
          ComValue retval(fnt->GetName());
          push_stack(retval);
      }
      return;
  }

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
    /* psfonttoxfont hands back its own argument for a name already in X form,
       so only replace the buffer when it actually converted -- otherwise the
       free would leave xfontval dangling for the strdup and the retry below */
    if (xfontval != fontvaldup) {
      free(fontvaldup);
      fontvaldup = strdup(xfontval);
    }
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

    /* A wildcarded name -- the form font() hands back, and the form :font
       exports -- loads fine but carries none of these properties.
       XGetFontProperty then leaves value untouched, and XGetAtomName(0) is a
       BadAtom that takes the whole editor down, so every lookup has to be
       guarded and the atom freed.  FindFont defaults the print font and size
       when they arrive empty, which is the right answer for a name that
       simply does not carry them. */
    fontname[0] = '\0';
    if (XGetFontProperty(xfs, XA_FONT_NAME, &value) && value) {
      char* atom = XGetAtomName(dpy, (Atom)value);
      if (atom) {
	strncpy(fontname, atom, sizeof(fontname)-1);
	fontname[sizeof(fontname)-1] = '\0';
	XFree(atom);
      }
    }

    fontsizeptr[0] = '\0';
    if (XGetFontProperty(xfs, XA_POINT_SIZE, &value) && value)
      snprintf(fontsizeptr, sizeof(fontsizeptr),"%d",(unsigned int)(value/10));

    font = catalog->FindFont(fontvaldup,fontname,fontsizeptr);
  }
  free(fontvaldup);
  FontCmd* cmd = nil;
  
  if (font) {
    OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
    cmd = kit->make_font_cmd(_ed, font, 0, fontval);
    execute_log(cmd);
  }
  
}
/*****************************************************************************/
ColorRgbFunc::ColorRgbFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
 }

void ColorRgbFunc::execute() {
  if (nargs()==0 && nkeys()==0) {
      /* colorsrgb() -- return the current editor colors as a fgname,bgname
         literal, same getter as bare colors() */
      reset_stack();
      ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");
      PSColor* fgcolor = colVar ? colVar->GetFgColor() : nil;
      PSColor* bgcolor = colVar ? colVar->GetBgColor() : nil;
      if (!fgcolor || !bgcolor) {
          push_stack(ComValue::nullval());
      } else {
          AttributeValueList* avl = new AttributeValueList();
          avl->Append(new ComValue(fgcolor->GetName()));
          avl->Append(new ComValue(bgcolor->GetName()));
          ComValue retval(avl);
          push_stack(retval);
      }
      return;
  }

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
  /* route through the kit factory so DrawKit produces a LinkColorCmd for
     distribution; LinkColorCmd::dist_script() serializes by RGB intensities
     so the colors("#RRGGBB") form distributes correctly */
  OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
  ColorCmd* cmd = kit->make_color_cmd(_ed, fgcolor, bgcolor, 0, 0);
  execute_log(cmd);
}
/*****************************************************************************/

BrushFunc::BrushFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void BrushFunc::execute() {
    static int none_sym = symbol_add("none");

    if (nargs()==0 && nkeys()==0) {
        /* brush() -- return the current editor brush as a linepat,width
           literal (valid input to brush(linepat,width)), or the attrlist
           singleton (:none true) for the none brush.  A keyword-flag literal
           travels as an attrlist, never as a raw KeywordType value --
           eager stack_key() scans frames by type, so a stored keyword
           would become a live keyword in any frame it entered. */
        reset_stack();
        BrushVar* brVar = (BrushVar*) _ed->GetState("BrushVar");
        PSBrush* br = brVar ? brVar->GetBrush() : nil;
        if (!br) {
            push_stack(ComValue::nullval());
        } else if (br->None()) {
            AttributeList* al = new AttributeList();
            al->add_attr(none_sym, new AttributeValue(1, AttributeValue::BooleanType));
            ComValue retval(AttributeList::class_symid(), al);
            push_stack(retval);
        } else {
            AttributeValueList* avl = new AttributeValueList();
            avl->Append(new ComValue(br->GetLinePattern()));
            avl->Append(new ComValue(br->Width()));
            ComValue retval(avl);
            push_stack(retval);
        }
        return;
    }

    ComValue bnum(stack_arg(0));
    ComValue nonev(stack_key(none_sym));
    reset_stack();

    PSBrush* brush = nil;

    /* the attrlist singleton (:none true) returned by bare brush() is accepted
       back positionally, so saved=brush(); ...; brush(saved) round-trips
       the none brush */
    boolean none_by_value = false;
    if (bnum.is_attributelist()) {
        AttributeList* bal = (AttributeList*)bnum.geta(AttributeList::class_symid());
        Attribute* battr = bal ? bal->GetAttr(none_sym) : nil;
        none_by_value = battr && battr->Value() && battr->Value()->is_true();
    }

    if (nonev.is_true() || none_by_value) {
        /* brush(:none) -- none brush */
        brush = new PSBrush();

    } else if (bnum.is_array()) {
        /* brush(linepat,width) -- brush by value */
        AttributeValueList* avl = bnum.array_val();
        if (avl && avl->Number() >= 2) {
            Iterator it;
            avl->First(it);
            int linepat = avl->GetAttrVal(it)->int_val();
            avl->Next(it);
            int width = avl->GetAttrVal(it)->int_val();
            brush = new PSBrush(linepat, width);
        }

    } else {
        /* brush(brushnum) -- brush by menu index */
        int bn = bnum.int_val();
        Catalog* catalog = unidraw->GetCatalog();
        brush = catalog->ReadBrush("brush", bn);
    }

    BrushCmd* cmd = nil;

    if (brush) {
        OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
        cmd = kit->make_brush_cmd(_ed, brush);
        execute_log(cmd);
    }

}

/*****************************************************************************/

PatternFunc::PatternFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PatternFunc::execute() {
    if (nargs()==0 && nkeys()==0) {
        /* pattern() -- return the current editor pattern as its menu number
           (valid input to pattern(patternnum)); the catalog caches pattern
           reads, so the state var's pointer matches its menu entry.  nil if
           the current pattern is not a menu pattern (e.g. patternmask). */
        reset_stack();
        PatternVar* patVar = (PatternVar*) _ed->GetState("PatternVar");
        PSPattern* pat = patVar ? patVar->GetPattern() : nil;
        Catalog* catalog = unidraw->GetCatalog();
        int pn = 0;
        if (pat) {
            int i = 1;
            while (catalog->GetAttribute(catalog->Name("pattern", i))) {
                if (catalog->ReadPattern("pattern", i) == pat) { pn = i; break; }
                i++;
            }
        }
        if (pn) {
            ComValue retval(pn);
            push_stack(retval);
        } else
            push_stack(ComValue::nullval());
        return;
    }

    ComValue pnum(stack_arg(0));
    int pn = pnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSPattern* pattern = catalog->ReadPattern("pattern", pn);

    PatternCmd* cmd = nil;

    if (pattern) {
        OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
	cmd = kit->make_pattern_cmd(_ed, pattern, pn);
	execute_log(cmd);
    }

}

/*****************************************************************************/

PatternMaskFunc::PatternMaskFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PatternMaskFunc::execute() {
    ComValue bitsv(stack_arg(0));
    reset_stack();

    PSPattern* pattern = nil;
    std::string maskargs;

    if (bitsv.is_int()) {
      pattern = new PSPattern(bitsv.int_val(), -1);
      char buf[32];
      snprintf(buf, sizeof(buf), "%d", bitsv.int_val());
      maskargs = buf;
    } else if (bitsv.is_array()) {
      AttributeValueList* avl = bitsv.array_val();
      if (avl->Number()!=16) {
	fprintf(stderr, "patternbits list not 16 ints\n");
	push_stack(ComValue::nullval());
	return;
      }
      int mask[16];
      std::ostringstream mbuf;
      for(int i=0; i<16; i++) {
	mask[i] = avl->Get(i)->int_val();
	if (i) mbuf << ",";
	mbuf << mask[i];
      }
      pattern = new PSPattern(mask, 16);
      maskargs = mbuf.str();
    } else {
      fprintf(stderr, "patternbits argument not int or list\n");
      push_stack(ComValue::nullval());
      return;
    }

    PatternCmd* cmd = nil;

    if (pattern) {
        OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
	cmd = kit->make_pattern_cmd(_ed, pattern, 0, maskargs.c_str());
	execute_log(cmd);
    }

}

/*****************************************************************************/

static ColorRgbFunc* _color_rgb_func = NULL;

ColorFunc::ColorFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ColorFunc::execute() {
    if (nargs()==0 && nkeys()==0) {
        /* colors() -- return the current editor colors as a fgname,bgname
           literal (valid input to colors(fgname bgname)) */
        reset_stack();
        ColorVar* colVar = (ColorVar*) _ed->GetState("ColorVar");
        PSColor* fgcolor = colVar ? colVar->GetFgColor() : nil;
        PSColor* bgcolor = colVar ? colVar->GetBgColor() : nil;
        if (!fgcolor || !bgcolor) {
            push_stack(ComValue::nullval());
        } else {
            AttributeValueList* avl = new AttributeValueList();
            avl->Append(new ComValue(fgcolor->GetName()));
            avl->Append(new ComValue(bgcolor->GetName()));
            ComValue retval(avl);
            push_stack(retval);
        }
        return;
    }

    ComValue& fgv = stack_arg(0, true);
    if (fgv.is_string()) {
        if (_color_rgb_func == NULL) {
            _color_rgb_func = new ColorRgbFunc(comterp(), editor());
        } else {
            _color_rgb_func->comterp(comterp());
            _color_rgb_func->editor(editor());
        }
        _color_rgb_func->execute();
        return;
    }

    ComValue fgnum(stack_arg(0));
    ComValue bgnum(stack_arg(1));
    int fgn = fgnum.int_val();
    int bgn = bgnum.int_val();
    reset_stack();

    Catalog* catalog = unidraw->GetCatalog();
    PSColor* fgcolor = catalog->ReadColor("fgcolor", fgn);
    PSColor* bgcolor = catalog->ReadColor("bgcolor", bgn);

    /* pass fgn/bgn through to make_color_cmd so LinkColorCmd can emit
       colors(fgn bgn) on the wire -- same pattern as brush(pat,width) */
    OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
    ColorCmd* cmd = kit->make_color_cmd(_ed, fgcolor, bgcolor, fgn, bgn);
    execute_log(cmd);
}

/*****************************************************************************/

SelectFunc::SelectFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

ChildrenFunc::ChildrenFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ChildrenFunc::execute() {
    ComValue parentv(stack_arg(0));
    reset_stack();

    Viewer* viewer = _ed->GetViewer();
    GraphicView* gv = nil;
    if (parentv.object_compview()) {
      ComponentView* comview = (ComponentView*)parentv.obj_val();
      OverlayComp* comp = (OverlayComp*)comview->GetSubject();
      if (comp) gv = comp->FindView(viewer);
    } else
      gv = ((OverlayEditor*)_ed)->GetFrame();

    AttributeValueList* avl = new AttributeValueList();
    if (gv) {
      Iterator i;
      for (gv->First(i); !gv->Done(i); gv->Next(i)) {
	GraphicView* subgv = gv->GetView(i);
	OverlayComp* comp = subgv ? (OverlayComp*)subgv->GetGraphicComp() : nil;
	if (comp)
	  avl->Append(new ComValue(new OverlayViewRef(comp), comp->classid()));
      }
    }
    ComValue retval(avl);
    push_stack(retval);
}

void SelectFunc::execute() {
    static int all_symid = symbol_add("all");
    ComValue all_flagv(stack_key(all_symid));
    boolean all_flag = all_flagv.is_true();
    static int clear_symid = symbol_add("clear");
    ComValue clear_flagv(stack_key(clear_symid));
    boolean clear_flag = clear_flagv.is_true();
    static int unlock_symid = symbol_add("unlock");
    ComValue unlockv(stack_key(unlock_symid));
    static int lock_symid = symbol_add("lock");
    ComValue lockv(stack_key(lock_symid));
    
    OverlaySelection* sel = (OverlaySelection*)_ed->GetViewer()->GetSelection();
    if (clear_flag) {
      sel->Clear();
      unidraw->Update();
      reset_stack();
      return;
    }
      
    OverlaySelection* newSel = ((OverlayEditor*)_ed)->overlay_kit()->MakeSelection();
    newSel->CopyFlags(sel);
    if (sel->HandlesDisabled()) {
      newSel->DisableHandles();
    }
    
    Viewer* viewer = _ed->GetViewer();
    AttributeValueList* avl = new AttributeValueList();
    if (all_flag) {

      GraphicView* gv = ((OverlayEditor*)_ed)->GetFrame();
      Iterator i;
      int count=0;
      for (gv->First(i); !gv->Done(i); gv->Next(i)) {
	GraphicView* subgv = gv->GetView(i);
	newSel->Append(subgv);
      }

    } else if (nargs()==0) {
      Iterator i;
      int count=0;
      for (sel->First(i); !sel->Done(i); sel->Next(i)) {
	GraphicView* grview = sel->GetView(i);
	OverlayComp* comp = grview ? (OverlayComp*)grview->GetSubject() : nil;
	ComValue* compval = comp ? new ComValue(new OverlayViewRef(comp), comp->classid()) : nil;

	if (compval) {
	  avl->Append(compval);
	}
      }
      /* the query form installs no selection at all.  inside the loop this ran
	 only when there was something to report, so querying an empty selection
	 fell through to the block below and cleared the editor's selection --
	 and, once select() waits, blocked on the pending count as well. */
      delete newSel;
      newSel = nil;

    } else {

      for (int i=0; i<nargsfixed(); i++) {
        ComValue& obj = stack_arg(i);
	if (obj.object_compview()) {
	  ComponentView* comview = (ComponentView*)obj.obj_val();
	  OverlayComp* comp = (OverlayComp*)comview->GetSubject();
	  if (comp) {
	    GraphicView* view = comp->FindView(viewer);
	    if (view)
	      newSel->Append(view);
	  }
	} else if (obj.is_array()) {
	  Iterator it;
	  AttributeValueList* al = obj.array_val();
	  al->First(it);
	  while (!al->Done(it)) {
	    if (al->GetAttrVal(it)->object_compview()) {
	      ComponentView* comview = (ComponentView*)al->GetAttrVal(it)->obj_val();
	      OverlayComp* comp = (OverlayComp*)comview->GetSubject();
	      if (comp) {
		GraphicView* view = comp->FindView(viewer);
		if (view)
		  newSel->Append(view);
	      }
	    }
	    al->Next(it);
	  }
	}
      }
    }

    if (newSel){
      if (unlockv.is_string())
        newSel->unlock_key(unlockv.string_ptr());
      sel->Clear();
      delete sel;
      _ed->SetSelection(newSel);
      /* Reserve() (the DrawServ wire-protocol claim on these graphics --
         a no-op below that layer) must fire at select time, but the
         repaint that Selection::Update wrapped around it must NOT: it
         repaired all pending damage per select() call, repainting
         body-by-body in animation loops that select/move/rotate many
         graphics between update() calls.  Nothing paints until update()
         -- the deferred unidraw->Update() below repaints at the end of
         the typed command line, so interactive select feedback (handles
         at next repaint) is unchanged. */
      newSel->Reserve();   // sees unlocked()==true
      if (lockv.is_string())
        newSel->lock_key(lockv.string_ptr());  // clear after Reserve()

      resolve_requests(newSel);

      /* report what was acquired rather than what was asked for: Reserve()
         removes the graphics another session is holding, so a list built while
         appending describes the request, and a select() that was refused reads
         back as one that succeeded.  read the editor's selection rather than
         newSel: resolve_requests() may have run the event loop, and anything
         that arrived during it could have put a different selection in place. */
      OverlaySelection* cursel = (OverlaySelection*)_ed->GetSelection();
      if (cursel) {
        Iterator si;
        for (cursel->First(si); !cursel->Done(si); cursel->Next(si)) {
          GraphicView* grview = cursel->GetView(si);
          OverlayComp* comp = grview ? (OverlayComp*)grview->GetSubject() : nil;
          if (comp)
            avl->Append(new ComValue(new OverlayViewRef(comp), comp->classid()));
        }
      }

      /* Clear() above hid the outgoing selection's tic marks and highlighting,
	 and nothing here put them back: repairing damage repaints graphics and
	 never draws a handle.  A grant arriving does redraw them, AddComp
	 ending in Selection::Update, which is why they are only missing when a
	 select resolves without one -- everything either already ours or
	 refused outright.  Restore them in the order Selection::Update uses,
	 with the repaint below standing in for its Repair, and read the
	 editor's selection since a wait may have replaced ours. */
      unidraw->Update();

      /* Clear() above hid the outgoing selection's tic marks and highlighting
	 and nothing put them back.  unidraw->Update() cannot: it defers, and
	 the repaint it schedules repairs the damage that hiding them made --
	 over anything drawn here beforehand.  Selection::Update repairs and
	 then draws, which is the order that survives, and it is what #210 took
	 out of here for the cost of its Repair; keep that saving where it was
	 aimed, at animation loops, which run with handles disabled.  Read the
	 editor's selection since a wait may have replaced ours. */
      OverlaySelection* shown = (OverlaySelection*)_ed->GetSelection();
      if (shown && shown->HandlesEnabled())
	shown->Update(viewer);
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
    ComValue retval(_ed->GetViewer()->GetMagnification());
    push_stack(retval);
    
}

/*****************************************************************************/

ZoomInFunc::ZoomInFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ZoomInFunc::execute() {
    reset_stack();
    ZoomCmd* cmd = new ZoomCmd(_ed, 2.0);
    execute_log(cmd);

    ComValue retval(_ed->GetViewer()->GetMagnification());
    push_stack(retval);
}

/*****************************************************************************/

ZoomOutFunc::ZoomOutFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ZoomOutFunc::execute() {
    reset_stack();
    ZoomCmd* cmd = new ZoomCmd(_ed, 0.5);
    execute_log(cmd);

    ComValue retval(_ed->GetViewer()->GetMagnification());
    push_stack(retval);
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

    const char* ifile = symbol_pntr(ifilev.symbol_ref());
    const char* ofile = symbol_pntr(ofilev.symbol_ref());

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
    
    static int apply_sym = symbol_add("apply");
    static int set_sym    = symbol_add("set");

    ComValue objv(stack_arg(0));
    ComValue transv(stack_arg(1));
    ComValue applyv(stack_key(apply_sym));
    ComValue setv(stack_key(set_sym));
    reset_stack();
    if (objv.object_compview()) {
      ComponentView* compview = (ComponentView*)objv.obj_val();
      if (compview && compview->GetSubject()) {
	OverlayComp* comp = (OverlayComp*)compview->GetSubject();
	Graphic* gr = comp->GetGraphic();
	if (gr) {
	  Transformer* trans = gr->GetTransformer();
	  if (trans == nil) {
	    trans = new Transformer();
	  }
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

	    Transformer* want = new Transformer(a00, a01, a10, a11, a20, a21);

	    /* TransformCmd applies what it is given on top of what the graphic
	       already has (GraphicComp::Interpret postmultiplies), so :apply
	       hands it the matrix directly.  The default -- and :set -- impose
	       the matrix instead, which is the same operation once the current
	       transform is backed out first: postmultiply is C*t with C applied
	       first, so the delta that carries C to the wanted D is inverse(C)*D.
	       Interpret then lands on D, and Uninterpret inverts the same delta
	       to restore C, so undo comes free either way. */
	    Transformer* delta = nil;
	    if (applyv.is_true() && !setv.is_true()) {
	      delta = want;
	      Resource::ref(delta);

	    } else if (!gr->GetTransformer()) {
	      /* nothing to back out -- imposing and applying agree */
	      delta = want;
	      Resource::ref(delta);

	    } else {
	      Transformer* cur = gr->GetTransformer();
	      float c00, c01, c10, c11, c20, c21;
	      cur->matrix(c00, c01, c10, c11, c20, c21);
	      if (c00*c11 - c01*c10 == 0.0) {
		/* a degenerate current transform cannot be backed out; say so
		   rather than hand invert() a matrix it has no answer for */
		fprintf(stderr, "trans: current transform is not invertible, cannot impose a new one (try :apply)\n");
		Unref(want);
		push_stack(ComValue::nullval());
		return;
	      }
	      delta = new Transformer(*cur);
	      Resource::ref(delta);
	      delta->invert();
	      delta->postmultiply(*want);
	      Unref(want);
	    }

	    OverlayKit* kit = ((OverlayEditor*)_ed)->overlay_kit();
	    TransformCmd* cmd = kit->make_transform_cmd(_ed, delta);
	    cmd->SetClipboard(new Clipboard(comp));
	    Unref(delta);
	    execute_log(cmd);

	    ComValue compval(new OverlayViewRef(comp), comp->class_symid());
	    push_stack(compval);
	  }
	}
      } 	
    }
}

/*****************************************************************************/

GrParentFunc::GrParentFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GrParentFunc::execute() {
  ComValue compv(stack_arg(0));
  reset_stack();

  if(compv.is_object() && compv.object_compview()) {
    ComponentView* compview = (ComponentView*)compv.obj_val();
    OverlayComp* comp = (OverlayComp*)compview->GetSubject();
    if (comp && comp->GetParent()) {
      ComValue retval(new OverlayViewRef((OverlayComp*)comp->GetParent()), 
		      ((OverlayComp*)comp->GetParent())->classid());
      push_stack(retval);
      return;
    } 
  }
  push_stack(ComValue::nullval());
  return;
}

/*****************************************************************************/

#ifdef LEAKCHECK
#include <leakchecker.h>

CompLeakFunc::CompLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void CompLeakFunc::execute() {
  reset_stack();
  if (OverlayComp::_leakchecker==nil) {
    push_stack(ComValue::zeroval());
    return;
  }
  ComValue retval(OverlayComp::_leakchecker->alive(), ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

ViewLeakFunc::ViewLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ViewLeakFunc::execute() {
  reset_stack();
  if (OverlayView::_leakchecker==nil) {
    push_stack(ComValue::zeroval());
    return;
  }
  ComValue retval(OverlayView::_leakchecker->alive(), ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

AlistLeakFunc::AlistLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void AlistLeakFunc::execute() {
  reset_stack();
  ComValue retval(AttributeValueList::_leakchecker ? AttributeValueList::_leakchecker->alive() : 0, ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

AttrvLeakFunc::AttrvLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void AttrvLeakFunc::execute() {
  reset_stack();
  ComValue retval(AttributeValue::_leakchecker ? AttributeValue::_leakchecker->alive() : 0, ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

MlineLeakFunc::MlineLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void MlineLeakFunc::execute() {
  reset_stack();
  ComValue retval(MultiLineObj::_leakchecker ? MultiLineObj::_leakchecker->alive() : 0, ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

GraphicLeakFunc::GraphicLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GraphicLeakFunc::execute() {
  reset_stack();
  ComValue retval(Graphic::_leakchecker ? Graphic::_leakchecker->alive() : 0, ComValue::IntType);
  push_stack(retval);
}

/*****************************************************************************/

CommandLeakFunc::CommandLeakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void CommandLeakFunc::execute() {
  reset_stack();
  ComValue retval(Command::_leakchecker ? Command::_leakchecker->alive() : 0, ComValue::IntType);
  push_stack(retval);
}
#endif

/*****************************************************************************/

HideCompFunc::HideCompFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void HideCompFunc::execute() {
    ComValue viewval(stack_arg(0));
    reset_stack();
    if (!viewval.is_object()) {
      push_stack(ComValue::nullval());
      return;
    }

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    if(comp) {
      comp->GetGraphic()->Hide();
      comp->Notify();
    }
    push_stack(viewval);
}

/*****************************************************************************/

ShowCompFunc::ShowCompFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ShowCompFunc::execute() {
    ComValue viewval(stack_arg(0));
    reset_stack();
    if (!viewval.is_object()) {
      push_stack(ComValue::nullval());
      return;
    }

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    if(comp) {
      comp->GetGraphic()->Show();
      comp->Notify();
    }
    push_stack(viewval);
}

/*****************************************************************************/

DesensitizeCompFunc::DesensitizeCompFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void DesensitizeCompFunc::execute() {
    ComValue viewval(stack_arg(0));
    reset_stack();
    if (!viewval.is_object()) {
      push_stack(ComValue::nullval());
      return;
    }

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    if(comp) {
      comp->GetGraphic()->Desensitize();
      comp->Notify();
    }
    push_stack(viewval);
}

/*****************************************************************************/

SensitizeCompFunc::SensitizeCompFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void SensitizeCompFunc::execute() {
    ComValue viewval(stack_arg(0));
    reset_stack();
    if (!viewval.is_object()) {
      push_stack(ComValue::nullval());
      return;
    }

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    if(comp) {
      comp->GetGraphic()->Sensitize();
      comp->Notify();
    }
    push_stack(viewval);
}

/*****************************************************************************/

FlipHorizontalFunc::FlipHorizontalFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FlipHorizontalFunc::execute() {
    reset_stack();

    ScaleCmd* cmd = nil;

    cmd = new ScaleCmd(_ed, -1.0, 1.0);

    execute_log(cmd);
}

/*****************************************************************************/

FlipVerticalFunc::FlipVerticalFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void FlipVerticalFunc::execute() {
    reset_stack();

    ScaleCmd* cmd = nil;

    cmd = new ScaleCmd(_ed, 1.0, -1.0);

    execute_log(cmd);
}

