/*
 * Copyright (c) 1994 Vectaport Inc.
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
class SF_Ellipse;
class TextGraphic;
class TopoNode;
class Viewer;
class istream;
class ostream;

class NodeComp : public OverlayComp {
public:
    NodeComp(SF_Ellipse*, TextGraphic*, 
	boolean reqlabel = false, OverlayComp* parent = nil);
    NodeComp(SF_Ellipse*, TextGraphic*, SF_Ellipse*, GraphComp*, 
	boolean reqlabel = false, OverlayComp* parent = nil);
    NodeComp(Picture*, boolean reqlabel =false, OverlayComp* parent = nil);
    NodeComp(GraphComp*);
    NodeComp(istream&, OverlayComp* parent = nil);
    virtual ~NodeComp();

    void SetGraph(GraphComp*);
    GraphComp* GetGraph();
    void GraphGraphic(SF_Ellipse* e = nil);

    virtual Component* Copy();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    SF_Ellipse* GetEllipse();
    TextGraphic* GetText();
    SF_Ellipse* GetEllipse2();
    EdgeComp* SubEdgeComp(int);
    ArrowLine* SubEdgeGraphic(int);

    TopoNode* Node() { return _node; }
    NodeView* GetNodeView(Viewer*);
    boolean RequireLabel() { return _reqlabel; }

    virtual boolean operator == (OverlayComp&);
protected:
    NodeComp(OverlayComp* parent = nil);

    GraphComp* _graph;
    TopoNode* _node;
    boolean _reqlabel;

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _node_params;
};

inline void NodeComp::SetGraph(GraphComp* comp) { _graph = comp; }
inline GraphComp* NodeComp::GetGraph() { return _graph; }

class NodeView : public OverlayView {
public:
    NodeView(NodeComp* = nil);
    virtual ~NodeView();

    virtual void Update();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Graphic* GetGraphic();
    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual Command* InterpretManipulator(Manipulator*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual Graphic* HighlightGraphic();

    SF_Ellipse* GetEllipse();
    TextGraphic* GetText();
    SF_Ellipse* GetEllipse2();
    int SubEdgeIndex(ArrowLine*);

protected:
    static FullGraphic* _nv_gs;
};

class NodeScript : public OverlayScript {
public:
    NodeScript(NodeComp* = nil);

    virtual boolean Definition(ostream&);
    void Attributes(ostream& out);
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    
    static int ReadGraph(istream&, void*, void*, void*, void*);
    static int ReadEllipse(istream&, void*, void*, void*, void*);
    static int ReadText(istream&, void*, void*, void*, void*);
    static int ReadEllipseTransform(istream&, void*, void*, void*, void*);
    static int ReadTextTransform(istream&, void*, void*, void*, void*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
