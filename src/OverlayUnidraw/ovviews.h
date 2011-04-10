/*
 * Copyright (c) 2000 IET Inc.
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

//: base class for graphic view of an OverlayComp.
class OverlayView : public GraphicView {
public:
    OverlayComp* GetOverlayComp();

    virtual void DrawHandles();
    // draw tic mark handles and highlight some things with a new graphic state.
    virtual void RedrawHandles();
    // redraw tic mark handles and highlight some things with a new graphic state.
    virtual void InitHandles();
    // initialize tic mark handles and set up to highlight some things with a new 
    // graphic state.
    virtual void EraseHandles();
    // erase tic mark handles and unhighlight some things by replacing their
    // old graphic state.

    virtual boolean Highlightable();
    // true if set up to be highlighted with a graphic state. 
    virtual boolean Highlighted();
    // true if highlighted with a graphic state. 
    virtual void Highlight();
    // cause highlighting by graphic state to happen.
    virtual void Unhighlight();
    // undo any highlighting by graphic state to happen.

    virtual boolean Hidable();
    // true for this class.
    virtual boolean Hidden();
    // true if hidden.
    virtual void Hide();
    // hide the graphic.
    virtual void Show();
    // unhide the graphic.

    virtual boolean Desensitizable();
    // true for this class.
    virtual boolean Desensitized();
    // true if desensitized.
    virtual void Desensitize();
    // desensitize the graphic (ignore mouse events)
    virtual void Sensitize();
    // resensitize the graphic (pay attention to mouse events)

    virtual Graphic* HighlightGraphic();
    // graphic used to highlight by changing graphic state.  
    // A nil returned from this method disables the mechanism.

    virtual void HighlightGraphic(Graphic* hilite_gs);
    // graphic used to highlight by changing graphic state.  
    // A nil returned from this method disables the mechanism.

    virtual Selection* MakeSelection();
    // factor method to construct an OverlaySelection.

    virtual void AdjustForZoom(float factor, Coord cx, Coord cy);
    // called once per OverlayView before each zoom, to let
    // fixed size graphics adjust accordingly.
    virtual void AdjustForPan(float dx, float dy);
    // called once per OverlayView before each pan, to let
    // fixed location graphics adjust accordingly.

    virtual void Interpret(Command*);
    // interpret hide-view, desensitize-view, (un)fix-size, and (un)fix-location 
    // commands.
    virtual void Uninterpret(Command*);
    // uninterpret hide-view, desensitize-view, (un)fix-size, and (un)fix-location 
    // commands.

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create move tool manipulator.
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void FixSize(float factor=1.0);
    // fix size of graphic, with optional factor to reduce or increase effect.
    void UnfixSize();
    // disable fixing of graphic size.
    boolean FixedSize();
    // return flag to indicate if graphic is of fixed size.

    void FixLocation();
    // enable fixing of graphic location.
    void UnfixLocation();
    // disable fixing of graphic location.
    boolean FixedLocation();
    // return flag to indicate if graphic is of fixed location.

    Manipulator* CreateStretchManip(Viewer*, Event&, Transformer*, Tool*);
    // specialized method to construct OpaqueDragManip instead of DragManip.
protected:
    OverlayView(OverlayComp* = nil);

    OverlayView* View(UList*);
    OverlayView* GetOverlayView(Graphic*);


    boolean _touched;
    boolean _fixed_size;
    float _fixed_size_factor;
    boolean _fixed_location;
    Graphic* _hilite_gs;
};

//: graphical view of OverlaysComp.
class OverlaysView : public OverlayView {
public:
    OverlaysView(OverlaysComp* = nil);
    virtual ~OverlaysView();

    virtual void Interpret(Command*);
    // interpret align-to-grid command, pass rest to base class.
    virtual void Update();

    virtual Graphic* GetGraphic();
    OverlaysComp* GetOverlaysComp();
    // return pointer to associated graphic.

    virtual void First(Iterator&);
    // set iterator to first sub-view.
    virtual void Last(Iterator&);
    // set iterator to last sub-view.
    virtual void Next(Iterator&);
    // set iterator to next sub-view.
    virtual void Prev(Iterator&);
    // set iterator to previous sub-view.
    virtual boolean Done(Iterator);
    // return true if iterator off the end or beginning of list of sub-views.
    int Index(Iterator);
    // return index of where the iterator is pointing in the list of sub-views.

    virtual GraphicView* GetView(Iterator);
    // return sub-view pointed at by iterator.
    virtual void SetView(GraphicView*, Iterator&);
    // set sub-view pointed at by iterator.

    virtual Selection* SelectAll();
    // return selection with all sub-views.
    virtual Selection* ViewContaining(Coord, Coord);
    // return selection of foremost subview that contains a point.
    virtual Selection* ViewsContaining(Coord, Coord);
    // return selection of all subviews that contains a point.
    virtual Selection* ViewIntersecting(Coord, Coord, Coord, Coord);
    // return selection of foremost subview that intersect a rectangle.
    virtual Selection* ViewsIntersecting(Coord, Coord, Coord, Coord);
    // return selection of all subviews that intersect a rectangle.
    virtual Selection* ViewsWithin(Coord, Coord, Coord, Coord);
    // return selection of all subviews that fit within a rectangle.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void AdjustForZoom(float factor, Coord cx, Coord cy);
    // called once per OverlayView before each zoom, to let
    // fixed size graphics adjust accordingly.
    virtual void AdjustForPan(float dx, float dy);
    // called once per OverlayView before each pan, to let
    // fixed location graphics adjust accordingly.

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulator for laying down composite graphic
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulator by copying prototype
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

//: graphical view of OverlayIdrawComp.
class OverlayIdrawView : public OverlaysView {
public:
    OverlayIdrawView(OverlayIdrawComp* = nil);
    
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#include <IV-2_6/_leave.h>

#endif
