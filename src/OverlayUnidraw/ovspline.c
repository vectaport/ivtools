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
 * Overlay Spline component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/ovviewer.h>

#include <IVGlyph/observables.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Graphic/splines.h>
#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubverts.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <stream.h>

/****************************************************************************/

ParamList* SplineOvComp::_ovspline_params = nil;
ParamList* ClosedSplineOvComp::_ovclosed_spline_params = nil;
int SplineOvComp::_symid = -1;

ClassId SplineOvComp::GetClassId () { return OVSPLINE_COMP; }

boolean SplineOvComp::IsA (ClassId id) {
    return OVSPLINE_COMP == id || VerticesOvComp::IsA(id);
}

Component* SplineOvComp::Copy () {
    SplineOvComp* comp =
      new SplineOvComp((SFH_OpenBSpline*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

SplineOvComp::SplineOvComp (SFH_OpenBSpline* graphic, OverlayComp* parent) 
: VerticesOvComp(graphic, parent) { }

SplineOvComp::SplineOvComp(istream& in, OverlayComp* parent) 
: VerticesOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* SplineOvComp::GetParamList() {
    if (!_ovspline_params) 
	GrowParamList(_ovspline_params = new ParamList());
    return _ovspline_params;
}

void SplineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, &SplineScript::ReadPoints,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

SFH_OpenBSpline* SplineOvComp::GetSpline () {
    return (SFH_OpenBSpline*) GetGraphic();
}

/****************************************************************************/

SplineOvView::SplineOvView (SplineOvComp* subj) : VerticesOvView(subj) { }
SplineOvComp* SplineOvView::GetSplineOvComp () { return (SplineOvComp*) GetSubject(); }
ClassId SplineOvView::GetClassId () { return OVSPLINE_VIEW; }

boolean SplineOvView::IsA (ClassId id) {
    return OVSPLINE_VIEW == id || VerticesOvView::IsA(id);
}

boolean SplineOvView::VertexChanged () { 
    SFH_OpenBSpline* gview = (SFH_OpenBSpline*) GetGraphic();
    SFH_OpenBSpline* gsubj = (SFH_OpenBSpline*) GetSplineOvComp()->GetGraphic();

    return *gview != *gsubj;
}

Manipulator* SplineOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
        v->Constrain(e.x, e.y);
        Coord x[1], y[1];
        x[0] = e.x;
        y[0] = e.y;
        GrowingVertices* rub = new GrowingBSpline(
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
        GrowingBSpline* rub = new GrowingBSpline(
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

Command* SplineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_ospl);
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
            SFH_OpenBSpline* spline = new SFH_OpenBSpline(x, y, n, pg);

            if (brVar != nil) spline->SetBrush(brVar->GetBrush());
            if (patVar != nil) spline->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        spline->FillBg(!colVar->GetBgColor()->None());
                spline->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            spline->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new SplineOvComp(spline)));
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

        SFH_OpenBSpline* spline = new SFH_OpenBSpline(x, y, n, GetGraphic());
	delete x;
	delete y;
        spline->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new SplineOvComp(spline));

    } else {
        cmd = VerticesOvView::InterpretManipulator(m);
    }
    return cmd;
}

/*****************************************************************************/

ClassId SplinePS::GetClassId () { return SPLINE_PS; }

boolean SplinePS::IsA (ClassId id) { 
    return SPLINE_PS == id || VerticesPS::IsA(id);
}

SplinePS::SplinePS (OverlayComp* subj) : VerticesPS(subj) { }
const char* SplinePS::Name () { return "BSpl"; }

/*****************************************************************************/

ClassId SplineScript::GetClassId () { return SPLINE_SCRIPT; }

boolean SplineScript::IsA (ClassId id) { 
    return SPLINE_SCRIPT == id || VerticesScript::IsA(id);
}

SplineScript::SplineScript (SplineOvComp* subj) : VerticesScript(subj) { }
const char* SplineScript::Name () { return "bspline"; }

int SplineScript::ReadPoints (istream& in, void* addr1, 
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
        cerr << "abnormal exit from SplineScript::ReadPoints\n";
	return -1;
    }
    else {
        *(SFH_OpenBSpline**)addr1 = new SFH_OpenBSpline(x, y, n);
	delete x;
	delete y;
        return 0;
    }
}


/*****************************************************************************/

int ClosedSplineOvComp::_symid = -1;

ClassId ClosedSplineOvComp::GetClassId () { return OVCLOSEDSPLINE_COMP; }

boolean ClosedSplineOvComp::IsA (ClassId id) {
    return OVCLOSEDSPLINE_COMP == id || VerticesOvComp::IsA(id);
}

Component* ClosedSplineOvComp::Copy () {
    ClosedSplineOvComp* comp = 
      new ClosedSplineOvComp((SFH_ClosedBSpline*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

ClosedSplineOvComp::ClosedSplineOvComp (
    SFH_ClosedBSpline* graphic, OverlayComp* parent
) : VerticesOvComp(graphic, parent) { }

ClosedSplineOvComp::ClosedSplineOvComp(istream& in, OverlayComp* parent) 
: VerticesOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* ClosedSplineOvComp::GetParamList() {
    if (!_ovclosed_spline_params) 
	GrowParamList(_ovclosed_spline_params = new ParamList());
    return _ovclosed_spline_params;
}

void ClosedSplineOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, &ClosedSplineScript::ReadPoints,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

SFH_ClosedBSpline* ClosedSplineOvComp::GetClosedSpline () {
    return (SFH_ClosedBSpline*) GetGraphic();
}

/****************************************************************************/

ClosedSplineOvView::ClosedSplineOvView (
    ClosedSplineOvComp* subj
) : VerticesOvView(subj) { }

ClosedSplineOvComp* ClosedSplineOvView::GetClosedSplineOvComp () { 
    return (ClosedSplineOvComp*) GetSubject();
}

ClassId ClosedSplineOvView::GetClassId () { return OVCLOSEDSPLINE_VIEW; }

boolean ClosedSplineOvView::IsA (ClassId id) {
    return OVCLOSEDSPLINE_VIEW == id || VerticesOvView::IsA(id);
}

boolean ClosedSplineOvView::VertexChanged () { 
    SFH_ClosedBSpline* gview = (SFH_ClosedBSpline*) GetGraphic();
    SFH_ClosedBSpline* gsubj
        = (SFH_ClosedBSpline*) GetClosedSplineOvComp()->GetGraphic();

    return gview->GetOriginal() != gsubj->GetOriginal();
}

Manipulator* ClosedSplineOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
        v->Constrain(e.x, e.y);
        Coord x[1], y[1];
        x[0] = e.x;
        y[0] = e.y;
        GrowingVertices* rub = new GrowingClosedBSpline(
            nil, nil, x, y, 1, -1, HANDLE_SIZE
        );
        m = new VertexManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else if (tool->IsA(RESHAPE_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
	Coord* x, *y;
	int n;

        v->Constrain(e.x, e.y);
	GetVertices(x, y, n);
        GrowingClosedBSpline* rub = new GrowingClosedBSpline(
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

Command* ClosedSplineOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_cspl);
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
            SFH_ClosedBSpline* cbs = new SFH_ClosedBSpline(x, y, n, pg);

            if (brVar != nil) cbs->SetBrush(brVar->GetBrush());
            if (patVar != nil) cbs->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        cbs->FillBg(!colVar->GetBgColor()->None());
                cbs->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            cbs->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new ClosedSplineOvComp(cbs)));
        }
        delete x;
        delete y;

    } else if (tool->IsA(RESHAPE_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_cspl);
        Coord* x, *y;
        int n, pt;
        gv->RemoveVertex();
        gv->GetCurrent(x, y, n, pt);

        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }

        SFH_ClosedBSpline* cbs = new SFH_ClosedBSpline(x, y, n, GetGraphic());
	delete x;
	delete y;
        cbs->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new ClosedSplineOvComp(cbs));

    } else {
        cmd = VerticesOvView::InterpretManipulator(m);
    }
    return cmd;
}

/*****************************************************************************/

ClassId ClosedSplinePS::GetClassId () { return CLOSEDSPLINE_PS; }

boolean ClosedSplinePS::IsA (ClassId id) { 
    return CLOSEDSPLINE_PS == id || VerticesPS::IsA(id);
}

ClosedSplinePS::ClosedSplinePS (OverlayComp* subj) : VerticesPS(subj) { }
const char* ClosedSplinePS::Name () { return "CBSpl"; }

/*****************************************************************************/

ClassId ClosedSplineScript::GetClassId () { return CLOSEDSPLINE_SCRIPT; }

boolean ClosedSplineScript::IsA (ClassId id) { 
    return CLOSEDSPLINE_SCRIPT == id || VerticesScript::IsA(id);
}

ClosedSplineScript::ClosedSplineScript (ClosedSplineOvComp* subj) : VerticesScript(subj) { }
const char* ClosedSplineScript::Name () { return "closedspline"; }

int ClosedSplineScript::ReadPoints (istream& in, void* addr1, 
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
        cerr << "abnormal exit from ClosedSplineScript::ReadPoints\n";
	return -1;
    }
    else {
        *(SFH_ClosedBSpline**)addr1 = new SFH_ClosedBSpline(x, y, n);
	delete x;
	delete y;
        return 0;
    }
}
