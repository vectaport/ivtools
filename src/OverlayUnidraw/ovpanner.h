/*
 * Copyright (c) 1995,1999 Vectaport Inc.
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 */

/*
 * OverlayPanner - an interactor for two-dimensional scrolling and zooming.
 */

#ifndef ovpanner_h
#define ovpanner_h

#include <IV-2_6/InterViews/scene.h>

#include <IV-2_6/_enter.h>

//: flexible panner/zoomer/slider window.
class OverlayPanner : public MonoScene {
public:
    OverlayPanner(Interactor*, int size = 0, boolean panner = true,
		  boolean zoomer = true, boolean slider = true);
    OverlayPanner(const char*, Interactor*, int size = 0, 
		  boolean panner = true, boolean zoomer = true, 
		  boolean slider = true);
    virtual ~OverlayPanner();

    boolean& panner_on() { return _panner_on; }
    // set/get flag to enable/disable panner.
    boolean& zoomer_on() { return _zoomer_on; }
    // set/get flag to enable/disable zoomer.
    boolean& slider_on() { return _slider_on; }
    // set/get flag to enable/disable slider.
protected:
    int size;
    boolean _panner_on;
    boolean _zoomer_on;
    boolean _slider_on;

    virtual void Reconfig();
private:
    Scene* adjusters;
    Interactor* islider;

    void Init(Interactor*, int, boolean, boolean, boolean);
};

//: slider window within an OverlayPanner.
class OverlaySlider : public Interactor {
public:
    OverlaySlider(Interactor*);
    OverlaySlider(const char*, Interactor*);
    virtual ~OverlaySlider();

    virtual void Draw();
    virtual void Handle(Event&);
    virtual void Update();
    virtual void Reshape(Shape&);
    virtual void Resize();
protected:
    virtual void Reconfig();
    virtual void Redraw(IntCoord, IntCoord, IntCoord, IntCoord);
private:
    Interactor* interactor;
    Perspective* view;
    Perspective* shown;
    IntCoord left, bottom, right, top;
    IntCoord prevl, prevb, prevr, prevt;	// for smart update
    IntCoord llim, blim, rlim, tlim;	// sliding limits
    boolean constrained, syncScroll;
    int moveType;
    IntCoord origx, origy;

    void Init(Interactor*);
    IntCoord ViewX(IntCoord);
    IntCoord ViewY(IntCoord);
    IntCoord SliderX(IntCoord);
    IntCoord SliderY(IntCoord);
    void CalcLimits(Event&);		// calculate sliding limits
    void SizeKnob();			// calculate size of slider knob
    boolean Inside(Event&);		// true if inside slider knob
    void Constrain(Event&);		// constrain slider knob motion
    void Move(IntCoord dx, IntCoord dy);// move view to reflect slider position
    void Slide(Event&);			// rubberband rect while mousing
    void Jump(Event&);			// for click outside knob
};

#include <IV-2_6/_leave.h>

#endif
