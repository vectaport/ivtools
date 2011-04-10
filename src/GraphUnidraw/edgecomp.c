/*
 * Copyright (c) 2007 Scott E. Johnston
 * Copyright (c) 1994-1996, 1999 Vectaport Inc.
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

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphdata.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/paramlist.h>

#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Graphic/damage.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/geomobjs.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/picture.h>
#include <Unidraw/Tools/grcomptool.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/manips.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>
#include <Unidraw/ulist.h>
#include <Unidraw/viewer.h>
#include <Unidraw/unidraw.h>

#include <UniIdraw/idarrows.h>

#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubgroup.h>
#include <IV-2_6/InterViews/rubline.h>

#include <InterViews/transformer.h>

#include <TopoFace/topoedge.h>
#include <TopoFace/toponode.h>

#include <Attribute/attrlist.h>

#include <OS/math.h>
#include <iostream.h>

FullGraphic* EdgeView::_ev_gs = nil;

static const int xradius = 35;
static const int yradius = 20;
static const float axis = 0.42;
static const float seen = 1.025;

/*****************************************************************************/

ParamList* EdgeComp::_edge_params = nil;
int EdgeComp::_symid = -1;

ClassId EdgeComp::GetClassId() { return EDGE_COMP; }

boolean EdgeComp::IsA(ClassId id) {
    return id == EDGE_COMP || OverlayComp::IsA(id);
}

EdgeComp::EdgeComp(ArrowLine* g, OverlayComp* parent, int start_subedge,
    int end_subedge) : OverlayComp(g, parent)
{
    _edge = new TopoEdge(this);
    _start_subedge = start_subedge;
    _end_subedge = end_subedge;
}

EdgeComp::EdgeComp(istream& in, OverlayComp* parent) 
    : OverlayComp(nil, parent) {
    _start_subedge = _end_subedge = -1;
    _edge = new TopoEdge(this);
    _valid = GetParamList()->read_args(in, this);
}
    
EdgeComp::EdgeComp(OverlayComp* parent) : OverlayComp(nil, parent) {
    _start_subedge = _end_subedge = -1;
    _edge = new TopoEdge(this);
}

EdgeComp::~EdgeComp() {
    delete _edge;
}

ParamList* EdgeComp::GetParamList() {
    if (!_edge_params) 
	GrowParamList(_edge_params = new ParamList());
    return _edge_params;
}

void EdgeComp::GrowParamList(ParamList* pl) {
    pl->add_param("original", ParamStruct::required, &ArrowLineScript::ReadOriginal,
		  this, &_gr);
    pl->add_param("arrowscale", ParamStruct::keyword, &ArrowLineScript::ReadScale,
		  this, &_gr);
    pl->add_param("head", ParamStruct::keyword, &ArrowLineScript::ReadHead,
		  this, &_gr);
    pl->add_param("tail", ParamStruct::keyword, &ArrowLineScript::ReadTail,
		  this, &_gr);
    pl->add_param("startnode", ParamStruct::keyword, &ParamList::read_int,
		  this, &_start_node);
    pl->add_param("endnode", ParamStruct::keyword, &ParamList::read_int,
		  this, &_end_node);
    OverlayComp::GrowParamList(pl);
    return;
}

Component* EdgeComp::Copy() {
    EdgeComp* comp = NewEdgeComp((ArrowLine*) GetArrowLine()->Copy());
    if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));
    comp->_start_node = _start_node;
    comp->_end_node = _end_node;
    comp->_start_subedge = _start_subedge;
    comp->_end_subedge = _end_subedge;
    return comp;
}

static void CalcControlPts (Ellipse* ell, Transformer* t,
		 Coord* _x, Coord* _y) {
    Coord _x0, _y0;
    int _r1, _r2;
    ell->GetOriginal(_x0, _y0, _r1, _r2);
    if (t == nil) {
        Coord px1, py1, px2, py2;

	px1 = Math::round(float(_r1)*axis); py1 = Math::round(float(_r2)*axis);
	px2 = Math::round(float(_r1)*seen); py2 = Math::round(float(_r2)*seen);

        _x[0] = _x0 + px1;    _y[0] = _y0 + py2;
        _x[1] = _x0 - px1;    _y[1] = _y[0];
        _x[2] = _x0 - px2;    _y[2] = _y0 + py1;
        _x[3] = _x[2];        _y[3] = _y0 - py1;
        _x[4] = _x[1];	      _y[4] = _y0 - py2;
        _x[5] = _x[0];	      _y[5] = _y[4];
        _x[6] = _x0 + px2;    _y[6] = _y[3];
        _x[7] = _x[6];	      _y[7] = _y[2];
    
    } else {
        float fx1, fy1, fx2, fy2, tx[8], ty[8], tmpx, tmpy;

        fx1 = float(_r1)*axis; fy1 = float(_r2)*axis;
        fx2 = float(_r1)*seen; fy2 = float(_r2)*seen;

        tx[0] = _x0 + fx1;    ty[0] = _y0 + fy2;
        tx[1] = _x0 - fx1;    ty[1] = ty[0];
        tx[2] = _x0 - fx2;    ty[2] = _y0 + fy1;
        tx[3] = tx[2];        ty[3] = _y0 - fy1;
        tx[4] = tx[1];	      ty[4] = _y0 - fy2;
        tx[5] = tx[0];	      ty[5] = ty[4];
        tx[6] = _x0 + fx2;    ty[6] = ty[3];
        tx[7] = tx[6];	      ty[7] = ty[2];

        for (int i = 0; i < 8; ++i) {
            t->Transform(tx[i], ty[i], tmpx, tmpy);
            _x[i] = Math::round(tmpx);
            _y[i] = Math::round(tmpy);
        }
    }
}

boolean EdgeComp::clipline(Coord x0, Coord y0, Coord x1, Coord y1, Ellipse* ell,
	       boolean clip1, Coord &nx0, Coord &ny0)
{
    Coord x[8], y[8];
    FullGraphic gs;
    Transformer* t = new Transformer(ell->GetTransformer());
    Graphic* parent = ell->Parent();
    while (parent) {
	if (parent->GetTransformer())
	    t->postmultiply(*parent->GetTransformer());
	parent = parent->Parent();
    }
    CalcControlPts(ell, t, x, y);
    MultiLineObj ml;
    ml.ClosedSplineToPolygon(x, y, 8);
    float fx0, fy0, fx1, fy1;
    fx0 = x0;
    fy0 = y0;
    fx1 = x1;
    fy1 = y1;
    LineObj origline(Math::round(fx0), Math::round(fy0), Math::round(fx1), Math::round(fy1));
    float origslope;
    float b1, c1;
    boolean origslopegood;
    if (fx1-fx0 == 0)
	origslopegood = false;
    else {
	origslope = float(fy1-fy0) / float(fx1-fx0);
	b1 = -origslope;
	c1 = fy0 - origslope * fx0;
	origslopegood = true;
    }
    for (int i = 1; i < ml._count; i++) {
	LineObj lineobj(ml._x[i-1], ml._y[i-1], ml._x[i], ml._y[i]);
	if (origline.Intersects(lineobj)) {
	    float slope;
	    float b2, c2;
	    boolean slopegood;
	    if (lineobj._p2._x - lineobj._p1._x == 0)
		slopegood = false;
	    else {
		slope = float(lineobj._p2._y - lineobj._p1._y) / 
		    float(lineobj._p2._x - lineobj._p1._x);
		b2 = -slope;
		c2 = lineobj._p1._y - slope * lineobj._p1._x;
		slopegood = true;
	    }
	    if (origslopegood && slopegood) {
		ny0 = Math::round((c1*b2 - c2*b1) / (b2 - b1));
		nx0 = Math::round((c2 - c1) / (b2 - b1));
	    }
	    else if (!origslopegood) {
		nx0 = clip1? x1 : x0;
		ny0 = lineobj._p1._y;
	    }
	    else {
		nx0 = lineobj._p1._x;
		ny0 = clip1 ? y1 : y0;
	    }
	    return true;
	}
    }
    return false;
}

void EdgeComp::Interpret(Command* cmd) {
    if (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD)) {
	if (cmd->IsA(OV_DELETE_CMD)) 
	    ((OvDeleteCmd*)cmd)->Reversable(false);
	if (cmd->IsA(GRAPHDELETE_CMD))
	    ((GraphDeleteCmd*)cmd)->connections->Append(new UList(
		new EdgeData(this, (TopoNode*)Edge()->start_node(),
			     (TopoNode*)Edge()->end_node())));
	#if defined(GRAPH_OBSERVABLES)
	if (NodeStart() && NodeEnd()) NodeStart()->detach(NodeEnd());
	#endif
	Edge()->attach_nodes(nil, nil);
    }
    else if (cmd->IsA(EDGECONNECT_CMD)) {
	EdgeConnectCmd* ecmd = (EdgeConnectCmd*)cmd;
	TopoNode** nodes = new TopoNode*[2];
	nodes[0] = (TopoNode*)Edge()->start_node();
	nodes[1] = (TopoNode*)Edge()->end_node();
	ecmd->Store(this, new VoidData(nodes));
	Edge()->attach_nodes(ecmd->Node1() ? ecmd->Node1()->Node() : nil,
			     ecmd->Node2() ? ecmd->Node2()->Node() : nil);
	if(ecmd->Node1() && ecmd->Node2()) {
	  NodeComp* start_node_comp = (NodeComp*)ecmd->Node1();
	  NodeComp* end_node_comp = (NodeComp*)ecmd->Node2();
	  #if defined(GRAPH_OBSERVABLES)
	  if (start_node_comp && start_node_comp->IsA(NODE_COMP) &&
	      end_node_comp && end_node_comp->IsA(NODE_COMP)) {
	    start_node_comp->attach(end_node_comp);
	  }
	  #endif
	}
        ArrowLine* subgr1 = ecmd->Node1() ? ecmd->Node1()->SubEdgeGraphic(_start_subedge) : nil;
        if (subgr1) {
  	    subgr1->Hide();
            subgr1->Desensitize();
	    ecmd->Node1()->Notify();
        }
        Graphic* subgr2 = ecmd->Node2() ? ecmd->Node2()->SubEdgeGraphic(_end_subedge) : nil;
        if (subgr2) {
  	    subgr2->Hide();
            subgr2->Desensitize();
	    ecmd->Node2()->Notify();
        }

	EdgeUpdateCmd eucmd(ecmd->GetEditor(), this);
	eucmd.Execute();
    }
    else if (cmd->IsA(EDGEUPDATE_CMD)) {
	int x0, y0, x1, y1;
	GetArrowLine()->GetOriginal(x0, y0, x1, y1);
	GetArrowLine()->SetTransformer(new Transformer());
	if (Edge()->start_node()) {
	    float fx, fy;
	    ((NodeComp*)NodeStart())
		->GetEllipse()->GetCenter(fx, fy);
	    x0 = Math::round(fx);
	    y0 = Math::round(fy);
	}
	if (Edge()->end_node()) {
	    float fx, fy;
	    ((NodeComp*)NodeEnd())
		->GetEllipse()->GetCenter(fx, fy);
	    x1 = Math::round(fx);
	    y1 = Math::round(fy);
	}
	Coord nx0, ny0;
	if (Edge()->start_node()) {
	  SF_Ellipse* e1;
	  boolean newe = false;
	  if (((NodeComp*)NodeStart())->GetClassId() == NODE_COMP)
	    e1 = ((NodeComp*)NodeStart())->GetEllipse();
	  else {
	    int ex0, ey0, ex1, ey1;
	    ((NodeComp*)NodeStart())->GetGraphic()->
	      GetBox(ex0, ey0, ex1, ey1);
	    e1 = new SF_Ellipse(ex0+(ex1-ex0)/2, ey0+(ey1-ey0)/2,
				(ex1-ex0)/2, (ey1-ey0)/2);
	    newe = true;
	  }
	  
	  if (clipline(x0, y0, x1, y1, e1, false /* clip x0, y0 */,
		       nx0, ny0)) {
	    x0 = nx0;
	    y0 = ny0;
	  }
	  if (newe)
	    delete e1;
#if defined(GRAPH_OBSERVABLES)
	  ((NodeComp*)Edge()->NodeStart())->notify();
#endif
	}
	Coord nx1, ny1;
	if (Edge()->end_node()) {
	  SF_Ellipse* e2;
	  boolean newe = false;
	  if (((NodeComp*)NodeEnd())->GetClassId() == NODE_COMP)
	    e2 = ((NodeComp*)NodeEnd())->GetEllipse();
	  else {
	    int ex0, ey0, ex1, ey1;
	    ((NodeComp*)NodeEnd())->GetGraphic()->
	      GetBox(ex0, ey0, ex1, ey1);
	    e2 = new SF_Ellipse(ex0+(ex1-ex0)/2, ey0+(ey1-ey0)/2,
			     (ex1-ex0)/2, (ey1-ey0)/2);
	    newe = true;
	  }
	  if (clipline(x0, y0, x1, y1, e2, true /* clip x1,y1 */,
		       nx1, ny1)) {
	    x1 = nx1;
	    y1 = ny1;
	  }
	  if (newe)
	    delete e2;
	}
	GetArrowLine()->SetOriginal(x0, y0, x1, y1);
	Notify();
	
    } else if (cmd->IsA(MOVE_CMD)) {
        float dx, dy;
        ((MoveCmd*) cmd)->GetMovement(dx, dy);
	int x0, y0, x1, y1;
	GetArrowLine()->GetOriginal(x0, y0, x1, y1);
	if (!Edge()->start_node()) {
	    x0 += (int) dx;
	    y0 += (int) dy;
	}
	if (!Edge()->end_node()) {
	    x1 += (int) dx;
	    y1 += (int) dy;
	}
	GetArrowLine()->SetOriginal(x0, y0, x1, y1);
	Notify();
    }
    else
	OverlayComp::Interpret(cmd);
}

void EdgeComp::Uninterpret(Command* cmd) {
    if (cmd->IsA(EDGECONNECT_CMD)) {
	VoidData* data = (VoidData*)cmd->Recall(this);
	TopoNode** nodes = (TopoNode**)data->_void;
	Edge()->attach_nodes(nodes[0], nodes[1]);
        EdgeConnectCmd* ecmd = (EdgeConnectCmd*)cmd;
        ArrowLine* subgr1 = ecmd->Node1() ? ecmd->Node1()->SubEdgeGraphic(_start_subedge) : nil;
        if (subgr1) {
  	    subgr1->Show();
            subgr1->Sensitize();
	    ecmd->Node1()->Notify();
        }
        Graphic* subgr2 = ecmd->Node2() ? ecmd->Node2()->SubEdgeGraphic(_end_subedge) : nil;
        if (subgr2) {
  	    subgr2->Show();
            subgr2->Sensitize();
	    ecmd->Node2()->Notify();
        }
	_start_subedge = _end_subedge = -1;
    } else if (cmd->IsA(DELETE_CMD)) {
	if (cmd->Reversible())
	    OverlayComp::Uninterpret(cmd);
	if (cmd->IsA(GRAPHDELETE_CMD)) {
	    GraphDeleteCmd* gdcmd = (GraphDeleteCmd*)cmd;
	    UList* list = gdcmd->connections;
	    UList* conn = gdcmd->connections->First();
	    while (conn != list) {
		if (((GraphData*)(*conn)())->IsA(EDGE_DATA) &&
		    ((EdgeData*)(*conn)())->edge == this)
		    {
			EdgeData* data = (EdgeData*)(*conn)();
			Edge()->attach_nodes(data->start, data->end);
			#if defined(GRAPH_OBSERVABLES)
			if (data->start && data->end) 
			  NodeStart()->attach(NodeEnd());
			#endif
			break;
		    }
		conn = conn->Next();
	    }
	}
    } else if (cmd->IsA(MOVE_CMD)) {
        float dx, dy;
        ((MoveCmd*) cmd)->GetMovement(dx, dy);
	int x0, y0, x1, y1;
	GetArrowLine()->GetOriginal(x0, y0, x1, y1);
	if (!Edge()->start_node()) {
	    x0 -= (int) dx;
	    y0 -= (int) dy;
	}
	if (!Edge()->end_node()) {
	    x1 -= (int) dx;
	    y1 -= (int) dy;
	}
	GetArrowLine()->SetOriginal(x0, y0, x1, y1);
	Notify();
    } else
	OverlayComp::Uninterpret(cmd);
}

EdgeView* EdgeComp::GetEdgeView(Viewer* v) {
    for (UList* u = _views->First(); u != _views->End(); u = u->Next()) {
	if (((GraphicView*)View(u))->GetViewer() == v)
	    return (EdgeView*)View(u);
    }
    return nil;
}

boolean EdgeComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    IntCoord ax0, ay0, ax1, ay1;
    GetArrowLine()->GetOriginal(ax0, ay0, ax1, ay1);
    IntCoord bx0, by0, bx1, by1;
    ((EdgeComp&)comp).GetArrowLine()->GetOriginal(bx0, by0, bx1, by1);
    return ax0 == bx0 && ay0 == by0 && ax1 == bx1 && ay1 == by1 &&
	GetArrowLine()->Head() == ((EdgeComp&)comp).GetArrowLine()->Head() &&
	GetArrowLine()->Tail() == ((EdgeComp&)comp).GetArrowLine()->Tail() &&
	*(OverlayComp*)this == (OverlayComp&)comp;
}

NodeComp* EdgeComp::NodeStart() const {
  TopoEdge* edge = Edge();
  if (edge) {
    TopoNode* start = edge->start_node();
    if (start) {
      return (NodeComp*)start->value();
    }
  }
  return nil;
}

NodeComp* EdgeComp::NodeEnd() const {
  TopoEdge* edge = Edge();
  if (edge) {
    TopoNode* end = edge->end_node();
    if (end) {
      return (NodeComp*)end->value();
    }
  }
  return nil;
}

/*****************************************************************************/

EdgeView::EdgeView(EdgeComp* comp)
: OverlayView(comp)
{

}

EdgeView::~EdgeView() {}

void EdgeView::Interpret(Command* cmd) {
    GraphicView::Interpret(cmd);
}

void EdgeView::Uninterpret(Command* cmd) {
    GraphicView::Uninterpret(cmd);
}

void EdgeView::Update () {
    ArrowLine* line = GetArrowLine();
    IncurDamage(line);
    Coord x0, y0, x1, y1;
    ((EdgeComp*)GetGraphicComp())->GetArrowLine()->GetOriginal(x0, y0, x1, y1);
    line->SetOriginal(x0, y0, x1, y1);
    *line = *GetGraphicComp()->GetGraphic();
    IncurDamage(line);
    EraseHandles();
}

void EdgeView::GetEndpoints (IntCoord& x0, IntCoord& y0, IntCoord& x1, IntCoord& y1) {
    ArrowLine* line = GetArrowLine();
    Transformer t;

    line->GetOriginal(x0, y0, x1, y1);
    line->TotalTransformation(t);
    t.Transform(x0, y0);
    t.Transform(x1, y1);
}

Manipulator* EdgeView::CreateManipulator(
    Viewer* v, Event& e, Transformer* rel, Tool* tool)
{
    Coord x0, y0, x1, y1;
    Coord l, r, b, t;
    Rubberband* rub = nil;
    Manipulator* m = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        v->Constrain(e.x, e.y);
        rub = new RubberLine(nil, nil, e.x, e.y, e.x, e.y);
        m = new DragManip(
	    v, rub, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);
    } else if (tool->IsA(MOVE_TOOL)) {
	RubberGroup* rubgroup = new RubberGroup(nil,nil);
        v->Constrain(e.x, e.y);
	GetEndpoints(x0, y0, x1, y1);
	rub = new SlidingLine(nil, nil, x0, y0, x1, y1, e.x, e.y);
	rubgroup->Append(rub);
	TopoEdge* edge = ((EdgeComp*)GetGraphicComp())->Edge();
	if (edge->start_node()) {
	    NodeComp* nodecmp = (NodeComp*)((EdgeComp*)GetGraphicComp())->NodeStart();
	    NodeView* nodeview = nodecmp->GetNodeView(GetViewer());
	    rub = nodeview->MakeRubberband(e.x, e.y);
	    rubgroup->Append(rub);
	    Iterator i;
	    TopoNode* node = nodecmp->Node();
	    for (node->first(i); !node->done(i); node->next(i)) {
		TopoEdge* nedge = node->edge(node->elem(i));
		if (nedge != edge) {
		    int x0, y0, x1, y1;
		    if (nedge->end_node() == node) {
			((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			    GetOriginal(x0, y0, x1, y1);
		    }
		    else {
			((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			    GetOriginal(x1, y1, x0, y0);
		    }
		    Transformer trans;
		    ((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			TotalTransformation(trans);
		    trans.Transform(x0, y0);
		    trans.Transform(x1, y1);
		    RubberLine* rubline = new RubberLine(nil, nil, x0-(x1-e.x), y0-(y1-e.y), x1, y1,
						     x1 - e.x, y1 - e.y);
		    rubgroup->Append(rubline);
		}
	    }
	}
	if (edge->end_node()) {
	    NodeComp* nodecmp = ((EdgeComp*)GetGraphicComp())->NodeEnd();
	    NodeView* nodeview = nodecmp->GetNodeView(GetViewer());
	    rub = nodeview->MakeRubberband(e.x, e.y);
	    rubgroup->Append(rub);
	    Iterator i;
	    TopoNode* node = nodecmp->Node();
	    for (node->first(i); !node->done(i); node->next(i)) {
		TopoEdge* nedge = node->edge(node->elem(i));
		if (nedge != edge) {
		    int x0, y0, x1, y1;
		    if (nedge->end_node() == node) {
			((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			    GetOriginal(x0, y0, x1, y1);
		    }
		    else {
			((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			    GetOriginal(x1, y1, x0, y0);
		    }
		    Transformer trans;
		    ((EdgeComp*)nedge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
			TotalTransformation(trans);
		    trans.Transform(x0, y0);
		    trans.Transform(x1, y1);
		    RubberLine* rubline = new RubberLine(nil, nil, x0-(x1-e.x), y0-(y1-e.y), x1, y1,
						     x1 - e.x, y1 - e.y);
		    rubgroup->Append(rubline);
		}
	    }
	}
        m = new DragManip(
	    v, rubgroup, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);

    } else {
        m = GraphicView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}

Command* EdgeView::InterpretManipulator(Manipulator* m) {
    DragManip* dm = (DragManip*) m;
    //IdrawEditor* ed = (IdrawEditor*)dm->GetViewer()->GetEditor();
    //GraphicViews* views = (GraphicViews*)dm->GetViewer()->GetGraphicView();
    OverlayEditor* ed = (OverlayEditor*)dm->GetViewer()->GetEditor();
    OverlaysView* views = ed->GetFrame();
    Tool* tool = dm->GetTool();
    Transformer* rel = dm->GetTransformer();
    Command* cmd = nil;
    Viewer* v = GetViewer();
    float mag = dm->GetViewer()->GetMagnification();

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        RubberLine* rl = (RubberLine*) dm->GetRubberband();
        Coord x0, y0, x1, y1;
        rl->GetCurrent(x0, y0, x1, y1);

        if (x0 != x1 || y0 != y1) {
            BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
            ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");

	    NodeView* gv0 = nil;
	    NodeView* gv1 = nil;
	    Iterator i;
	    Selection* s0 = views->ViewsContaining(x0, y0);
	    for (s0->Last(i); !s0->Done(i); s0->Prev(i)) {
		if (s0->GetView(i)->IsA(NODE_VIEW)) {
		    gv0 = (NodeView*)s0->GetView(i);
		    break;
		}
	    }
	    Selection* s1 = views->ViewsContaining(x1, y1);
	    for (s1->Last(i); !s1->Done(i); s1->Prev(i)) {
		if (s1->GetView(i)->IsA(NODE_VIEW)) {
		    gv1 = (NodeView*)s1->GetView(i);
		    break;
		}
	    }
            ArrowLine* pg = ((EdgeComp*)GetGraphicComp())->GetArrowLine();

            int start_subedge = -1;
            int end_subedge = -1;
	    if (gv0) {
   	        PointObj pt0(x0, y0);
	        Graphic* subgr0 = ((Picture*)gv0->GetGraphic())->LastGraphicContaining(pt0);
	        if (subgr0 && subgr0->CompId() == ARROWLINE_COMP) {
		    int index0 = gv0->SubEdgeIndex((ArrowLine*)subgr0);

	            EdgeComp* subedgecomp = ((NodeComp*)gv0->GetGraphicComp())->SubEdgeComp(index0);
		    if (!subedgecomp || (subedgecomp && (subedgecomp->Edge()->start_node() == nil)))
		        return cmd;
 		    start_subedge = index0;
	        }
	    }

	    if (gv1) {
	        PointObj pt1(x1, y1);
	        Graphic* subgr1 = ((Picture*)gv1->GetGraphic())->LastGraphicContaining(pt1);
	        if (subgr1 && subgr1->CompId() == ARROWLINE_COMP) {
		    int index1 = gv1->SubEdgeIndex((ArrowLine*)subgr1);
	            EdgeComp* subedgecomp = ((NodeComp*)gv1->GetGraphicComp())->SubEdgeComp(index1);
		    if (!subedgecomp || (subedgecomp && (subedgecomp->Edge()->end_node() == nil)))
		        return cmd;
		    end_subedge = index1;
	        }
	    }

	    if (rel) {
		rel->InvTransform(x0, y0);
		rel->InvTransform(x1, y1);
	    }
            ArrowLine* line = new ArrowLine(x0, y0, x1, y1, false, true, 1.5, pg);
            if (brVar != nil) 
		line->SetBrush(brVar->GetBrush());
            if (colVar != nil) {
	        line->FillBg(!colVar->GetBgColor()->None());
                line->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
	    }

	    EdgeComp* newedge = NewEdgeComp(line, nil, start_subedge, end_subedge);
	    if (gv0 || gv1)
		cmd = new MacroCmd(
		    ed,
		    new PasteCmd(ed, new Clipboard(newedge)),
		    new EdgeConnectCmd(ed, newedge,
				       gv0 ? (NodeComp*)gv0->GetGraphicComp() : nil,
				       gv1 ? (NodeComp*)gv1->GetGraphicComp() : nil)
		);
	    else
		cmd = new PasteCmd(ed, new Clipboard(newedge));
        }
    }
    else if (tool->IsA(MOVE_TOOL)) {
	cmd = new MacroCmd(ed);
        Coord x0, y0, x1, y1, dummy1, dummy2;
        float fx0, fy0, fx1, fy1;

        SlidingLine* sl = (SlidingLine*) ((RubberGroup*)dm->GetRubberband())->First();
        sl->GetOriginal(x0, y0, dummy1, dummy2);
        sl->GetCurrent(x1, y1, dummy1, dummy2);

        if (rel != nil) {
            rel->InvTransform(float(x0), float(y0), fx0, fy0);
            rel->InvTransform(float(x1), float(y1), fx1, fy1);
        }
        ((MacroCmd*)cmd)->Append(new MoveCmd(ed, fx1 - fx0, fy1 - fy0));
	TopoEdge* edge = ((EdgeComp*)GetGraphicComp())->Edge();
	if (edge->start_node()) {
	    NodeComp* nodecmp = (NodeComp*)((EdgeComp*)GetGraphicComp())->NodeStart();
	    NodeView* nodeview = nodecmp->GetNodeView(GetViewer());
	    v->GetSelection()->Append(nodeview);
	}
	if (edge->end_node()) {
	    NodeComp* nodecmp = (NodeComp*)((EdgeComp*)GetGraphicComp())->NodeEnd();
	    NodeView* nodeview = nodecmp->GetNodeView(GetViewer());
	    v->GetSelection()->Append(nodeview);
	}
    }
    else {
        cmd = GraphicView::InterpretManipulator(m);
    }
    return cmd;
}

ClassId EdgeView::GetClassId() { return EDGE_VIEW; }

boolean EdgeView::IsA(ClassId id) {
    return id == EDGE_VIEW || OverlayView::IsA(id);
}

Graphic* EdgeView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        EdgeComp* edgeComp = (EdgeComp*)GetGraphicComp();
        graphic = edgeComp->GetArrowLine()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

Graphic* EdgeView::HighlightGraphic() {
    if (!_ev_gs) {
	Catalog* catalog = unidraw->GetCatalog();
	_ev_gs = new FullGraphic();
	_ev_gs->SetBrush(catalog->FindBrush(0xffff, 2));
	_ev_gs->SetColors(catalog->FindColor("red"), catalog->FindColor("red"));
    }

    return _ev_gs;
}

/*****************************************************************************/

EdgePS::EdgePS (EdgeComp* subj) : PostScriptView (subj) { 
}

ClassId EdgePS::GetClassId () { return EDGE_PS; }

boolean EdgePS::IsA (ClassId id) {
    return EDGE_PS == id || PostScriptView::IsA(id);
}

boolean EdgePS::Definition (ostream& out) {
    EdgeComp* comp = (EdgeComp*) GetSubject();
    ArrowLine* aline = comp->GetArrowLine();
    int start_node_index = -1;
    int end_node_index = -1;
    IndexNodes(start_node_index, end_node_index);


    Coord x0, y0, x1, y1;
    aline->GetOriginal(x0, y0, x1, y1);
    float arrow_scale = aline->ArrowScale();

    out << "Begin " << MARK << " Edge\n";
    MinGS(out);
    out << MARK << " " << start_node_index << " " << end_node_index << "\n";
    out << MARK << "\n";
    out << x0 << " " << y0 << " " << x1 << " " << y1 << " Line\n";
    out << MARK << " " << arrow_scale << "\n";
    out << "End\n\n";

    return out.good();
}

// this code is entirely for compatibility with older versions of idraw

void EdgePS::Brush (ostream& out) {
    EdgeComp* comp = (EdgeComp*) GetSubject();
    PSBrush* brush = (PSBrush*) GetGraphicComp()->GetGraphic()->GetBrush();
    boolean head, tail;
    head = comp->GetArrowLine()->Head();
    tail = comp->GetArrowLine()->Tail();

    if (brush == nil) {
	out << MARK << " b u\n";

    } else if (brush->None()) {
	out << "none SetB " << MARK << " b n\n";

    } else {
	int p = brush->GetLinePattern();
	out << MARK << " b " << p << "\n";

	int w = brush->Width();
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

void EdgePS::IndexNodes(int &start, int &end) {
    TopoEdge* edge = ((EdgeComp*)_subject)->Edge();
    const TopoNode* node;
    if (edge->start_node())
	start = IndexNode(((EdgeComp*)_subject)->NodeStart());
    if (edge->end_node())
	end  = IndexNode(((EdgeComp*)_subject)->NodeEnd());
    return;
}

int EdgePS::IndexNode (NodeComp *comp) {
    GraphicComp* comps = (GraphicComp*)GetSubject()->GetParent();
    Iterator i;
    int index = -1;

    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
	GraphicComp* tcomp = comps->GetComp(i);
	if (tcomp->IsA(NODE_COMP))
	    index++;
        if (tcomp == comp)
	    return index;
    }
    return -1;
}

/*****************************************************************************/

EdgeScript::EdgeScript (EdgeComp* subj) : OverlayScript(subj) { }
ClassId EdgeScript::GetClassId () { return EDGE_SCRIPT; }

boolean EdgeScript::IsA (ClassId id) { 
    return EDGE_SCRIPT == id || OverlayScript::IsA(id);
}

boolean EdgeScript::Definition (ostream& out) {
    EdgeComp* comp = (EdgeComp*) GetSubject();
    ArrowLine* arrowline = comp->GetArrowLine();
    int start_node_index = -1;
    int end_node_index = -1;
    IndexNodes(start_node_index, end_node_index);

    Coord x0, y0, x1, y1;
    arrowline->GetOriginal(x0, y0, x1, y1);
    float arrow_scale = arrowline->ArrowScale();
    boolean head, tail;
    head = arrowline->Head();
    tail = arrowline->Tail();

    out << script_name() << "(";
    out << x0 << "," << y0 << "," << x1 << "," << y1;
    if (arrow_scale != 1 )
	out << " :arrowscale " << arrow_scale;
    if (head) 
	out << " :head";
    if (tail)
	out << " :tail";
    out << " :startnode " << start_node_index << " :endnode " << end_node_index;
    MinGS(out);
    Annotation(out);
    out << ")";

    return out.good();
}

void EdgeScript::IndexNodes(int &start, int &end) {
    TopoEdge* edge = ((EdgeComp*)_subject)->Edge();
    const TopoNode* node;
    if (edge->start_node())
	start = IndexNode(((EdgeComp*)_subject)->NodeStart());
    if (edge->end_node())
	end = IndexNode(((EdgeComp*)_subject)->NodeEnd());
    return;
}

int EdgeScript::IndexNode (NodeComp *comp) {
    GraphicComp* comps = (GraphicComp*)GetSubject()->GetParent();
    Iterator i;
    int index = -1;

    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
	GraphicComp* tcomp = comps->GetComp(i);
	if (tcomp->IsA(NODE_COMP))
	    index++;
        if (tcomp == comp)
	    return index;
    }
    return -1;
}


