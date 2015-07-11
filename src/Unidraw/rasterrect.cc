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
 * Implementation of RasterRect, a Graphic that draws a Raster.
 */

#include <Unidraw/classes.h>
#include <Unidraw/globals.h>
#include <Unidraw/Graphic/rasterrect.h>

#include <IV-2_6/InterViews/painter.h>
#include <InterViews/raster.h>
#include <InterViews/transformer.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

RasterRect::RasterRect (Raster* r, Graphic* gr) : Graphic(gr) {
    _raster = r;
    Ref(_raster);
}
RasterRect::~RasterRect () { Unref(_raster); }

Graphic* RasterRect::Copy () {
    return new RasterRect(new Raster(*_raster), this);
}

ClassId RasterRect::CompId() { return RASTER_COMP; }

void RasterRect::draw (Canvas *c, Graphic* gs) {
    update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5 || __GNUC__>=3
#undef RasterRect
    _p->RasterRect(c, 0, 0, _raster);
#define RasterRect _lib_iv(RasterRect)
#else
    _p->RasterRect(c, 0, 0, _raster);
#endif /* RasterRect */
}

void RasterRect::getExtent (
    float& x0, float& y0, float& cx, float& cy, float& tol, Graphic* gs
) {
    if (gs->GetTransformer() == nil) {
	x0 = y0 = 0;
	cx = _raster->Width() / 2;
	cy = _raster->Height() / 2;
    } else {
	transformRect(0,0,_raster->Width(),_raster->Height(),x0,y0,cx,cy,gs);
	cx = (cx + x0)/2;
	cy = (cy + y0)/2;
    }
    tol = 0;
}

boolean RasterRect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt._x, pt._y, gs);
    BoxObj b (0, 0, _raster->Width(), _raster->Height());
    return b.Contains(pt);
}

boolean RasterRect::intersects (BoxObj& userb, Graphic* gs) {
    Transformer* t = gs->GetTransformer();
    Coord xmax = _raster->Width();
    Coord ymax = _raster->Height();
    Coord tx0, ty0, tx1, ty1;
    
    if (t != nil && t->Rotated()) {
	Coord x[4], tx[5];
	Coord y[4], ty[5];
    
	x[0] = x[3] = y[0] = y[1] = 0;
	x[2] = x[1] = xmax;
	y[2] = y[3] = ymax;
	transformList(x, y, 4, tx, ty, gs);
	tx[4] = tx[0];
	ty[4] = ty[0];
	FillPolygonObj fp (tx, ty, 5);
	return fp.Intersects(userb);
    
    } else if (t != nil) {
	t->Transform(0, 0, tx0, ty0);
	t->Transform(xmax, ymax, tx1, ty1);
	BoxObj b1 (tx0, ty0, tx1, ty1);
	return b1.Intersects(userb);

    } else {
	BoxObj b2 (0, 0, xmax, ymax);
	return b2.Intersects(userb);
    }
}
