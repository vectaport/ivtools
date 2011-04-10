/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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
 * OverlayViewer - a Viewer with support for overlay operations
 */

#ifndef overlay_viewer_h
#define overlay_viewer_h

#include <Unidraw/viewer.h>

#include <IV-2_6/InterViews/alignment.h>

#include <IV-2_6/_enter.h>

class OverlayView;
class Transformer;

//: specialized Viewer.
class OverlayViewer : public Viewer {
public:
    OverlayViewer(
        Editor*, GraphicView*, UPage*, Grid* = nil, 
        Coord = 0, Coord = 0, Orientation = Normal,
	Alignment = Center, Zooming = Continuous
    );
    virtual ~OverlayViewer();

    void Update();
    // double-buffered damage repair.
    void Draw();
    // double-buffered drawing of entire screen.
    void Redraw(Coord, Coord, Coord, Coord);
    // double-buffered drawing of sub-screen.
    void Resize();
    // called after screen canvas is allocated.  From here the ::Configure
    // method is called on any components, and the OverlayEditor::InitCommands
    // is called as well.

    virtual void UseTool(Tool*, Event&);

    virtual void StartBuffering();
    // start up double buffering.  All subsequent draws go to an offscreen buffer.
    virtual void FinishBuffering(boolean);
    // finish up double buffering, and copy modified part of buffer to the screen.

    virtual OverlayView* GetOverlayView();

    void Chain(boolean pan = true, boolean zoom = true);
    // chain the panning or zooming of this viewer to other chained viewers.
    void Unchain(boolean pan = true, boolean zoom = true);
    // unchain the panning or zooming of this viewer from the other chained viewers.
    boolean Chained();
    // true if panning or zooming chained.
    boolean ChainedPan();
    // true if panning chained.
    boolean ChainedZoom();
    // true if zooming chained.

    void SetColorMap();
    // handle these command line arguments: -color6, -color5, -gray7, -gray6, -gray5.

    virtual void Adjust(Perspective&);

    void ScreenToDrawing(float xscreen, float yscreen, float& xdraw, float& ydraw);
    // utility method for converting screen coordinates to drawing coordinates.
    void ScreenToDrawing(Coord xscreen, Coord yscreen, float& xdraw, float& ydraw);
    // utility method for converting screen coordinates to drawing coordinates.
    void DrawingToScreen(float xdraw, float ydraw, float& xscreen, float& yscreen);
    // utility method for converting drawing coordinates to screen coordinates.
    void DrawingToScreen(float xdraw, float ydraw, Coord& xscreen, Coord& yscreen);
    // utility method for converting drawing coordinates to screen coordinates.

    void CenterToScreen(int sx, int sy);
    // pan xo that 'sx','sy' is at the center of the screen.

    void ScreenToGraphic
      (float xscreen, float yscreen, Graphic* gr, float& xgr, float& ygr);
    // utility method for converting screen coordinates to graphic relative 
    // coordinates.  The graphic coordinates are relative to the origin of the 
    // screen when the graphic was originally drawn , expect for rasters, stencils, 
    // and text which use their own origin instead.
    void ScreenToGraphic
      (Coord xscreen, Coord yscreen, Graphic* gr, float& xgr, float& ygr);
    // utility method for converting screen coordinates to graphic relative
    // coordinates.  
    void GraphicToScreen
      (Graphic* gr, float xgr, float ygr, float &xscreen, float& yscreen);    
    // utility method for converting graphic relative coordinates to screen
    // coordinates.  The graphic coordinates are relative to the origin of the 
    // screen when the graphic was originally drawn , expect for rasters, stencils, 
    // and text which use their own origin instead.
    void GraphicToScreen
      (Graphic* gr, float xgr, float ygr, int &xscreen, int& yscreen);    
    // utility method for converting graphic relative coordinates to screen
    // coordinates. 

    virtual OverlayView* GetCurrent() { return GetOverlayView(); }

    boolean scribble_pointer() { return _scribble_pointer; }
    // return flag that indicates whether the mouse is in continuous mode,
    // for smooth drawing of polygons and other multi-point graphics.
    void scribble_pointer(boolean flag) { _scribble_pointer = flag; }
    // set flag that indicates whether the mouse is in continuous mode,
    // for smooth drawing of polygons and other multi-point graphics.

    virtual void SetMagnification(float);

    void ExecuteCmd(Command* cmd);
    // indirect command execution for distributed whiteboard mechanism.
    // bulk of mechanism implemented in ComEditor.

protected:
    virtual void Zoom(Perspective&);
    virtual void Scroll(Perspective&);
    virtual void Manipulate(Manipulator*, Event&); // direct manipulation loop
    void PrepareDoubleBuf();

protected:
    boolean _needs_resize;
    boolean _pan_chain;
    boolean _zoom_chain;
    boolean _scribble_pointer;

    static Painter* xorPainter;

};

#include <IV-2_6/_leave.h>

#endif
