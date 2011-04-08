/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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
 * Overlay Vertices component declarations.  A vertices component's
 * geometry is defined by a set of vertices.
 */

#ifndef overlay_vertices_h
#define overlay_vertices_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

#include <IV-2_6/_enter.h>

class Vertices;
class istream;

//: clone of VerticesComp derived from OverlayComp.
// base class for all multi-point components.
class VerticesOvComp : public OverlayComp {
public:
    Vertices* GetVertices();
    // return generic pointer to graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void GrowParamList(ParamList*);
    virtual boolean operator == (OverlayComp&);
protected:
    VerticesOvComp(Vertices* = nil, OverlayComp* parent = nil);
    VerticesOvComp(istream&, OverlayComp* parent = nil);
};

//: graphical view of VerticesOvComp.
// base class for all multi-point OverlayView objects.
class VerticesOvView : public OverlayView {
public:
    virtual void Interpret(Command*);
    virtual void Update();

    virtual void GetVertices(Coord*&, Coord*&, int&);
    VerticesOvComp* GetVerticesOvComp();
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    VerticesOvView(VerticesOvComp* = nil);

    virtual void CreateHandles();
    virtual boolean VertexChanged();
protected:
    int _reshapePt;
};

//: "PostScript" view of VerticesOvComp.
// base class for all multi-point OverlayPS objects.
class VerticesPS : public OverlayPS {
public:
    virtual boolean Definition(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    VerticesPS(OverlayComp* = nil);

    virtual const char* Name();
    // method to be filled in by derivative classes to tell them apart.
};

//: serialized view of VerticesOvComp.
// base class for all multi-point OverlayScript objects.
class VerticesScript : public OverlayScript {
public:
    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadPts(istream&, void*, void*, void*, void*);
    // read and set point list in a Vertices graphic.
protected:
    VerticesScript(VerticesOvComp* = nil);

    virtual const char* Name();
    // method to be filled in by derivative classes to tell them apart.
};

#include <IV-2_6/_leave.h>

#endif
