/*
 * Copyright (c) 1994,1995,1999 Vectaport Inc.
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

#ifndef ovfile_h
#define ovfile_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

//: component used to import a sub-tree by pathname.
class OverlayFileComp : public OverlaysComp {
public:
    OverlayFileComp(OverlayComp* parent = nil);
    // empty constructor.
    OverlayFileComp(Graphic*, OverlayComp* parent = nil);
    // construct for arbitrary graphic -- pathname set later.
    OverlayFileComp(istream&, OverlayComp* parent = nil);
    // construct from pathname read from istream.
    virtual ~OverlayFileComp();

    virtual void Append(GraphicComp*);
    // regular append then copy pointer to attribute list up to OverlayFileComp.

    virtual void Interpret(Command*);
    // disallow ungroup command, otherwise pass to base class.
    virtual void Uninterpret(Command*);
    // disallow undoing ungroup command, otherwise pass to base class.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void SetPathName(const char*);
    // concrete implementation of this method.
    const char* GetPathName();
    // concrete implementation of this method.
    OverlayIdrawComp* GetIdrawComp();
    // return point to underlying top-level component.
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _overlay_file_params;
    char * _pathname;
};

//: graphical view of OverlayFileComp.
class OverlayFileView : public OverlaysView {
public:
    OverlayFileView();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

};

//: serialized view of OverlayFileComp.
class OverlayFileScript : public OverlaysScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    OverlayFileScript(OverlayFileComp* = nil);
    virtual boolean Definition (ostream&);
    // output record with only pathname to represent entire subtree.
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    // skip OverlaysScript::EmitGS for OverlayScript::EmitGS.

    static int ReadPathName(istream&, void*, void*, void*, void*);
    // read pathname from istream and used to construct OverlayIdrawComp
    // inside an OverlayFileComp.
};

#endif
