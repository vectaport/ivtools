/*
 * Copyright (c) 1994 Vectaport Inc.
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
 * Overlay Ellipse component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/paramlist.h>


#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/grid.h>
#include <Unidraw/manips.h>
#include <Unidraw/statevars.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/Graphic/ellipses.h>

#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <stream.h>

/*****************************************************************************/

ParamList* EllipseOvComp::_ovellipse_params = nil;

ClassId EllipseOvComp::GetClassId () { return OVELLIPSE_COMP; }

boolean EllipseOvComp::IsA (ClassId id) {
    return OVELLIPSE_COMP == id || OverlayComp::IsA(id);
}

Component* EllipseOvComp::Copy () {
    return new EllipseOvComp((SF_Ellipse*) GetGraphic()->Copy());
}

EllipseOvComp::EllipseOvComp (SF_Ellipse* graphic) : OverlayComp(graphic) { }

EllipseOvComp::EllipseOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* EllipseOvComp::GetParamList() {
    if (!_ovellipse_params) 
	GrowParamList(_ovellipse_params = new ParamList());
    return _ovellipse_params;
}

void EllipseOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("original", ParamStruct::required, &EllipseScript::ReadOriginal,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

SF_Ellipse* EllipseOvComp::GetEllipse () { return (SF_Ellipse*) GetGraphic(); }

boolean EllipseOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    SF_Ellipse* ella = GetEllipse();
    SF_Ellipse* ellb = ((EllipseOvComp&)comp).GetEllipse();
    IntCoord ax, ay, bx, by;
    int ar1, ar2, br1, br2;
    ella->GetOriginal(ax, ay, ar1, ar2);
    ellb->GetOriginal(bx, by, br1, br2);
    return 
	ax == bx && ay == by && ar1 == br1 && ar2 == br2 &&
	OverlayComp::operator==(comp);
}

/*****************************************************************************/

EllipseOvComp* EllipseOvView::GetEllipseOvComp () { 
    return (EllipseOvComp*) GetSubject();
}

ClassId EllipseOvView::GetClassId () { return OVELLIPSE_VIEW; }

boolean EllipseOvView::IsA (ClassId id) {
    return OVELLIPSE_VIEW == id || OverlayView::IsA(id);
}

EllipseOvView::EllipseOvView (EllipseOvComp* subj) : OverlayView(subj) { }

void EllipseOvView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        float cx, cy;
        GetGraphic()->GetCenter(cx, cy);
        ((AlignToGridCmd*) cmd)->Align(this, cx, cy);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void EllipseOvView::Update () {
    Graphic* ellipse = GetGraphic();

    IncurDamage(ellipse);
    *ellipse = *GetEllipseOvComp()->GetGraphic();
    IncurDamage(ellipse);
    EraseHandles();
}

Manipulator* EllipseOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Rubberband* rub = nil;
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        v->Constrain(e.x, e.y);
        rub = new RubberEllipse(nil, nil, e.x, e.y, e.x, e.y);
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(XYEqual | Gravity)
	);
    } else {
        m = OverlayView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* EllipseOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        RubberEllipse* re = (RubberEllipse*) dm->GetRubberband();
        Coord x, y, dummy1, dummy2;
        re->GetCurrent(x, y, dummy1, dummy2);

        if (dummy1 != x || dummy2 != y) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
            Coord xr, yr;
            re->CurrentRadii(xr, yr);

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }

            Graphic* pg = GetGraphicComp()->GetGraphic();
            SF_Ellipse* ellipse = new SF_Ellipse(x, y, xr, yr, pg);

            if (brVar != nil) ellipse->SetBrush(brVar->GetBrush());
            if (patVar != nil) ellipse->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
                ellipse->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            ellipse->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new EllipseOvComp(ellipse)));
        }

    } else {
        cmd = GraphicView::InterpretManipulator(m);
    }
    return cmd;
}

Graphic* EllipseOvView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        EllipseOvComp* ellipseComp = GetEllipseOvComp();
        graphic = ellipseComp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

/*****************************************************************************/

EllipsePS::EllipsePS (OverlayComp* subj) : OverlayPS(subj) { }
ClassId EllipsePS::GetClassId () { return ELLIPSE_PS; }

boolean EllipsePS::IsA (ClassId id) { 
    return ELLIPSE_PS == id || OverlayPS::IsA(id);
}

boolean EllipsePS::Definition (ostream& out) {
    Coord x0, y0;
    int rx, ry;

    SF_Ellipse* ellipse = (SF_Ellipse*) GetGraphicComp()->GetGraphic();
    ellipse->GetOriginal(x0, y0, rx, ry);

    out << "Begin " << MARK << " Elli\n";
    MinGS(out);
    out << MARK << "\n";
    out << x0 << " " << y0 << " " << rx << " " << ry << " Elli\n";
    out << "End\n\n";

    return out.good();
}

/*****************************************************************************/

EllipseScript::EllipseScript (EllipseOvComp* subj) : OverlayScript(subj) { }
ClassId EllipseScript::GetClassId () { return ELLIPSE_SCRIPT; }

boolean EllipseScript::IsA (ClassId id) { 
    return ELLIPSE_SCRIPT == id || OverlayScript::IsA(id);
}

boolean EllipseScript::Definition (ostream& out) {
    Coord x0, y0;
    int r1, r2;

    EllipseOvComp* comp = (EllipseOvComp*) GetSubject();
    comp->GetEllipse()->GetOriginal(x0, y0, r1, r2);

    out << "ellipse(";
    out << x0 << "," << y0 << "," << r1 << "," << r2;
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int EllipseScript::ReadOriginal (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord x0, y0;
    int r1, r2;
    char delim;

    char ch = in.peek();
    if (ch != ')' && ch != ':') 
	in >> x0 >> delim >> y0 >> delim >> r1 >> delim >> r2;
    else {
	x0 = y0 = 0;
	r1 = r2 = 0;
    }

    if (!in.good()) {
	return -1;
    }
    else {
        *(SF_Ellipse**)addr1 = new SF_Ellipse(x0, y0, r1, r2);
        return 0;
    }
}

