/*
 * Copyright (c) 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * planar figures
 */

#ifndef figure_h
#define figure_h

#include <InterViews/glyph.h>

class Brush;
class Color;

/*
 * GAK!
 * These are renamed to prevent a nasty name collision when trying to
 * use Unidraw Lines with Figure Lines.
 */

class Fig31 : public Glyph {
public:
    virtual void request(Requisition&) const;
    virtual void allocate(Canvas*, const Allocation&, Extension&);
    virtual void draw(Canvas*, const Allocation&) const;
protected:
    Fig31 (
        const Brush* brush, const Color* stroke, const Color* fill,
        boolean closed, boolean curved, int coords
    );
    virtual ~Fig31 ();

    void add_point (Coord x, Coord y);
    void add_curve (Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2);
    void Bspline_move_to (
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    void Bspline_curve_to (
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );

    const Brush* _brush;
    const Color* _stroke;
    const Color* _fill;

    boolean _closed;
    boolean _curved;
    int _count;
    Coord* _x;
    Coord* _y;

    Coord _xmin;
    Coord _xmax;
    Coord _ymin;
    Coord _ymax;
};

class Fig31Line : public Fig31 {
public:
    Fig31Line (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x1, Coord y1, Coord x2, Coord y2
    );
protected:
    virtual ~Fig31Line ();
};

class Fig31Rectangle : public Fig31 {
public:
    Fig31Rectangle (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord l, Coord b, Coord r, Coord t
    );
protected:
    virtual ~Fig31Rectangle ();
};

class Fig31Circle : public Fig31 {
public:
    Fig31Circle (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x, Coord y, Coord r
    );
protected:
    virtual ~Fig31Circle ();
};

class Fig31Ellipse : public Fig31 {
public:
    Fig31Ellipse (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord x, Coord y, Coord rx, Coord ry
    );
protected:
    virtual ~Fig31Ellipse ();
};

class Fig31Open_BSpline : public Fig31 {
public:
    Fig31Open_BSpline (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int count
    );
protected:
    virtual ~Fig31Open_BSpline ();
};

class Fig31Closed_BSpline : public Fig31 {
public:
    Fig31Closed_BSpline (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int count
    );
protected:
    virtual ~Fig31Closed_BSpline ();
};

class Fig31Polyline : public Fig31 {
public:
    Fig31Polyline (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int count
    );
protected:
    virtual ~Fig31Polyline ();
};

class Fig31Polygon : public Fig31 {
public:
    Fig31Polygon (
        const Brush* brush, const Color* stroke, const Color* fill,
        Coord* x, Coord* y, int count
    );
protected:
    virtual ~Fig31Polygon ();
};

#endif

