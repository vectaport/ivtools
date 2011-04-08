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
 * Overlay Polygon component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/paramlist.h>
#include <OverlayUnidraw/ovviewer.h>

#include <IVGlyph/observables.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubline.h>
#include <IV-2_6/InterViews/rubverts.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <stream.h>

#include <iostream>
#include <fstream>

using std::cerr;

/****************************************************************************/

ParamList* PolygonOvComp::_ovpolygon_params = nil;
int PolygonOvComp::_symid = -1;

ClassId PolygonOvComp::GetClassId () { return OVPOLYGON_COMP; }

boolean PolygonOvComp::IsA (ClassId id) {
    return OVPOLYGON_COMP == id || VerticesOvComp::IsA(id);
}

Component* PolygonOvComp::Copy () { 
    PolygonOvComp* comp =
      new PolygonOvComp((SF_Polygon*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

PolygonOvComp::PolygonOvComp (SF_Polygon* graphic, OverlayComp* parent) 
: VerticesOvComp(graphic, parent) { }

PolygonOvComp::PolygonOvComp(istream& in, OverlayComp* parent) 
: VerticesOvComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* PolygonOvComp::GetParamList() {
    if (!_ovpolygon_params) 
	GrowParamList(_ovpolygon_params = new ParamList());
    return _ovpolygon_params;
}

void PolygonOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("points", ParamStruct::required, (param_callback)&PolygonScript::ReadPoints,
		  this, &_gr);
    VerticesOvComp::GrowParamList(pl);
    return;
}

SF_Polygon* PolygonOvComp::GetPolygon () {
    return (SF_Polygon*) GetGraphic();
}

/****************************************************************************/

PolygonOvView::PolygonOvView (PolygonOvComp* subj) : VerticesOvView(subj) { }

PolygonOvComp* PolygonOvView::GetPolygonOvComp () { 
    return (PolygonOvComp*) GetSubject();
}

ClassId PolygonOvView::GetClassId () { return OVPOLYGON_VIEW; }

boolean PolygonOvView::IsA (ClassId id) {
    return OVPOLYGON_VIEW == id || VerticesOvView::IsA(id);
}

boolean PolygonOvView::VertexChanged () { 
    SF_Polygon* gview = (SF_Polygon*) GetGraphic();
    SF_Polygon* gsubj = (SF_Polygon*) GetPolygonOvComp()->GetGraphic();

    return *gview != *gsubj;
}

Manipulator* PolygonOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(OverlayKit::mouse_tack);
        v->Constrain(e.x, e.y);
        Coord x[1], y[1];
        x[0] = e.x;
        y[0] = e.y;
        GrowingVertices* rub = new GrowingPolygon(
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
        GrowingPolygon* rub = new GrowingPolygon(
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

Command* PolygonOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_poly);
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
            SF_Polygon* polygon = new SF_Polygon(x, y, n, pg);

            if (brVar != nil) polygon->SetBrush(brVar->GetBrush());
            if (patVar != nil) polygon->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
	        polygon->FillBg(!colVar->GetBgColor()->None());
                polygon->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            polygon->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new PolygonOvComp(polygon)));
        }
        delete x;
        delete y;

    } else if (tool->IsA(RESHAPE_TOOL)) {
        GrowingVertices* gv = (GrowingVertices*) dm->GetRubberband();
        ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(OverlayKit::mouse_poly);
        Coord* x, *y;
        int n, pt;
        gv->RemoveVertex();
        gv->GetCurrent(x, y, n, pt);
        
        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }

        SF_Polygon* polygon = new SF_Polygon(x, y, n, GetGraphic());
	delete x;
	delete y;
        polygon->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new PolygonOvComp(polygon));

    } else {
        cmd = VerticesOvView::InterpretManipulator(m);
    }
    return cmd;
}

/*****************************************************************************/

ClassId PolygonPS::GetClassId () { return POLYGON_PS; }

boolean PolygonPS::IsA (ClassId id) { 
    return POLYGON_PS == id || VerticesPS::IsA(id);
}

PolygonPS::PolygonPS (OverlayComp* subj) : VerticesPS(subj) { }
const char* PolygonPS::Name () { return "Poly"; }

/*****************************************************************************/

ClassId PolygonScript::GetClassId () { return POLYGON_SCRIPT; }

boolean PolygonScript::IsA (ClassId id) { 
    return POLYGON_SCRIPT == id || VerticesScript::IsA(id);
}

PolygonScript::PolygonScript (PolygonOvComp* subj) : VerticesScript(subj) { }
const char* PolygonScript::Name () { return "polygon"; }

int PolygonScript::ReadPoints (istream& in, void* addr1, 
    void* addr2, void* addr3, void* addr4) {
    Coord* x, *y;
    int n, bad;

    char ch = in.peek();
    if (ch != ')' && ch  != ':') 
	bad = ParamList::parse_points(in, x, y, n);
    else {
	x = y = nil;
	n =0;
	bad = 0;
    }

    if (!in.good() || bad) {
	delete x;
	delete y;
        cerr << "abnormal exit from PolygonScript::ReadPoints\n";
	return -1;
    }
    else {
        *(SF_Polygon**)addr1 = new SF_Polygon(x, y, n);
	delete x;
	delete y;
        return 0;
    }
}
