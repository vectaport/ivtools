/*
 * Copyright (c) 1994 Vectaport Inc.
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

class OverlayViewer : public Viewer {
public:
    OverlayViewer(
        Editor*, GraphicView*, UPage*, Grid* = nil, 
        Coord = 0, Coord = 0, Orientation = Normal,
	Alignment = Center, Zooming = Binary
    );
    virtual ~OverlayViewer();

    void Update();
    void Draw();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();

    virtual void UseTool(Tool*, Event&);

    virtual void StartBuffering();
    virtual void FinishBuffering(boolean);

    Transformer* GetRel();

    virtual OverlayView* GetOverlayView();

    void Chain(boolean pan = true, boolean zoom = true);
    void Unchain(boolean pan = true, boolean zoom = true);
    boolean Chained();
    boolean ChainedPan();
    boolean ChainedZoom();

    void SetColorMap();

    virtual void Adjust(Perspective&);

    void ScreenToDrawing(float xscreen, float yscreen, float& xdraw, float& ydraw);
    void ScreenToDrawing(Coord xscreen, Coord yscreen, float& xdraw, float& ydraw);
    void DrawingToScreen(float xdraw, float ydraw, float& xscreen, float& yscreen);
    void DrawingToScreen(float xdraw, float ydraw, Coord& xscreen, Coord& yscreen);

    void CenterToScreen(int sx, int sy);

    void ScreenToGraphic
      (float xscreen, float yscreen, Graphic* gr, float& xgr, float& ygr);
    void ScreenToGraphic
      (Coord xscreen, Coord yscreen, Graphic* gr, float& xgr, float& ygr);
    void GraphicToScreen
      (Graphic* gr, float xgr, float ygr, float &xscreen, float& yscreen);    
    void GraphicToScreen
      (Graphic* gr, float xgr, float ygr, Coord &xscreen, Coord& yscreen);    

    virtual OverlayView* GetCurrent() { return GetOverlayView(); }

    boolean scribble_pointer() { return _scribble_pointer; }
    void scribble_pointer(boolean flag) { _scribble_pointer = flag; }

    virtual void SetMagnification(float);

protected:
    virtual void Zoom(Perspective&);
    virtual void Scroll(Perspective&);
    virtual void Manipulate(Manipulator*, Event&); // direct manipulation loop
    void PrepareDoubleBuf();
protected:
    boolean _init;
    boolean _pan_chain;
    boolean _zoom_chain;
    boolean _scribble_pointer;

    static Painter* xorPainter;

};

#include <IV-2_6/_leave.h>

#endif
