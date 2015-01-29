/*
 * Copyright (c) 1995-1996 Vectaport Inc.
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

#include "scrollable.h"

#include <IV-look/kit.h>
#include <InterViews/canvas.h>
#include <InterViews/layout.h>
#include <InterViews/hit.h>
#include <InterViews/page.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <InterViews/session.h>
#include <OS/math.h>

Scrollable::Scrollable(Glyph* g, Page *pg, Coord w, Coord h)
: Patch(g) {
    page_ = pg;
    width_ = w;
    height_ = h;
    scale_ = 1.0;
    dx_ = dy_ = 0;
    scroll_incr_ = 10;
    page_incr_ = 50;
}

Scrollable::~Scrollable () {
}

void Scrollable::request(Requisition& req) const {
    Scrollable* s = (Scrollable*) this;
    Patch::request(s->requisition_);
    Requirement& box_x = req.x_requirement();
    box_x.natural(width_);
    box_x.stretch(fil);
    box_x.shrink(width_);
    box_x.alignment(0.0);

    Requirement& box_y = req.y_requirement();
    box_y.natural(height_);
    box_y.stretch(fil);
    box_y.shrink(height_);
    box_y.alignment(0.0);
}

void Scrollable::allocate(Canvas* c, const Allocation& a, Extension& ext) {
    Patch::allocate(c, a, ext);
    notify(Dimension_X);
    notify(Dimension_Y);
}

void Scrollable::draw(Canvas* c, const Allocation& a) const {
    c->damage(0, 0, width_, height_);
    Extension e;
    e.set(c, a);
    c->push_clipping();
    c->push_transform();
    c->transformer(Transformer());
    c->clip_rect(e.left(), e.bottom(), e.right(), e.top());
    Patch::draw(c, a);
    c->pop_transform();
    c->pop_clipping();
}

void Scrollable::reallocate() {
    Patch::reallocate();
}

void Scrollable::pick(Canvas* c, const Allocation& a, int depth, Hit& h) {
    Patch::pick(c, a, depth, h);
}

Coord Scrollable::cur_lower(DimensionName d) const {
    if (d == Dimension_X) return dx_;
    else return dy_;
}
Coord Scrollable::cur_upper(DimensionName d) const {
    if (d == Dimension_X)
	return dx_ + scale_ * allocation().x_allotment().span();
    else
	return dy_ + scale_ * allocation().y_allotment().span();
}
Coord Scrollable::cur_length(DimensionName d) const {
    return scale_ * allocation().allotment(d).span();
}
Coord Scrollable::lower(DimensionName) const {
    return 0;
}
Coord Scrollable::upper(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return height_;
}
Coord Scrollable::length(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return height_;
}

void Scrollable::scroll_to(DimensionName d, Coord lower) {
    Coord p = lower;
    Coord prevdx = dx_;
    Coord prevdy = dy_;
    constrain(d, p);
    if (p != cur_lower(d)) {
	if (d == Dimension_X) dx_ = p;
	else dy_ = p;
	redraw();
	for (int i=0; i<page_->count(); i++) {
	    Coord x, y;
	    page_->location(i, x, y);
	    page_->move(i, x+prevdx-dx_, y+prevdy-dy_);
	}
	notify(d);
	redraw();
    };
}
    
void Scrollable::scale_to(DimensionName, float fraction_visible) {
    zoom_to(fraction_visible);
}

void Scrollable::zoom_to(float magnification) {
    if (magnification <= (1./16)) {
	magnification = 1./16;
    } else if (magnification > 16) {
	magnification = 16;
    }
    if (scale_ != magnification) {
	scale_ = magnification;
	Transformer t(
	    scale_, 0,
	    0, scale_,
	    - scale_ * dx_, - scale_ * dy_
	);
	redraw();
	xform_->transformer(t);
	reallocate();
	redraw();
	notify(Dimension_X);
	notify(Dimension_Y);
    }
}

float Scrollable::scale() { return scale_; }
