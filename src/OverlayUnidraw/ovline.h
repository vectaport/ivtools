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
 * OvLine and OvMultiLine component declarations.
 */

#ifndef overlay_line_h
#define overlay_line_h

#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/scriptview.h>

#include <IV-2_6/_enter.h>

class Line;
class ParamList;
class SF_MultiLine;

//: clone of LineComp derived from OverlayComp.
class LineOvComp : public OverlayComp {
public:
    LineOvComp(Line* = nil, OverlayComp* parent = nil);
    LineOvComp(istream&, OverlayComp* parent = nil);

    virtual void Interpret(Command*);
    // interpret pattern command, otherwise pass to base class.
    Line* GetLine();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
     ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovline_params;

};

//: graphical view of LineOvComp.
class LineOvView : public OverlayView {
public:
    LineOvView(LineOvComp* = nil);

    virtual void Interpret(Command*);
    // interpret align-to-grid command, otherwise pass to base class.
    virtual void Update();

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulators for tools that create, move, scale, rotate, or reshape.
    virtual Command* InterpretManipulator(Manipulator*);
    // intepret manipulators for tools that create, move, scale, rotate, or reshape.

    virtual void GetEndpoints(Coord&, Coord&, Coord&, Coord&);
    LineOvComp* GetLineOvComp();
    // return pointer to associated ocmponent.
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void CreateHandles();
};

//: "PostScript" view of LineOvComp.
class LinePS : public OverlayPS {
public:
    LinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output PostScript fragment for Line.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of LineOvComp.
class LineScript : public OverlayScript {
public:
    LineScript(LineOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadOriginal(istream&, void*, void*, void*, void*);
    // read x0,y0,x1,y1 and construct Line graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: clone of MultiLineComp derived from OverlayComp.
class MultiLineOvComp : public VerticesOvComp {
public:
    MultiLineOvComp(SF_MultiLine* = nil, OverlayComp* parent = nil);
    MultiLineOvComp(istream&, OverlayComp* parent = nil);

    SF_MultiLine* GetOvMultiLine();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovmultiline_params;

};

//: graphical view of MultiLineOvComp.
class MultiLineOvView : public VerticesOvView {
public:
    MultiLineOvView(MultiLineOvComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulators for tools that create or reshape.
    virtual Command* InterpretManipulator(Manipulator*);
    // intepret manipulators for tools that create or reshape.
    MultiLineOvComp* GetMultiLineOvComp();
    // return pointer to associated ocmponent.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean VertexChanged();
};

//: "PostScript" view of MultiLineOvComp.
class MultiLinePS : public VerticesPS {
public:
    MultiLinePS(OverlayComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesPS.
};

//: serialized view of LineOvComp.
class MultiLineScript : public VerticesScript {
public:
    MultiLineScript(MultiLineOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read set of points and construct SF_MultiLine graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesScript.
};

#include <IV-2_6/_leave.h>

#endif
