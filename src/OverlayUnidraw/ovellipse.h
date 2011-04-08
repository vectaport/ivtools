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
 * Overlay Ellipse component declarations.
 */

#ifndef overlay_ellipse_h
#define overlay_ellipse_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

#include <IV-2_6/_enter.h>

class SF_Ellipse;
class istream;

//: clone of EllipseComp derived from OverlayComp.
class EllipseOvComp : public OverlayComp {
public:
    EllipseOvComp(SF_Ellipse* = nil);
    // construct with stroke-filled ellipse graphic.
    EllipseOvComp(istream&, OverlayComp* parent = nil);
    // construct from istream.

    SF_Ellipse* GetEllipse();
    // return pointer to stroke-filled ellipse graphic.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovellipse_params;

friend OverlaysScript;

    CLASS_SYMID("EllipseComp"); 
};

//: graphical view of EllipseOvComp.
class EllipseOvView : public OverlayView {
public:
    EllipseOvView(EllipseOvComp* = nil);

    virtual void Interpret(Command*);
    // interpret align-to-grid command; pass rest to base class.
    virtual void Update();
    // update from graphic in component.

    virtual Manipulator* CreateManipulator(Viewer*, Event&,Transformer*,Tool*);
    // create rubber ellipse manipulator.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret rubber ellipse manipulator to construct stroke-filled ellipse.

    EllipseOvComp* GetEllipseOvComp();
    // return pointer to component.
    virtual Graphic* GetGraphic();
    // return pointer to view-side graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: "PostScript" view of EllipseOvComp.
class EllipsePS : public OverlayPS {
public:
    EllipsePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // generate "PostScript" fragment for SF_Ellipse.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of EllipseOvComp.
class EllipseScript : public OverlayScript {
public:
    EllipseScript(EllipseOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadOriginal(istream&, void*, void*, void*, void*);
    // read arguments for ellipse and construct an SF_Ellipse.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#include <IV-2_6/_leave.h>

#endif
