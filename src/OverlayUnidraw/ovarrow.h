/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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
 * ArrowLineOvComp, ArrowMultiLineOvComp, ArrowSplineOvComp and related
 * classes for components with arrowheads.
 */

#ifndef ovarrow_h
#define ovarrow_h

#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovspline.h>

class ArrowLine;
class ArrowMultiLine;
class ArrowOpenBSpline;
class ParamList;

//: clone of ArrowLineComp derived from OverlayComp.
class ArrowLineOvComp : public LineOvComp {
public:
    ArrowLineOvComp(ArrowLine* = nil);
    ArrowLineOvComp(istream&, OverlayComp* parent = nil);

    ArrowLine* GetArrowLine();
    // return pointer to graphic.
    virtual void Interpret(Command*);
    // interpret ArrowCmd, PatternCmd, or pass to base class method.
    virtual void Uninterpret(Command*);
    // uninterpret ArrowCmd or pass to base class method.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList(); // used by istream constructor.
    void GrowParamList(ParamList*);
    static ParamList* _ovarrow_line_params;

friend OverlaysScript;
};

//: graphical view of ArrowLineOvComp.
class ArrowLineOvView : public LineOvView {
public:
    ArrowLineOvView(ArrowLineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    // create or reshape arrow line, or pass to base class method.
    virtual void Update();
    // update view from component.
    
    ArrowLineOvComp* GetArrowLineOvComp();
    // return pointer to component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId); 
};

//: "PostScript" view of ArrowLineOvComp.
class ArrowLinePS : public LinePS {
public:
    ArrowLinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output PostScript fragment for rendering ArrowLine.
    virtual void Brush(ostream&);
    // output PostScript fragment for brush definition,
    // for compatibility with older versions of idraw.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of ArrowLineOvComp.
class ArrowLineScript : public LineScript {
public:
    ArrowLineScript(ArrowLineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream& out);
    // output variable-length ASCII record that defines the component.
    static int ReadOriginal(istream&, void*, void*, void*, void*);
    // read x0,y0,x1,y1 and construct ArrowLine graphic.
    static int ReadScale(istream&, void*, void*, void*, void*);
    // read arrow scale.
    static int ReadHead(istream&, void*, void*, void*, void*);
    // read arrow head flag.
    static int ReadTail(istream&, void*, void*, void*, void*);
    // read arrow tail flag.
};

//: clone of ArrowMultiLineComp derived from OverlayComp.
class ArrowMultiLineOvComp : public MultiLineOvComp {
public:
    ArrowMultiLineOvComp(ArrowMultiLine* = nil);
    ArrowMultiLineOvComp(istream&, OverlayComp* parent = nil);

    ArrowMultiLine* GetArrowMultiLine();
    // return pointer to graphic.
    virtual void Interpret(Command*);
    // interpret ArrowCmd, PatternCmd, or pass to base class method.
    virtual void Uninterpret(Command*);
    // uninterpret ArrowCmd or pass to base class method.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList(); // used by istream constructor.
    void GrowParamList(ParamList*); 
    static ParamList* _ovarrow_multiline_params;

friend OverlaysScript;
};

//: graphical view of ArrowMultiLineOvComp.
class ArrowMultiLineOvView : public MultiLineOvView {
public:
    ArrowMultiLineOvView(ArrowMultiLineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    // create or reshape arrow line, or pass to base class method.
    virtual void Update();
    // update view from component.
    
    ArrowMultiLineOvComp* GetArrowMultiLineOvComp();
    // return pointer to component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: "PostScript" view of ArrowMultiLineOvComp.
class ArrowMultiLinePS : public MultiLinePS {
public:
    ArrowMultiLinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output PostScript fragment for ArrowMultiLine.
    virtual void Brush(ostream&);
    // output PostScript fragment for brush definition,
    // for compatibility with older versions of idraw.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of ArrowMultiLineOvComp.
class ArrowMultiLineScript : public MultiLineScript {
public:
    ArrowMultiLineScript(ArrowMultiLineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read point list and construct ArrowMultiLine graphic.
    static int ReadScale(istream&, void*, void*, void*, void*);
    // read arrow scale.
    static int ReadHead(istream&, void*, void*, void*, void*);
    // read arrow head flag.
    static int ReadTail(istream&, void*, void*, void*, void*);
    // read arrow tail flag.

protected:
    virtual const char* Name();
    // name used for script output.
};

//: clone of ArrowSplineComp derived from OverlayComp.
class ArrowSplineOvComp : public SplineOvComp {
public:
    ArrowSplineOvComp(ArrowOpenBSpline* = nil);
    ArrowSplineOvComp(istream&, OverlayComp* parent = nil);

    ArrowOpenBSpline* GetArrowOpenBSpline();
    // return pointer to graphic.
    virtual void Interpret(Command*);
    // interpret ArrowCmd, PatternCmd, or pass to base class method.
    virtual void Uninterpret(Command*);
    // uninterpret ArrowCmd or pass to base class method.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList(); // used by istream constructor.
    void GrowParamList(ParamList*);
    static ParamList* _ovarrow_spline_params;

friend OverlaysScript;
};

//: graphical view of ArrowSplineOvComp.
class ArrowSplineOvView : public SplineOvView {
public:
    ArrowSplineOvView(ArrowSplineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    // create or reshape arrow line, or pass to base class method.
    virtual void Update();
    // update view from component.
    
    ArrowSplineOvComp* GetArrowSplineOvComp();
    // return pointer to component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: "PostScript" view of ArrowSplineOvComp.
class ArrowSplinePS : public SplinePS {
public:
    ArrowSplinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output PostScript fragment for ArrowSpline.
    virtual void Brush(ostream&);
    // output PostScript fragment for brush definition,
    // for compatibility with older versions of idraw.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of ArrowSplineOvComp.
class ArrowSplineScript : public SplineScript {
public:
    ArrowSplineScript(ArrowSplineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read point list and construct ArrowSpline graphic.
    static int ReadScale(istream&, void*, void*, void*, void*);
    // read arrow scale.
    static int ReadHead(istream&, void*, void*, void*, void*);
    // read arrow head flag.
    static int ReadTail(istream&, void*, void*, void*, void*);
    // read arrow tail flag.

protected:
    virtual const char* Name();
    // name used for script output.
};

#endif
