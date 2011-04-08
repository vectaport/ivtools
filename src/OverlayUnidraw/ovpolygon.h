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
 * Overlay Polygon component declarations.
 */

#ifndef overlay_polygon_h
#define overlay_polygon_h

#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/scriptview.h>

class SF_Polygon;

//: clone of PolygonComp derived from OverlayComp.
class PolygonOvComp : public VerticesOvComp {
public:
    PolygonOvComp(SF_Polygon* = nil, OverlayComp* parent = nil);
    PolygonOvComp(istream&, OverlayComp* parent = nil);

    SF_Polygon* GetPolygon();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovpolygon_params;

friend OverlaysScript;

    CLASS_SYMID("PolygonComp");
};

//: graphical view of PolygonOvComp.
class PolygonOvView : public VerticesOvView {
public:
    PolygonOvView(PolygonOvComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulators for tools that create or reshape.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulators for tools that create or reshape.
    PolygonOvComp* GetPolygonOvComp();
    // return pointer to associated component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean VertexChanged();
};

//: "PostScript" view of PolygonOvComp.
class PolygonPS : public VerticesPS {
public:
    PolygonPS(OverlayComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesPS.
};

//: serialized view of PolygonOvComp.
class PolygonScript : public VerticesScript {
public:
    PolygonScript(PolygonOvComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadPoints(istream&, void*, void*, void*, void*);
    // read point list and construct an SF_Polygon.
protected:
    virtual const char* Name();
    // return name to differentiate from other VerticesScript.
};

#endif
