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
 * Overlay Line and MultiLine component definitions.
 */

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovmanips.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/paramlist.h>

#include <IVGlyph/observables.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/Graphic/lines.h>

#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubline.h>
#include <IV-2_6/InterViews/rubverts.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <math.h>
#include <stream.h>

#include <fstream>
#include <iostream>

using std::cerr;

/*****************************************************************************/

ParamList* LineOvComp::_ovline_params = nil;
ParamList* MultiLineOvComp::_ovmultiline_params = nil;
int LineOvComp::_symid = -1;

ClassId LineOvComp::GetClassId () { return OVLINE_COMP; }

boolean LineOvComp::IsA (ClassId id) {
    return OVLINE_COMP == id || OverlayComp::IsA(id);
}

Component* LineOvComp::Copy () {
    LineOvComp* comp =
      new LineOvComp((Line*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

LineOvComp::LineOvComp (Line* graphic, OverlayComp* parent) 
: OverlayComp(graphic, parent) { }

LineOvComp::LineOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* LineOvComp::GetParamList() {
    if (!_ovline_params) 
	GrowParamList(_ovline_params = new ParamList());
    return _ovline_params;
}

void LineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("original", ParamStruct::required, &LineScript::ReadOriginal,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

Line* LineOvComp::GetLine () { return (Line*) GetGraphic(); }

void LineOvComp::Interpret (Command* cmd) {
    if (!cmd->IsA(PATTERN_CMD)) {
        OverlayComp::Interpret(cmd);
    }
}

boolean LineOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    Line* linea = GetLine();
    Line* lineb = ((LineOvComp&)comp).GetLine();
    IntCoord ax0, ay0, ax1, ay1;
    IntCoord bx0, by0, bx1, by1;
    linea->GetOriginal(ax0, ay0, ax1, ay1);
    lineb->GetOriginal(bx0, by0, bx1, by1);
    return 
	ax0 = bx0 && ay0 == by0 && ax1 == bx1 && ay1 == by1 &&
	OverlayComp::operator==(comp);
}

/****************************************************************************/

LineOvComp* LineOvView::GetLineOvComp () { return (LineOvComp*) GetSubject(); }
ClassId LineOvView::GetClassId () { return OVLINE_VIEW; }

boolean LineOvView::IsA (ClassId id) {
    return OVLINE_VIEW == id || OverlayView::IsA(id);
}

LineOvView::LineOvView (LineOvComp* subj) : OverlayView(subj) { }

void LineOvView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        Line* line = (Line*) GetGraphic();
        Transformer total;
        line->TotalTransformation(total);

        Coord x0, y0, x1, y1;
        float tx0, ty0;

        line->GetOriginal(x0, y0, x1, y1);
        total.Transform(float(x0), float(y0), tx0, ty0);
        ((AlignToGridCmd*) cmd)->Align(this, tx0, ty0);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void LineOvView::Update () {
    Graphic* line = GetGraphic();

    IncurDamage(line);
    *line = *GetLineOvComp()->GetGraphic();
    IncurDamage(line);
    EraseHandles();
}

void LineOvView::CreateHandles () {
    Coord x[2], y[2];
    Viewer* v = GetViewer();
    
    if (v != nil) {
        GetEndpoints(x[0], y[0], x[1], y[1]);
        _handles = new RubberHandles(nil, nil, x, y, 2, 0, HANDLE_SIZE);
        v->InitRubberband(_handles);
    }
}

Manipulator* LineOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Coord x0, y0, x1, y1;
    Rubberband* rub = nil;
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        v->Constrain(e.x, e.y);
        rub = new RubberLine(nil, nil, e.x, e.y, e.x, e.y);
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else if (tool->IsA(MOVE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetEndpoints(x0, y0, x1, y1);
	rub = new SlidingLine(nil, nil, x0, y0, x1, y1, e.x, e.y);
        m = new OpaqueDragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity),
	    GetGraphic()
	);

    } else if (tool->IsA(SCALE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetEndpoints(x0, y0, x1, y1);
        rub = new ScalingLine(nil, nil, x0, y0, x1, y1, (x0+x1)/2, (y0+y1)/2);
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else if (tool->IsA(ROTATE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetEndpoints(x0, y0, x1, y1);
        rub = new RotatingLine(
            nil, nil, x0, y0, x1, y1, (x0+x1)/2, (y0+y1)/2, e.x, e.y
        );
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else if (tool->IsA(RESHAPE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetEndpoints(x0, y0, x1, y1);
	PointObj p1(x0, y0), p2(x1, y1), cp(e.x, e.y);

	if (p1.Distance(cp) < p2.Distance(cp)) {
	    rub = new RubberLine(nil, nil, x1, y1, e.x, e.y);
	} else {
	    rub = new RubberLine(nil, nil, x0, y0, e.x, e.y);
	}
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else {
        m = OverlayView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* LineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        RubberLine* rl = (RubberLine*) dm->GetRubberband();
        Coord x0, y0, x1, y1;
        rl->GetCurrent(x0, y0, x1, y1);

        if (x0 != x1 || y0 != y1) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }

            Graphic* pg = GetGraphicComp()->GetGraphic();
            Line* line = new Line(x0, y0, x1, y1, pg);

            if (brVar != nil) line->SetBrush(brVar->GetBrush());

            if (colVar != nil) {
	        line->FillBg(!colVar->GetBgColor()->None());
                line->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            line->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new LineOvComp(line)));
        }

    } else if (tool->IsA(MOVE_TOOL)) {
        Coord x0, y0, x1, y1, dummy1, dummy2;
        float fx0, fy0, fx1, fy1;

        SlidingLine* sl = (SlidingLine*) dm->GetRubberband();
        sl->GetOriginal(x0, y0, dummy1, dummy2);
        sl->GetCurrent(x1, y1, dummy1, dummy2);

        if (rel != nil) {
            rel->InvTransform(float(x0), float(y0), fx0, fy0);
            rel->InvTransform(float(x1), float(y1), fx1, fy1);
        }
        cmd = new MoveCmd(ed, fx1 - fx0, fy1 - fy0);

    } else if (tool->IsA(SCALE_TOOL)) {
        ScalingLine* sl = (ScalingLine*) dm->GetRubberband();
        float sxy = sl->CurrentScaling();

        cmd = new ScaleCmd(ed, sxy, sxy);

    } else if (tool->IsA(ROTATE_TOOL)) {
        RotatingLine* rl = (RotatingLine*) dm->GetRubberband();
        float angle = rl->CurrentAngle() - rl->OriginalAngle();

        cmd = new RotateCmd(ed, angle);

    } else if (tool->IsA(RESHAPE_TOOL)) {
        RubberLine* rl = (RubberLine*) dm->GetRubberband();
        Coord x0, y0, x1, y1;
        rl->GetCurrent(x0, y0, x1, y1);

        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }
        Line* line = new Line(x0, y0, x1, y1, GetGraphic());
        line->SetTransformer(rel);
        Unref(rel);
	cmd = new ReplaceCmd(ed, new LineOvComp(line));

    } else {
        cmd = OverlayView::InterpretManipulator(m);
    }
    return cmd;
}

void LineOvView::GetEndpoints (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    Line* line = (Line*) GetGraphic();
    Transformer t;

    line->GetOriginal(x0, y0, x1, y1);
    line->TotalTransformation(t);
    t.Transform(x0, y0);
    t.Transform(x1, y1);
}

Graphic* LineOvView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();

    if (graphic == nil) {
        LineOvComp* lineComp = GetLineOvComp();
        graphic = lineComp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

/****************************************************************************/

ClassId LinePS::GetClassId () { return LINE_PS; }

boolean LinePS::IsA (ClassId id) { 
    return LINE_PS == id || OverlayPS::IsA(id);
}

LinePS::LinePS (OverlayComp* subj) : OverlayPS(subj) { }

boolean LinePS::Definition (ostream& out) {
    Coord x0, y0, x1, y1;

    Line* line = (Line*) GetGraphicComp()->GetGraphic();
    line->GetOriginal(x0, y0, x1, y1);

    out << "Begin " << MARK << " Line\n";
    MinGS(out);
    out << MARK << "\n";
    out << x0 << " " << y0 << " " << x1 << " " << y1 << " Line\n";
    out << "End\n\n";

    return out.good();
}    

/****************************************************************************/

ClassId LineScript::GetClassId () { return LINE_SCRIPT; }

boolean LineScript::IsA (ClassId id) { 
    return LINE_SCRIPT == id || OverlayScript::IsA(id);
}

LineScript::LineScript (LineOvComp* subj) : OverlayScript(subj) { }

boolean LineScript::Definition (ostream& out) {
    Coord x0, y0, x1, y1;

    LineOvComp* comp = (LineOvComp*) GetSubject();
    comp->GetLine()->GetOriginal(x0, y0, x1, y1);

    out << "line(";
    out << x0 << "," << y0 << "," << x1 << "," << y1;
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}
 
int LineScript::ReadOriginal (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord x0, y0, x1, y1;
    char delim;

    char ch = in.peek();
    if (ch != ')' && ch != ':')
	in >> x0 >> delim >> y0 >> delim >> x1 >> delim >> y1;
    else
	x0 = y0 = x1 = y1 = 0;

    if (!in.good()) {
	return -1;
    }
    else {
        *(Line**)addr1 = new Line(x0, y0, x1, y1);
        return 0;
    }
}

/****************************************************************************/

int MultiLineOvComp::_symid = -1;

ClassId MultiLineOvComp::GetClassId () { return OVMULTILINE_COMP; }

boolean MultiLineOvComp::IsA (ClassId id) {
    return OVMULTILINE_COMP == id || VerticesOvComp::IsA(id);
}

Component* MultiLineOvComp::Copy () {
    MultiLineOvComp* comp = 
      new MultiLineOvComp((SF_MultiLine*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

MultiLineOvComp::MultiLineOvComp (SF_MultiLine* graphic, OverlayComp* parent) 
: VerticesOvComp(graphic, parent) {}

MultiLineOvComp::MultiLineOvComp(istream& in, OverlayComp* parent) 
: VerticesOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* MultiLineOvComp::GetParamList() {
    if (!_ovmultiline_params) 
	GrowParamList(_ovmultiline_params = new ParamList());
    return _ovmultiline_params;
}

void MultiLineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, &MultiLineScript::ReadPoints,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

SF_MultiLine* MultiLineOvComp::GetOvMultiLine () {
    return (SF_MultiLine*) GetGraphic();
}

/****************************************************************************/

MultiLineOvView::MultiLineOvView (MultiLineOvComp* subj) : VerticesOvView(subj) { }

MultiLineOvComp* MultiLineOvView::GetMultiLineOvComp () { 
    return (MultiLineOvComp*) GetSubject();
}

ClassId MultiLineOvView::GetClassId () { return OVMULTILINE_VIEW; }

boolean MultiLineOvView::IsA (ClassId id) {
    return OVMULTILINE_VIEW == id || VerticesOvView::IsA(id);
}

boolean MultiLineOvView::VertexChanged () { 
    SF_MultiLine* gview = (SF_MultiLine*) GetGraphic();
    SF_MultiLine* gsubj =
        (SF_MultiLine*) GetMultiLineOvComp()->GetGraphic();

    return *gview != *gsubj;
}

Manipulator* MultiLineOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
        v->Constrain(e.x, e.y);
        Coord x[1], y[1];
        x[0] = e.x;
        y[0] = e.y;
        GrowingVertices* rub = new GrowingMultiLine(
            nil, nil, x, y, 1, -1, HANDLE_SIZE
        );
	if (((OverlayViewer*)v)->scribble_pointer()) 
            m = new ScribbleVertexManip(
	        v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	    );
	else
            m = new VertexManip(
	        v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	    );

    } else if (tool->IsA(RESHAPE_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
	Coord* x, *y;
	int n;

        v->Constrain(e.x, e.y);
	GetVertices(x, y, n);
        GrowingMultiLine* rub = new GrowingMultiLine(
            nil, nil, x, y, n, ClosestPoint(x, y, n, e.x, e.y), HANDLE_SIZE
        );
	delete x;
	delete y;

        m = new VertexManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else {
        m = VerticesOvView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* MultiLineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        Coord* x, *y;
        int n, pt;
        gv->GetCurrent(x, y, n, pt);

        if (n > 2 || x[0] != x[1] || y[0] != y[1]) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }

            Graphic* pg = GetGraphicComp()->GetGraphic();
            SF_MultiLine* polygon = new SF_MultiLine(x, y, n, pg);

            if (brVar != nil) polygon->SetBrush(brVar->GetBrush());
            if (patVar != nil) polygon->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        polygon->FillBg(!colVar->GetBgColor()->None());
                polygon->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            polygon->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new MultiLineOvComp(polygon)));
        }
        delete x;
        delete y;

    } else if (tool->IsA(RESHAPE_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        Coord* x, *y;
        int n, pt;
        gv->RemoveVertex();
        gv->GetCurrent(x, y, n, pt);

	if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }

        SF_MultiLine* polygon = new SF_MultiLine(x, y, n, GetGraphic());
	delete x;
	delete y;
        polygon->SetTransformer(rel);
        Unref(rel);
	cmd = new ReplaceCmd(ed, new MultiLineOvComp(polygon));

    } else {
        cmd = VerticesOvView::InterpretManipulator(m);
    }
    return cmd;
}

/*****************************************************************************/

ClassId MultiLinePS::GetClassId () { return MULTILINE_PS; }

boolean MultiLinePS::IsA (ClassId id) { 
    return MULTILINE_PS == id || VerticesPS::IsA(id);
}

MultiLinePS::MultiLinePS (OverlayComp* subj) : VerticesPS(subj) { }
const char* MultiLinePS::Name () { return "MLine"; }

/*****************************************************************************/

ClassId MultiLineScript::GetClassId () { return MULTILINE_SCRIPT; }

boolean MultiLineScript::IsA (ClassId id) { 
    return MULTILINE_SCRIPT == id || VerticesScript::IsA(id);
}

MultiLineScript::MultiLineScript (MultiLineOvComp* subj) : VerticesScript(subj) { }
const char* MultiLineScript::Name () { return "multiline"; }

boolean MultiLineScript::Definition (ostream& out) {
    const Coord* x;
    const Coord* y;
    int n;

    MultiLineOvComp* comp = (MultiLineOvComp*)GetSubject();
    n = comp->GetVertices()->GetOriginal(x, y);

    out << Name() << "(";
    Clipboard* cb = GetPtsList();
    if (cb) {
	out << " :pts " << MatchedPts(cb);
    } else {
	for (int i = 0; i < n; i++) {
	    out << "(" << x[i] << "," << y[i] << ")";
	    if (i+1 < n ) out << ",";
	}
    }
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int MultiLineScript::ReadPoints (istream& in, void* addr1, 
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
        cerr << "abnormal exit from MultiLineScript::ReadPoints\n";
	return -1;
    }
    else {
        *(SF_MultiLine**)addr1 = new SF_MultiLine(x, y, n);
	delete x;
	delete y;
        return 0;
    }
}


