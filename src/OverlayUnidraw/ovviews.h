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
 * OverlayView - view of an OverlayComp.
 * OverlaysView - view of an OverlaysComp.
 */

#ifndef overlay_views_h
#define overlay_views_h

#include <Unidraw/Components/grview.h>
#include <Unidraw/Components/psview.h>

#include <IV-2_6/_enter.h>

class Canvas;
class OverlayComp;
class OverlaysComp;
class OverlayIdrawComp;

class OverlayView : public GraphicView {
public:
    OverlayComp* GetOverlayComp();

    virtual void DrawHandles();
    virtual void RedrawHandles();
    virtual void InitHandles();
    virtual void EraseHandles();

    virtual boolean Highlightable();
    virtual boolean Highlighted();
    virtual void Highlight();
    virtual void Unhighlight();

    virtual boolean Hidable();
    virtual boolean Hidden();
    virtual void Hide();
    virtual void Show();

    virtual boolean Desensitizable();
    virtual boolean Desensitized();
    virtual void Desensitize();
    virtual void Sensitize();

    virtual Graphic* HighlightGraphic();

    virtual Selection* MakeSelection();

    virtual void AdjustForZoom(float factor, Coord cx, Coord cy);
    virtual void AdjustForPan(float dx, float dy);

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void FixSize(float factor=1.0);
    void UnfixSize();
    boolean FixedSize();
    void FixLocation();
    void UnfixLocation();
    boolean FixedLocation();

protected:
    OverlayView(OverlayComp* = nil);

    OverlayView* View(UList*);
    OverlayView* GetOverlayView(Graphic*);


    boolean _touched;
    boolean _fixed_size;
    float _fixed_size_factor;
    boolean _fixed_location;
};

class OverlaysView : public OverlayView {
public:
    OverlaysView(OverlaysComp* = nil);
    virtual ~OverlaysView();

    virtual void Interpret(Command*);
    virtual void Update();

    virtual Graphic* GetGraphic();
    OverlaysComp* GetOverlaysComp();

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual boolean Done(Iterator);
    int Index(Iterator);

    virtual GraphicView* GetView(Iterator);
    virtual void SetView(GraphicView*, Iterator&);

    virtual Selection* SelectAll();
    virtual Selection* ViewContaining(Coord, Coord);
    virtual Selection* ViewsContaining(Coord, Coord);
    virtual Selection* ViewIntersecting(Coord, Coord, Coord, Coord);
    virtual Selection* ViewsIntersecting(Coord, Coord, Coord, Coord);
    virtual Selection* ViewsWithin(Coord, Coord, Coord, Coord);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void AdjustForZoom(float factor, Coord cx, Coord cy);
    virtual void AdjustForPan(float dx, float dy);

protected:
    UList* Elem(Iterator);

    virtual void Add(GraphicView*);
    virtual void Append(GraphicView*);
    virtual void InsertBefore(Iterator, GraphicView*);
    virtual void Remove(Iterator&);
    virtual void DeleteView(Iterator&);
protected:
    UList* _views;
};

class OverlayIdrawView : public OverlaysView {
public:
    OverlayIdrawView(OverlayIdrawComp* = nil);
    
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#include <IV-2_6/_leave.h>

#endif
