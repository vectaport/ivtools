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
 * Overlay Spline component declarations.
 */

#ifndef overlay_spline_h
#define overlay_spline_h

#include <OverlayUnidraw/ovvertices.h>

class SFH_OpenBSpline;
class SFH_ClosedBSpline;
class istream;

//: clone of SplineComp derived from OverlayComp.
class SplineOvComp : public VerticesOvComp {
public:
    SplineOvComp(SFH_OpenBSpline* = nil, OverlayComp* parent = nil);
    SplineOvComp(istream&, OverlayComp* parent = nil);

    SFH_OpenBSpline* GetSpline();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:

    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovspline_params;

friend OverlaysScript;
};

//: graphic view of SplineOvComp.
class SplineOvView : public VerticesOvView {
public:
    SplineOvView(SplineOvComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulator to create or reshape a spline.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulator to create or reshape a spline.

    SplineOvComp* GetSplineOvComp();
    // return pointer to associated component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean VertexChanged();
};

//: "PostScript" view of SplineOvComp.
class SplinePS : public VerticesPS {
public:
    SplinePS(OverlayComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesPS.
};

//: serialized view of SplineOvComp.
class SplineScript : public VerticesScript {
public:
    SplineScript(SplineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read set of points and construct a SFH_OpenBSpline.
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesScript.
};

//: clone of ClosedSplineComp derived from OverlayComp.
class ClosedSplineOvComp : public VerticesOvComp {
public:
    ClosedSplineOvComp(SFH_ClosedBSpline* = nil, OverlayComp* parent = nil);
    ClosedSplineOvComp(istream&, OverlayComp* parent = nil);

    SFH_ClosedBSpline* GetClosedSpline();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovclosed_spline_params;

friend OverlaysScript;
};

//: graphic view of ClosedSplineOvComp.
class ClosedSplineOvView : public VerticesOvView {
public:
    ClosedSplineOvView(ClosedSplineOvComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulator to create or reshape a spline.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulator to create or reshape a spline.

    ClosedSplineOvComp* GetClosedSplineOvComp();
    // return pointer to associated component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean VertexChanged();
};

//: "PostScript" view of ClosedSplineOvComp.
class ClosedSplinePS : public VerticesPS {
public:
    ClosedSplinePS(OverlayComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesPS.
};

//: serialized view of ClosedSplineOvComp.
class ClosedSplineScript : public VerticesScript {
public:
    ClosedSplineScript(ClosedSplineOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read set of points and construct a SFH_ClosedBSpline.
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesScript.
};

#endif
