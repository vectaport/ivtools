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
 * Spline implementations.
 */

#include <Unidraw/classes.h>
#include <Unidraw/globals.h>
#include <Unidraw/Graphic/splines.h>
#include <Unidraw/Graphic/util.h>

#include <IV-2_6/InterViews/painter.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

OpenBSpline::OpenBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : Vertices(gr) { 
    count = count + 4;
    Coord *newx = new Coord[count];
    Coord *newy = new Coord[count];

    ArrayCopy(x, y, count-4, newx+2, newy+2);

    newx[0] = newx[1] = newx[2];
    newy[0] = newy[1] = newy[2];
    newx[count-1] = newx[count-2] = newx[count-3];
    newy[count-1] = newy[count-2] = newy[count-3];
    
    _pts = MultiLineObj::make_pts(newx, newy, count);
    Resource::ref(_pts);
    delete newx;
    delete newy;
}

int OpenBSpline::GetOriginal (const Coord*& x, const Coord*& y) {
    x = Vertices::x() + 2;
    y = Vertices::y() + 2;
    return count() - 4;
}

boolean OpenBSpline::s_contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;
    getBox(b, gs);

    if (b.Contains(pt)) {
	invTransform(pt._x, pt._y, gs);
	MultiLineObj ml;
	ml.SplineToMultiLine(x(), y(), count());
	return ml.Contains(pt);
    }
    return false;
}

boolean OpenBSpline::f_contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;
    getBox(b, gs);

    if (b.Contains(pt)) {
	invTransform(pt._x, pt._y, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(x(), y(), count());
	return fp.Contains(pt);
    }
    return false;
}

boolean OpenBSpline::s_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    getBox(b, gs);

    if (b.Intersects(userb)) {
	convx = new Coord[count()];
	convy = new Coord[count()];
	transformList(x(), y(), count(), convx, convy, gs);
	MultiLineObj ml;
	ml.SplineToMultiLine(convx, convy, count());
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

boolean OpenBSpline::f_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    getBox(b, gs);

    if (b.Intersects(userb)) {
	convx = new Coord[count()];
	convy = new Coord[count()];
	transformList(x(), y(), count(), convx, convy, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(convx, convy, count());
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

/*****************************************************************************/

S_OpenBSpline::S_OpenBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : OpenBSpline(x, y, count, gr) {
    _br = nil;
    if (gr != nil) {
        S_OpenBSpline::SetBrush(gr->GetBrush());
    }
}

S_OpenBSpline::~S_OpenBSpline () { Unref(_br); }

void S_OpenBSpline::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
        invalidateCaches();
    }
}

PSBrush* S_OpenBSpline::GetBrush () { return _br; }

Graphic* S_OpenBSpline::Copy () {
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new S_OpenBSpline(x, y, count, this);
}

void S_OpenBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean S_OpenBSpline::contains (PointObj& po, Graphic* gs) {
    return s_contains(po, gs);
}

boolean S_OpenBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return s_intersects(userb, gs);
}

void S_OpenBSpline::draw (Canvas *c, Graphic* gs) {
    if (!gs->GetBrush()->None()) {
        update(gs);
        _p->BSpline(c, x(), y(), count());
    }
}

/*****************************************************************************/

F_OpenBSpline::F_OpenBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : OpenBSpline(x, y, count, gr) {
    _pat = nil;
    if (gr != nil) {
        F_OpenBSpline::SetPattern(gr->GetPattern());
    }
}

F_OpenBSpline::~F_OpenBSpline () { Unref(_pat); }

void F_OpenBSpline::SetPattern (PSPattern* pat) {
    Ref(pat);
    Unref(_pat);
    _pat = pat;
}

PSPattern* F_OpenBSpline::GetPattern () { return _pat; }

Graphic* F_OpenBSpline::Copy () {
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new F_OpenBSpline(x, y, count, this);
}

void F_OpenBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    f_getExtent(l, b, cx, cy, tol, gs);
}

boolean F_OpenBSpline::contains (PointObj& po, Graphic* gs) {
    return !gs->GetPattern()->None() && f_contains(po, gs);
}

boolean F_OpenBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return !gs->GetPattern()->None() && f_intersects(userb, gs);
}

void F_OpenBSpline::draw (Canvas *c, Graphic* gs) {
    if (!gs->GetPattern()->None()) {
        update(gs);
        _p->FillBSpline(c, x(), y(), count());
    }
}

/*****************************************************************************/

SF_OpenBSpline::SF_OpenBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : OpenBSpline(x, y, count, gr) {
    _br = nil;
    _pat = nil;

    if (gr != nil) {
        SF_OpenBSpline::SetBrush(gr->GetBrush());
        SF_OpenBSpline::SetPattern(gr->GetPattern());
    }
}

SF_OpenBSpline::~SF_OpenBSpline () {
    Unref(_br);
    Unref(_pat);
}

void SF_OpenBSpline::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
        invalidateCaches();
    }
}

PSBrush* SF_OpenBSpline::GetBrush () { return _br; }

void SF_OpenBSpline::SetPattern (PSPattern* pat) {
    Ref(pat);
    Unref(_pat);
    _pat = pat;
}

PSPattern* SF_OpenBSpline::GetPattern () { return _pat; }

Graphic* SF_OpenBSpline::Copy () {
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new SF_OpenBSpline(x, y, count, this);
}

void SF_OpenBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean SF_OpenBSpline::contains (PointObj& po, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_contains(po, gs)) ||
        s_contains(po, gs);
}

boolean SF_OpenBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_intersects(userb, gs)) ||
        s_intersects(userb, gs);
}

void SF_OpenBSpline::draw (Canvas *c, Graphic* gs) {
    update(gs);
    if (!gs->GetPattern()->None()) {
        _p->FillBSpline(c, x(), y(), count());
    }
    if (!gs->GetBrush()->None()) {
        _p->BSpline(c, x(), y(), count());
    }
}

/*****************************************************************************/

SFH_OpenBSpline::SFH_OpenBSpline(
    Coord* x, Coord* y, int count, Graphic* gr
) : SF_OpenBSpline(x, y, count, gr) { }

Graphic* SFH_OpenBSpline::Copy () {
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new SFH_OpenBSpline(x, y, count, this);
}

boolean SFH_OpenBSpline::contains (PointObj& po, Graphic* gs) {
    const Coord *x, *y;
    int count = GetOriginal(x, y);
    Transformer* t = gs->GetTransformer();

    if (t != nil) {
        t->InvTransform(po._x, po._y);
    }

    for (int i = 0; i < count; i++) {
	if (x[i] == po._x && y[i] == po._y) {
            return true;
        }
    }
    return SF_OpenBSpline::contains(po, gs);
}

boolean SFH_OpenBSpline::intersects (BoxObj& userb, Graphic* gs) {
    PointObj po;
    const Coord *x, *y;
    int count = GetOriginal(x, y);
    Transformer* t = gs->GetTransformer();

    for (int i = 0; i < count; i++) {
	po._x = x[i];
	po._y = y[i];

        if (t != nil) {
            t->Transform(po._x, po._y);
        }
	if (userb.Contains(po)) {
            return true;
        }
    }
    return SF_OpenBSpline::intersects(userb, gs);
}

/*****************************************************************************/

ClosedBSpline::ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : Vertices(x, y, count, gr) { }

boolean ClosedBSpline::s_contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;
    getBox(b, gs);

    if (b.Contains(pt)) {
	invTransform(pt._x, pt._y, gs);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(x(), y(), count());
	return ml.Contains(pt);
    }
    return false;
}

boolean ClosedBSpline::f_contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;
    getBox(b, gs);

    if (b.Contains(pt)) {
	invTransform(pt._x, pt._y, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(x(), y(), count());
	return fp.Contains(pt);
    }
    return false;
}

boolean ClosedBSpline::s_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    getBox(b, gs);

    if (b.Intersects(userb)) {
	convx = new Coord[count()];
	convy = new Coord[count()];
	transformList(x(), y(), count(), convx, convy, gs);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(convx, convy, count());
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

boolean ClosedBSpline::f_intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    getBox(b, gs);

    if (b.Intersects(userb)) {
	convx = new Coord[count()];
	convy = new Coord[count()];
	transformList(x(), y(), count(), convx, convy, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(convx, convy, count());
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

/*****************************************************************************/

S_ClosedBSpline::S_ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : ClosedBSpline(x, y, count, gr) {
    _br = nil;
    if (gr != nil) {
        S_ClosedBSpline::SetBrush(gr->GetBrush());
    }
}

S_ClosedBSpline::~S_ClosedBSpline () { Unref(_br); }

void S_ClosedBSpline::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
        invalidateCaches();
    }
}

PSBrush* S_ClosedBSpline::GetBrush () { return _br; }

Graphic* S_ClosedBSpline::Copy () {
    return new S_ClosedBSpline(x(), y(), count(), this);
}

ClassId SFH_OpenBSpline::CompId() { return SPLINE_COMP; }

void S_ClosedBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean S_ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    return s_contains(po, gs);
}

boolean S_ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return s_intersects(userb, gs);
}

void S_ClosedBSpline::draw (Canvas *c, Graphic* gs) {
    if (!gs->GetBrush()->None()) {
        update(gs);
        _p->BSpline(c, x(), y(), count());
    }
}

/*****************************************************************************/

F_ClosedBSpline::F_ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : ClosedBSpline(x, y, count, gr) {
    _pat = nil;
    if (gr != nil) {
        F_ClosedBSpline::SetPattern(gr->GetPattern());
    }
}

F_ClosedBSpline::~F_ClosedBSpline () { Unref(_pat); }

void F_ClosedBSpline::SetPattern (PSPattern* pat) {
    Ref(pat);
    Unref(_pat);
    _pat = pat;
}

PSPattern* F_ClosedBSpline::GetPattern () { return _pat; }

Graphic* F_ClosedBSpline::Copy () {
    return new F_ClosedBSpline(x(), y(), count(), this);
}

void F_ClosedBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    f_getExtent(l, b, cx, cy, tol, gs);
}

boolean F_ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    return !gs->GetPattern()->None() && f_contains(po, gs);
}

boolean F_ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return !gs->GetPattern()->None() && f_intersects(userb, gs);
}

void F_ClosedBSpline::draw (Canvas *c, Graphic* gs) {
    if (!gs->GetPattern()->None()) {
        update(gs);
        _p->FillBSpline(c, x(), y(), count());
    }
}

/*****************************************************************************/

SF_ClosedBSpline::SF_ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : ClosedBSpline(x, y, count, gr) {
    _br = nil;
    _pat = nil;

    if (gr != nil) {
        SF_ClosedBSpline::SetBrush(gr->GetBrush());
        SF_ClosedBSpline::SetPattern(gr->GetPattern());
    }
}

SF_ClosedBSpline::~SF_ClosedBSpline () {
    Unref(_br);
    Unref(_pat);
}

void SF_ClosedBSpline::SetBrush (PSBrush* br) {
    if (_br != br) {
        Ref(br);
        Unref(_br);
        _br = br;
        invalidateCaches();
    }
}

PSBrush* SF_ClosedBSpline::GetBrush () { return _br; }

void SF_ClosedBSpline::SetPattern (PSPattern* pat) {
    Ref(pat);
    Unref(_pat);
    _pat = pat;
}

PSPattern* SF_ClosedBSpline::GetPattern () { return _pat; }

Graphic* SF_ClosedBSpline::Copy () {
    return new SF_ClosedBSpline(x(), y(), count(), this);
}

void SF_ClosedBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    s_getExtent(l, b, cx, cy, tol, gs);
}

boolean SF_ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_contains(po, gs)) ||
        s_contains(po, gs);
}

boolean SF_ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    return
        (!gs->GetPattern()->None() && f_intersects(userb, gs)) ||
        s_intersects(userb, gs);
}

void SF_ClosedBSpline::draw (Canvas *c, Graphic* gs) {
    update(gs);
    if (!gs->GetPattern()->None()) {
        _p->FillBSpline(c, x(), y(), count());
    }
    if (!gs->GetBrush()->None()) {
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef ClosedBSpline
        _p->ClosedBSpline(c, x(), y(), count());
#define ClosedBSpline _lib_iv(ClosedBSpline)
#else
        _p->ClosedBSpline(c, x(), y(), count());
#endif /* ClosedBSpline */
    }
}

/*****************************************************************************/

SFH_ClosedBSpline::SFH_ClosedBSpline(
    Coord* x, Coord* y, int count, Graphic* gr
) : SF_ClosedBSpline(x, y, count, gr) { }

Graphic* SFH_ClosedBSpline::Copy () {
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new SFH_ClosedBSpline(x, y, count, this);
}

ClassId SFH_ClosedBSpline::CompId() { return CLOSEDSPLINE_COMP; }

boolean SFH_ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    const Coord *x, *y;
    int count = GetOriginal(x, y);
    Transformer* t = gs->GetTransformer();

    if (t != nil) {
        t->InvTransform(po._x, po._y);
    }

    for (int i = 0; i < count; i++) {
	if (x[i] == po._x && y[i] == po._y) {
            return true;
        }
    }
    return SF_ClosedBSpline::contains(po, gs);
}

boolean SFH_ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    PointObj po;
    const Coord *x, *y;
    int count = GetOriginal(x, y);
    Transformer* t = gs->GetTransformer();

    for (int i = 0; i < count; i++) {
	po._x = x[i];
	po._y = y[i];

        if (t != nil) {
            t->Transform(po._x, po._y);
        }
	if (userb.Contains(po)) {
            return true;
        }
    }
    return SF_ClosedBSpline::intersects(userb, gs);
}
