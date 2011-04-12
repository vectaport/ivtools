/*
 * Copyright (c) 2006-2007 Scott E. Johnston
 * Copyright (c) 1994,1999 Vectaport Inc.
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

#ifndef nodecomp_h
#define nodecomp_h

#include <GraphUnidraw/graphcomp.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Components/grcomp.h>
#include <Unidraw/Components/psview.h>

class ArrowLine;
class Command;
class EdgeComp;
class FullGraphic;
class GraphComp;
class NodeView;
class Picture;
class Rubberband;
class SF_Ellipse;
class TextGraphic;
class TopoNode;
class Viewer;
#include <iosfwd>

//: node component
// component for a node in an edge-node directed graph.  Default appearance
// is a single ellipse with centered text for regular nodes, a double
// ellipse with centered text for nodes with nested graphs inside them.
// Uses underlying TopoNode to represent topology.
class NodeComp : public OverlayComp {
public:
    NodeComp(SF_Ellipse*, TextGraphic*, 
	boolean reqlabel = false, OverlayComp* parent = nil);
    // construct node component with ellipse and text graphic
    // (text display not requiredd by default).
    NodeComp(SF_Ellipse*, TextGraphic*, SF_Ellipse*, GraphComp*, 
	boolean reqlabel = false, OverlayComp* parent = nil);
    // construct node component with internal graph
    // (text display not requiredd by default).
    NodeComp(Picture*, boolean reqlabel =false, OverlayComp* parent = nil);
    // construct node component with pre-made Picture graphic.
    NodeComp(GraphComp*);
    // construct node component around a graph.
    NodeComp(istream&, OverlayComp* parent = nil);
    // construct node component from istream, relying on GraphCatalog
    // to re-establish connections between edges and nodes.
    NodeComp(OverlayComp* parent = nil);
    // construct node component but defer anything graphical
    virtual ~NodeComp();

    virtual NodeComp* NewNodeComp(SF_Ellipse* ell, TextGraphic* txt, 
	boolean reqlabel = false, OverlayComp* parent = nil)
      { return new NodeComp(ell, txt, reqlabel, parent); }
    // virtual constructor for use of derived classes
    virtual NodeComp* NewNodeComp(SF_Ellipse* ell1, TextGraphic* txt, SF_Ellipse* ell2, GraphComp* comp, 
	boolean reqlabel = false, OverlayComp* parent = nil)
      { return new NodeComp(ell1, txt, ell2, comp, reqlabel, parent); }
    // virtual constructor for use of derived classes
    virtual NodeComp* NewNodeComp(Picture* pict, boolean reqlabel =false, OverlayComp* parent = nil)
      { return new NodeComp(pict, reqlabel, parent); }
    // virtual constructor for use of derived classes
    virtual NodeComp* NewNodeComp(GraphComp* comp)
      { return new NodeComp(comp); }
    // virtual constructor for use of derived classes
    virtual NodeComp* NewNodeComp(istream& strm, OverlayComp* parent = nil)
      { return new NodeComp(strm, parent); }
    // virtual constructor for use of derived classes
    virtual NodeComp* NewNodeComp(OverlayComp* parent = nil)
      { return new NodeComp(parent); }
    // virtual constructor for use of derived classes

    void SetGraph(GraphComp*);
    // set internal graph for this node.
    GraphComp* GetGraph();
    // return internal graph for this node.
    void GraphGraphic(SF_Ellipse* e = nil);
    // set auxiliary graphic used to indicate internal graph.

    virtual Component* Copy();
    virtual void Interpret(Command*);
    // handle delete, cut, move, and change-text commands, pass rest to base class.
    virtual void Uninterpret(Command*);
    // handle undoing move and graph-delete command, pass rest to base class.

    virtual void Read(istream&); // archaic read method.
    virtual void Write(ostream&); // archaic write method.
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    SF_Ellipse* GetEllipse();
    // return pointer to ellipse graphic.
    TextGraphic* GetText();
    // return pointer to text graphic.
    void SetText(TextGraphic*);
    // set pointer to text graphic.
    SF_Ellipse* GetEllipse2();
    // return pointer to second ellipse graphic used to indicate internal graph.
    EdgeComp* SubEdgeComp(int);
    // return pointer to nth edge component on internal graph.
    ArrowLine* SubEdgeGraphic(int);
    // return pointer to nth edge graphic on internal graph.

    TopoNode* Node() const { return _node; }
    // return pointer to underlying TopoNode.
    NodeView* GetNodeView(Viewer*);
    // return pointer to NodeView of this component in given viewer.
    boolean RequireLabel() { return _reqlabel; }
    // flag to indicate whether node must have label (text graphic).
    void RequireLabel(boolean flag) { _reqlabel = flag; }
    // flag to indicate whether node must have label (text graphic).

#if 0
    void update(Observable*);
    // update notification received from Observable.
#endif

    virtual void Notify(); 	 
    // override OverlayComp::Notify, separating view update from 
    // attribute list update. 

    EdgeComp* EdgeIn(int n) const;
    // return pointer to nth incoming edge.

    EdgeComp* EdgeOut(int n) const;
    // return pointer to nth outgoing edge.

    int EdgeInOrder(EdgeComp*) const;
    // return order of incoming edge.

    int EdgeOutOrder(EdgeComp*) const;
    // return order of outgoing edge.

    void nedges (int &nin, int &nout) const;
    // count number of input and ouput edges

    EdgeComp* EdgeByDir(int n, boolean out_edge) const;
    // return pointer to nth edge of given direction.

    NodeComp* NodeIn(int n) const;
    // return pointer to node on other side of nth incoming edge.

    NodeComp* NodeOut(int n) const;
    // return pointer to node on other side of nth outgoing edge.

    virtual boolean operator == (OverlayComp&);
    
    void index(int i) {_index = i; }
    // set temporary index for writing to file
    int index() { return _index; }
    // get temporary index for writing to file
protected:

    GraphComp* _graph;
    TopoNode* _node;
    boolean _reqlabel;
    int _index;

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _node_params;

    CLASS_SYMID("NodeComp");
};

inline void NodeComp::SetGraph(GraphComp* comp) { if (_graph) delete _graph; _graph = comp; }
inline GraphComp* NodeComp::GetGraph() { return _graph; }

//: graphical view of NodeComp.
class NodeView : public OverlayView {
public:
    NodeView(NodeComp* = nil);
    virtual ~NodeView();

    virtual void Update();
    // update view based on any changes to component.
    virtual void Interpret(Command*);
    // pass to GraphicView::Interpret.
    virtual void Uninterpret(Command*);
    // pass to GraphicView::Uninterpret.

    virtual Graphic* GetGraphic();
    // return pointer to graphic used for this view.
    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create tool-specific manipulator for creating, moving, or reshaping the node.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret tool-specific manipulator for creating, moving, or reshaping 
    // the node.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual Graphic* HighlightGraphic();
    // default highlighting -- solid brush of width 2, pattern #4 from catalog.

    SF_Ellipse* GetEllipse();
    // return pointer to view's ellipse graphic.
    TextGraphic* GetText();
    // return pointer to view's text graphic.
    SF_Ellipse* GetEllipse2();
    // return pointer to view's second ellipse graphic, the one for indicating
    // an internal graph.
    int SubEdgeIndex(ArrowLine*);
    // return index of ArrowLine graphic relative to edges on internal graph.

    virtual NodeComp* NewNodeComp(SF_Ellipse* ellipse, TextGraphic* txt, boolean reqlabel = false) 
      { return new NodeComp(ellipse, txt, reqlabel); }
    // virtual function to allow construction of specialized NodeComp's by specialized NodeView's

    virtual Rubberband* MakeRubberband(IntCoord x, IntCoord y);
    // make Rubberband specific to this node.

protected:
    static FullGraphic* _nv_gs;
};

//: serialized view of NodeComp.
class NodeScript : public OverlayScript {
public:
    NodeScript(NodeComp* = nil);

    virtual const char* script_name() { return "node"; }
    // for overriding in derived classes
    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    void Attributes(ostream& out);
    // specialized output of property list.
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    // specialized output of graphic states.
    
    static int ReadGraph(istream&, void*, void*, void*, void*);
    // read sub-graph and construct internal GraphComp.
    static int ReadEllipse(istream&, void*, void*, void*, void*);
    // read arguments of ellipse, and construct SF_Ellipse.
    static int ReadText(istream&, void*, void*, void*, void*);
    // read arguments of text, and construct TextGraphic.
    static int ReadEllipseTransform(istream&, void*, void*, void*, void*);
    // read transform for ellipse graphic.
    static int ReadTextTransform(istream&, void*, void*, void*, void*);
    // read transform for text graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
