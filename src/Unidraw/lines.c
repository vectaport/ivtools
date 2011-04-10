/*
 * Copyright (c) 1990, 1991 Stanford University
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
 * Implementation of Points, Lines, and MultiLines, objects derived from
 * Graphic.
 */

#include <Unidraw/classes.h>
#include <Unidraw/globals.h>
#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/util.h>

#include <IV-2_6/InterViews/painter.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

Point::Point (Coord x, Coord y, Graphic* gr) : Graphic(gr) { 
    _br = nil;
    if (gr != nil) {
	Point::SetBrush(gr->GetBrush());
    }
    _x = x; 
    _y = y;
}

Point::~Point () { Unref(_br); }

void Point::GetOriginal (Coord& x, Coord& y) {
    x = _x;
    y = _y;
}

void Point::SetOriginal (Coord x, Coord y) {
    _x = x;
    _y = y;
    uncacheExtent();
}

void Point::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
	_br = br;
	invalidateCaches();
    }
}

PSBrush* Point::GetBrush () { return _br; }
Graphic* Point::Copy () { return new Point(_x, _y, this); }

void Point::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float width;

    width = float(gs->GetBrush()->Width());
    tol = (width > 1) ? width / 2 : 0;
    transform(float(_x), float(_y), cx, cy, gs);
    l = cx;
    b = cy;
}

boolean Point::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt._x, pt._y, gs);
    return (pt._x == _x) && (pt._y == _y);
}

boolean Point::intersects (BoxObj& b, Graphic* gs) {
    PointObj pt (_x, _y);
        
    transform(pt._x, pt._y, gs);
    return b.Contains(pt);
}

void Point::draw (Canvas* c, Graphic* gs) {
    if (!gs->GetBrush()->None()) {
	update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef Point
	_p->Point(c, _x, _y);
#define Point _lib_iv(Point)
#else
	_p->Point(c, _x, _y);
#endif /* Point */
    }
}

/*****************************************************************************/

Line::Line (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : Graphic(gr) {
    _br = nil;
    if (gr != nil) {
	Line::SetBrush(gr->GetBrush());
    }
    _x0 = x0;
    _y0 = y0;
    _x1 = x1;
    _y1 = y1;
}

Line::~Line () { Unref(_br); }

void Line::GetOriginal (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = _x0;
    y0 = _y0;
    x1 = _x1;
    y1 = _y1;
}    

void Line::SetOriginal (Coord x0, Coord y0, Coord x1, Coord y1) {
    _x0 = x0;
    _y0 = y0;
    _x1 = x1;
    _y1 = y1;
    uncacheExtent();
}    

void Line::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
	_br = br;
	invalidateCaches();
    }
}

PSBrush* Line::GetBrush () { return _br; }
Graphic* Line::Copy () { return new Line(_x0, _y0, _x1, _y1, this); }
ClassId Line::CompId () { return LINE_COMP; }

void Line::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float r, t, width;

    width = float(gs->GetBrush()->Width());
    tol = (width > 1) ? width / 2 : 0;
    
    transform(float(_x0+_x1)/2, float(_y0+_y1)/2, cx, cy, gs);
    transform(float(_x0), float(_y0), l, b, gs);
    transform(float(_x1), float(_y1), r, t, gs);
    l = min(l, r);
    b = min(b, t);
}

boolean Line::contains (PointObj& po, Graphic* gs) {
    LineObj l(_x0, _y0, _x1, _y1);
    PointObj pt (&po);
    invTransform(pt._x, pt._y, gs);
    return l.Contains(pt);
}

boolean Line::intersects (BoxObj& b, Graphic* gs) {
    LineObj l (_x0, _y0, _x1, _y1);
    transform(l._p1._x, l._p1._y, gs);
    transform(l._p2._x, l._p2._y, gs);
    return b.Intersects(l);
}

void Line::draw (Canvas* c, Graphic* gs) {
    if (!gs->GetBrush()->None()) {
	update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef Line
	_p->Line(c, _x0, _y0, _x1, _y1);
#define Line _lib_iv(Line)
#else
	_p->Line(c, _x0, _y0, _x1, _y1);
#endif /* Line */
    }
}

/*****************************************************************************/

MultiLine::MultiLine (
    Coord* x, Coord* y, int count, Graphic* gr
) : Vertices(x, y, count, gr) { }

boolean MultiLine::s_contains (PointObj& po, Graphic* gs) {
    MultiLineObj &ml = *_pts;
    PointObj pt (&po);
    BoxObj b;
    getBox(b, gs);

    if (b.Contains(po)) {
	invTransform(pt._x, pt._y, gs);
	return ml.Contains(pt);
    }
    return false;
}

boolean MultiLine::f_contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    PointObj pt (&po);
    getBox(b, gs);

    if (b.Contains(pt)) {
	FillPolygonObj fp (x(), y(), count());
	invTransform(pt._x, pt._y, gs);
	return fp.Contains(pt);
    }
    return false;
}

boolean MultiLine::s_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count()];
	convy = new Coord[count()];
	transformList(x(), y(), count(), convx, convy, gs);
	MultiLineObj ml (convx, convy, count());
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

boolean MultiLine::f_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    getBox(b, gs);

    if (b.Intersects(userb)) {
	convx = new Coord[count()+1];
	convy = new Coord[count()+1];
	transformList(x(), y(), count(), convx, convy, gs);
	FillPolygonObj fp (convx, convy, count());
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;    
}

/*****************************************************************************/

S_MultiLine::S_MultiLine (
    Coord* x, Coord* y, int count, Graphic* gr
) : MultiLine(x, y, count, gr) {
    _br = nil;
    if (gr != nil) {
	S_MultiLine::SetBrush(gr->GetBrush());
    }
}

S_MultiLine::~S_MultiLine () { Unref(_br); }

void S_MultiLine::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
	invalidateCaches();
    }
}

PSBrush* S_MultiLine::GetBrush () { return _br; }
Graphic* S_MultiLine::Copy () { return new S_MultiLine(x(), y(), count(), this); }

void S_MultiLine::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean S_MultiLine::contains (PointObj& po, Graphic* gs) {
    return s_contains(po, gs);
}

boolean S_MultiLine::intersects (BoxObj& userb, Graphic* gs) {
    return s_intersects(userb, gs);
}

void S_MultiLine::draw (Canvas *c, Graphic* gs) {
    if (!gs->GetBrush()->None()) {
	update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef MultiLine
	_p->MultiLine(c, x(), y(), count());
#define MultiLine _lib_iv(MultiLine)
#else
	_p->MultiLine(c, x(), y(), count());
#endif /* MultiLine */
    }
}

/*****************************************************************************/

SF_MultiLine::SF_MultiLine (
    Coord* x, Coord* y, int count, Graphic* gr
) : MultiLine(x, y, count, gr) {
    _br = nil;
    _pat = nil;

    if (gr != nil) {
        SF_MultiLine::SetBrush(gr->GetBrush());
	SF_MultiLine::SetPattern(gr->GetPattern());
    }
}

SF_MultiLine::~SF_MultiLine () {
    Unref(_br);
    Unref(_pat);
}

void SF_MultiLine::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
        invalidateCaches();
    }
}

PSBrush* SF_MultiLine::GetBrush () { return _br; }

void SF_MultiLine::SetPattern (PSPattern* pat) {
    Ref(pat);
    Unref(_pat);
    _pat = pat;
}

PSPattern* SF_MultiLine::GetPattern () { return _pat; }
Graphic* SF_MultiLine::Copy () { return new SF_MultiLine(x(), y(),count(),this); }
ClassId SF_MultiLine::CompId () { return MULTILINE_COMP; }

void SF_MultiLine::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean SF_MultiLine::contains (PointObj& po, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_contains(po, gs)) ||
        s_contains(po, gs);
}

boolean SF_MultiLine::intersects (BoxObj& userb, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_intersects(userb, gs)) ||
        s_intersects(userb, gs);
}

void SF_MultiLine::draw (Canvas *c, Graphic* gs) {
    update(gs);
    if (!gs->GetPattern()->None()) {
        _p->FillPolygon(c, x(), y(), count());
    }
    if (!gs->GetBrush()->None()) {
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef MultiLine
        _p->MultiLine(c, x(), y(), count());
#define MultiLine _lib_iv(MultiLine)
#else
        _p->MultiLine(c, x(), y(), count());
#endif /* MultiLine */
    }
}
