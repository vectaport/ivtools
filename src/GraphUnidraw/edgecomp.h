/*
 * Copyright (c) 1994, 1999 Vectaport Inc.
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

#ifndef edgecomp_h
#define edgecomp_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Components/psview.h>

class ArrowLine;
class Command;
class EdgeView;
class Ellipse;
class FullGraphic;
class NodeComp;
class TopoEdge;
class Viewer;
class istream;
class ostream;

//: edge component
// component for an edge in an edge-node directed graph.  Default appearance
// is a solid line of width 1 with arrow pointing in the direction drawn.
// Uses underlying TopoEdge to represent topology.
class EdgeComp : public OverlayComp {
public:
    EdgeComp(ArrowLine*, OverlayComp* parent = nil, int start_subedge = -1, 
	int end_subedge = -1);
    // construct edge component with given ArrowLine graphic, optional
    // 'parent', and optional specification of sub-edges in graphs internal
    // to nodes it gets connected to.
    EdgeComp(istream&, OverlayComp* parent = nil);
    // construct edge component from istream, relying on GraphCatalog
    // to re-establish connections between edges and nodes.
    virtual ~EdgeComp();

    virtual Component* Copy();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    ArrowLine* GetArrowLine () { return (ArrowLine*) GetGraphic(); }
    // return pointer to graphic.

    TopoEdge* Edge() const { return _edge; }
    // return pointer to underlying TopoEdge.
    EdgeView* GetEdgeView(Viewer*);
    // return pointer to EdgeView of this component in given viewer.

    int GetStartNode() { return _start_node; }
    // get index of node on tail end of arrow.
    void SetStartNode(int n) { _start_node = n; }
    // set index of node on tail end of arrow.

    int GetEndNode() { return _end_node; }
    // get index of node on head end of arrow.
    void SetEndNode(int n) { _end_node = n; }
    // set index of node on head end of arrow.

    NodeComp* NodeStart() const;
    // return pointer to start node.

    NodeComp* NodeEnd() const;
    // return pointer to end node.

    int StartSubEdge() { return _start_subedge; }
    // index of connected edge in sub-graph of start node
    int EndSubEdge() { return _end_subedge; }
    // index of connected edge in sub-graph of end node

    virtual boolean operator == (OverlayComp&);

    static boolean clipline(Coord, Coord, Coord, Coord, Ellipse*, Coord&, Coord&);
    // clip edge graphic with node's ellipse graphic 

protected:
    TopoEdge* _edge;
    int _start_subedge;
    int _end_subedge;

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _edge_params;
    int _start_node;
    int _end_node;

    CLASS_SYMID("EdgeComp");
};

//: graphical view of EdgeComp
class EdgeView : public OverlayView {
public:
    EdgeView(EdgeComp* = nil);
    virtual ~EdgeView();

    virtual void Update();
    // update view based on any changes to component.
    virtual void Interpret(Command*);
    // handle delete, cut, edge-connect, edge-update, and move commands;
    // pass rest to base class.
    virtual void Uninterpret(Command*);
    // handle undoing edge-connect, delete, and move commands;
    // pass rest to base class.

    virtual void GetEndpoints(IntCoord&, IntCoord&, IntCoord&, IntCoord&);
    // return current end points of edge graphic.
    virtual Graphic* GetGraphic();
    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // make manipulators for create and move tools; pass rest to base class.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulators for create and move tools; pass rest to base class.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual Graphic* HighlightGraphic();
    // highlight edge graphic with red brush of width 2.
    ArrowLine* GetArrowLine () { return (ArrowLine*) GetGraphic(); }
    // return pointer to view's ArrowLine graphic.

protected:
    static FullGraphic* _ev_gs;
};

//: "PostScript" view of EdgeComp.
class EdgePS : public PostScriptView {
public:
    EdgePS(EdgeComp* = nil);

    virtual boolean Definition(ostream&);
    // output arrow line postscript fragment with embedded connectivity information.
    void Brush (ostream&);
    // backward compatible code for outputting brush definition.
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void IndexNodes(int& start, int& stop);
    // return index of start and stop nodes.
    int IndexNode (NodeComp *comp);
    // return index of given node.
};

//: serialized view of EdgeComp.
class EdgeScript : public OverlayScript {
public:
    EdgeScript(EdgeComp* = nil);

    virtual boolean Definition(ostream&);
    // output serialized view with information to allow restoring connectitivy.
    void IndexNodes(int& start, int& stop);
    // return index of start and stop nodes.
    int IndexNode (NodeComp *comp);
    // return index of given node.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
