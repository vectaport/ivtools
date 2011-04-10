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
 * Overlay Stencil component declarations.
 */

#ifndef overlay_stencil_h
#define overlay_stencil_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

class StencilScript;
class UStencil;
class istream;

//: clone of StencilComp derived from OverlayComp.
class StencilOvComp : public OverlayComp {
public:
    StencilOvComp(UStencil* = nil, const char* pathname = nil, OverlayComp* parent = nil);
    StencilOvComp(istream&, OverlayComp* parent = nil);
    virtual ~StencilOvComp();

    UStencil* GetStencil();
    // return pointer to graphic.
    virtual void SetPathName(const char*);
    // set pathname associated with this stencil
    const char* GetPathName();
    // return pathname associated with this stencil
    virtual boolean GetByPathnameFlag() { return _by_pathname;}
    // return flag that indicates whether component will be serialized
    // by data or by pathname.
    virtual void SetByPathnameFlag(boolean flag) { _by_pathname = flag;}
    // set flag that indicates whether component will be serialized
    // by data or by pathname.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovstencil_params;

protected:
    char* _pathname;
    boolean _by_pathname;

friend StencilScript;
};

//: graphical view of StencilOvComp.
class StencilOvView : public OverlayView {
public:
    StencilOvView(StencilOvComp* = nil);

    virtual void Update();

    StencilOvComp* GetStencilOvComp();
    // return pointer to associated component.
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: "PostScript" view of StencilOvComp.
class StencilPS : public OverlayPS {
public:
    StencilPS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output "PostScript" fragment for this component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of StencilOvComp.
class StencilScript : public OverlayScript {
public:
    StencilScript(StencilOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadStencil(istream&, void*, void*, void*, void*);
    // read stencil pathname if it exists, and construct a Ustencil
    // with two Bitmap's inside (image and mask).
    static int ReadImageBitmap(istream&, void*, void*, void*, void*);
    // read bits (1's and 0's separated by commas) of the image part of
    // a stencil directly from istream.
    static int ReadMaskBitmap(istream&, void*, void*, void*, void*);
    // read bits (1's and 0's separated by commas) of the mask part of
    // a stencil directly from istream.
    static Bitmap* read_bitmap(istream&);
    // read bits (1's and 0's separated by commas) and construct a Bitmap.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean GetByPathnameFlag();
};

#endif
