/*
 * Copyright (c) 1999 Vectaport Inc.
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

//: base class of "PostScript" views for OverlayComp objects.
class OverlayPS : public PostScriptView {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual UList* GetPSFonts();

    void SetCommand(Command*);
    // set command associated with this view, for reference by sub-views
    // when updating themselves.
    Command* GetCommand();
    // get command associated with this view, for reference by sub-views
    // when updating themselves.
    OverlayPS* CreateOvPSView(GraphicComp*);
    // utility method for creating a "PostScript" view from a component.
    OverlayPS* CreateOvPSViewFromGraphic(Graphic*, boolean comptree=false);
    // utility method for creating a "PostScript" view from a graphic.
    // The 'comptree' flag indicates whether any composite graphic is part 
    // of a tree of components or not.

    OverlayComp* GetOverlayComp();
protected:
    OverlayPS(OverlayComp* = nil);

    Command* _command;
};

//: "PostScript" view of an OverlaysComp.
class OverlaysPS : public OverlayPS {
public:
    OverlaysPS(OverlayComp* = nil);
    virtual ~OverlaysPS();

    virtual boolean Emit(ostream&);
    // output entire "PostScript" document to ostream.
    virtual boolean Definition(ostream&);
    // output fragment of "PostScript" document that corresponds to this
    // sub-tree of the entire component tree.
    virtual void Update();
    OverlaysComp* GetOverlaysComp();

    virtual ExternView* GetView(Iterator);
    // get sub-view pointed to by Iterator.
    virtual void SetView(ExternView*, Iterator&);
    // set sub-view pointed to by Iterator.

    virtual void First(Iterator&);
    // set iterator to first sub-view.
    virtual void Last(Iterator&);
    // set iterator to last sub-view.
    virtual void Next(Iterator&);
    // set iterator to sub-view immediately following current Iterator setting.
    virtual void Prev(Iterator&);
    // set iterator to sub-view immediately proceeding current Iterator setting.
    virtual boolean Done(Iterator);
    // return true if Iterator points off the end or beginning of list of sub-views.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    UList* Elem(Iterator);
    void DeleteView(Iterator&);
    void DeleteViews();
protected:
    UList* _views;
};

//: "PostScript" view of OverlayIdrawComp.
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
