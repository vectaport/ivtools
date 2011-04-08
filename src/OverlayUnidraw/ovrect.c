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
 * Overlay Rect component definitions.
 */

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovmanips.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/Components/polygon.h>

#include <Unidraw/Graphic/polygons.h>

#include <Unidraw/Tools/tool.h>

#include <InterViews/event.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubgroup.h>
#include <IV-2_6/InterViews/rubline.h>
#include <IV-2_6/InterViews/rubrect.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

#include <Attribute/attrlist.h>

#include <stream.h>

/*****************************************************************************/

ParamList* RectOvComp::_ovrect_params = nil;
int RectOvComp::_symid = -1;

ClassId RectOvComp::GetClassId () { return OVRECT_COMP; }

boolean RectOvComp::IsA (ClassId id) {
    return OVRECT_COMP == id || OverlayComp::IsA(id);
}

Component* RectOvComp::Copy () {
    RectOvComp* comp =
      new RectOvComp((SF_Rect*) GetGraphic()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    return comp;
}

RectOvComp::RectOvComp (SF_Rect* graphic, OverlayComp* parent) 
: OverlayComp(graphic, parent) { }

RectOvComp::RectOvComp(istream& in, OverlayComp* parent) 
: OverlayComp(nil, parent) {
    _valid = GetParamList()->read_args(in, this);
}
    
ParamList* RectOvComp::GetParamList() {
    if (!_ovrect_params) 
	GrowParamList(_ovrect_params = new ParamList());
    return _ovrect_params;
}

void RectOvComp::GrowParamList(ParamList* pl) {
    pl->add_param("original", ParamStruct::required, &RectScript::ReadOriginal,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

SF_Rect* RectOvComp::GetRect () { return (SF_Rect*) GetGraphic(); }

boolean RectOvComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    SF_Rect* recta = GetRect();
    SF_Rect* rectb = ((RectOvComp&)comp).GetRect();
    IntCoord ax0, ay0, ax1, ay1;
    IntCoord bx0, by0, bx1, by1;
    recta->GetOriginal(ax0, ay0, ax1, ay1);
    rectb->GetOriginal(bx0, by0, bx1, by1);
    return 
	ax0 = bx0 && ay0 == by0 && ax1 == bx1 && ay1 == by1 &&
	OverlayComp::operator==(comp);
}

/*****************************************************************************/

RectOvComp* RectOvView::GetRectOvComp () { return (RectOvComp*) GetSubject(); }
ClassId RectOvView::GetClassId () { return OVRECT_VIEW; }

boolean RectOvView::IsA (ClassId id) {
    return OVRECT_VIEW == id || OverlayView::IsA(id);
}

RectOvView::RectOvView (RectOvComp* subj) : OverlayView(subj) { }

void RectOvView::Interpret (Command* cmd) {
    if (cmd->IsA(ALIGNTOGRID_CMD)) {
        SF_Rect* rect = (SF_Rect*) GetGraphic();
        Transformer total;
        rect->TotalTransformation(total);

        Coord x0, y0, x1, y1;
        float tx0, ty0;

        rect->GetOriginal(x0, y0, x1, y1);
        total.Transform(float(x0), float(y0), tx0, ty0);
        ((AlignToGridCmd*) cmd)->Align(this, tx0, ty0);

    } else {
        OverlayView::Interpret(cmd);
    }
}

void RectOvView::Update () {
    Graphic* rect = GetGraphic();

    IncurDamage(rect);
    *rect = *GetRectOvComp()->GetGraphic();
    IncurDamage(rect);
    EraseHandles();
}

void RectOvView::CreateHandles () {
    Coord x[4], y[4];
    Viewer* v = GetViewer();
    
    if (v != nil) {
        GetCorners(x, y);
        _handles = new RubberHandles(nil, nil, x, y, 4, 0, HANDLE_SIZE);
        v->InitRubberband(_handles);
    }
}

Manipulator* RectOvView::CreateManipulator (
    Viewer* v, Event& e, Transformer* rel, Tool* tool
) {
    Coord x[5], y[5];
    Rubberband* rub = nil;
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        v->Constrain(e.x, e.y);
        rub = new RubberRect(nil, nil, e.x, e.y, e.x, e.y);
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(XYEqual | Gravity)
	);

    } else if (tool->IsA(RESHAPE_TOOL)) {
	RubberGroup* rub = new RubberGroup(nil, nil);
	Coord x[4], y[4];
        v->Constrain(e.x, e.y);
	GetCorners(x, y);
	_reshapeCorner = ClosestPoint(x, y, 4, e.x, e.y);

	if (_reshapeCorner > 0) {
	    rub->Append(
		new RubberLine(
                    nil, nil, x[_reshapeCorner-1], y[_reshapeCorner-1], e.x,e.y
                )
	    );
	} else { 
	    rub->Append(new RubberLine(nil,nil,x[3],y[3],e.x,e.y));
	}

	if (_reshapeCorner < 3) {
	    rub->Append(
		new RubberLine(
                    nil, nil, x[_reshapeCorner+1], y[_reshapeCorner+1], e.x,e.y
                )
	    );
	} else { 
	    rub->Append(new RubberLine(nil, nil, x[0], y[0], e.x, e.y));
	}
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else if (tool->IsA(MOVE_TOOL) && !FixedLocation()) {
	v->Constrain(e.x, e.y);
	GetCorners(x, y);
        x[4] = x[0]; y[4] = y[0];

	rub = new SlidingLineList(nil, nil, x, y, 5, e.x, e.y);
        m = new OpaqueDragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity),
	    GetGraphic()
	);

    } else if (tool->IsA(SCALE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetCorners(x, y);
        x[4] = x[0]; y[4] = y[0];
        rub = new ScalingLineList(nil,nil,x,y,5, (x[0]+x[2])/2, (y[0]+y[2])/2);
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else if (tool->IsA(ROTATE_TOOL)) {
        v->Constrain(e.x, e.y);
        GetCorners(x, y);
        x[4] = x[0]; y[4] = y[0];
        rub = new RotatingLineList(
            nil, nil, x, y, 5, (x[0]+x[2])/2, (y[0]+y[2])/2, e.x, e.y
        );
        m = new OpaqueDragManip(v, rub, rel, tool, Gravity, GetGraphic());

    } else {
        m = OverlayView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* RectOvView::InterpretManipulator (Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    Editor* ed = dm->GetViewer()->GetEditor();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        RubberRect* rr = (RubberRect*) dm->GetRubberband();
        Coord x0, y0, x1, y1;
        rr->GetCurrent(x0, y0, x1, y1);

        if (x0 != x1 || y0 != y1) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

            if (rel != nil) {
                rel = new Transformer(rel);
                rel->Invert();
            }

            Graphic* pg = GetGraphicComp()->GetGraphic();
            SF_Rect* rect = new SF_Rect(x0, y0, x1, y1, pg);

            if (brVar != nil) rect->SetBrush(brVar->GetBrush());
            if (patVar != nil) rect->SetPattern(patVar->GetPattern());

            if (colVar != nil) {
                rect->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
            }
            rect->SetTransformer(rel);
            Unref(rel);
            cmd = new PasteCmd(ed, new Clipboard(new RectOvComp(rect)));
        }

    } else if (tool->IsA(RESHAPE_TOOL)) {
        RubberGroup* rubberGroup = (RubberGroup*) dm->GetRubberband();
	RubberLine* rubberLine = (RubberLine*) rubberGroup->First();
        SF_Polygon* polygon;
        Coord x[4], y[4];
	Coord x0, y0;
        
	GetCorners(x, y);
	rubberLine->GetCurrent(x0, y0, x[_reshapeCorner], y[_reshapeCorner]);

        if (rel != nil) {
            rel = new Transformer(rel);
            rel->Invert();
        }
        polygon = new SF_Polygon(x, y, 4, GetGraphic());
        polygon->SetTransformer(rel);
        Unref(rel);
        cmd = new ReplaceCmd(ed, new PolygonOvComp(polygon));

    } else if (tool->IsA(MOVE_TOOL)) {
        SlidingLineList* sll;
        Transformer* rel = dm->GetTransformer();
        Coord* ox, *oy, *cx, *cy;
        float fx0, fy0, fx1, fy1;
        int n;

        sll = (SlidingLineList*) dm->GetRubberband();
        sll->GetOriginal(ox, oy, n);
        sll->GetCurrent(cx, cy, n);
        if (rel != nil) {
            rel->InvTransform(float(ox[0]), float(oy[0]), fx0, fy0);
            rel->InvTransform(float(cx[0]), float(cy[0]), fx1, fy1);
        }
        delete ox; delete oy; delete cx; delete cy;
        cmd = new MoveCmd(ed, fx1 - fx0, fy1 - fy0);

    } else if (tool->IsA(SCALE_TOOL)) {
        ScalingLineList* sll = (ScalingLineList*) dm->GetRubberband();
        float sxy = sll->CurrentScaling();

        cmd = new ScaleCmd(ed, sxy, sxy);

    } else if (tool->IsA(ROTATE_TOOL)) {
        RotatingLineList* rll = (RotatingLineList*) dm->GetRubberband();
        float angle = rll->CurrentAngle() - rll->OriginalAngle();

        cmd = new RotateCmd(ed, angle);

    } else {
        cmd = OverlayView::InterpretManipulator(m);
    }
    return cmd;
}

void RectOvView::GetCorners (Coord* x, Coord* y) {
    SF_Rect* rect = (SF_Rect*) GetGraphic();
    Coord tx[4], ty[4];
    Transformer t;

    rect->GetOriginal(tx[0], ty[0], tx[2], ty[2]);
    rect->GetOriginal(tx[3], ty[1], tx[1], ty[3]);
    rect->TotalTransformation(t);
    t.TransformList((Coord*) tx, (Coord*) ty, 4, x, y);
}

Graphic* RectOvView::GetGraphic () {
    Graphic* graphic = OverlayView::GetGraphic();
    
    if (graphic == nil) {
        RectOvComp* rectComp = GetRectOvComp();
        graphic = rectComp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

/*****************************************************************************/

RectPS::RectPS (OverlayComp* subj) : OverlayPS(subj) { }
ClassId RectPS::GetClassId () { return RECT_PS; }

boolean RectPS::IsA (ClassId id) { 
    return RECT_PS == id || OverlayPS::IsA(id);
}

boolean RectPS::Definition (ostream& out) {
    Coord l, b, r, t;

    Rect* rect = (Rect*) GetGraphicComp()->GetGraphic();
    rect->GetOriginal(l, b, r, t);

    out << "Begin " << MARK << " Rect\n";
    MinGS(out);
    out << MARK << "\n";
    out << l << " " << b << " " << r << " " << t << " Rect\n";
    out << "End\n\n";

    return out.good();
}

/*****************************************************************************/

RectScript::RectScript (RectOvComp* subj) : OverlayScript(subj) { }
ClassId RectScript::GetClassId () { return RECT_SCRIPT; }

boolean RectScript::IsA (ClassId id) { 
    return RECT_SCRIPT == id || OverlayScript::IsA(id);
}

boolean RectScript::Definition (ostream& out) {
    Coord l, b, r, t;

    RectOvComp* comp = (RectOvComp*) GetSubject();
    comp->GetRect()->GetOriginal(l, b, r, t);

    out << "rectangle(";
    out << l << "," << b << "," << r << "," << t;
    MinGS(out);
    Annotation(out);
    Attributes(out);
    out << ")";

    return out.good();
}

int RectScript::ReadOriginal (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord l, b, r , t ;
    char delim;

    char ch = in.peek();
    if (ch != ')' && ch != ':') 
	in >> l >> delim >> b >> delim >> r  >> delim >> t ;
    else
	l = b = r = t = 0;

    if (!in.good()) {
	return -1;
    }
    else {
        *(SF_Rect**)addr1 = new SF_Rect(l, b, r, t);
        return 0;
    }
}

