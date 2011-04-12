/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/nodecomp.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovstencil.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/textfile.h>
#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/paramlist.h>

#include <TopoFace/topoedge.h>

#include <UniIdraw/idarrows.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Graphic/picture.h>

#include <InterViews/transformer.h>

#include <Attribute/attrlist.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

static Transformer* SaveTransformer (Graphic* g) {
    Transformer* orig = g->GetTransformer();
    Ref(orig);
    g->SetTransformer(new Transformer(orig));
    return orig;
}

static void RestoreTransformer (Graphic* g, Transformer* orig) {
    g->SetTransformer(orig);
    Unref(orig);
}

// ScaleToPostscriptCoords scales the picture to Postscript
// coordinates if screen and Postscript inches are different.

static void ScaleToPostScriptCoords (Graphic* g) {
    const double ps_inch = 72.;

    if (ivinch != ps_inch) {
	double factor = ps_inch / ivinch;
	g->Scale(factor, factor);
    }
}

/*****************************************************************************/

ParamList* GraphComp::_graph_params = nil;
int GraphComp::_symid = -1;

GraphComp::GraphComp (const char* pathname, OverlayComp* parent) 
    : OverlaysComp(parent) {
    _graphedges = new UList();
    _pathname = _basedir = _file = nil;
    if (pathname) 
	SetPathName(pathname);
    _gslist = nil;
}

GraphComp::GraphComp (Graphic* g, OverlayComp* parent) 
    : OverlaysComp(g, parent) {
    _graphedges = new UList();
    _pathname = _basedir = _file = nil;
    _gslist = nil;
}

GraphComp::GraphComp (istream& in, const char* pathname, OverlayComp* parent) 
    : OverlaysComp(parent) {
    _gslist = nil;
    _graphedges = new UList();
    _pathname = _basedir = _file = nil;
    SetPathName(pathname);
    _valid = GetParamList()->read_args(in, this);
}

GraphComp::~GraphComp () {
    delete _pathname;
    delete _basedir;
    delete _file;
    delete _graphedges;
    delete _gslist;
}

ClassId GraphComp::GetClassId () { return GRAPH_COMP; }

boolean GraphComp::IsA (ClassId id) {
    return GRAPH_COMP == id || OverlaysComp::IsA(id);
}

ParamList* GraphComp::GetParamList() {
    if (!_graph_params) 
	GrowParamList(_graph_params = new ParamList());
    return _graph_params;
}

Component* GraphComp::Copy () {
    GraphComp* comps = new GraphComp(GetPathName());
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    EdgeComp* edgecomp;
    for (UList* u = _graphedges->First(); u != _graphedges->End(); u = u->Next()) {
        edgecomp = (EdgeComp*) (*u)();
	comps->AppendEdge(edgecomp);
    }
    return comps;
}

void GraphComp::GrowParamList(ParamList* pl) {
    pl->add_param("edges_nodes", ParamStruct::required, &ParamList::read_int,
		  this, &_num_edge, &_num_node);
    pl->add_param("kids", ParamStruct::required, &GraphScript::ReadChildren, this, this);
    OverlaysComp::GrowParamList(pl); 
}

void GraphComp::AppendEdge(EdgeComp* comp) {
    _graphedges->Append(new UList(comp));
}

void GraphComp::SetPathName(const char* pathname) {
    delete _pathname;
    _pathname = pathname ? strdup(pathname) : nil;
    delete _basedir;
    _basedir = pathname ? strdup(pathname) : nil;
    if (_basedir) {
	char* last_slash = strrchr(_basedir, '/');
	if (last_slash)
	    *(last_slash+1) = '\0';
	else 
	    _basedir[0] = '\0';
    }
    delete _file;
    _file = pathname ? strdup(pathname) : nil;
    if (_file) {
	char* last_slash = strrchr(_file, '/');
	if (last_slash) {
	    delete _file;
	    _file = strdup(last_slash+1);
        }
        else
	    _file[0] = '\0';
    }
}

const char* GraphComp::GetPathName() { return _pathname; }

const char* GraphComp::GetBaseDir() { return _basedir; }

const char* GraphComp::GetFile() { return _file; }

void GraphComp::GrowIndexedGS(Graphic* gs) {
    if (!_gslist) _gslist = new Picture();
    _gslist->Append(gs);
}

Graphic* GraphComp::GetIndexedGS(int index) {
    if (_gslist) {
	Iterator i;
	for (_gslist->First(i); !_gslist->Done(i); _gslist->Next(i)) {
	    if (index==0) return _gslist->GetGraphic(i);
	    index--;
	}
	return nil;
    }
    return nil;
}

/*****************************************************************************/

GraphView::GraphView (GraphComp* subj) : OverlaysView(subj) { }

ClassId GraphView::GetClassId () { return GRAPH_VIEW; }

boolean GraphView::IsA (ClassId id) {
    return GRAPH_VIEW == id || OverlaysView::IsA(id);
}

/*****************************************************************************/

GraphScript::GraphScript (GraphComp* subj) : OverlaysScript(subj) {}

ClassId GraphScript::GetClassId () { return GRAPH_SCRIPT; }

boolean GraphScript::IsA (ClassId id) { 
    return GRAPH_SCRIPT == id || OverlaysScript::IsA(id);
}

void GraphScript::SetGSList(Clipboard* cb) {
    _gslist = cb;
}

Clipboard* GraphScript::GetGSList() {
    return _gslist;
}

boolean GraphScript::Definition (ostream& out) {
    GraphComp* comp = (GraphComp*) GetSubject();

    out << "graph(";

    GraphicComp* comps = GetGraphicComp();
    Iterator i;

    int num_edge = 0;
    int num_node = 0;
    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
        GraphicComp* comp = comps->GetComp(i);
if (comp->IsA(NODE_COMP))
	    num_node++;
	if (comp->IsA(EDGE_COMP))
	    num_edge++;
    }
    out << num_edge << "," << num_node;

    boolean status = true;
    First(i);
    out << "\n";
    for (; status && !Done(i); ) {
	ExternView* ev = GetView(i);
	Indent(out, 1);
        status = ev->Definition(out);
	Next(i);
	if (!Done(i)) out << ",\n";
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Indent(out);
    out << ")";
    return status;
}

int GraphScript::ReadChildren (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  OverlayComp* child = nil;
  GraphComp* comps = (GraphComp*)addr1;
  int num_edge = comps->GetNumEdge();
  int num_node = comps->GetNumNode();
  
  int startnode[num_edge];
  int endnode[num_edge];
  EdgeComp* edges[num_edge];
  NodeComp* nodes[num_node];
  int edge_cnt = 0;
  int node_cnt = 0;
  
  char buf[BUFSIZ];
  
  while (in.good()) {
    if (read_name(in, buf, BUFSIZ)) break;

    int status;
    if (status = read_gsptspic(buf, in, comps)) {
      if (status==-1) break;
    }
    
    else if (strcmp(buf, "node") == 0) {
      child = new NodeComp(in, comps);
      nodes[node_cnt] = (NodeComp*)child;
      node_cnt++;
    }

    else if (strcmp(buf, "edge") == 0) {
      child = new EdgeComp(in, comps);
      EdgeComp* comp = (EdgeComp*)child;
      startnode[edge_cnt] = comp->GetStartNode();
      endnode[edge_cnt] = comp->GetEndNode();
      edges[edge_cnt] = comp;
      edge_cnt++;
    } 

    else {
      child = read_obj(buf, in, comps);
      if (!child) return -1;
    }

    if (child) {
      if (in.good() && child->valid()) {
	comps->Append(child);
      } else {
	/* report failure even if one child fails */
	delete child;
	return -1;
      }
    }
  }
  for (int i=0; i<num_edge; i++) {
    int start_id = startnode[i];
    int end_id = endnode[i];
    if (start_id < 0 || end_id < 0)
      comps->AppendEdge(edges[i]);
    edges[i]->Edge()->
      attach_nodes(start_id < 0 ? nil : nodes[start_id]->Node(), 
		   end_id < 0 ? nil : nodes[end_id]->Node());
  }
  return 0;
}

/*****************************************************************************/


ParamList* GraphIdrawComp::_graph_idraw_params = nil;
int GraphIdrawComp::_symid = -1;

GraphIdrawComp::GraphIdrawComp (const char* pathname, OverlayComp* parent) 
   : OverlayIdrawComp(pathname, parent) { }

ClassId GraphIdrawComp::GetClassId () { return GRAPH_IDRAW_COMP; }

boolean GraphIdrawComp::IsA (ClassId id) {
    return GRAPH_IDRAW_COMP == id || OverlayIdrawComp::IsA(id);
}

GraphIdrawComp::GraphIdrawComp (istream& in, const char* pathname, OverlayComp* parent) 
    : OverlayIdrawComp(pathname, parent) {
    _valid = GetParamList()->read_args(in, this);
}

ParamList* GraphIdrawComp::GetParamList() {
    if (!_graph_idraw_params) 
	GrowParamList(_graph_idraw_params = new ParamList());
    return _graph_idraw_params;
}

void GraphIdrawComp::GrowParamList(ParamList* pl) {
    pl->add_param("edges_nodes", ParamStruct::required, &ParamList::read_int,
		  this, &_num_edge, &_num_node);
    pl->add_param("kids", ParamStruct::required, &GraphIdrawScript::ReadChildren, this, this);
    OverlayIdrawComp::GrowParamList(pl); 
    return;
}

Component* GraphIdrawComp::Copy () {
    GraphIdrawComp* comps = new GraphIdrawComp(GetPathName());
    if (attrlist()) comps->SetAttributeList(new AttributeList(attrlist()));
    Iterator i;
    First(i);
    while (!Done(i)) {
	comps->Append((GraphicComp*)GetComp(i)->Copy());
	Next(i);
    }
    return comps;
}

void GraphIdrawComp::Interpret (Command* cmd) {
    Editor* ed = cmd->GetEditor();
    Graphic* gr = GetGraphic();

    if (gr == nil) {
        return;

    } else if (cmd->IsA(UNGROUP_CMD)) {
        UngroupCmd* ucmd = (UngroupCmd*) cmd;
        Component* edComp = ucmd->GetEditor()->GetComponent();

        if (edComp == (Component*) this) {
            Clipboard* cb = cmd->GetClipboard();
            Clipboard* kids = new Clipboard;
            ucmd->SetKids(kids);
            Iterator i;

            for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                OverlayComp* parent = (OverlayComp*)cb->GetComp(i);
                unidraw->CloseDependents(parent);
                Ungroup(parent, kids, cmd);
            }
            Notify();
            SelectClipboard(kids, ed);
            unidraw->Update();

        } else {
            cmd->GetClipboard()->Append(this);
        }

    } 
    else
	OverlayIdrawComp::Interpret(cmd);
}

void GraphIdrawComp::Ungroup (OverlayComp* parent, Clipboard* cb, Command* cmd) {
    Iterator i, insertPt;
    parent->First(i);

    if (!parent->Done(i)) {
        SetComp(parent, insertPt);

        for (parent->First(i); !parent->Done(i); parent->Next(i)) {
            OverlayComp* kid = (OverlayComp*) parent->GetComp(i);
            cmd->Store(kid, new UngroupData(parent, kid->GetGraphic()));
        }

        cmd->Store(parent, new GSData(parent->GetGraphic()));
        for (parent->First(i); !parent->Done(i); parent->Next(i)) {
            OverlayComp* kid = (OverlayComp*) parent->GetComp(i);
	    Graphic* gr = kid->GetGraphic();
	    gr->concat(gr, parent->GetGraphic(), gr);
	    if (kid->IsA(EDGE_COMP)) {
		int x0, y0, x1, y1;
		EdgeComp* edge = ((EdgeComp*)kid);
		edge->GetArrowLine()->GetOriginal(x0, y0, x1, y1);
		edge->GetArrowLine()->GetTransformer()->Transform(x0, y0, x0, y0);
		edge->GetArrowLine()->GetTransformer()->Transform(x1, y1, x1, y1);
		edge->GetArrowLine()->SetOriginal(x0, y0, x1, y1);
		edge->GetArrowLine()->SetTransformer(nil);
	    }
	}
        parent->First(i);

        do {
            OverlayComp* kid = (OverlayComp*)parent->GetComp(i);
            parent->Remove(i);
            InsertBefore(insertPt, kid);
            cb->Append(kid);
        } while (!parent->Done(i));

        Remove(parent);
    }
}

/*****************************************************************************/

GraphIdrawView::GraphIdrawView (GraphIdrawComp* subj) : OverlayIdrawView(subj) { }

ClassId GraphIdrawView::GetClassId () { return GRAPH_IDRAW_VIEW; }

boolean GraphIdrawView::IsA (ClassId id) {
    return GRAPH_IDRAW_VIEW == id || OverlayIdrawView::IsA(id);
}

/*****************************************************************************/

GraphIdrawScript::GraphIdrawScript (GraphIdrawComp* subj) : OverlayIdrawScript(subj) {}

ClassId GraphIdrawScript::GetClassId () { return GRAPH_IDRAW_SCRIPT; }

boolean GraphIdrawScript::IsA (ClassId id) { 
    return GRAPH_IDRAW_SCRIPT == id || OverlayIdrawScript::IsA(id);
}

boolean GraphIdrawScript::Emit (ostream& out) {
    out << "graphdraw(";

    GraphicComp* comps = GetGraphicComp();
    Iterator i;

    int num_edge = 0;
    int num_node = 0;
    for (comps->First(i); !comps->Done(i); comps->Next(i)) {
        GraphicComp* comp = comps->GetComp(i);
	if (comp->IsA(NODE_COMP)) {
	    ((NodeComp*)comp)->index(num_node);
	    num_node++;
	}
	if (comp->IsA(EDGE_COMP))
	    num_edge++;
    }
    out << num_edge << "," << num_node;

    /* make list and output unique graphic states */
    _gslist = new Clipboard();
    boolean gsout = EmitGS(out, _gslist, false);

    boolean status = true;
    First(i);
    if (!Done(i) ) {
	if (gsout) out << ",";
	out << "\n";
    }
    for (; status && !Done(i); ) {
	ExternView* ev = GetView(i);
	Indent(out);
        status = ev->Definition(out);
	Next(i);
	if (!Done(i)) out << ",\n";
    }

    out << "\n";
    FullGS(out);
    Annotation(out);
    Attributes(out);
    out << ")\n";
    return status;
}

int GraphIdrawScript::ReadChildren (istream& in, void* addr1, void* addr2, void* addr3, void* addr4) {
  OverlayComp* child = nil;
  GraphIdrawComp* comps = (GraphIdrawComp*)addr1;
  int num_edge = comps->GetNumEdge();
  int num_node = comps->GetNumNode();
  
  int startnode[num_edge];
  int endnode[num_edge];
  EdgeComp* edges[num_edge];
  NodeComp* nodes[num_node];
  int edge_cnt = 0;
  int node_cnt = 0;
  
  char buf[BUFSIZ];
  
  while (in.good()) {
    if (read_name(in, buf, BUFSIZ)) break;

    int status;
    if (status = read_gsptspic(buf, in, comps)) {
      if (status==-1) break;
    }
    
    else if (strcmp(buf, "node") == 0) {
      child = new NodeComp(in, comps);
      nodes[node_cnt] = (NodeComp*)child;
      node_cnt++;
    }

    else if (strcmp(buf, "edge") == 0) {
      child = new EdgeComp(in, comps);
      EdgeComp* comp = (EdgeComp*)child;
      startnode[edge_cnt] = comp->GetStartNode();
      endnode[edge_cnt] = comp->GetEndNode();
      edges[edge_cnt] = comp;
      edge_cnt++;
    } 

    else {
      child = read_obj(buf, in, comps);
      if (!child) return -1;
    }

    if (child) {
      if (in.good() && child->valid()) {
	comps->Append(child);
      } else {
	/* report failure even if one child fails */
	delete child;
	return -1;
      }
    }
  }
  for (int i=0; i<num_edge; i++) {
    int start_id = startnode[i];
    int end_id = endnode[i];
    edges[i]->Edge()->
      attach_nodes(start_id < 0 ? nil : nodes[start_id]->Node(), 
		   end_id < 0 ? nil : nodes[end_id]->Node());
    #if defined(GRAPH_OBSERVABLES)
    if (start_id >=0 && end_id >=0) 
      edges[i]->NodeStart()->attach(edges[i]->NodeEnd());
    #endif
  }
  return 0;
}
