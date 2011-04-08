/*
 * Copyright (c) 1994 Vectaport Inc.
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

class ArrowLineOvComp : public LineOvComp {
public:
    ArrowLineOvComp(ArrowLine* = nil);
    ArrowLineOvComp(istream&, OverlayComp* parent = nil);

    ArrowLine* GetArrowLine();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovarrow_line_params;

friend OverlaysScript;
};

class ArrowLineOvView : public LineOvView {
public:
    ArrowLineOvView(ArrowLineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    virtual void Update();
    
    ArrowLineOvComp* GetArrowLineOvComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowLinePS : public LinePS {
public:
    ArrowLinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    virtual void Brush(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowLineScript : public LineScript {
public:
    ArrowLineScript(ArrowLineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream&);
    static int ReadOriginal(istream&, void*, void*, void*, void*);
    static int ReadScale(istream&, void*, void*, void*, void*);
    static int ReadHead(istream&, void*, void*, void*, void*);
    static int ReadTail(istream&, void*, void*, void*, void*);
};

class ArrowMultiLineOvComp : public MultiLineOvComp {
public:
    ArrowMultiLineOvComp(ArrowMultiLine* = nil);
    ArrowMultiLineOvComp(istream&, OverlayComp* parent = nil);

    ArrowMultiLine* GetArrowMultiLine();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovarrow_multiline_params;

friend OverlaysScript;
};

class ArrowMultiLineOvView : public MultiLineOvView {
public:
    ArrowMultiLineOvView(ArrowMultiLineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    virtual void Update();
    
    ArrowMultiLineOvComp* GetArrowMultiLineOvComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowMultiLinePS : public MultiLinePS {
public:
    ArrowMultiLinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    virtual void Brush(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowMultiLineScript : public MultiLineScript {
public:
    ArrowMultiLineScript(ArrowMultiLineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream&);
    static int ReadPoints(istream&, void*, void*, void*, void*);
    static int ReadScale(istream&, void*, void*, void*, void*);
    static int ReadHead(istream&, void*, void*, void*, void*);
    static int ReadTail(istream&, void*, void*, void*, void*);

protected:
    virtual const char* Name();
};

class ArrowSplineOvComp : public SplineOvComp {
public:
    ArrowSplineOvComp(ArrowOpenBSpline* = nil);
    ArrowSplineOvComp(istream&, OverlayComp* parent = nil);

    ArrowOpenBSpline* GetArrowOpenBSpline();
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovarrow_spline_params;

friend OverlaysScript;
};

class ArrowSplineOvView : public SplineOvView {
public:
    ArrowSplineOvView(ArrowSplineOvComp* = nil);

    virtual Command* InterpretManipulator(Manipulator*);
    virtual void Update();
    
    ArrowSplineOvComp* GetArrowSplineOvComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowSplinePS : public SplinePS {
public:
    ArrowSplinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    virtual void Brush(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class ArrowSplineScript : public SplineScript {
public:
    ArrowSplineScript(ArrowSplineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Definition(ostream&);
    static int ReadPoints(istream&, void*, void*, void*, void*);
    static int ReadScale(istream&, void*, void*, void*, void*);
    static int ReadHead(istream&, void*, void*, void*, void*);
    static int ReadTail(istream&, void*, void*, void*, void*);

protected:
    virtual const char* Name();
};

#endif
