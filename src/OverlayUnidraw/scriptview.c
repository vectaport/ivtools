/*
 * Copyright (c) 1994-1998 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * OverlayScript implementation.
 */
#include <OverlayUnidraw/scriptview.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>

#include <ComTerp/parser.h>

#include <Attribute/aliterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Attribute/lexscan.h>
#include <Attribute/paramlist.h>

#include <UniIdraw/idarrow.h>
#include <UniIdraw/idarrows.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/picture.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/Graphic/ustencil.h>
#include <Unidraw/Graphic/verts.h>

#include <InterViews/bitmap.h>
#include <InterViews/textbuffer.h>
#include <InterViews/transformer.h>

#include <OS/math.h>

#include <ctype.h>
#include <stream.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

boolean OverlayScript::_ptlist_parens = true;

ClassId OverlayScript::GetClassId () { return OVERLAY_SCRIPT; }

boolean OverlayScript::IsA (ClassId id) {
    return OVERLAY_SCRIPT == id || OverlayPS::IsA(id);
}

OverlayScript::OverlayScript (OverlayComp* subj) : OverlayPS(subj) { 
    _parent = nil;
}

OverlayScript::~OverlayScript () { 
}

void OverlayScript::FillBg (ostream& out) {
    int filled = GetOverlayComp()->GetGraphic()->BgFilled();
    out << " :fillbg " << filled;
}

void OverlayScript::Brush (ostream& out) {
    PSBrush* brush = (PSBrush*) GetOverlayComp()->GetGraphic()->GetBrush();

    if (brush != nil) {

	if (brush->None()) {
	    out << " :nonebr";

	} else {
	    out << " :brush ";
	    int p = brush->GetLinePattern();
	    out << p << ",";

	    float w = brush->width();
	    out << w;
	}
    }
}

void OverlayScript::FgColor (ostream& out) {
    PSColor* fgcolor = (PSColor*) GetOverlayComp()->GetGraphic()->GetFgColor();

    if (fgcolor != nil) {
	const char* name = fgcolor->GetName();
	out << " :fgcolor \"" << name << "\"";

	ColorIntensity r, g, b;
	fgcolor->GetIntensities(r, g, b);
	out << "," << r << "," << g << "," << b;
    }
}

void OverlayScript::BgColor (ostream& out) {
    PSColor* bgcolor = (PSColor*) GetOverlayComp()->GetGraphic()->GetBgColor();

    if (bgcolor != nil) {
	const char* name = bgcolor->GetName();
	out << " :bgcolor \"" << name << "\"";

	ColorIntensity r, g, b;
	bgcolor->GetIntensities(r, g, b);
	out << "," << r << "," << g << "," << b;
    }
}

void OverlayScript::Font (ostream& out) {
    PSFont* font = (PSFont*) GetOverlayComp()->GetGraphic()->GetFont();
    if (font != nil) {
	const char* name = font->GetName();
	out << " :font \"" << name << "\"" << ",";
	const char* pf = font->GetPrintFont();
	out << "\"" << pf << "\"" << ",";
	const char* ps = font->GetPrintSize();
	out << ps;
    }
}

void OverlayScript::Pattern (ostream& out) {
    PSPattern* pat = (PSPattern*) GetOverlayComp()->GetGraphic()->GetPattern();

    if (pat != nil) {


	if (pat->None()) {
	    out << " :nonepat";
	} else if (pat->GetSize() > 0) {
	    const int* data = pat->GetData();
	    int size = pat->GetSize();
	    char buf[BUFSIZ];
	    out << " :pattern ";

	    if (size <= 8) {
		for (int i = 0; i < 8; i++) {
		    sprintf(buf, "0x%02x", data[i] & 0xff);
		    out << buf;
		    if (i < 7 ) out << ",";
		}

	    } else {
		for (int i = 0; i < patternHeight; i++) {
		    sprintf(buf, "0x%0*x", patternWidth/4, data[i]);
		    if (i != patternHeight - 1) {
			out << buf << ",";
		    } else {
			out << buf;
		    }
		}
	    }

	} else {
	    float graylevel = pat->GetGrayLevel();
	    out << " :graypat " << graylevel;
	}
    }
}

void OverlayScript::Transformation (ostream& out) {
    Transformation(out, "transform");
}

void OverlayScript::Transformation (ostream& out, char* keyword, Graphic* gr) {
    Transformer* t = gr ? gr->GetTransformer() : GetOverlayComp()->GetGraphic()->GetTransformer();
    Transformer identity;

    if (t != nil && *t != identity) {
	char key[strlen(keyword)+4];
	sprintf(key," :%s ",keyword);
	float a00, a01, a10, a11, a20, a21;
	t->GetEntries(a00, a01, a10, a11, a20, a21);
	out << key;
	out << a00 << "," << a01 << "," << a10 << ",";
	out << a11 << "," << a20 << "," << a21;
    }
}

void OverlayScript::Annotation (ostream& out) {
    OverlayComp* comp = GetOverlayComp();
    const char* anno = comp->GetAnnotation();
    if (!anno) 
        return;
    out << " :annotation " << "\n";
    int indent = Indent(out);
    ParamList::output_text(out, anno, indent);
}

OverlayScript* OverlayScript::CreateOverlayScript (OverlayComp* comp) {
    OverlayScript* sv = (OverlayScript*) comp->Create(OVERLAY_SCRIPT);

    if (sv != nil) {
        comp->Attach(sv);
	sv->SetCommand(GetCommand());
        sv->Update();
    }
    return sv;
}

int OverlayScript::Indent (ostream& out, int extra) {
    Component* comp = this->GetSubject();
    int i = 0;
    do {
	out << "    ";
	comp = comp->GetParent(); 
	i++;
    } 
    while (comp != nil);

    for (int x=0; x!=extra; x++)
	out << "    ";

    return i;
}    

boolean OverlayScript::GetByPathnameFlag() {
    return _parent ? _parent->GetByPathnameFlag() : false;
}

boolean OverlayScript::skip_comp(istream& in) {

  char ch; 
  ParamList::skip_space(in);
  ch = in.get();
  if (ch==',') {
    ParamList::skip_space(in);
    ch = in.get();
    ParamList::skip_space(in);
  }
  in.unget();
  if (ch=='(') {
    Parser parser(in);
    if (!parser.skip_matched_parens())
      cerr << "error in skipping matched parens\n";
  } else {
    cerr << "not positioned at left-paren for skipping component\n";
  }
  return true;
}
  

/*****************************************************************************/

int OverlayScript::ReadGS (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    OverlayComp* comp = (OverlayComp*)addr1;
    Graphic* gs = *(Graphic**)addr2;
    if (!gs) {
        gs = new FullGraphic();
        comp->SetGraphic(gs);
    }
    int id;
    in >> id;
    Graphic* gr = comp->GetIndexedGS(id);
    if (gr) 
	*gs = *gr;
#if 0
    else 
	cerr << ":gs reference without gs records\n";
#endif
    return in.good() ? 0 : -1;
}

int OverlayScript::ReadFillBg (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    Graphic* gs = *(Graphic**)addr1;
    int filled;
    
    in >> filled;
    if (!in.good()) {
        return -1;
    }
    else {
        gs->FillBg(filled);
        return 0;
    }
}

int OverlayScript::ReadNoneBr (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    Graphic* gs = *(Graphic**)addr1;
 
    if (!in.good()) {
	gs->SetBrush(nil);
        return -1;
    }
    else {
	PSBrush* brush = OverlayCatalog::Instance()->FindNoneBrush();
        gs->SetBrush(brush);
        return 0;
    }
}

int OverlayScript::ReadBrush (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    int p;
    float w;
    char delim;
    Graphic* gs = *(Graphic**)addr1;

    ParamList::skip_space(in);
    in >> p >> delim >> w;
    if (!in.good()) {
	gs->SetBrush(nil);
	return -1;
    }
    else {
        PSBrush* brush = OverlayCatalog::Instance()->FindBrush(p,w);
	gs->SetBrush(brush);
	return 0;
    }
}

int OverlayScript::ReadFgColor (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    char lookahead = '"';
    char name[BUFSIZ];
    boolean name_arg = false;
    ColorIntensity r = 0, g = 0, b = 0;
    Graphic* gs = *(Graphic**)addr1;
    char delim;
    char buf[BUFSIZ];

    ParamList::skip_space(in);
    in >> lookahead; 
    in.putback(lookahead);

    if (lookahead == '"') {
        name_arg = true;
	ParamList::parse_string(in, name, BUFSIZ);
        if (!in.good()) {
	    gs->SetColors(nil, gs->GetBgColor());
	    return -1;
        }
    }

    if (name_arg) {
        in >> lookahead; 
        in.putback(lookahead);
    }

    if (lookahead == ',' || !name_arg) {
        in >> delim >> r >> delim >> g >> delim >> b;
	if (!in.good()) {
	    gs->SetColors(nil, gs->GetBgColor());
	    return -1;
        }                
	else {
	    int ir = Math::round(r * float(0xffff));
	    int ig = Math::round(g * float(0xffff));
	    int ib = Math::round(b * float(0xffff));
	    PSColor* fgcolor = OverlayCatalog::Instance()->FindColor(name_arg ? name : "no_name", ir, ig, ib);
	    gs->SetColors(fgcolor, gs->GetBgColor());
            return 0;
	}
    }
    return -1;
}

int OverlayScript::ReadBgColor (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    char lookahead = '"';
    char name[BUFSIZ];
    boolean name_arg = false;
    ColorIntensity r = 0, g = 0, b = 0;
    Graphic* gs = *(Graphic**)addr1;
    char delim;

    ParamList::skip_space(in);
    in >> lookahead; 
    in.putback(lookahead);

    if (lookahead == '"') {
        name_arg = true;
	ParamList::parse_string(in, name, BUFSIZ);
        if (!in.good()) {
	    gs->SetColors(gs->GetFgColor(), nil);
	    return -1;
        }
    }

    if (name_arg) {
        in >> lookahead; 
        in.putback(lookahead);
    }

    if (lookahead == ',' || !name_arg) {
        in >> delim >> r >> delim >> g >> delim >> b;
	if (!in.good()) {
	    gs->SetColors(gs->GetFgColor(), nil);
	    return -1;
        }                
	else {
	    int ir = Math::round(r * float(0xffff));
	    int ig = Math::round(g * float(0xffff));
	    int ib = Math::round(b * float(0xffff));

	    PSColor* bgcolor = OverlayCatalog::Instance()->FindColor(name_arg ? name : "no_name", ir, ig, ib);
	    gs->SetColors(gs->GetFgColor(), bgcolor);
            return 0;
	}
    }
    return -1;
}

int OverlayScript::ReadFont (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    char name[BUFSIZ];
    char printfont[BUFSIZ];
    int printsize;
    char printsizebuf[BUFSIZ];
    char delim;
    Graphic* gs = *(Graphic**)addr1;
    
    boolean pf = false;
    boolean ps = false;
    
    ParamList::skip_space(in);
    ParamList::parse_string(in, name, BUFSIZ);
    in >> delim;
    if (in.good() && delim == ',') {
        ParamList::parse_string(in, printfont, BUFSIZ);
        pf = true;
	in >> delim;
	if (in.good() && delim == ',') {
	    in >> printsize;
	    ps = true;
	    sprintf(printsizebuf, "%d", printsize);
	}
    }
    if (!in.good()) {
	gs->SetFont(nil);
	return -1;
    }
    else {
        PSFont* font = OverlayCatalog::Instance()->FindFont(name, pf ? printfont : "", ps ? printsizebuf : "");
	gs->SetFont(font);
	return 0;
    }
}

int OverlayScript::ReadNonePat (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    Graphic* gs = *(Graphic**)addr1;
 
    if (!in.good()) {
	gs->SetPattern(nil);
        return -1;
    }
    else {
	PSPattern* pattern = OverlayCatalog::Instance()->FindNonePattern();
        gs->SetPattern(pattern);
        return 0;
    }
}

int OverlayScript::ReadPattern (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    char delim = ',';
    int data[patternHeight];
    int size = 0;
    Graphic* gs = *(Graphic**)addr1;

    char buf[BUFSIZ];
    ParamList::skip_space(in);
    while(delim == ',' && size < patternHeight) {
	ParamList::parse_token(in, buf, BUFSIZ, delim);
	char* bufptr = buf;
	if (buf[0] == '0' && buf[1] == 'x') 
	    bufptr += 2;
	sscanf(bufptr, "%x", &data[size++]);
	in.get(delim);
    }
    if (!in.good()) {
	gs->SetPattern(nil);
	return -1;
    }
    else {
        if (delim==')')
	    in.putback(delim);
        PSPattern* pattern = OverlayCatalog::Instance()->FindPattern(data, size);
	gs->SetPattern(pattern);
	return 0;
    }
}

int OverlayScript::ReadGrayPat (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    Graphic* gs = *(Graphic**)addr1;
    float graylevel;
 
    ParamList::skip_space(in);
    in >> graylevel;
    if (!in.good()) {
	gs->SetPattern(nil);
        return -1;
    }
    else {
	PSPattern* pattern = OverlayCatalog::Instance()->FindGrayLevel(graylevel);
        gs->SetPattern(pattern);
        return 0;
    }
}

int OverlayScript::ReadTransform (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    float a00, a01, a10, a11, a20, a21;
    char delim;
    Graphic* gs = *(Graphic**)addr1;
 
    ParamList::skip_space(in);
    in >> a00 >> delim >> a01 >> delim >> a10 >> delim >> a11 >> delim >> a20 >> delim >> a21;
    if (!in.good()) {
        return -1;
    }
    else {
        Transformer* t = new Transformer(a00, a01, a10, a11, a20, a21);
        if (gs) gs->SetTransformer(t);
        else fprintf(stderr, "OverlayScript::ReadTransform:  no graphic for transformer\n");
	Unref(t);
        return 0;
    }
}

int OverlayScript::ReadAnnotation (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    char* buf = ParamList::parse_textbuf(in);

    if (!in.good()) {
	delete buf;
        return -1;
    }
    else {
	*(char**)addr1 = buf;
        return 0;
    }    
}

int OverlayScript::ReadOther(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    OverlayComp* comp = (OverlayComp*)addr1;
    AttributeList* attrlist = comp->GetAttributeList();
    char* keyword = (char*)addr4;
    AttributeValueList* avlist = nil;
    AttributeValue* val;

    do {
    char ch;
    if ((ch=in.peek()) == '\"') {
	sbuf[0] = '\"';
	ParamList::parse_string(in, sbuf+1, SBUFSIZE-1);
	strcat(sbuf, "\"\n");
    }
    else if (ch!=')') {
	ParamList::parse_token(in, sbuf, SBUFSIZE, " \t\n,");
	strcat(sbuf, "\n");
    } else
      strcpy(sbuf, "1\n");

    if (!in.good() && attrlist && keyword) {
        return -1;
    }
    else {

        int negate = sbuf[0] == '-';
	int slen = strlen(sbuf);
        val = 
	    ParamList::lexscan()->get_attr(sbuf+negate, slen-negate);
	if (negate) val->negate();

	//attrlist->add_attr(keyword, val);
        //return 0;

	ParamList::skip_space(in);
	if (in.peek() == ',') {
	  char comma;
	  in.get(comma);
	  if (!avlist) {
	    avlist = new AttributeValueList();
	    Resource::ref(avlist);
	  }
	  avlist->Append(val);
	}
	else {
	  if (avlist)
	    avlist->Append(val);
	  break;
	}
    }
    } while(1);
    if (avlist == nil) {
	attrlist->add_attr(keyword, val);
        return 0;
    }
    else {
      attrlist->add_attr(keyword, new AttributeValue(avlist));
      return 0;
    }
}

boolean OverlayScript::DefaultGS() {
    Graphic* gr = GetGraphicComp()->GetGraphic();
    return !gr->GetBrush() && !gr->GetFgColor() &&
	!gr->GetBgColor() && !gr->GetFont() && !gr->GetPattern();
}

int OverlayScript::MatchedGS(Clipboard* cb) {
    int count;
    MatchedGS(cb, count);
    return count;
}

Iterator OverlayScript::MatchedGS(Clipboard* cb, int& count) {
    Graphic* gr = GetGraphicComp()->GetGraphic();
    Iterator i;
    count = 0;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	Graphic* test = cb->GetComp(i)->GetGraphic();
	if (gr->GetBrush() == test->GetBrush() &&
	    gr->GetFgColor() == test->GetFgColor() &&
	    gr->GetBgColor() == test->GetBgColor() &&
	    gr->GetFont() == test->GetFont() &&
	    gr->GetPattern() == test->GetPattern() &&
	    gr->BgFilled() == test->BgFilled()) 
	    return i;
	count++;
    }
    Iterator j;
    count = -1;
    return j;
}

boolean OverlayScript::EmitGS(ostream& out, Clipboard* cb, boolean prevout) {
    if ( !DefaultGS() && MatchedGS(cb)<0) {
	if (prevout) {
	    out << ",\n";
        } else {
	    out << "\n";
        }
	prevout = true;
	out << "    gs(";
	FillBg(out);
	Brush(out);
	FgColor(out);
	BgColor(out);
	Font(out);
	Pattern(out);
	out << ")";
	cb->Append(GetGraphicComp());
    }
    return prevout;
}

int OverlayScript::MatchedPts(Clipboard* cb) {
    int count;
    MatchedPts(cb, count);
    return count;
}

Iterator OverlayScript::MatchedPts(Clipboard* cb, int& count) {
    OverlayComp* comp = GetOverlayComp();
    count = -1;
    Iterator j;
    if (!comp->IsA(OVVERTICES_COMP)) return j;
    Vertices* verts = (Vertices*)comp->GetGraphic();
    Iterator i;
    count = 0;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	Vertices* test = (Vertices*) cb->GetComp(i)->GetGraphic();
	if ((void*) test->GetOriginal() == (void*) verts->GetOriginal()) 
	    return i;
	count++;
    }
    count = -1;
    return j;
}

int OverlayScript::MatchedPic(Clipboard* cb) {
    int count;
    MatchedPic(cb, count);
    return count;
}

Iterator OverlayScript::MatchedPic(Clipboard* cb, int& count) {
    count = -1;
    OverlayComp* comp = GetOverlayComp();
    Iterator j;
    if (!comp->IsA(OVERLAYS_COMP)) return j;
    Iterator i;
    count = 0;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	if (*comp == *(OverlayComp*)cb->GetComp(i))
	    return i;
	count++;
    }
    count = -1;
    return j;
}

boolean OverlayScript::EmitPts(ostream& out, Clipboard* cb, boolean prevout) {
    if (GetGraphicComp()->IsA(OVVERTICES_COMP) && MatchedPts(cb)<0) {
	MultiLineObj* pts = ((Vertices*)GetGraphicComp()->GetGraphic())->GetOriginal();
	if (pts && pts->count()>0) {
	  if (prevout) 
	    out << ",\n    ";
	  else
	    out << "\n    ";
	  prevout = true;
	  out << "pts(";
	  const int rowsz = 10;
	  Coord* x = pts->x();
	  Coord* y = pts->y();
	  int count = pts->count();
	  for (int i=0; i<count; i+= rowsz) {
	    if (i!=0) out << ",\n        ";
	    for (int j=i; j<i+rowsz && j<count; j++) {
	      if (j!=i) out << ",";
	      out << "(" << x[j] << "," << y[j] << ")";
	    }
	  }
	  out << ")";
	  cb->Append(GetGraphicComp());
	}
    }
    return prevout;
}

boolean OverlayScript::EmitPic(ostream& out, Clipboard* cb1, Clipboard* cb2, boolean prevout) {
    return prevout;
}

/*--------------------------------------------------------------------*/

void OverlayScript::MinGS (ostream& out) {
    if (!DefaultGS()) {
	Clipboard* cb = GetGSList();
	if (cb) 
	    out << " :gs " << MatchedGS(cb);
	else {
	    FillBg(out);
	    Brush(out);
	    FgColor(out);
	    BgColor(out);
	    Pattern(out);
	}
    }
    Transformation(out);
}

void OverlayScript::FullGS (ostream& out) {
    if (!DefaultGS()) {
	Clipboard* cb = GetGSList();
	if (cb) 
	    out << " :gs " << MatchedGS(cb);
	else {
	    FillBg(out);
	    Brush(out);
	    FgColor(out);
	    BgColor(out);
	    Font(out);
	    Pattern(out);
	} 
    }
    Transformation(out);
}

void OverlayScript::TextGS (ostream& out) {
    if (!DefaultGS()) {
	Clipboard* cb = GetGSList();
	if (cb) 
	    out << " :gs " << MatchedGS(cb);
	else {
	    FillBg(out);
	    FgColor(out);
	    Font(out);
	} 
    }
    Transformation(out);
}

void OverlayScript::StencilGS (ostream& out) {
    if (!DefaultGS()) {
	Clipboard* cb = GetGSList();
	if (cb) 
	    out << " :gs " << MatchedGS(cb);
	else {
	    FgColor(out);
	    BgColor(out);
	} 
    }
    Transformation(out);
}

void OverlayScript::Attributes(ostream& out) {
    AttributeList* attrlist = GetOverlayComp()->GetAttributeList();
    out << *attrlist;
}

Clipboard* OverlayScript::GetGSList() {
    OverlayScript* curr = this;
    OverlayScript* parent = (OverlayScript*) GetParent();
    while (parent != nil) {
	curr = parent;
	parent = (OverlayScript*) curr->GetParent();
    }
    return curr != this ? curr->GetGSList() : nil;
}

Clipboard* OverlayScript::GetPtsList() {
    OverlayScript* curr = this;
    OverlayScript* parent = (OverlayScript*) GetParent();
    while (parent != nil) {
	curr = parent;
	parent = (OverlayScript*) curr->GetParent();
    }
    return curr != this ? curr->GetPtsList() : nil;
}

Clipboard* OverlayScript::GetPicList() {
    OverlayScript* curr = this;
    OverlayScript* parent = (OverlayScript*) GetParent();
    while (parent != nil) {
	curr = parent;
	parent = (OverlayScript*) curr->GetParent();
    }
    return curr != this ? curr->GetPicList() : nil;
}

/*****************************************************************************/

ClassId OverlaysScript::GetClassId () { return OVERLAYS_SCRIPT; }

boolean OverlaysScript::IsA (ClassId id) {
    return OVERLAYS_SCRIPT == id || OverlayScript::IsA(id);
}

OverlaysScript::OverlaysScript (OverlaysComp* subj) : OverlayScript(subj) {
    _views = new UList;
}

OverlaysScript::~OverlaysScript () {
    DeleteViews();
    delete _views;
}

boolean OverlaysScript::Definition (ostream& out) {
    Iterator i;
    boolean status = true;

    Clipboard* cb = GetPicList();
    if (cb) {
	out << "picture( :pic " << MatchedPic(cb);
    } else {

	out << "picture(\n";

	static int readonly_symval = symbol_add("readonly");
	boolean outflag = false;
	for (First(i); status && !Done(i); ) {
	    OverlayScript* sv = (OverlayScript*)GetView(i);
	    AttributeList* al;
	    boolean readonly = false;
	    if (al = sv->GetOverlayComp()->attrlist()) {
	      AttributeValue* av = al->find(readonly_symval);
	      if (av) readonly = av->is_true();
	    }
	    if (!readonly) {
	      if (outflag) out << "\n";
	      Indent(out);
	      status = sv->Definition(out);
	      outflag = true;
	    }
	    Next(i);
	}
    }

    if (!cb) {
	out << "\n";
	Indent(out);
        FullGS(out);
	Annotation(out);
	Attributes(out);
    } else {
	Transformation(out);
    }
    out << ")";

    return status;
}    

boolean OverlaysScript::EmitGS(ostream& out, Clipboard* cb, boolean prevout) {
    prevout = OverlayScript::EmitGS(out, cb, prevout);
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitGS(out, cb, prevout);
    }
    return prevout;
}

boolean OverlaysScript::EmitPts(ostream& out, Clipboard* cb, boolean prevout) {
    prevout = OverlayScript::EmitPts(out, cb, prevout);
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitPts(out, cb, prevout);
    }
    return prevout;
}

boolean OverlaysScript::EmitPic(ostream& out, Clipboard* cb1, Clipboard* cb2, boolean prevout) {

    if (!GetGraphicComp()->IsA(OVERLAYS_COMP) )
	return prevout;
    
    /* operate on all the children first */
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitPic(out, cb1, cb2, prevout);
    }
    
    if (prevout) 
	out << ",\n    ";
    else
	out << "\n    ";
    out << "pic(\n";
    prevout = true;
    
    boolean status = true;
    for (First(i); status && !Done(i); ) {
	OverlayScript* sv = (OverlayScript*)GetView(i);
	out << "        ";
	status = sv->Definition(out);
	Next(i);
	if (!Done(i)) out << ",\n";
    }
    
    out << ")";
    
    cb1->Append(GetGraphicComp());

    return prevout;
}

int OverlaysScript::read_name(istream& in, char* buf, int bufsiz) {
  char close_paren = ')';
  char key = ':';
  char lookahead;
  ParamList::skip_space(in);
  in.get(lookahead);
  
  /* keyword section */
  if (lookahead == ':' || lookahead == ')') {
    in.putback(lookahead);
    return -1;
  }
  if (lookahead != ',')
    in.putback(lookahead);          
  else
    ParamList::skip_space(in);
  
  ParamList::parse_token(in, buf, bufsiz);
  return 0;
}

int OverlaysScript::read_gsptspic(const char* name, istream& in, OverlaysComp* comps) {
  if (strcmp(name, "gs") == 0) {
    OverlayComp* gscomp = new OverlayComp(in);
    comps->GrowIndexedGS(gscomp->GetGraphic()->Copy());
    delete gscomp;
    return 1;
  }
  
  else if (strcmp(name, "pts") == 0) {
    Coord *x = nil; 
    Coord *y = nil;
    int n = 0;
    ParamList::skip_space(in);
    char ch = in.get();
    if (ch != '(') {
      cerr << "missing (\n";
      return -1;
    }
    int status = ParamList::parse_points(in, x, y, n);
    if (!in.good() || status!= 0) 
      cerr << "bad point list\n";
    else {
      MultiLineObj* mlo = MultiLineObj::make_pts(x, y, n);
      comps->GrowIndexedPts(mlo);
    }
    delete x;
    delete y;
    ParamList::skip_space(in);
    ch = in.get();
    if (ch != ')') {
      cerr << "missing )\n";
      return -1;
    }
    return 1;
  }
  
  else if (strcmp(name, "pic") == 0 ) {
    OverlaysComp* gscomp = new OverlaysComp(in,comps);
    comps->GrowIndexedPic(gscomp);
    return 1;
  }
  return 0;
}

int OverlaysScript::ReadChildren (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  OverlayComp* child = nil;
  OverlaysComp* comps = (OverlaysComp*)addr1;
  char buf1[BUFSIZ];
  char buf2[BUFSIZ];
  char* buf = buf1;
  
  while (in.good()) {
    if (read_name(in, buf, BUFSIZ)) break;

    int status;
    if (status = read_gsptspic(buf, in, comps)) {
      if (status==-1) break;
    }

    else {
      child = read_obj(buf, in, comps);
    }
  
    if (child) {
      if (in.good() && child->valid()) {
	comps->Append(child);
      } else {
	/* report failure even if one child fails */
	if (!*buf && (buf==buf1 ? *buf2 : *buf1)) 
	  cerr << "Error after reading " << (buf==buf1 ? buf2 : buf1) << "\n";
	delete child;
	return -1;
      }
    }
    buf = buf==buf1 ? buf2 : buf1;
  }
  return 0;
}

void OverlaysScript::Update () {
    DeleteViews();

    OverlayComp* comps = GetOverlayComp();
    Iterator i;

    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
        OverlayComp* comp = (OverlayComp*) comps->GetComp(i);
        OverlayScript* sv = CreateOverlayScript(comp);

        if (sv != nil) {
            _views->Append(new UList(sv));
	    SetParent(sv, this);
	}
    }
}

OverlaysComp* OverlaysScript::GetOverlaysComp () {
    return (OverlaysComp*) GetSubject();
}

UList* OverlaysScript::Elem (Iterator i) { return (UList*) i.GetValue(); }
void OverlaysScript::First (Iterator& i) { i.SetValue(_views->First()); }
void OverlaysScript::Last (Iterator& i) { i.SetValue(_views->Last()); }
void OverlaysScript::Next (Iterator& i) { i.SetValue(Elem(i)->Next()); }
void OverlaysScript::Prev (Iterator& i) { i.SetValue(Elem(i)->Prev()); }
boolean OverlaysScript::Done (Iterator i) { return Elem(i) == _views->End(); }
ExternView* OverlaysScript::GetView (Iterator i) { return View(Elem(i)); }

void OverlaysScript::SetView (ExternView* ev, Iterator& i) {
    i.SetValue(_views->Find(ev));
}

OverlayScript* OverlaysScript::GetScript (Iterator i) { return (OverlayScript*) View(Elem(i)); }

void OverlaysScript::SetScript (OverlayScript* ev, Iterator& i) {
    i.SetValue(_views->Find(ev));
}

void OverlaysScript::DeleteView (Iterator& i) {
    UList* doomed = Elem(i);
    ExternView* view = GetView(i);

    Next(i);
    _views->Remove(doomed);
    SetParent(view, nil);
    delete doomed;
    delete view;
}

void OverlaysScript::DeleteViews () {
    Iterator i;

    First(i);
    while (!Done(i)) {
        DeleteView(i);
    }
}    

ComponentView* OverlayScript::GetParent() { return _parent; }
void OverlayScript::SetParent(ComponentView* view, ComponentView* parent) { 
    if (parent && view->IsA(OVERLAY_SCRIPT) && parent->IsA(OVERLAY_SCRIPT))
	((OverlayScript*)view)->_parent = (OverlayScript*) parent;
    else if (!parent && view->IsA(OVERLAY_SCRIPT))
	((OverlayScript*)view)->_parent = nil;
}
	
void OverlaysScript::SetCompactions(boolean gs, boolean pts, boolean pic) {
}

int OverlaysScript::ReadPic (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) { 
    OverlayComp* comp = (OverlayComp*)addr1;
    int id;
    in >> id;
    OverlaysComp* pic = comp->GetIndexedPic(id);
    if (pic) {
	Iterator i;
	for (pic->First(i); !pic->Done(i); pic->Next(i)) 
	    comp->Append((OverlayComp*)pic->GetComp(i)->Copy());
    } else 
	cerr << ":pic reference without pic records\n";
    return in.good() ? 0 : -1;
}

OverlayComp* OverlaysScript::read_obj(const char* name, istream& in, OverlaysComp* comps) {
  OverlayCatalog* catalog = OverlayCatalog::Instance();
  return catalog->ReadComp(name, in, comps);
}
   
/*****************************************************************************/

OverlayIdrawScript::OverlayIdrawScript (OverlayIdrawComp* subj) : OverlaysScript(subj) {
    _gslist = nil;
    _ptslist = nil;
    _piclist1 =  _piclist2 = nil;
    _gs_compacted = _pts_compacted = _pic_compacted = false;
    _by_pathname = true;
}

OverlayIdrawScript::~OverlayIdrawScript() {
    delete _gslist;
    delete _ptslist;
    delete _piclist1;
    delete _piclist2;
}

ClassId OverlayIdrawScript::GetClassId () { return OVERLAY_IDRAW_SCRIPT; }

boolean OverlayIdrawScript::IsA (ClassId id) { 
    return OVERLAY_IDRAW_SCRIPT == id || OverlaysScript::IsA(id);
}

boolean OverlayIdrawScript::Emit (ostream& out) {
    out << "drawtool(";

    /* make list and output unique point lists */
    boolean prevout = false;
    if (_pts_compacted) {
	_ptslist = new Clipboard();
	prevout = EmitPts(out, _ptslist, prevout);
    }

    /* make list and output unique graphic states */
    if (_gs_compacted) {
	_gslist = new Clipboard();
	prevout = EmitGS(out, _gslist, prevout);
    }

    /* make list and output unique picture graphics */
    if (_pic_compacted) {
	_piclist1 = new Clipboard();
	_piclist2 = new Clipboard();
	prevout = EmitPic(out, _piclist1, _piclist2, prevout);
    }

    /* output graphic components */
    boolean status = true;
    Iterator i;
    First(i);
    if (!Done(i) ) {
	if (prevout) out << ",";
	out << "\n";
    }

    static int readonly_symval = symbol_add("readonly");
    for (; status && !Done(i); ) {
	OverlayScript* ev = (OverlayScript*)GetView(i);
	boolean readonly = false;
	AttributeList *al;
	if (al = ev->GetOverlayComp()->attrlist()) {
	  AttributeValue* av = al->find(readonly_symval);
	  if (av) readonly = av->is_true();
	}
	if (!readonly) {
	  Indent(out);
	  status = ev->Definition(out);
	}
	Next(i);
	if (!Done(i) && !readonly) out << ",\n";
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")\n";
    return status;
}


boolean OverlayIdrawScript::EmitPic(ostream& out, Clipboard* cb1, Clipboard* cb2, boolean prevout) {
    Iterator i;
    for (First(i); !Done(i); Next(i)) {
	prevout = GetScript(i)->EmitPic(out, cb1, cb2, prevout);
    }
    return prevout;
}

Clipboard* OverlayIdrawScript::GetGSList() {
    return _gslist;
}

Clipboard* OverlayIdrawScript::GetPtsList() {
    return _ptslist;
}

Clipboard* OverlayIdrawScript::GetPicList() {
    return _piclist1;
}

void OverlayIdrawScript::SetCompactions(boolean gs, boolean pts, boolean pic) {
    _gs_compacted = gs;
    _pts_compacted = pts;
    _pic_compacted = pic;
}

void OverlayIdrawScript::SetByPathnameFlag(boolean flag) {
    _by_pathname = flag;
}

boolean OverlayIdrawScript::GetByPathnameFlag() {
    return _by_pathname;
}

