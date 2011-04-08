/*
 * Copyright (c) 1995 Vectaport Inc.
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
 * 
 */

/*
 * OverlayPanner implementation.
 */

#include <OverlayUnidraw/ovadjuster.h>
#include <OverlayUnidraw/ovpanner.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

#include <InterViews/pattern.h>
#include <IV-2_6/InterViews/border.h>
#include <IV-2_6/InterViews/box.h>
#include <IV-2_6/InterViews/glue.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/perspective.h>
#include <IV-2_6/InterViews/rubrect.h>
#include <IV-2_6/InterViews/sensor.h>
#include <IV-2_6/InterViews/shape.h>
#include <OS/math.h>
#include <string.h>


#include <IV-2_6/_enter.h>

/*****************************************************************************/

OverlayPanner::OverlayPanner (Interactor* i, int size, boolean panner,
			      boolean zoomer,  boolean slider) {

    Init(i, size, panner, zoomer, slider);
}

OverlayPanner::OverlayPanner (const char* name, Interactor* i, int size,
			      boolean panner, boolean zoomer,
			      boolean slider) {
    SetInstance(name);
    Init(i, size, panner, zoomer, slider);
}

OverlayPanner::~OverlayPanner() { }

/* 0.3 second delay for auto-repeat */
static int DELAY = 3;

void OverlayPanner::Init (Interactor* i, int n, boolean panner,
			  boolean zoomer, boolean slider) {

    panner_on() = panner;
    zoomer_on() = zoomer;
    slider_on() = slider;

    SetClassName("OverlayPanner");
    size = n;
    
    VBox* panner_vbox = nil;
    if (panner_on())
      panner_vbox = 
	new VBox(
		 new VGlue,
		 new OvUpMover(i, DELAY),
		 new HBox(
			  new HGlue,
			  new OvLeftMover(i, DELAY),
			  new HGlue,
			  new OvRightMover(i, DELAY),
			  new HGlue
			  ),
		 new OvDownMover(i, DELAY),
		 new VGlue
		 );
    
    VBox* zoomer_vbox = nil;
    if (zoomer_on())
      zoomer_vbox = 
	new VBox(
		 new VGlue(2),
		 new Enlarger(i),
		 new VGlue(4),
		 new Reducer(i),
		 new VGlue(2)
		 );

    
    if (panner_vbox || zoomer_vbox) {
      adjusters = new HBox();
      adjusters->Insert(new HGlue());
      if (panner_vbox) {
	adjusters->Insert(panner_vbox);
	adjusters->Insert(new HGlue);
      }
      if (zoomer_vbox) {
	adjusters->Insert(zoomer_vbox);
	adjusters->Insert(new HGlue);
      }

      if (slider_on()) {
	islider = new OverlaySlider(i);
	Insert(new VBox(adjusters, new HBorder, islider));
      } else 
	Insert(adjusters);
    } 
      
}
    
void OverlayPanner::Reconfig () {
    MonoScene::Reconfig();
    Shape a = *adjusters->GetShape();
    if (a.vstretch != 0 || a.vshrink != a.height / 3) {
        if (size != 0) {
            a.width = size;
            a.hshrink = a.hstretch = 0;
        }
        a.vstretch = 0;
        a.vshrink = a.height/3;
        adjusters->Reshape(a);
    }
    if (slider_on()) { 
      Shape* s = islider->GetShape();
      if (s->width != a.width)
        islider->Reshape(a);
    }
}

static const int MIN_SLIDER_HT = 20;
enum MoveType { MOVE_HORIZ, MOVE_VERT, MOVE_UNDEF };

OverlaySlider::OverlaySlider (Interactor* i) {
    Init(i);
}

OverlaySlider::OverlaySlider (const char* name, Interactor* i) {
    SetInstance(name);
    Init(i);
}

void OverlaySlider::Init (Interactor* i) {
    SetClassName("OverlaySlider");
    interactor = i;
    view = i->GetPerspective();
    view->Attach(this);
    shown = new Perspective;
    constrained = false;
    moveType = MOVE_UNDEF;
    *shown = *view;
    shape->vstretch = shape->vshrink = 0;
    prevl = prevb = prevr = prevt = 0;
    input = new Sensor(updownEvents);
}

void OverlaySlider::Reconfig () {
    Painter* p = new Painter(output);
    p->Reference();
    Unref(output);
    output = p;

    const char* attrib = GetAttribute("syncScroll");
    syncScroll = attrib != nil &&
        (strcmp(attrib, "true") == 0 || strcmp(attrib, "on") == 0);
}

void OverlaySlider::Reshape (Shape& ns) {
    if (shown->width == 0) {
	*shape = ns;
    } else {
	shape->width = (canvas == nil) ? ns.width : xmax + 1;
	float aspect = float(shown->height) / float(shown->width);
	int h = Math::round(aspect * float(shape->width));
	if (h != shape->height) {
	    shape->height = h;
	    Scene* p = Parent();
	    if (p != nil) {
		p->Change(this);
	    }
	}
    }
}

void OverlaySlider::Draw () {
    if (canvas != nil) {
	output->SetPattern(new Pattern(Pattern::lightgray));
	output->FillRect(canvas, 0, 0, xmax, ymax);
	output->SetPattern(new Pattern(Pattern::clear));
	output->FillRect(canvas, left, bottom, right, top);
	output->SetPattern(new Pattern(Pattern::solid));
	output->Rect(canvas, left, bottom, right, top);
#ifdef Line
#undef Line
	output->Line(canvas, left+1, bottom-1, right+1, bottom-1);
	output->Line(canvas, right+1, bottom-1, right+1, top-1);
#define Line _lib_iv(Line)
#else
	output->Line(canvas, left+1, bottom-1, right+1, bottom-1);
	output->Line(canvas, right+1, bottom-1, right+1, top-1);
#endif

	prevl = left; prevb = bottom;
	prevr = right; prevt = top;
    }
}

void OverlaySlider::Redraw (
    IntCoord left, IntCoord bottom, IntCoord right, IntCoord top
) {
    output->Clip(canvas, left, bottom, right, top);
    Draw();
    output->NoClip();
}

inline IntCoord OverlaySlider::ViewX (IntCoord x) {
    return Math::round(float(x) * float(shown->width) / float(xmax));
}

inline IntCoord OverlaySlider::ViewY (IntCoord y) {
    return Math::round(float(y) * float(shown->height) / float(ymax));
}

inline IntCoord OverlaySlider::SliderX (IntCoord x) {
    return Math::round(float(x) * float(xmax) / float(shown->width));
}

inline IntCoord OverlaySlider::SliderY (IntCoord y) {
    return Math::round(float(y) * float(ymax) / float(shown->height));
}

void OverlaySlider::Move (IntCoord dx, IntCoord dy) {
    shown->curx += dx;
    shown->cury += dy;
}

boolean OverlaySlider::Inside (Event& e) {
    return e.x > left && e.x < right && e.y > bottom && e.y < top;
}

void OverlaySlider::CalcLimits (Event& e) {
    llim = e.x - Math::max(0, left);
    blim = e.y - Math::max(0, bottom);
    rlim = e.x + Math::max(0, xmax - right);
    tlim = e.y + Math::max(0, ymax - top);
    constrained = e.shift;
    moveType = MOVE_UNDEF;
    origx = e.x;
    origy = e.y;
}

static int CONSTRAIN_THRESH = 2;    
    // difference between x and y movement needed to decide which direction
    // is constrained

void OverlaySlider::Constrain (Event& e) {
    IntCoord dx, dy;

    if (constrained && moveType == MOVE_UNDEF) {
	dx = Math::abs(e.x - origx);
	dy = Math::abs(e.y - origy);
	if (Math::abs(dx - dy) < CONSTRAIN_THRESH) {
	    e.x = origx;
	    e.y = origy;
	} else if (dx > dy) {
	    moveType = MOVE_HORIZ;
	} else {
	    moveType = MOVE_VERT;
	}
    }

    if (!constrained) {
	e.x = Math::min(Math::max(e.x, llim), rlim);
	e.y = Math::min(Math::max(e.y, blim), tlim);

    } else if (moveType == MOVE_HORIZ) {
	e.x = Math::min(Math::max(e.x, llim), rlim);
	e.y = origy;

    } else if (moveType == MOVE_VERT) {
	e.x = origx;
	e.y = Math::min(Math::max(e.y, blim), tlim);

    }
}

void OverlaySlider::Slide (Event& e) {
    IntCoord newleft, newbot, dummy;
    boolean control = e.control;

    Listen(allEvents);
    SlidingRect r(output, canvas, left, bottom, right, top, e.x, e.y);
    CalcLimits(e);
    do {
	switch (e.eventType) {
	    case MotionEvent:
		e.target->GetRelative(e.x, e.y, this);
		Constrain(e);
		r.Track(e.x, e.y);

                if ((syncScroll && !control) || (!syncScroll && control)) {
                    r.Erase();
                    r.GetCurrent(newleft, newbot, dummy, dummy);
                    Move(ViewX(newleft - left), ViewY(newbot - bottom));
                    interactor->Adjust(*shown);
                }

		break;
	    default:
		break;
	}
	Read(e);
    } while (e.eventType != UpEvent);

    r.GetCurrent(newleft, newbot, dummy, dummy);
    Move(ViewX(newleft - left), ViewY(newbot - bottom));
    Listen(input);
}

void OverlaySlider::Jump (Event& e) {
    register Perspective* s = shown;
    IntCoord dx, dy;
    
    if (e.button == RIGHTMOUSE) {
	dx = ViewX(e.x) - s->curx - s->curwidth/2;
	dy = ViewY(e.y) - s->cury - s->curheight/2;
    } else {
	if (e.button == LEFTMOUSE) {
	    dx = s->sx;
	    dy = s->sy;
	} else {
	    dx = s->lx;
	    dy = s->ly;
	}

	if (e.x < left) {
	    dx = -dx;
	} else if (e.x < right) {
	    dx = 0;
	}
	if (e.y < bottom) {
	    dy = -dy;
	} else if (e.y < top) {
	    dy = 0;
	}
    }
    dx = Math::min(
	Math::max(s->x0 - s->curx, dx),
	s->x0 + s->width - s->curx - s->curwidth
    );
    dy = Math::min(
	Math::max(s->y0 - s->cury, dy),
	s->y0 + s->height - s->cury - s->curheight
    );
    Move(dx, dy);
}	

void OverlaySlider::Handle (Event& e) {
    Perspective oldp = *shown;
    if (e.eventType == DownEvent) {
	if (Inside(e)) {
	    Slide(e);
	} else {
	    Jump(e);
	}
	interactor->Adjust(*shown);
	if (((OverlayViewer*)interactor)->Chained()) {
	    IntCoord dx = shown->curx - oldp.curx;
	    IntCoord dy = shown->cury - oldp.cury;
	    Iterator i;
	    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
		OverlayViewer* v = (OverlayViewer*)unidraw->GetEditor(i)->GetViewer();
		Perspective p  = *v->GetPerspective();
		if (interactor != v && v->Chained()) {
		    p.curx += dx * p.width / oldp.width;
		    p.cury += dy * p.height / oldp.height;
		    v->Adjust(p);
		}
	    }
	}
    }
}
    
static const int MIN_SIZE = 2;

void OverlaySlider::SizeKnob () {
    register Perspective* s = shown;
    
    if (canvas != nil) {
	left = SliderX(s->curx - s->x0);
	bottom = SliderY(s->cury - s->y0);
	right = left + Math::max(SliderX(s->curwidth), MIN_SIZE);
	top = bottom + Math::max(SliderY(s->curheight), MIN_SIZE);
    }
}    

void OverlaySlider::Update () {
    register Perspective* p = shown;
    int h, oldwidth, oldheight;
    float aspect;
    Scene* s;
    Shape ns;

    oldwidth = p->width;
    oldheight = p->height;
    *p = *view;
    aspect = float(p->height) / float(p->width);

    SizeKnob();
    if (p->width != oldwidth || p->height != oldheight) {
	h = Math::round(aspect * float(shape->width));
	if (h == shape->height) {
	    Draw();
	} else {
	    shape->height = h;
	    if ((s = Parent()) != nil) {
		s->Change(this);
	    }
	}
    } else if (
	prevl != left || prevb != bottom || prevr != right || prevt != top
    ) {
	Draw();
    }
}

void OverlaySlider::Resize () {
    int w = xmax + 1;
    if (shape->width != w) {
	Shape ns = *shape;
	ns.width = w;
	Reshape(ns);
    }
    SizeKnob();
}

OverlaySlider::~OverlaySlider () {
    view->Detach(this);
    Unref(shown);
}
