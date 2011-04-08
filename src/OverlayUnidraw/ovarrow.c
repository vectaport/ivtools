/*
 * Copyright (c) 1994,1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * Implementation of ArrowLineOvComp, ArrowMultiLineOvComp, and ArrowSplineOvComp
 * and related classes.
 */

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/paramlist.h>

#include <IVGlyph/observables.h>

#include <UniIdraw/idarrow.h>
#include <UniIdraw/idarrows.h>
#include <UniIdraw/idcmds.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Components/line.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/picture.h>
#include <Unidraw/Graphic/util.h>
#include <Unidraw/Tools/tool.h>

#include <InterViews/rubcurve.h>
#include <InterViews/rubline.h>
#include <InterViews/rubverts.h>
#include <InterViews/transformer.h>

#include <Attribute/attrlist.h>

#include <math.h>
#include <stdio.h>
#include <stream.h>

/****************************************************************************/

ParamList* ArrowLineOvComp::_ovarrow_line_params = nil;
ParamList* ArrowMultiLineOvComp::_ovarrow_multiline_params = nil;
ParamList* ArrowSplineOvComp::_ovarrow_spline_params = nil;
int ArrowLineOvComp::_symid = -1;

ArrowLineOvComp::ArrowLineOvComp (ArrowLine* graphic) : LineOvComp(graphic) { }

ArrowLineOvComp::ArrowLineOvComp(istream& in, OverlayComp* parent) 
: LineOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* ArrowLineOvComp::GetParamList() {
    if (!_ovarrow_line_params) 
	GrowParamList(_ovarrow_line_params = new ParamList());
    return _ovarrow_line_params;
}

void ArrowLineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("original", ParamStruct::required, &ArrowLineScript::ReadOriginal,
		  this, &_gr);
    pl->add_param("arrowscale", ParamStruct::keyword, &ArrowLineScript::ReadScale,
		  this, &_gr);
    pl->add_param("head", ParamStruct::keyword, &ArrowLineScript::ReadHead,
		  this, &_gr);
    pl->add_param("tail", ParamStruct::keyword, &ArrowLineScript::ReadTail,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

ArrowLine* ArrowLineOvComp::GetArrowLine () { return (ArrowLine*) GetGraphic(); }
ClassId ArrowLineOvComp::GetClassId() { return OVARROWLINE_COMP; }

boolean ArrowLineOvComp::IsA (ClassId id) {
    return OVARROWLINE_COMP == id || LineOvComp::IsA(id);
}

Component* ArrowLineOvComp::Copy () {
    ArrowLineOvComp* comp = 
      new ArrowLineOvComp((ArrowLine*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

void ArrowLineOvComp::Interpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowLine* line = GetArrowLine();

	if (line != nil) {
	    ArrowCmd* arrowCmd = (ArrowCmd*) cmd;
	    cmd->Store(this, new _ArrowData(line->Head(), line->Tail()));
	    line->SetArrows(arrowCmd->Head(), arrowCmd->Tail());
	    Notify();
	}

    } else if (cmd->IsA(PATTERN_CMD)) {
	OverlayComp::Interpret(cmd);

    } else {
	LineOvComp::Interpret(cmd);
    }
}

void ArrowLineOvComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowLine* line = GetArrowLine();

	if (line != nil) {
            _ArrowData* ad = (_ArrowData*) cmd->Recall(this);

            if (ad != nil) {
                line->SetArrows(ad->_head, ad->_tail);
                Notify();
            }
	}

    } else {
	LineOvComp::Uninterpret(cmd);
    }
}

boolean ArrowLineOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    ArrowLine* alinea = GetArrowLine();
    ArrowLine* alineb= ((ArrowLineOvComp&)comp).GetArrowLine();
    return alinea->Head() == alineb->Head() &&
	alinea->Tail() == alineb->Tail() &&
	alinea->ArrowScale() == alineb->ArrowScale() &&
	LineOvComp::operator==(comp);
}

/****************************************************************************/

ArrowLineOvComp* ArrowLineOvView::GetArrowLineOvComp () {
    return (ArrowLineOvComp*) GetSubject();
}

ArrowLineOvView::ArrowLineOvView (ArrowLineOvComp* subj) : LineOvView(subj) { }
ClassId ArrowLineOvView::GetClassId () { return OVARROWLINE_VIEW; }

boolean ArrowLineOvView::IsA (ClassId id) {
    return OVARROWLINE_VIEW == id || LineOvView::IsA(id);
}

void ArrowLineOvView::Update () {
    ArrowLine* line = (ArrowLine*) GetGraphic();
    ArrowLine* subj = GetArrowLineOvComp()->GetArrowLine();

    IncurDamage(line);
    *line = *subj;
    IncurDamage(line);
    EraseHandles();
}

Command* ArrowLineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;
    ArrowVar* aVar = (ArrowVar*) ed->GetState("ArrowVar");

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        RubberLine* rl = (RubberLine*) dm->GetRubberband();
        Coord x0, y0, x1, y1;
        rl->GetCurrent(x0, y0, x1, y1);

        if (x0 != x1 || y0 != y1) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }

            ArrowLine* aline = new ArrowLine(
                x0, y0, x1, y1, aVar->Head(), aVar->Tail(), 
                dm->GetViewer()->GetMagnification(), stdgraphic
            );

            if (brVar != nil) aline->SetBrush(brVar->GetBrush());
            if (patVar != nil) { aline->SetPattern(patVar->GetPattern()); }

            if (colVar != nil) {
                aline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }

            aline->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new ArrowLineOvComp(aline)));
        }

    } else if (tool->IsA(RESHAPE_TOOL)) {
        RubberLine* rl = (RubberLine*) dm->GetRubberband();
        Coord epx0, epy0, epx1, epy1;
        GetEndpoints(epx0, epy0, epx1, epy1);

        Coord x0, y0, x1, y1;
        rl->GetCurrent(x0, y0, x1, y1);

        if (x0 == epx1 && y0 == epy1) {
            x0 = x1; y0 = y1;
            x1 = epx1; y1 = epy1;
        }

        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }
        ArrowLine* orig = GetArrowLineOvComp()->GetArrowLine();
        ArrowLine* aline = new ArrowLine(
	    x0, y0, x1, y1, orig->Head(), orig->Tail(),
            dm->GetViewer()->GetMagnification(), GetGraphic()
	);
        aline->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new ArrowLineOvComp(aline));

    } else {
        cmd = LineOvView::InterpretManipulator(m);
    }
    return cmd;
}

/****************************************************************************/

ArrowLinePS::ArrowLinePS (OverlayComp* subj) : LinePS (subj) { }
ClassId ArrowLinePS::GetClassId () { return ARROWLINE_PS; }

boolean ArrowLinePS::IsA (ClassId id) {
    return ARROWLINE_PS == id || LinePS::IsA(id);
}

boolean ArrowLinePS::Definition (ostream& out) {
    ArrowLine* aline = (ArrowLine*) GetGraphicComp()->GetGraphic();

    Coord x0, y0, x1, y1;
    aline->GetOriginal(x0, y0, x1, y1);
    float arrow_scale = aline->ArrowScale();

    out << "Begin " << MARK << " Line\n";
    MinGS(out);
    out << MARK << "\n";
    out << x0 << " " << y0 << " " << x1 << " " << y1 << " Line\n";
    out << MARK << " " << arrow_scale << "\n";
    out << "End\n\n";

    return out.good();
}

// this code is entirely for compatibility with older versions of idraw

void ArrowLinePS::Brush (ostream& out) {
    ArrowLine* arrowline = (ArrowLine*) GetGraphicComp()->GetGraphic();
    PSBrush* brush = (PSBrush*) arrowline->GetBrush();
    boolean head, tail;
    head = arrowline->Head();
    tail = arrowline->Tail();

    if (brush == nil) {
	out << MARK << " b u\n";

    } else if (brush->None()) {
	out << "none SetB " << MARK << " b n\n";

    } else {
	int p = brush->GetLinePattern();
	out << MARK << " b " << p << "\n";

	float w = brush->width();
	out << w << " " << head << " " << tail << " ";

	const int* dashpat = brush->GetDashPattern();
	int dashpatsize = brush->GetDashPatternSize();
	int dashoffset = brush->GetDashOffset();

	if (dashpatsize <= 0) {
	    out << "[] " << dashoffset << " ";
	} else {
	    out << "[";
	    int i;
	    for (i = 0; i < dashpatsize - 1; i++) {
		out << dashpat[i] << " ";
	    }
	    out << dashpat[i] << "] " << dashoffset << " ";
	}
	out << "SetB\n";
    }

}

/****************************************************************************/

ArrowLineScript::ArrowLineScript (ArrowLineOvComp* subj) : LineScript (subj) { }
ClassId ArrowLineScript::GetClassId () { return ARROWLINE_SCRIPT; }

boolean ArrowLineScript::IsA (ClassId id) {
    return ARROWLINE_SCRIPT == id || LineScript::IsA(id);
}

boolean ArrowLineScript::Definition (ostream& out) {
    ArrowLineOvComp* comp = (ArrowLineOvComp*) GetSubject();
    ArrowLine* aline = comp->GetArrowLine();

    Coord x0, y0, x1, y1;
    aline->GetOriginal(x0, y0, x1, y1);
    float arrow_scale = aline->ArrowScale();

    boolean head, tail;
    head = comp->GetArrowLine()->Head();
    tail = comp->GetArrowLine()->Tail();

    out << "arrowline(";
    out << x0 << "," << y0 << "," << x1 << "," << y1;
    if (arrow_scale != 1 )
	out << " :arrowscale " << arrow_scale;
    if (head) 
	out << " :head";
    if (tail)
	out << " :tail";
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int ArrowLineScript::ReadOriginal (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord x0, y0, x1, y1;
    char delim;

    char ch = in.peek();
    if (ch!=')' && ch!=':') 
	in >> x0 >> delim >> y0 >> delim >> x1 >> delim >> y1;
    else
	x0 = y0 = x1 = y1 = 0;
    if (!in.good()) {
	return -1;
    }
    else {
        *(ArrowLine**)addr1 = new ArrowLine(x0, y0, x1, y1, false, false, 1);
        return 0;
    }
}

int ArrowLineScript::ReadScale (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    float scale;

    ParamList::skip_space(in);
    in >> scale;

    if (!in.good()) {
	return -1;
    }
    else {
        ArrowLine* gs = *(ArrowLine**)addr1;
        gs->ScaleArrows(scale);
        return 0;
    }
}

int ArrowLineScript::ReadHead (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    if (!in.good()) {
	return -1;
    }
    else {
        ArrowLine* gs = *(ArrowLine**)addr1;
        gs->SetArrows(true, gs->Tail());
        return 0;
    }
}

int ArrowLineScript::ReadTail (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    if (!in.good()) {
	return -1;
    }
    else {
        ArrowLine* gs = *(ArrowLine**)addr1;
        gs->SetArrows(gs->Head(), true);
        return 0;
    }
}

/****************************************************************************/

int ArrowMultiLineOvComp::_symid = -1;

ArrowMultiLineOvComp::ArrowMultiLineOvComp (ArrowMultiLine* g) : MultiLineOvComp(g){}

ArrowMultiLineOvComp::ArrowMultiLineOvComp(istream& in, OverlayComp* parent) 
: MultiLineOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* ArrowMultiLineOvComp::GetParamList() {
    if (!_ovarrow_multiline_params) 
	GrowParamList(_ovarrow_multiline_params = new ParamList());
    return _ovarrow_multiline_params;
}

void ArrowMultiLineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, &ArrowMultiLineScript::ReadPoints,
		  this, &_gr);
    pl->add_param("arrowscale", ParamStruct::keyword, &ArrowMultiLineScript::ReadScale,
		  this, &_gr);
    pl->add_param("head", ParamStruct::keyword, &ArrowMultiLineScript::ReadHead,
		  this, &_gr);
    pl->add_param("tail", ParamStruct::keyword, &ArrowMultiLineScript::ReadTail,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

ArrowMultiLine* ArrowMultiLineOvComp::GetArrowMultiLine () {
    return (ArrowMultiLine*) GetGraphic();
}

ClassId ArrowMultiLineOvComp::GetClassId() { return OVARROWMULTILINE_COMP; }

boolean ArrowMultiLineOvComp::IsA (ClassId id) {
    return OVARROWMULTILINE_COMP == id || MultiLineOvComp::IsA(id);
}

Component* ArrowMultiLineOvComp::Copy () {
    ArrowMultiLineOvComp* comp = 
      new ArrowMultiLineOvComp((ArrowMultiLine*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

void ArrowMultiLineOvComp::Interpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowMultiLine* amline = GetArrowMultiLine();

	if (amline != nil) {
	    ArrowCmd* arrowCmd = (ArrowCmd*) cmd;
	    cmd->Store(this, new _ArrowData(amline->Head(), amline->Tail()));
	    amline->SetArrows(arrowCmd->Head(), arrowCmd->Tail());
	    Notify();
	}

    } else if (cmd->IsA(PATTERN_CMD)) {
	OverlayComp::Interpret(cmd);

    } else {
	MultiLineOvComp::Interpret(cmd);
    }
}

void ArrowMultiLineOvComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowMultiLine* amline = GetArrowMultiLine();

	if (amline != nil) {
            _ArrowData* ad = (_ArrowData*) cmd->Recall(this);

            if (ad != nil) {
                amline->SetArrows(ad->_head, ad->_tail);
                Notify();
            }
	}

    } else {
	MultiLineOvComp::Uninterpret(cmd);
    }
}

boolean ArrowMultiLineOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    ArrowMultiLine* amlinea = GetArrowMultiLine();
    ArrowMultiLine* amlineb= ((ArrowMultiLineOvComp&)comp).GetArrowMultiLine();
    return amlinea->Head() == amlineb->Head() &&
	amlinea->Tail() == amlineb->Tail() &&
	amlinea->ArrowScale() == amlineb->ArrowScale() &&
	VerticesOvComp::operator==(comp);
}

/****************************************************************************/

ArrowMultiLineOvComp* ArrowMultiLineOvView::GetArrowMultiLineOvComp () {
    return (ArrowMultiLineOvComp*) GetSubject();
}

ArrowMultiLineOvView::ArrowMultiLineOvView (
    ArrowMultiLineOvComp* s
) : MultiLineOvView(s) { }

ClassId ArrowMultiLineOvView::GetClassId () { return OVARROWMULTILINE_VIEW; }

boolean ArrowMultiLineOvView::IsA (ClassId id) {
    return OVARROWMULTILINE_VIEW == id || MultiLineOvView::IsA(id);
}

void ArrowMultiLineOvView::Update () {
    ArrowMultiLine* amline = (ArrowMultiLine*) GetGraphic();
    ArrowMultiLine* subj = GetArrowMultiLineOvComp()->GetArrowMultiLine();

    IncurDamage(amline);
    *amline = *subj;
    IncurDamage(amline);
    EraseHandles();
}

Command* ArrowMultiLineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;
    ArrowVar* aVar = (ArrowVar*) ed->GetState("ArrowVar");

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_mlin);
        Coord* x, *y;
        int n;
        gv->GetCurrent(x, y, n);

        if (n > 2 || x[0] != x[1] || y[0] != y[1]) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }
            ArrowMultiLine* aml = new ArrowMultiLine(
                x, y, n, aVar->Head(), aVar->Tail(), 
                dm->GetViewer()->GetMagnification(), stdgraphic
            );

            if (brVar != nil) aml->SetBrush(brVar->GetBrush());
            if (patVar != nil) aml->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        aml->FillBg(!colVar->GetBgColor()->None());

                aml->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            aml->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new ArrowMultiLineOvComp(aml)));
        }
        delete x;
        delete y;

    } else if (tool->IsA(RESHAPE_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_mlin);
        Coord* x, *y;
        int n, pt;
        gv->RemoveVertex();
        gv->GetCurrent(x, y, n, pt);

	if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }

        ArrowMultiLine* orig = GetArrowMultiLineOvComp()->GetArrowMultiLine();
        ArrowMultiLine* aml = new ArrowMultiLine(
            x, y, n, orig->Head(), orig->Tail(), 
            dm->GetViewer()->GetMagnification(), GetGraphic()
        );
        delete x;
        delete y;
        aml->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new ArrowMultiLineOvComp(aml));

    } else {
        cmd = MultiLineOvView::InterpretManipulator(m);
    }
    return cmd;
}

/****************************************************************************/

ArrowMultiLinePS::ArrowMultiLinePS (OverlayComp* s) : MultiLinePS(s) { }
ClassId ArrowMultiLinePS::GetClassId () { return ARROWLINE_PS; }

boolean ArrowMultiLinePS::IsA (ClassId id) { 
    return ARROWLINE_PS == id || MultiLinePS::IsA(id);
}

boolean ArrowMultiLinePS::Definition (ostream& out) {

    Command* cmd = GetCommand();
    boolean idraw_format = OverlayPS::idraw_format;
    if (cmd) {
      if (cmd->IsA(OVPRINT_CMD)) 
	idraw_format = ((OvPrintCmd*)cmd)->idraw_format();
      else if (cmd->IsA(OV_EXPORT_CMD))
	idraw_format = true;
    }
      
    if (idraw_format) {
	ArrowMultiLineOvComp* comp = (ArrowMultiLineOvComp*) GetSubject();
	ArrowMultiLine* aml = comp->GetArrowMultiLine();
	
	const Coord* x, *y;
	int n = aml->GetOriginal(x, y);
	float arrow_scale = aml->ArrowScale();
	
	out << "Begin " << MARK << " " << Name() << "\n";
	MinGS(out);
	out << MARK << " " << n << "\n";
	for (int i = 0; i < n; i++) {
	    out << x[i] << " " << y[i] << "\n";
	}
	out << n << " " << Name() << "\n";
	out << MARK << " " << arrow_scale << "\n";
	out << "End\n\n";
	
	return out.good();
    }

    ArrowMultiLine* aml = (ArrowMultiLine*) GetGraphicComp()->GetGraphic();

    const Coord* x, *y;
    int numverts = aml->GetOriginal(x, y);
    float arrow_scale = aml->ArrowScale();

    boolean head = aml->Head();
    boolean tail = aml->Tail();
    
    const int limit = 32;
    int cnt = 0;
    for (int v=0; v<numverts; v+=limit-1) {

	int n = min(numverts-cnt,limit);

	if (v==0)
	    aml->SetArrows(head, false);
	else if ( v+limit>=numverts) 
	    aml->SetArrows(false, tail);
	else 
	    aml->SetArrows(false, false);

	out << "Begin " << MARK << " " << Name() << "\n";
	MinGS(out);
	out << MARK << " " << n << "\n";
	for (int i=0; i < n; i++, cnt++) {
	    out << x[cnt] << " " << y[cnt] << "\n";
	}
	out << n << " " << Name() << "\n";
	out << MARK << " " << arrow_scale << "\n";
	out << "End\n\n";

	cnt--;  /* back up so that split lines share a vertex */

    }

    aml->SetArrows(head, tail);

    return out.good();
}

// this code is entirely for compatibility with older versions of idraw

void ArrowMultiLinePS::Brush (ostream& out) {
    ArrowMultiLine* arrowmultiline = (ArrowMultiLine*) GetGraphicComp()->GetGraphic();
    PSBrush* brush = (PSBrush*) arrowmultiline->GetBrush();
    boolean head, tail;
    head = arrowmultiline->Head();
    tail = arrowmultiline->Tail();

    if (brush == nil) {
	out << MARK << " b u\n";

    } else if (brush->None()) {
	out << "none SetB " << MARK << " b n\n";

    } else {
	int p = brush->GetLinePattern();
	out << MARK << " b " << p << "\n";

	float w = brush->width();
	out << w << " " << head << " " << tail << " ";

	const int* dashpat = brush->GetDashPattern();
	int dashpatsize = brush->GetDashPatternSize();
	int dashoffset = brush->GetDashOffset();

	if (dashpatsize <= 0) {
	    out << "[] " << dashoffset << " ";
	} else {
	    out << "[";
	    int i;
	    for (i = 0; i < dashpatsize - 1; i++) {
		out << dashpat[i] << " ";
	    }
	    out << dashpat[i] << "] " << dashoffset << " ";
	}
	out << "SetB\n";
    }
}

/****************************************************************************/

ArrowMultiLineScript::ArrowMultiLineScript (ArrowMultiLineOvComp* s) : MultiLineScript(s) { }
ClassId ArrowMultiLineScript::GetClassId () { return ARROWLINE_SCRIPT; }

boolean ArrowMultiLineScript::IsA (ClassId id) { 
    return ARROWLINE_SCRIPT == id || MultiLineScript::IsA(id);
}

const char* ArrowMultiLineScript::Name () { return "arrowmultiline"; }

boolean ArrowMultiLineScript::Definition (ostream& out) {
    const Coord* x;
    const Coord* y;
    int n;

    ArrowMultiLineOvComp* comp = (ArrowMultiLineOvComp*) GetSubject();
    n = comp->GetVertices()->GetOriginal(x, y);

    ArrowMultiLine* aml = comp->GetArrowMultiLine();
    float arrow_scale = aml->ArrowScale();

    boolean head, tail;
    head = comp->GetArrowMultiLine()->Head();
    tail = comp->GetArrowMultiLine()->Tail();

    out << Name() << "(";
    Clipboard* cb = GetPtsList();
    if (cb) {
	out << " :pts " << MatchedPts(cb);
    } else {
	for (int i = 0; i < n;) {
	    for (int j=0; j < 10 && i < n; j++, i++) {
		out << "(" << x[i] << "," << y[i] << ")";
		if (i+1 < n ) out << ",";
	    }
	    if (i+1 < n ) {
		out << "\n";
		Indent(out);
	    }
	}
    }
    if (arrow_scale != 1 )
	out << " :arrowscale " << arrow_scale;
    if (head) 
	out << " :head";
    if (tail)
	out << " :tail";
    MinGS(out);
    Annotation(out);
    Attributes(out);    
    out << ")";

    return out.good();
}

int ArrowMultiLineScript::ReadPoints (istream& in, void* addr1, 
    void* addr2, void* addr3, void* addr4) {
    Coord* x, *y;
    int n, bad;

    char ch = in.peek();
    if (ch != ')' && ch != ':') 
	bad = ParamList::parse_points(in, x, y, n);
    else {
	x = y = nil;
	n = 0;
	bad = 0;
    }
    if (!in.good() || bad) {
	delete x; 
	delete y;
        cerr << "abnormal exit from ArrowSplineScript::ReadPoints\n";
	return -1;
    }
    else {
        *(ArrowMultiLine**)addr1 = new ArrowMultiLine(x, y, n, false, false, 1);
	delete x; 
	delete y;
        return 0;
    }
}

int ArrowMultiLineScript::ReadScale (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    float scale;

    ParamList::skip_space(in);
    in >> scale;

    if (!in.good()) {
        cerr << "abnormal exit from ArrowMultiLineScript::ReadScale\n";
	return -1;
    }
    else {
        ArrowMultiLine* gs = *(ArrowMultiLine**)addr1;
        gs->ScaleArrows(scale);
        return 0;
    }
}

int ArrowMultiLineScript::ReadHead (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    if (!in.good()) {
        cerr << "abnormal exit from ArrowMultiLineScript::ReadHead\n";
	return -1;
    }
    else {
        ArrowMultiLine* gs = *(ArrowMultiLine**)addr1;
        gs->SetArrows(true, gs->Tail());
        return 0;
    }
}

int ArrowMultiLineScript::ReadTail (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    if (!in.good()) {
        cerr << "abnormal exit from ArrowLineScript::ReadTail\n";
	return -1;
    }
    else {
        ArrowMultiLine* gs = *(ArrowMultiLine**)addr1;
        gs->SetArrows(gs->Head(), true);
        return 0;
    }
}

/****************************************************************************/

int ArrowSplineOvComp::_symid = -1;

ArrowSplineOvComp::ArrowSplineOvComp (ArrowOpenBSpline* g) : SplineOvComp(g) {}


ArrowSplineOvComp::ArrowSplineOvComp(istream& in, OverlayComp* parent) 
: SplineOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* ArrowSplineOvComp::GetParamList() {
    if (!_ovarrow_spline_params) 
	GrowParamList(_ovarrow_spline_params = new ParamList());
    return _ovarrow_spline_params;
}

void ArrowSplineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, &ArrowSplineScript::ReadPoints,
		  this, &_gr);
    pl->add_param("arrowscale", ParamStruct::keyword, &ArrowSplineScript::ReadScale,
		  this, &_gr);
    pl->add_param("head", ParamStruct::keyword, &ArrowSplineScript::ReadHead,
		  this, &_gr);
    pl->add_param("tail", ParamStruct::keyword, &ArrowSplineScript::ReadTail,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

ArrowOpenBSpline* ArrowSplineOvComp::GetArrowOpenBSpline () {
    return (ArrowOpenBSpline*) GetGraphic();
}

ClassId ArrowSplineOvComp::GetClassId() { return OVARROWSPLINE_COMP; }

boolean ArrowSplineOvComp::IsA (ClassId id) {
    return OVARROWSPLINE_COMP == id || SplineOvComp::IsA(id);
}

Component* ArrowSplineOvComp::Copy () {
    ArrowSplineOvComp* comp = 
      new ArrowSplineOvComp((ArrowOpenBSpline*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

void ArrowSplineOvComp::Interpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowOpenBSpline* amline = GetArrowOpenBSpline();

	if (amline != nil) {
	    ArrowCmd* arrowCmd = (ArrowCmd*) cmd;
	    cmd->Store(this, new _ArrowData(amline->Head(), amline->Tail()));
	    amline->SetArrows(arrowCmd->Head(), arrowCmd->Tail());
	    Notify();
	}

    } else if (cmd->IsA(PATTERN_CMD)) {
	OverlayComp::Interpret(cmd);

    } else {
	SplineOvComp::Interpret(cmd);
    }
}

void ArrowSplineOvComp::Uninterpret (Command* cmd) {
    if (cmd->IsA(ARROW_CMD)) {
	ArrowOpenBSpline* amline = GetArrowOpenBSpline();

	if (amline != nil) {
            _ArrowData* ad = (_ArrowData*) cmd->Recall(this);

            if (ad != nil) {
                amline->SetArrows(ad->_head, ad->_tail);
                Notify();
            }
	}

    } else {
	SplineOvComp::Uninterpret(cmd);
    }
}

boolean ArrowSplineOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    ArrowOpenBSpline* abspla = GetArrowOpenBSpline();
    ArrowOpenBSpline* absplb= ((ArrowSplineOvComp&)comp).GetArrowOpenBSpline();
    return abspla->Head() == absplb->Head() &&
	abspla->Tail() == absplb->Tail() &&
	abspla->ArrowScale() == absplb->ArrowScale() &&
	VerticesOvComp::operator==(comp);
}

/****************************************************************************/

ArrowSplineOvComp* ArrowSplineOvView::GetArrowSplineOvComp () {
    return (ArrowSplineOvComp*) GetSubject();
}

ArrowSplineOvView::ArrowSplineOvView (ArrowSplineOvComp* s) : SplineOvView(s) { }
ClassId ArrowSplineOvView::GetClassId () { return OVARROWSPLINE_VIEW; }

boolean ArrowSplineOvView::IsA (ClassId id) {
    return OVARROWSPLINE_VIEW == id || SplineOvView::IsA(id);
}

void ArrowSplineOvView::Update () {
    ArrowOpenBSpline* amline = (ArrowOpenBSpline*) GetGraphic();
    ArrowOpenBSpline* subj = GetArrowSplineOvComp()->GetArrowOpenBSpline();

    IncurDamage(amline);
    *amline = *subj;
    IncurDamage(amline);
    EraseHandles();
}

Command* ArrowSplineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;
    ArrowVar* aVar = (ArrowVar*) ed->GetState("ArrowVar");

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_ospl);
        Coord* x, *y;
        int n;
        gv->GetCurrent(x, y, n);

        if (n > 2 || x[0] != x[1] || y[0] != y[1]) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }
            ArrowOpenBSpline* aml = new ArrowOpenBSpline(
                x, y, n, aVar->Head(), aVar->Tail(), 
                dm->GetViewer()->GetMagnification(), stdgraphic
            );

            if (brVar != nil) aml->SetBrush(brVar->GetBrush());
            if (patVar != nil) aml->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        aml->FillBg(!colVar->GetBgColor()->None());
                aml->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            aml->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new ArrowSplineOvComp(aml)));
        }
        delete x;
        delete y;

    } else if (tool->IsA(RESHAPE_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_ospl);
        Coord* x, *y;
        int n, pt;
        gv->RemoveVertex();
        gv->GetCurrent(x, y, n, pt);

        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }

        ArrowOpenBSpline* orig = GetArrowSplineOvComp()->GetArrowOpenBSpline();
        ArrowOpenBSpline* aml = new ArrowOpenBSpline(
            x, y, n, orig->Head(), orig->Tail(), 
            dm->GetViewer()->GetMagnification(), GetGraphic()
        );
        delete x;
        delete y;
        aml->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new ArrowSplineOvComp(aml));

    } else {
        cmd = SplineOvView::InterpretManipulator(m);
    }
    return cmd;
}

/****************************************************************************/

ArrowSplinePS::ArrowSplinePS (OverlayComp* s) : SplinePS(s) { }
ClassId ArrowSplinePS::GetClassId () { return ARROWLINE_PS; }

boolean ArrowSplinePS::IsA (ClassId id) { 
    return ARROWLINE_PS == id || SplinePS::IsA(id);
}

boolean ArrowSplinePS::Definition (ostream& out) {
    ArrowOpenBSpline* aml = (ArrowOpenBSpline*) GetGraphicComp()->GetGraphic();

    const Coord* x, *y;
    int n = aml->GetOriginal(x, y);
    float arrow_scale = aml->ArrowScale();

    out << "Begin " << MARK << " " << Name() << "\n";
    MinGS(out);
    out << MARK << " " << n << "\n";
    for (int i = 0; i < n; i++) {
        out << x[i] << " " << y[i] << "\n";
    }
    out << n << " " << Name() << "\n";
    out << MARK << " " << arrow_scale << "\n";
    out << "End\n\n";

    return out.good();
}

// this code is entirely for compatibility with older versions of idraw

void ArrowSplinePS::Brush (ostream& out) {
    ArrowOpenBSpline* arrowspline = (ArrowOpenBSpline*) GetGraphicComp()->GetGraphic();
    PSBrush* brush = (PSBrush*) arrowspline->GetBrush();
    boolean head, tail;
    head = arrowspline->Head();
    tail = arrowspline->Tail();

    if (brush == nil) {
	out << MARK << " b u\n";

    } else if (brush->None()) {
	out << "none SetB " << MARK << " b n\n";

    } else {
	int p = brush->GetLinePattern();
	out << MARK << " b " << p << "\n";

	float w = brush->width();
	out << w << " " << head << " " << tail << " ";

	const int* dashpat = brush->GetDashPattern();
	int dashpatsize = brush->GetDashPatternSize();
	int dashoffset = brush->GetDashOffset();

	if (dashpatsize <= 0) {
	    out << "[] " << dashoffset << " ";
	} else {
	    out << "[";
	    int i;
	    for (i = 0; i < dashpatsize - 1; i++) {
		out << dashpat[i] << " ";
	    }
	    out << dashpat[i] << "] " << dashoffset << " ";
	}
	out << "SetB\n";
    }
}

/****************************************************************************/

ArrowSplineScript::ArrowSplineScript (ArrowSplineOvComp* s) : SplineScript(s) { }

ClassId ArrowSplineScript::GetClassId () { return ARROWLINE_SCRIPT; }

boolean ArrowSplineScript::IsA (ClassId id) { 
    return ARROWLINE_SCRIPT == id || SplineScript::IsA(id);
}

const char* ArrowSplineScript::Name () { return "arrowspline"; }

boolean ArrowSplineScript::Definition (ostream& out) {
    const Coord* x;
    const Coord* y;
    int n;

    ArrowSplineOvComp* comp = (ArrowSplineOvComp*) GetSubject();
    n = comp->GetVertices()->GetOriginal(x, y);

    ArrowOpenBSpline* aml = comp->GetArrowOpenBSpline();
    float arrow_scale = aml->ArrowScale();

    boolean head, tail;
    head = comp->GetArrowOpenBSpline()->Head();
    tail = comp->GetArrowOpenBSpline()->Tail();

    out << Name() << "(";
    Clipboard* cb = GetPtsList();
    if (cb) {
	out << " :pts " << MatchedPts(cb);
    } else {
	for (int i = 0; i < n; ) {
	    for (int j = 0; j < 10 && i < n; j++, i++) {
		out << "(" << x[i] << "," << y[i] << ")";
		if (i+1 < n ) out << ",";
	    }
	    if (i+1 < n ) {
		out << "\n";
		Indent(out);
	    }
	}
    }
    if (arrow_scale != 1 )
	out << " :arrowscale " << arrow_scale;
    if (head) 
	out << " :head";
    if (tail)
	out << " :tail";
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int ArrowSplineScript::ReadPoints (istream& in, void* addr1, 
    void* addr2, void* addr3, void* addr4) {
    Coord* x, *y;
    int n, bad;

    char ch = in.peek();
    if (ch != ')' && ch != ':') 
	bad = ParamList::parse_points(in, x, y, n);
    else {
	x = y = nil;
	n = 0;
	bad = 0;
    }
    if (!in.good() || bad) {
	delete x;
	delete y;
        cerr << "abnormal exit from ArrowSplineScript::ReadPoints\n";
	return -1;
    }
    else {
        *(ArrowOpenBSpline**)addr1 = new ArrowOpenBSpline(x, y, n, false, false, 1);
	delete x;
	delete y;
        return 0;
    }
}

int ArrowSplineScript::ReadScale (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    float scale;

    ParamList::skip_space(in);
    in >> scale;

    if (!in.good()) {
        cerr << "abnormal exit from ArrowSplineScript::ReadScale\n";
	return -1;
    }
    else {
        ArrowOpenBSpline* gs = *(ArrowOpenBSpline**)addr1;
        gs->ScaleArrows(scale);
        return 0;
    }
}

int ArrowSplineScript::ReadHead (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    if (!in.good()) {
        cerr << "abnormal exit from ArrowLineScript::ReadHead\n";
	return -1;
    }
    else {
        ArrowOpenBSpline* gs = *(ArrowOpenBSpline**)addr1;
        gs->SetArrows(true, gs->Tail());
        return 0;
    }
}

int ArrowSplineScript::ReadTail (istream& in, void* addr1, void* addr2, 
    void* addr3, void* addr4) {
    if (!in.good()) {
        cerr << "abnormal exit from ArrowLineScript::ReadTail\n";
	return -1;
    }
    else {
        ArrowOpenBSpline* gs = *(ArrowOpenBSpline**)addr1;
        gs->SetArrows(gs->Head(), true);
        return 0;
    }
}




