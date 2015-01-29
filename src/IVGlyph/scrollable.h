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

#ifndef scrollable_h
#define scrollable_h

#include <InterViews/adjust.h>
#include <InterViews/patch.h>
#include <stdio.h>

class TransformSetter;
class Page;

class Scrollable : public Patch, public Adjustable {
public:
    Scrollable(Glyph* g, Page* pg, Coord w, Coord h);
    virtual ~Scrollable();

    virtual void draw(Canvas*, const Allocation&) const;
    virtual void pick(Canvas*, const Allocation&, int depth, Hit&);
    virtual void reallocate();

    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);

    virtual Coord lower(DimensionName) const;
    virtual Coord upper(DimensionName) const;
    virtual Coord length(DimensionName) const;
    virtual Coord cur_lower(DimensionName) const;
    virtual Coord cur_upper(DimensionName) const;
    virtual Coord cur_length(DimensionName) const;

    virtual void scroll_to(DimensionName, Coord lower);
    virtual void scale_to(DimensionName, float fraction_visible);
    virtual void zoom_to(float magnification);

    virtual float scale();

    virtual void scroll_scr(Coord cnew) { scroll_incr_ = cnew; }
    virtual void page_scr(Coord cnew) { page_incr_ = cnew; }

    virtual void scroll_forward(DimensionName d) {
        Coord newpos;

        if (d == Dimension_X) {
            newpos = dx_ + scroll_incr_;
	} else {
            newpos = dy_ + scroll_incr_;
        };

        scroll_to(d, newpos);
    }
         
    virtual void scroll_backward(DimensionName d) {
        Coord newpos;

        if (d == Dimension_X) {
            newpos = dx_ - scroll_incr_;
	} else {
            newpos = dy_ - scroll_incr_;
        };

        scroll_to(d, newpos);
    }

    virtual void page_forward(DimensionName d) {
        Coord newpos;

        if (d == Dimension_X) {
            newpos = dx_ + page_incr_;
	} else {
            newpos = dy_ + page_incr_;
        };

        scroll_to(d, newpos);
    }

    virtual void page_backward(DimensionName d) {
        Coord newpos;

        if (d == Dimension_X) {
            newpos = dx_ - page_incr_;
	} else {
            newpos = dy_ - page_incr_;
        };

        scroll_to(d, newpos);
    }

    Coord width_, height_;
    Coord scale_;
    Coord dx_, dy_;
    Coord scroll_incr_;
    Coord page_incr_;

    Requisition requisition_;
    TransformSetter* xform_;
    Page* page_;
};

#endif
