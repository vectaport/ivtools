/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

class OverlayFileComp : public OverlaysComp {
public:
    OverlayFileComp(OverlayComp* parent = nil);
    OverlayFileComp(Graphic*, OverlayComp* parent = nil);
    OverlayFileComp(istream&, OverlayComp* parent = nil);
    virtual ~OverlayFileComp();

    virtual void Append(GraphicComp*);

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void SetPathName(const char*);
    const char* GetPathName();
    OverlayIdrawComp* GetIdrawComp();
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _overlay_file_params;
    char * _pathname;
};

class OverlayFileView : public OverlaysView {
public:
    OverlayFileView();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

};

class OverlayFileScript : public OverlaysScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    OverlayFileScript(OverlayFileComp* = nil);
    virtual boolean Definition (ostream&);
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);

    static int ReadPathName(istream&, void*, void*, void*, void*);
};

#endif
