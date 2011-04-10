/*
 * Copyright (c) 2006-2007 Scott E. Johnston
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

#include <GraphUnidraw/nodecomp.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphcatalog.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphdata.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/graphkit.h>

#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/paramlist.h>

#include <IVGlyph/observables.h>

#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Components/text.h>
#include <Unidraw/Graphic/ellipses.h>
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
#include <UniIdraw/idarrows.h>
#include <UniIdraw/ided.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubgroup.h>
#include <IV-2_6/InterViews/rubline.h>
#include <IV-2_6/InterViews/textbuffer.h>
#include <InterViews/transformer.h>

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <Attribute/attrlist.h>

#include <TopoFace/topoedge.h>
#include <TopoFace/toponode.h>

#include <OS/math.h>

#include <ctype.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <strstream>
#include <fstream>

using std::cerr;

FullGraphic* NodeView::_nv_gs = nil;

static const int xradius = 35;
static const int yradius = 20;
static const float axis = 0.42;
static const float seen = 1.025;

/*****************************************************************************/

// octal converts a character to the string \ddd where d is an octal digit.

static char* octal(unsigned char c, register char* p) {
    *p-- = '\0';		// backwards from terminating null...
    *p-- = (char)('0' + c%8);
    *p-- = (char)('0' + (c >>= 3)%8);
    *p-- = (char)('0' + (c >>= 3)%8);
    *p = '\\';			// ...to beginning backslash
    return p;
}

/*****************************************************************************/

ParamList* NodeComp::_node_params = nil;
int NodeComp::_symid = -1;

NodeComp::NodeComp(SF_Ellipse* ellipse, TextGraphic* txt, boolean rl, OverlayComp* parent) 
    : OverlayComp(nil, parent)
{
    _graph = nil;
    _node = new TopoNode(this);
    _reqlabel = rl;
    Picture* pic = new Picture();
    pic->Append(ellipse, txt);
    SetGraphic(pic);
    // kludge to fix ps: fonts are collected from comp\'s graphic, so we
    // need to add the font to the picture\'s gs
    pic->FillBg(ellipse->BgFilled() && !ellipse->GetBgColor()->None());
    pic->SetColors(ellipse->GetFgColor(), ellipse->GetBgColor());
    pic->SetPattern(ellipse->GetPattern());
    pic->SetBrush(ellipse->GetBrush());
    pic->SetFont(txt->GetFont());
}

NodeComp::NodeComp(SF_Ellipse* ellipse, TextGraphic* txt, SF_Ellipse* ellipse2, 
    GraphComp* graph, boolean rl, OverlayComp* parent) 
    : OverlayComp(nil, parent)
{
    _graph = graph;
    _node = new TopoNode(this);
    _reqlabel = rl;
    Picture* pic = new Picture();
    pic->Append(ellipse, txt);
    SetGraphic(pic);
    
    GraphGraphic(ellipse2);

    // kludge to fix ps: fonts are collected from comp\'s graphic, so we
    // need to add the font to the picture\'s gs
    pic->FillBg(ellipse->BgFilled() && !ellipse->GetBgColor()->None());
    pic->SetColors(ellipse->GetFgColor(), ellipse->GetBgColor());
    pic->SetPattern(ellipse->GetPattern());
    pic->SetBrush(ellipse->GetBrush());
    pic->SetFont(txt->GetFont());
}

NodeComp::NodeComp(Picture* pic, boolean rl, OverlayComp* parent) 
    : OverlayComp(pic, parent)
{
    _graph = nil;
    _node = new TopoNode(this);
    // kludge to fix ps: fonts are collected from comp\'s graphic, so we
    // need to add the font to the picture\'s gs
    Iterator it;
    pic->First(it);
    Graphic* first = pic->GetGraphic(it);
    if (first) {
      pic->FillBg(first->BgFilled() && !first->GetBgColor()->None());
      pic->SetColors(first->GetFgColor(), first->GetBgColor());
      pic->SetPattern(first->GetPattern());
      pic->SetBrush(first->GetBrush());
      if (GetText()) pic->SetFont(GetText()->GetFont());
    }
    _reqlabel = rl;
}

NodeComp::NodeComp(GraphComp* graph) 
    : OverlayComp(nil, nil)
{
    _graph = graph;
    _node = new TopoNode(this);
    _reqlabel = true;

    Picture* pic = new Picture();
    SF_Ellipse* ellipse = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    ellipse->SetPattern(unidraw->GetCatalog()->FindGrayLevel(1));
    TextGraphic* txt = new TextGraphic(graph->GetFile(), stdgraphic);
    ellipse->Align(4, txt, 4); // same as Center in IV-2_6/InterViews/alignment.h
    pic->Append(ellipse, txt);
    SetGraphic(pic);

    GraphGraphic();

    // kludge to fix ps: fonts are collected from comp\'s graphic, so we
    // need to add the font to the picture\'s gs
    pic->FillBg(ellipse->BgFilled() && !ellipse->GetBgColor()->None());
    pic->SetColors(ellipse->GetFgColor(), ellipse->GetBgColor());
    pic->SetPattern(ellipse->GetPattern());
    pic->SetBrush(ellipse->GetBrush());
    pic->SetFont(txt->GetFont());
}

NodeComp::NodeComp(OverlayComp* parent) : OverlayComp(nil, parent) {
    _graph = nil;
    _node = new TopoNode(this);
}

NodeComp::NodeComp(istream& in, OverlayComp* parent) : OverlayComp(nil, parent) {
    _graph = nil;
    _node = new TopoNode(this);
    Picture *pic = new Picture();
    SetGraphic(pic);
    _valid = GetParamList()->read_args(in, this);
    if (GetGraph())
	GraphGraphic();
    if(GetEllipse())
      GetGraphic()->concatGS(GetEllipse(), GetGraphic(), GetEllipse()); 
    if(GetText())
      GetGraphic()->concatGS(GetText(), GetGraphic(), GetText()); 
    if(GetEllipse2())
      GetGraphic()->concatGS(GetEllipse2(), GetGraphic(), GetEllipse2()); 
}
    
NodeComp::~NodeComp() {
    delete _graph;
    delete _node;
}

ClassId NodeComp::GetClassId() { return NODE_COMP; }

boolean NodeComp::IsA(ClassId id) {
    return id == NODE_COMP || OverlayComp::IsA(id);
}

Component* NodeComp::Copy() {
    NodeComp* comp = nil;
    if (GetGraph()) {
        comp = NewNodeComp((SF_Ellipse*)GetEllipse()->Copy(),
	    (TextGraphic*)GetText()->Copy(), (SF_Ellipse*)GetEllipse2()->Copy(), 
	    GetGraph() ? (GraphComp*)GetGraph()->Copy() : nil);
	if (attrlist()) comp->SetAttributeList(new AttributeList(attrlist()));

	Picture* pic = (Picture*)GetGraphic();
        Iterator i;
	pic->First(i);
	pic->Next(i);
	pic->Next(i);
	pic->Next(i);

	Picture* comppic = (Picture*)comp->GetGraphic();
        Iterator compi;
	comppic->First(compi);
	comppic->Next(compi);
	comppic->Next(compi);
	comppic->Next(compi);

	for (;!pic->Done(i);pic->Next(i)) {
	    ArrowLine* arrow = (ArrowLine*)pic->GetGraphic(i);
	    ArrowLine* comparrow = (ArrowLine*)comppic->GetGraphic(compi);
            arrow->Hidden() ? comparrow->Hide() : comparrow->Show();
	    arrow->Desensitized() ? comparrow->Desensitize() : comparrow->Sensitize();
	    comppic->Next(compi);
        }

    } else {
        comp = NewNodeComp((SF_Ellipse*)GetEllipse()->Copy(), 
            (TextGraphic*)GetText()->Copy());
    }
    return comp;
}

void NodeComp::GraphGraphic(SF_Ellipse* ellipse2) {
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    SF_Ellipse* ellipse = (SF_Ellipse*)pic->GetGraphic(i);

    Coord x, y;
    int r1, r2;
    ellipse->GetOriginal(x, y, r1, r2);
    if (!ellipse2) {
        ellipse2 = new SF_Ellipse(0, 0, r1-6, r2-6, stdgraphic);
        ellipse2->SetPattern(ellipse->GetPattern());
    }

    ellipse->Align(4, ellipse2, 4); // same as Center in IV-2_6/InterViews/alignment.h
    pic->InsertAfter(i, ellipse2);
    if (!GetText()->GetOriginal())
	GetText()->SetOriginal(GetGraph()->GetPathName());

    UList* edgelist = GetGraph()->GraphEdges();
    EdgeComp* edgecomp;
    SF_Ellipse* ellipse3 = new SF_Ellipse(0, 0, r1+11, r2+11, stdgraphic);
    ellipse->Align(4, ellipse3, 4); // same as Center in IV-2_6/InterViews/alignment.h
    for (UList* u = edgelist->First(); u != edgelist->End(); u = u->Next()) {
        edgecomp = (EdgeComp*) (*u)();
	Coord x0, y0, x1, y1, dx, dy, nx, ny;
	ArrowLine* origarrow = edgecomp->GetArrowLine();
	origarrow->GetOriginal(x0, y0, x1, y1);

        Transformer* t = new Transformer(origarrow->GetTransformer());
        Graphic* parent = origarrow->Parent();
        while (parent) {
	    if (parent->GetTransformer())
	        t->postmultiply(*parent->GetTransformer());
	    parent = parent->Parent();
        }
	t->Transform(x0, y0);
	t->Transform(x1, y1);
	delete t;

	float fx, fy;
	ellipse->GetCenter(fx, fy);
	dx = x1 - x0;
	dy = y1 - y0;
	ArrowLine* arrow;
	if (edgecomp->Edge()->start_node()) {
	    x0 = Math::round(fx);
	    y0 = Math::round(fy);
	    x1 = x0 + dx;
	    y1 = y0 + dy;
       	    arrow = new ArrowLine(x0, y0, x1, y1, false, true, 1.5);
	    if (EdgeComp::clipline(x0, y0, x1, y1, ellipse2, false, nx, ny)) {
	        x0 = nx;
	        y0 = ny;
		arrow->SetOriginal(x0, y0, x1, y1);
	    } 
	    if (EdgeComp::clipline(x0, y0, x1, y1, ellipse3, true, nx, ny)) {
	        x1 = nx;
	        y1 = ny;
		arrow->SetOriginal(x0, y0, x1, y1);
	    } 
	}
	if (edgecomp->Edge()->end_node()) {
	    x1 = Math::round(fx);
	    y1 = Math::round(fy);
	    x0 = x1 - dx;
	    y0 = y1 - dy;
       	    arrow = new ArrowLine(x1, y1, x0, y0, false, true, 1.5);
	    if (EdgeComp::clipline(x0, y0, x1, y1, ellipse2, true, nx, ny)) {
	        x1 = nx;
	        y1 = ny;
		arrow->SetOriginal(x0, y0, x1, y1);
	    }
	    if (EdgeComp::clipline(x0, y0, x1, y1, ellipse, false, nx, ny)) {
	        x0 = nx;
	        y0 = ny;
		arrow->SetOriginal(x0, y0, x1, y1);
	    } 
	}
        pic->Append(arrow);
    }
    delete ellipse3;
}

EdgeComp* NodeComp::SubEdgeComp(int index) {
    if (!GetGraph())
	return nil;
    UList* edgelist = GetGraph()->GraphEdges();
    UList* u;
    EdgeComp* comp;
    int i = 0;
    for (u = edgelist->First(); u != edgelist->End(); u = u->Next()) {
	 comp = (EdgeComp*) (*u)();
	 if (i == index)
	     return comp;
	 i++;
    }
    return nil;
}

ParamList* NodeComp::GetParamList() {
    if (!_node_params) 
	GrowParamList(_node_params = new ParamList());
    return _node_params;
}

void NodeComp::GrowParamList(ParamList* pl) {
    pl->add_param("graph", ParamStruct::keyword, &NodeScript::ReadGraph,
		  this, this, &_graph);
    pl->add_param("reqlabel", ParamStruct::keyword, &ParamList::read_int,
		  this, &_reqlabel);
    pl->add_param("ellipse", ParamStruct::keyword, &NodeScript::ReadEllipse,
		  this, &_gr);
    pl->add_param("ellipsetrans", ParamStruct::keyword, &NodeScript::ReadEllipseTransform,
		  this, &_gr);
    pl->add_param("text", ParamStruct::keyword, &NodeScript::ReadText,
		  this, &_gr);
    pl->add_param("texttrans", ParamStruct::keyword, &NodeScript::ReadTextTransform,
		  this, &_gr);
    OverlayComp::GrowParamList(pl);
    return;
}

SF_Ellipse* NodeComp::GetEllipse() {
    Picture* pic = (Picture*)GetGraphic();
    if (!pic) return nil;
    Iterator i;
    pic->First(i);
    if (pic->Done(i)) return nil;
    return (SF_Ellipse*)pic->GetGraphic(i);
}

SF_Ellipse* NodeComp::GetEllipse2() {
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    pic->Next(i);
    if (pic->Done(i))
	return nil;
    else
        return (SF_Ellipse*)pic->GetGraphic(i);
}    

TextGraphic* NodeComp::GetText() {
    Picture* pic = (Picture*)GetGraphic();
    if (!pic) return nil;
    Iterator i;
    pic->First(i);
    pic->Next(i);
    if (_graph)
        pic->Next(i);
    if (pic->Done(i))
	return nil;
    else
	return (TextGraphic*)pic->GetGraphic(i);
}

void NodeComp::SetText(TextGraphic* tg) {
  TextGraphic* oldtg = GetText();
  Transformer t;
  if (oldtg) {
    if (oldtg->GetTransformer()) 
      t = *oldtg->GetTransformer();
    ((Picture*)GetGraphic())->Remove(oldtg);
    delete oldtg;
  }
  tg->SetTransformer(new Transformer(t));
  Iterator it;
  GetGraphic()->First(it);
  GetGraphic()->InsertAfter(it, tg);
}


ArrowLine* NodeComp::SubEdgeGraphic(int index) {
    if (!GetGraph() || index == -1)
	return nil;
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    pic->Next(i);
    pic->Next(i);
    pic->Next(i);
    if (pic->Done(i))
	return nil;

    UList* edgelist = GetGraph()->GraphEdges();
    UList* u;
    int x = 0;
    for (u = edgelist->First(); u != edgelist->End(); u = u->Next()) {
	 if (x == index)
	     return (ArrowLine*)pic->GetGraphic(i);
	 x++;
         pic->Next(i);
    }
    return nil;
}

void NodeComp::Interpret(Command* cmd) {
    if (cmd->IsA(DELETE_CMD) || cmd->IsA(CUT_CMD)) {
	if (cmd->IsA(OV_DELETE_CMD)) 
	    ((OvDeleteCmd*)cmd)->Reversable(false);
	Iterator i;
	for (Node()->first(i); !Node()->done(i); ) {
	    TopoEdge* edge = Node()->edge(Node()->elem(i));
	    Node()->next(i); // advance now so iterator is good in the forloop
	    if (Node() == edge->start_node()) {
		if (cmd->IsA(GRAPHDELETE_CMD)) {
		    GraphDeleteCmd* gdcmd = (GraphDeleteCmd*)cmd;
		    gdcmd->connections->Append(new UList(
			new NodeData(this, edge, true)));
		}
		edge->attach_nodes(nil, (TopoNode*)edge->end_node());
	    }
	    else if (Node() == edge->end_node()) {
		if (cmd->IsA(GRAPHDELETE_CMD)) {
		    GraphDeleteCmd* gdcmd = (GraphDeleteCmd*)cmd;
		    gdcmd->connections->Append(new UList(
			new NodeData(this, edge, false)));
		}
		edge->attach_nodes((TopoNode*)edge->start_node(), nil);
	    }
	}
    }
    else if (cmd->IsA(MOVE_CMD)) {
        float dx, dy;
        ((MoveCmd*) cmd)->GetMovement(dx, dy);
        GetEllipse()->Translate(dx, dy);
	GetText()->Translate(dx, dy);
        if (GetGraph()) {
            GetEllipse2()->Translate(dx, dy);
            Picture* pic = (Picture*)GetGraphic();
            Iterator i;
            pic->First(i);
    	    pic->Next(i);
	    pic->Next(i);
	    pic->Next(i);
            for (; !pic->Done(i); ) {
		ArrowLine* a = (ArrowLine*)pic->GetGraphic(i);
		a->Translate(dx, dy);
	        pic->Next(i);
            }
        }
        Notify();

	Iterator i;
	TopoNode* node = Node();
	Editor* ed = cmd->GetEditor();
	for (node->first(i); !node->done(i); node->next(i)) {
	    TopoEdge* edge = node->edge(node->elem(i));
	    EdgeUpdateCmd eucmd(ed, (EdgeComp*)edge->value());
	    eucmd.Execute();
	}
    }
    else if (cmd->IsA(NODETEXT_CMD)) {
	NodeTextCmd* ntcmd = (NodeTextCmd*)cmd;
	TextGraphic* tg = ntcmd->Graphic();
	SetText(tg);
	Notify();
	unidraw->Update();
    } else if (cmd->IsA(ALIGN_CMD)) {
        OverlayComp::Interpret(cmd);
	Iterator i;
	TopoNode* node = Node();
	Editor* ed = cmd->GetEditor();
	for (node->first(i); !node->done(i); node->next(i)) {
	    TopoEdge* edge = node->edge(node->elem(i));
	    EdgeUpdateCmd eucmd(ed, (EdgeComp*)edge->value());
	    eucmd.Execute();
	}
    }
    else
	OverlayComp::Interpret(cmd);
}

void NodeComp::Uninterpret(Command* cmd) {
    if (cmd->IsA(MOVE_CMD)) {
        float dx, dy;
        ((MoveCmd*) cmd)->GetMovement(dx, dy);
        GetEllipse()->Translate(-dx, -dy);
	GetText()->Translate(-dx, -dy);
        if (GetGraph()) {
	    GetEllipse2()->Translate(-dx, -dy);
            Picture* pic = (Picture*)GetGraphic();
            Iterator i;
            pic->First(i);
    	    pic->Next(i);
	    pic->Next(i);
	    pic->Next(i);
            for (; !pic->Done(i); ) {
		ArrowLine* a = (ArrowLine*)pic->GetGraphic(i);
		a->Translate(-dx, -dy);
	        pic->Next(i);
            }
        }
        Notify();

	Iterator i;
	TopoNode* node = Node();
	Editor* ed = cmd->GetEditor();
	for (node->first(i); !node->done(i); node->next(i)) {
	    TopoEdge* edge = node->edge(node->elem(i));
	    EdgeUpdateCmd eucmd(ed, (EdgeComp*)edge->value());
	    eucmd.Execute();
	}
    }
    else if (cmd->IsA(GRAPHDELETE_CMD)) {
	OverlayComp::Uninterpret(cmd);
	GraphDeleteCmd* gdcmd = (GraphDeleteCmd*)cmd;
	UList* list = gdcmd->connections;
	UList* conn = gdcmd->connections->First();
	while (conn != list) {
	    if (((GraphData*)(*conn)())->IsA(NODE_DATA) &&
		((NodeData*)(*conn)())->node == this)
		{
		    NodeData* data = (NodeData*)(*conn)();
		    if (data->start)
			data->edge->attach_nodes(
			    Node(), (TopoNode*)data->edge->end_node());
		    else
			data->edge->attach_nodes(
			    (TopoNode*)data->edge->start_node(), Node());
		}
	    conn = conn->Next();
	}
    } else if (cmd->IsA(ALIGN_CMD)) {
        OverlayComp::Uninterpret(cmd);
	Iterator i;
	TopoNode* node = Node();
	Editor* ed = cmd->GetEditor();
	for (node->first(i); !node->done(i); node->next(i)) {
	    TopoEdge* edge = node->edge(node->elem(i));
	    EdgeUpdateCmd eucmd(ed, (EdgeComp*)edge->value());
	    eucmd.Execute();
	}
    }
    else
	OverlayComp::Uninterpret(cmd);
}

NodeView* NodeComp::GetNodeView(Viewer* v) {
    for (UList* u = _views->First(); u != _views->End(); u = u->Next()) {
	if (((GraphicView*)View(u))->GetViewer() == v)
	    return (NodeView*)View(u);
    }
    return nil;
}

void NodeComp::Read(istream&) {}

void NodeComp::Write(ostream&) {}

boolean NodeComp::operator == (OverlayComp& comp) {
    if (GetClassId() != comp.GetClassId()) return false;
    SF_Ellipse* ella = GetEllipse();
    SF_Ellipse* ellb = ((NodeComp&)comp).GetEllipse();
    TextGraphic* txta = GetText();
    TextGraphic* txtb = ((NodeComp&)comp).GetText();
    IntCoord ax, ay, bx, by;
    int ar1, ar2, br1, br2;
    ella->GetOriginal(ax, ay, ar1, ar2);
    ellb->GetOriginal(bx, by, br1, br2);
    int lha = txta->GetLineHeight();
    int lhb = txtb->GetLineHeight();
    const char* stra = txta->GetOriginal();
    const char* strb = txtb->GetOriginal();

    return
	ax == bx && ay == by && ar1 == br1 && ar2 == br2 &&
	GraphicEquals(ella, ellb) &&
	lha == lhb && strcmp(stra, strb) == 0 &&
	GraphicEquals(txta, txtb) &&
	GetGraph() == ((NodeComp&)comp).GetGraph() &&
	*(OverlayComp*)this == (OverlayComp&)comp;
}

#if defined(GRAPH_OBSERVABLES)
void NodeComp::update(Observable*) {
  AttributeList* al;
  if(al = attrlist()) {
    static int valexpr_symid = symbol_add("valexpr");
    static int val_symid = symbol_add("val");
    static int colexpr_symid = symbol_add("colexpr");
    AttributeValue* av = FindValue(valexpr_symid);
    if (av && av->is_string()) {
      Iterator it;
      unidraw->First(it);
      ComTerpServ* comterp = ((ComEditor*)unidraw->GetEditor(it))->
	GetComTerp();
      boolean old_brief = comterp->brief();
      comterp->brief(true);
      std::ostrstream outstr;
      NodeComp* node;
      int incnt = 1;
      while (node = NodeIn(incnt)) {
	out_form(outstr, "in%d=", incnt);
	AttributeValue* av = node->FindValue(val_symid);
	if (av) {
	  ComValue inval(*av);
	  inval.comterp(comterp);
	  outstr << inval << ";";
	} else
	  outstr << "'N';";
	incnt++;  
      }
      outstr << av->string_ptr() << '\0';
      cerr << "interpreting: " << outstr.str() << "\n";
      ComValue retval(comterp->run(outstr.str()));
      cerr << "return value: " << retval << "\n";
      if (retval.is_known()) {
	const int val_symid = symbol_add("val");
	al->add_attr(val_symid, retval);
	AttributeValue* cv = FindValue(colexpr_symid, false, false, false, true);
	if (cv && cv->is_string()) {
	  comterp->set_attributes(al);
	  ComValue colval(comterp->run(cv->string_ptr()));
	  comterp->set_attributes(nil);
	  PSColor *fgcolor = nil, *bgcolor = nil;
	  Catalog* catalog = unidraw->GetCatalog();
	  fgcolor = catalog->FindColor(colval.string_ptr());
	  GetGraphic()->SetColors(fgcolor, bgcolor);
	  Notify();
	}
	Observable::notify();
      }
      comterp->brief(old_brief);
    }
  }
}
#endif

void NodeComp::Notify() {
    GraphicComp::Notify();
}

EdgeComp* NodeComp::EdgeIn(int n) const {
  return EdgeByDir(n, false);
}

EdgeComp* NodeComp::EdgeOut(int n) const {
  return EdgeByDir(n, true);
}


EdgeComp* NodeComp::EdgeByDir(int n, boolean out_edge) const {
  TopoNode* toponode = Node();
  if (toponode) {
    Iterator it;
    toponode->first(it);
    while (!toponode->done(it)) {
      TopoEdge* edge = toponode->get_edge(it);
      if (edge && 

	  (out_edge?edge->start_node():edge->end_node())==toponode)
	n--;
      if (!n) return (EdgeComp*)edge->value();
      toponode->next(it);
    }
  } 
  return nil;
}

NodeComp* NodeComp::NodeIn(int n) const {
  EdgeComp* edgecomp = EdgeIn(n);
  if (edgecomp) {
    TopoEdge* edge = edgecomp->Edge();
    if (edge && edge->start_node()) {
      return (NodeComp*)edgecomp->NodeStart();
    }
  }  
  return nil;
}

NodeComp* NodeComp::NodeOut(int n) const {
  EdgeComp* edgecomp = EdgeOut(n);
  if (edgecomp) {
    TopoEdge* edge = edgecomp->Edge();
    if (edge && edge->end_node()) {
      return (NodeComp*)edgecomp->NodeEnd();
    }
  }  
  return nil;
}


/*****************************************************************************/

NodeView::NodeView(NodeComp* comp) : OverlayView(comp) {
}

NodeView::~NodeView() {
}

void NodeView::Interpret(Command* cmd) {
    GraphicView::Interpret(cmd);
}

void NodeView::Uninterpret(Command* cmd) {
    GraphicView::Uninterpret(cmd);
}

void NodeView::Update() {
    Graphic* list = GetGraphic();
    IncurDamage(list);
    *list = *((NodeComp*)GetGraphicComp())->GetGraphic();

    SF_Ellipse* view_ellipse = GetEllipse();
    SF_Ellipse* comp_ellipse = ((NodeComp*)GetGraphicComp())->GetEllipse();
    *(Graphic*) view_ellipse = *comp_ellipse;
    Coord x0, y0;
    int r1, r2;
    comp_ellipse->GetOriginal(x0, y0, r1, r2);
    view_ellipse->SetOriginal(x0, y0, r1, r2);

    TextGraphic* view_text = GetText();
    TextGraphic* comp_text = ((NodeComp*)GetGraphicComp())->GetText();
    *(Graphic*)view_text = *(Graphic*)comp_text;
    view_text->SetFont(comp_text->GetFont());
    view_text->SetLineHeight(comp_text->GetLineHeight());
    view_text->SetOriginal(comp_text->GetOriginal());

    if (((NodeComp*)GetGraphicComp())->GetGraph()) {
        SF_Ellipse* view_ellipse = GetEllipse2();
        SF_Ellipse* comp_ellipse = ((NodeComp*)GetGraphicComp())->GetEllipse2();
        *(Graphic*) view_ellipse = *comp_ellipse;
        Coord x0, y0;
        int r1, r2;
        comp_ellipse->GetOriginal(x0, y0, r1, r2);
        view_ellipse->SetOriginal(x0, y0, r1, r2);

        Iterator ci;
        Picture* comp_pic = (Picture*)((NodeComp*)GetGraphicComp())->GetGraphic();
        comp_pic->First(ci);
        comp_pic->Next(ci);
        comp_pic->Next(ci);
        comp_pic->Next(ci);

        Picture* pic = (Picture*)GetGraphic();
        Iterator vi;
        pic->First(vi);
        pic->Next(vi);
        pic->Next(vi);
        pic->Next(vi);

        ArrowLine* comp_a;
        ArrowLine* view_a;
        for (; !comp_pic->Done(ci); ) {
	    comp_a = (ArrowLine*)comp_pic->GetGraphic(ci);
            view_a = (ArrowLine*)pic->GetGraphic(vi);
            *(Graphic*) view_a = *comp_a;
            IntCoord y1, y2;
            comp_a->GetOriginal(x0, y0, y1, y2);
            view_a->SetOriginal(x0, y0, y1, y2);
	    comp_a->Hidden() ? view_a->Hide() : view_a->Show();
	    comp_a->Desensitized() ? view_a->Desensitize() : view_a->Sensitize();
            comp_pic->Next(ci);
            pic->Next(vi);
        }
    }
    IncurDamage(list);
    EraseHandles();
}

SF_Ellipse* NodeView::GetEllipse() {
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    return (SF_Ellipse*)pic->GetGraphic(i);
}


SF_Ellipse* NodeView::GetEllipse2() {
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    pic->Next(i);
    return (SF_Ellipse*)pic->GetGraphic(i);
}    

TextGraphic* NodeView::GetText() {
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    pic->Next(i);
    if (((NodeComp*)GetGraphicComp())->GetGraph())
        pic->Next(i);
    return (TextGraphic*)pic->GetGraphic(i);
}    

int NodeView::SubEdgeIndex(ArrowLine* a) {
    int index = 0;
    ArrowLine* arrow;
    Picture* pic = (Picture*)GetGraphic();
    Iterator i;
    pic->First(i);
    pic->Next(i);
    pic->Next(i);
    pic->Next(i);

    for (;!pic->Done(i); pic->Next(i)) {
        arrow = (ArrowLine*)pic->GetGraphic(i);
	if (a == arrow)
	    return index;
	index++;
    }
    return -1;
}

Manipulator* NodeView::CreateManipulator(Viewer* v, Event& e, Transformer* rel,
					 Tool* tool)
{
    Rubberband* rub = nil;
    Manipulator* m = nil;
    Coord l, r, b, t;
    Editor* ed = v->GetEditor();
    int tabWidth = Math::round(.5*ivinch);

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
	if (!((NodeComp*)GetGraphicComp())->RequireLabel()) {
	    v->Constrain(e.x, e.y);
	    m = new DragManip(v, nil, rel, tool, DragConstraint(XFixed | YFixed));
	}
	else {
            ((OverlayEditor*)v->GetEditor())->MouseDocObservable()->textvalue(GraphKit::mouse_labl);
	    FontVar* fontVar = (FontVar*) ed->GetState("FontVar");
	    ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
	    PSFont* font = (fontVar == nil) ? psstdfont : fontVar->GetFont();
	    PSColor* fg = (colVar == nil) ? psblack : colVar->GetFgColor();
	    int lineHt = font->GetLineHt();

	    Painter* painter = new Painter;
	    painter->FillBg(false);
	    painter->SetFont(font);
	    painter->SetColors(fg, nil);
	    painter->SetTransformer(rel);

	    m = new TextManip(v, painter, lineHt, tabWidth, tool);
	}

    } else if (tool->IsA(MOVE_TOOL)) {
	RubberGroup* rubgroup = new RubberGroup(nil,nil);
        v->Constrain(e.x, e.y);
	rub = MakeRubberband(e.x, e.y);
	rubgroup->Append(rub);
	Iterator i;
	TopoNode* node = ((NodeComp*)GetGraphicComp())->Node();
	for (node->first(i); !node->done(i); node->next(i)) {
	    TopoEdge* edge = node->edge(node->elem(i));
	    int x0, y0, x1, y1;
	    if (edge->end_node() == node) {
		((EdgeComp*)edge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
		    GetOriginal(x0, y0, x1, y1);
	    }
	    else {
		((EdgeComp*)edge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
		    GetOriginal(x1, y1, x0, y0);
	    }
	    Transformer trans;
	    ((EdgeComp*)edge->value())->GetEdgeView(GetViewer())->GetArrowLine()->
		TotalTransformation(trans);
	    trans.Transform(x0, y0);
	    trans.Transform(x1, y1);
	    RubberLine* rubline = new RubberLine(nil, nil, x0-(x1-e.x), y0-(y1-e.y), x1, y1,
						 x1 - e.x, y1 - e.y);
	    rubgroup->Append(rubline);
	}
        m = new DragManip(
	    v, rubgroup, rel, tool, DragConstraint(HorizOrVert | Gravity)
	);
    } else if (tool->IsA(RESHAPE_TOOL)) {
        TextGraphic* textgr = GetText();
        FontVar* fontVar = (FontVar*) v->GetEditor()->GetState("FontVar");
        PSFont* font = (fontVar == nil) ? psstdfont : fontVar->GetFont();
        Painter* painter = new Painter;
        int lineHt = textgr->GetLineHeight();
        Coord xpos, ypos;
        rel = new Transformer;
        const char* text = textgr->GetOriginal();
        int size = strlen(text);
        
        textgr->TotalTransformation(*rel);
	if (size == 0)
	    rel->Transform(0, lineHt/2, xpos, ypos);
	else
	    rel->Transform(0, 0, xpos, ypos);
        painter->SetFont(textgr->GetFont() ? textgr->GetFont() : font);
	painter->SetColors(textgr->GetFgColor(), nil);
        painter->SetTransformer(rel);
        Unref(rel);
	int tabWidth = Math::round(.5*ivinch);

        m = new TextManip(
            v, text, size, xpos, ypos, painter, lineHt, tabWidth, tool
        );

    } else {
        m = GraphicView::CreateManipulator(v, e, rel, tool);
    }
    return m;
}


Rubberband* NodeView::MakeRubberband(IntCoord x, IntCoord y) {
  Coord l, r, b, t;
  Viewer* v = GetViewer();
  GetEllipse()->GetBox(l, b, r, t);
  Coord cx, cy;
  int rx, ry;
  GetEllipse()->GetOriginal(cx, cy, rx, ry);
  Rubberband* rub = new SlidingEllipse(nil, nil, l+(r-l)/2, b+(t-b)/2,
				       Math::round(rx * v->GetMagnification()),
				       Math::round(ry * v->GetMagnification()),
				       x, y);
  return rub;
}

Command* NodeView::InterpretManipulator(Manipulator* m) {
    Tool* tool = m->GetTool();
    Command* cmd = nil;

    if (tool->IsA(GRAPHIC_COMP_TOOL)) {
        TextGraphic* tpg = (TextGraphic*)((NodeComp*)GetGraphicComp())->GetText();
        SF_Ellipse* epg = (SF_Ellipse*)((NodeComp*)GetGraphicComp())->GetEllipse();
        TextGraphic* textgr;
	SF_Ellipse* ellipse;
	Coord xpos, ypos;
	DragManip* dm = (DragManip*) m;
	Editor* ed = dm->GetViewer()->GetEditor();
	if (!((NodeComp*)GetGraphicComp())->RequireLabel()) {
	    Transformer* rel = dm->GetTransformer();
	    Event initial = dm->GraspEvent();
	    Coord xpos = initial.x;
	    Coord ypos = initial.y;
	    if (rel != nil)
		rel->InvTransform(xpos, ypos);

	    FontVar* fontVar = (FontVar*) ed->GetState("FontVar");
	    PSFont* font = (fontVar == nil) ? psstdfont : fontVar->GetFont();
	    textgr = new TextGraphic("", tpg);
	    textgr->SetFont(font);            
            textgr->SetTransformer(nil);
            textgr->Translate(xpos, ypos);

 	    Coord expos, eypos;
	    int exradius, eyradius;
	    epg->GetOriginal(expos, eypos, exradius, eyradius);

	    ellipse = new SF_Ellipse(xpos, ypos, exradius, eyradius, epg);
	    ellipse->SetTransformer(nil);
	    BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
	    PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
	    ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
	    if (brVar != nil) ellipse->SetBrush(brVar->GetBrush());
	    if (patVar != nil && patVar->GetPattern()->None())
		ellipse->SetPattern(unidraw->GetCatalog()->ReadPattern("pattern",3));
	    else if (patVar != nil)
		ellipse->SetPattern(patVar->GetPattern());
	    if (colVar != nil) {
	        ellipse->FillBg(!colVar->GetBgColor()->None());
		ellipse->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
	    }

	    #if 0
	    textgr->Align(Center, ellipse, Center);
	    #else
	    ellipse->Align(Center, textgr, Center);
	    #endif
	    cmd = new PasteCmd(ed, new Clipboard(NewNodeComp(ellipse, textgr)));
	}
	else {
	    TextManip* tm = (TextManip*) m;
            ((OverlayEditor*)ed)->MouseDocObservable()->textvalue(GraphKit::mouse_lnode);
	    Editor* ed = tm->GetViewer()->GetEditor();
	    int size;
	    const char* text = tm->GetText(size);
	    if (size > 0) {
		tm->GetPosition(xpos, ypos);
		Transformer* rel = tm->GetPainter()->GetTransformer();
		if (rel != nil) {
		    rel->InvTransform(xpos, ypos);
		}

		int lineHt = tm->GetLineHeight();
		textgr = new TextGraphic(text, lineHt, tpg);
		textgr->SetTransformer(nil);
		textgr->Translate(xpos, ypos);
		Painter* p = tm->GetPainter();
		textgr->SetFont((PSFont*) p->GetFont());
		textgr->SetColors((PSColor*) p->GetFgColor(), nil);

		ellipse = new SF_Ellipse(xpos, ypos, xradius, yradius, epg);
		ellipse->SetTransformer(nil);
                BrushVar* brVar = (BrushVar*) ed->GetState("BrushVar");
	        PatternVar* patVar = (PatternVar*) ed->GetState("PatternVar");
	        ColorVar* colVar = (ColorVar*) ed->GetState("ColorVar");
		if (brVar != nil) ellipse->SetBrush(brVar->GetBrush());
		if (patVar != nil && patVar->GetPattern()->None())
		    ellipse->SetPattern(unidraw->GetCatalog()->ReadPattern("pattern",3));
		else if (patVar != nil)
		    ellipse->SetPattern(patVar->GetPattern());
		if (colVar != nil) {
		    ellipse->FillBg(!colVar->GetBgColor()->None());
		    ellipse->SetColors(colVar->GetFgColor(), colVar->GetBgColor());
		}

		#if 0
		textgr->Align(Center, ellipse, Center);
		#else
		ellipse->Align(Center, textgr, Center);
		#endif

		cmd = new PasteCmd(ed, new Clipboard(NewNodeComp(ellipse, textgr, true)));
	    } else if (size == 0) {
		Viewer* v = m->GetViewer();
		v->Update();          // to repair text display-incurred damage
	    }
	}
    } else if (tool->IsA(MOVE_TOOL)) {
	DragManip* dm = (DragManip*) m;
	Editor* ed = dm->GetViewer()->GetEditor();
	Transformer* rel = dm->GetTransformer();
        SlidingEllipse* sr = (SlidingEllipse*) ((RubberGroup*)dm->GetRubberband())->First();
        Coord x0, y0, x1, y1, dummy1, dummy2;
        float fx0, fy0, fx1, fy1;

        sr->GetOriginal(x0, y0, dummy1, dummy2);
        sr->GetCurrent(x1, y1, dummy1, dummy2);
        if (rel != nil) {
            rel->InvTransform(float(x0), float(y0), fx0, fy0);
            rel->InvTransform(float(x1), float(y1), fx1, fy1);
        }
        cmd = new MoveCmd(ed, fx1-fx0, fy1-fy0);
    } else if (tool->IsA(RESHAPE_TOOL)) {
        TextManip* tm = (TextManip*) m;
        int size;
        const char* text = tm->GetText(size);
	tm->GetViewer()->Update();

	Coord xpos, ypos;
	tm->GetPosition(xpos, ypos);
	Painter* p = tm->GetPainter();
	Transformer* rel = tm->GetPainter()->GetTransformer();
	int lineHt = tm->GetLineHeight();
	
	FullGraphic* pg = new FullGraphic(stdgraphic);
	FontVar* fontVar = (FontVar*) tm->GetViewer()->GetEditor()->GetState("FontVar");
	PSFont* font = (fontVar == nil) ? psstdfont : fontVar->GetFont();
	pg->SetFont(font);
	TextGraphic* textgr = new TextGraphic(text, lineHt, pg);
	textgr->SetTransformer(nil);
	((NodeComp*)GetGraphicComp())->GetEllipse()->Align(Center, textgr, Center);
	textgr->SetFont((PSFont*) p->GetFont());
//	textgr->SetColors((PSColor*) p->GetFgColor(), nil);
	
	cmd = new NodeTextCmd(tm->GetViewer()->GetEditor(),
			      (NodeComp*)GetGraphicComp(),
			      textgr);

    } else {
        cmd = GraphicView::InterpretManipulator(m);
    }
    return cmd;
}

ClassId NodeView::GetClassId() { return NODE_VIEW; }

boolean NodeView::IsA(ClassId id) {
    return id == NODE_VIEW || OverlayView::IsA(id);
}

Graphic* NodeView::GetGraphic () {
    Graphic* graphic = GraphicView::GetGraphic();
    
    if (graphic == nil) {
        NodeComp* nodeComp = (NodeComp*)GetGraphicComp();
        graphic = nodeComp->GetGraphic()->Copy();
        SetGraphic(graphic);
    }
    return graphic;
}

Graphic* NodeView::HighlightGraphic() {
    if (!_nv_gs) {
	Catalog* catalog = unidraw->GetCatalog();
	_nv_gs = new FullGraphic();
	_nv_gs->SetBrush(catalog->FindBrush(0xffff, 2));
	_nv_gs->SetPattern(catalog->ReadPattern("pattern", 4));
    }

    return _nv_gs;
}

/*****************************************************************************/

NodeScript::NodeScript (NodeComp* subj) : OverlayScript(subj) { }
ClassId NodeScript::GetClassId () { return NODE_SCRIPT; }

boolean NodeScript::IsA (ClassId id) { 
    return NODE_SCRIPT == id || OverlayScript::IsA(id);
}

void NodeScript::Attributes(ostream& out) {
    NodeComp* comp = (NodeComp*) GetSubject();

    /* graph */
    GraphComp* graph = comp->GetGraph();
    if (graph)
        out << " :graph \"" << graph->GetPathName() << "\"";
#if 0
    if (graph) {
        out << "\n"; 
        Indent(out);
        GraphScript* script = (GraphScript*)graph->Create(SCRIPT_VIEW);
        graph->Attach(script);
        script->Update();
	script->SetGSList(GetGSList());
        script->Definition(out);
        graph->Detach(script);
        delete script;
    }
#endif
    boolean reqlabel = comp->RequireLabel();
    out << " :reqlabel " << reqlabel;

    /* SF_Ellipse */
    Coord x0, y0;
    int r1, r2;
    SF_Ellipse* ellipse = comp->GetEllipse();
    ellipse->GetOriginal(x0, y0, r1, r2);
    out << " :ellipse " << x0 << "," << y0 << "," << r1 << "," << r2;
    Transformation(out, "ellipsetrans", ellipse);

    /* TextGraphic */
    TextGraphic* textgraphic = comp->GetText();
    int h = textgraphic->GetLineHeight();
    out << " :text " << h << ",";

    if (reqlabel) {
        const char* text = textgraphic->GetOriginal();
        ParamList::output_text(out, text, 0);
    }
    else
        out << "\"\"";
    Transformation(out, "texttrans", textgraphic);

    /* Picture GS*/
    Picture* pic = (Picture*)comp->GetGraphic();
    FullGS(out);
    Annotation(out);
    OverlayScript::Attributes(out);
}

boolean NodeScript::Definition (ostream& out) {
    out << script_name() << "(" ;
    Attributes(out);
    out << ")";
    return out.good();
}

boolean NodeScript::EmitGS(ostream& out, Clipboard* cb, boolean prevout) {
    NodeComp* comp = (NodeComp*) GetSubject();

    /* graph */
    GraphComp* graph = comp->GetGraph();
    if (graph) {
        GraphScript* script = (GraphScript*)graph->Create(SCRIPT_VIEW);
        graph->Attach(script);
        script->Update();
        script->EmitGS(out, cb, prevout);
        graph->Detach(script);
        delete script;
    }
    return OverlayScript::EmitGS(out, cb, prevout);
}

int NodeScript::ReadGraph(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    NodeComp* comp = (NodeComp*)addr1;
    char pathname[BUFSIZ];
    if (ParamList::parse_pathname(in, pathname, BUFSIZ, nil) != 0)
	return -1;

    /* check pathname for recursion */
    OverlayComp* parent = (OverlayComp*) comp->GetParent();
    while (parent != nil) {
	if (parent->GetPathName() && strcmp(parent->GetPathName(), pathname) == 0) {
	    cerr << "pathname recursion not allowed (" << pathname << ")\n";
	    return -1;
	}
	parent = (OverlayComp*) parent->GetParent();
    }

    GraphComp* graph = nil;
    GraphCatalog* catalog = (GraphCatalog*) unidraw->GetCatalog();
    catalog->SetImport(true);
    if (catalog->GraphCatalog::Retrieve(pathname, (Component*&)graph)) {
        catalog->SetImport(false);
	catalog->Forget(graph);
        *(GraphComp**)addr2 = graph;
	return 0;
    } else {
        catalog->SetImport(false);
	return -1;
    }
}

int NodeScript::ReadEllipse(istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    Coord x0, y0;
    int r1, r2;
    char delim;

    ParamList::skip_space(in);
    in >> x0 >> delim >> y0 >> delim >> r1 >> delim >> r2;

    if (!in.good()) {
	return -1;
    }
    else {
        Picture* pic = *(Picture**)addr1;
	pic->Append(new SF_Ellipse(x0, y0, r1, r2));
        return 0;
    }
}

int NodeScript::ReadText (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
    int line_height;
    char delim;
    char buf[BUFSIZ];

    in >> line_height >> delim;
    if (in.good())
	ParamList::parse_text(in, buf, BUFSIZ);    

    if (!in.good()) {
	return -1;
    }
    else {
    	TextGraphic* tg = new TextGraphic(buf, line_height); 	
	tg->FillBg(false);
        Picture* pic = *(Picture**)addr1;        
	pic->Append(tg);
        return 0;
    }
}

int NodeScript::ReadEllipseTransform (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
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
        Picture* pic = *(Picture**)addr1;
        Iterator i;
        pic->First(i);
        SF_Ellipse* ellipse = (SF_Ellipse*)pic->GetGraphic(i);
        ellipse->SetTransformer(t);
	Unref(t);
        return 0;
    }
}

int NodeScript::ReadTextTransform (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
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
        Picture* pic = *(Picture**)addr1;
        Iterator i;
        pic->First(i);
        pic->Next(i);
	TextGraphic* text = (TextGraphic*)pic->GetGraphic(i);
        text->SetTransformer(t);
	Unref(t);
        return 0;
    }
}

void NodeComp::nedges(int &nin, int &nout) const {
  nin = 0;
  nout = 0;

  TopoNode* toponode = Node();
  if (toponode) {
    Iterator it;
    toponode->first(it);
    while (!toponode->done(it)) {
      TopoEdge* edge = toponode->get_edge(it);
      if (edge && edge->end_node()==toponode) nin++;
      if (edge && edge->start_node()==toponode) nout++;
      toponode->next(it);
    }
  } 
}

