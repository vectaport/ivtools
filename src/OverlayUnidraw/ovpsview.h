/*
 * Copyright (c) 1994 Vectaport Inc., Cartoactive Systems
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
 * OverlayPS and related classes
 */

#ifndef ovpsview_h
#define ovpsview_h

#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Components/psview.h>

#include <IV-2_6/_enter.h>

class Command;

class OverlayPS : public PostScriptView {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual UList* GetPSFonts();

    void SetCommand(Command*);
    Command* GetCommand();
    OverlayPS* CreateOvPSView(GraphicComp*);
    OverlayPS* CreateOvPSViewFromGraphic(Graphic*, boolean comptree=false);

    OverlayComp* GetOverlayComp();
protected:
    OverlayPS(OverlayComp* = nil);

    Command* _command;
};

class OverlaysPS : public OverlayPS {
public:
    OverlaysPS(OverlayComp* = nil);
    virtual ~OverlaysPS();

    virtual boolean Emit(ostream&);
    virtual boolean Definition(ostream&);
    virtual void Update();
    OverlaysComp* GetOverlaysComp();

    virtual ExternView* GetView(Iterator);
    virtual void SetView(ExternView*, Iterator&);

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual boolean Done(Iterator);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    UList* Elem(Iterator);
    void DeleteView(Iterator&);
    void DeleteViews();
protected:
    UList* _views;
};

class OverlayIdrawPS : public OverlaysPS {
public:
    OverlayIdrawPS(OverlayComp* = nil);
protected:

    virtual void MiscProcs(ostream&);
    virtual void Creator(ostream&);
    virtual void ArrowHeader(ostream&);
    virtual void ConstProcs(ostream&);
    virtual void GridSpacing(ostream&);
    virtual void LineProc(ostream&);
    virtual void MultiLineProc(ostream&);
    virtual void BSplineProc(ostream&);
    virtual void Prologue(ostream&);
    virtual void SetBrushProc(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#include <IV-2_6/_leave.h>

#endif
