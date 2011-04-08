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

class EdgeComp : public OverlayComp {
public:
    EdgeComp(ArrowLine*, OverlayComp* parent = nil, int start_subedge = -1, 
	int end_subedge = -1);
    EdgeComp(istream&, OverlayComp* parent = nil);
    virtual ~EdgeComp();

    virtual Component* Copy();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    ArrowLine* GetArrowLine () { return (ArrowLine*) GetGraphic(); }

    TopoEdge* Edge() { return _edge; }
    EdgeView* GetEdgeView(Viewer*);

    int GetStartNode() { return _start_node; }
    void SetStartNode(int n) { _start_node = n; }

    int GetEndNode() { return _end_node; }
    void SetEndNode(int n) { _end_node = n; }

    int StartSubEdge() { return _start_subedge; }
    int EndSubEdge() { return _end_subedge; }

    virtual boolean operator == (OverlayComp&);

    static boolean clipline(Coord, Coord, Coord, Coord, Ellipse*, Coord&, Coord&);

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
};

class EdgeView : public OverlayView {
public:
    EdgeView(EdgeComp* = nil);
    virtual ~EdgeView();

    virtual void Update();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual void GetEndpoints(IntCoord&, IntCoord&, IntCoord&, IntCoord&);
    virtual Graphic* GetGraphic();
    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual Command* InterpretManipulator(Manipulator*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual Graphic* HighlightGraphic();
    ArrowLine* GetArrowLine () { return (ArrowLine*) GetGraphic(); }

protected:
    static FullGraphic* _ev_gs;
};

class EdgePS : public PostScriptView {
public:
    EdgePS(EdgeComp* = nil);

    virtual boolean Definition(ostream&);
    void Brush (ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void IndexNodes(int&, int&);
    int IndexNode (NodeComp *comp);
};

class EdgeScript : public OverlayScript {
public:
    EdgeScript(EdgeComp* = nil);

    virtual boolean Definition(ostream&);
    void IndexNodes(int&, int&);
    int IndexNode (NodeComp *comp);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
