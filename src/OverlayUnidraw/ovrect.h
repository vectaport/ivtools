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
 * Overlay Rect component declarations.
 */

#ifndef overlay_rect_h
#define overlay_rect_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

#include <IV-2_6/_enter.h>

class SF_Rect;
#include <iosfwd>

//: clone of RectComp derived from OverlayComp.
class RectOvComp : public OverlayComp {
public:
    RectOvComp(SF_Rect* = nil, OverlayComp* parent = nil);
    RectOvComp(istream&, OverlayComp* parent = nil);

    SF_Rect* GetRect();
    // return pointer to graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovrect_params;

friend class OverlaysScript;

    CLASS_SYMID("RectComp");
};

//: graphical view of RectOvComp.
class RectOvView : public OverlayView {
public:
    RectOvView(RectOvComp* = nil);

    virtual void Interpret(Command*);
    // interpret align-to-grid command, otherwise pass to base class.
    virtual void Update();

    virtual Manipulator* CreateManipulator(Viewer*, Event&,Transformer*,Tool*);
    // create manipulator to create, reshape, move, scale, or rotate rectangle.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulator to create, reshape, move, scale, or rotate rectangle.

    virtual void GetCorners(Coord* xvals, Coord* yvals);
    // return four corners of rectangle in current screen coordinates.
    RectOvComp* GetRectOvComp();
    // return pointer to associated component.
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void CreateHandles();
protected:
    int _reshapeCorner;
};

//: "PostScript" view of RectOvComp.
class RectPS : public OverlayPS {
public:
    RectPS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output PostScript fragment for SF_Rect.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of RectOvComp.
class RectScript : public OverlayScript {
public:
    RectScript(RectOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadOriginal(istream&, void*, void*, void*, void*);
    // read l,b,r,t arguments and construct a SF_Rect.
};

#include <IV-2_6/_leave.h>

#endif
