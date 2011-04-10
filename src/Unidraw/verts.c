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
 */

/*
 * Vertices implementation.
 */

#include <Unidraw/Graphic/util.h>
#include <Unidraw/Graphic/verts.h>

#include <IV-2_6/_enter.h>

/*****************************************************************************/

Vertices::Vertices (Graphic* gr) : Graphic(gr) { _extent = nil; }

Vertices::Vertices (Coord* x, Coord* y, int count, Graphic* gr) : Graphic(gr){
    _extent = nil;
    if (x && y ) {
	_pts = MultiLineObj::make_pts(x, y, count);
	Resource::ref(_pts);
    } else 
	_pts = nil;
}

Vertices::~Vertices () {
    uncacheExtent();
    Unref(_pts);
}

int Vertices::GetOriginal (const Coord*& x, const Coord*& y) {
    x = _pts ? _pts->x() : nil;
    y = _pts ? _pts->y() : nil;
    return count();
}

MultiLineObj* Vertices::GetOriginal () {
    return _pts;
}

int Vertices::SetOriginal (const Coord* x, const Coord* y) {
    MultiLineObj* mlo = MultiLineObj::make_pts(x, y, count());
    Unref(_pts);
    _pts = mlo;
    Resource::ref(_pts);
    uncacheExtent();
    return count();
}

void Vertices::SetOriginal (MultiLineObj* mlo) {
    Unref(_pts);
    _pts = mlo;
    Resource::ref(_pts);
    uncacheExtent();
}

boolean Vertices::operator == (Vertices& ml) {
    if (count() == ml.count()) {
        for (int i = 0; i < count(); ++i) {
            if (x()[i] != ml.x()[i] || y()[i] != ml.y()[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

boolean Vertices::operator != (Vertices& ml) { return !(*this == ml); }
Graphic* Vertices::Copy () { return new Vertices(x(), y(), count(), this); }
boolean Vertices::extentCached () { return _caching && _extent != nil; }

void Vertices::cacheExtent (float l, float b, float cx, float cy, float tol) {
    if (_caching) {
	uncacheExtent();
	_extent = new Extent(l, b, cx, cy, tol);
    }
}

void Vertices::uncacheExtent() { 
    delete _extent; 
    _extent = nil;
}

void Vertices::getCachedExtent (
    float& l, float& b, float& cx, float& cy, float& tol
) {
    l = _extent->_left;
    b = _extent->_bottom;
    cx = _extent->_cx;
    cy = _extent->_cy;
    tol = _extent->_tol;
}

void Vertices::s_getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float bx0, by0, bx1, by1, tcx, tcy, width, dummy1, dummy2;

    if (extentCached()) {
	getCachedExtent(bx0, by0, tcx, tcy, tol);
	bx1 = 2*tcx - bx0;
	by1 = 2*tcy - by0;

    } else {
	width = float(gs->GetBrush()->Width());
	tol = (width > 1) ? width/2 : 0;
	bx0 = bx1 = x() ? x()[0] : 0.0; 
	by0 = by1 = y() ? y()[0] : 0.0;

	for (int i = 1; i < count(); ++i) {
	    bx0 = min(bx0, float(x()[i]));
	    by0 = min(by0, float(y()[i]));
	    bx1 = max(bx1, float(x()[i]));
	    by1 = max(by1, float(y()[i]));
	}
	tcx = (bx0 + bx1) / 2;
	tcy = (by0 + by1) / 2;
	cacheExtent(bx0, by0, tcx, tcy, tol);
    }
    transformRect(bx0, by0, bx1, by1, l, b, dummy1, dummy2, gs);
    transform(tcx, tcy, cx, cy, gs);
}

void Vertices::f_getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float bx0, by0, bx1, by1, tcx, tcy, dummy1, dummy2;
	
    if (extentCached()) {
	getCachedExtent(bx0, by0, tcx, tcy, tol);
	bx1 = 2*tcx - bx0;
	by1 = 2*tcy - by0;

    } else {
	bx0 = bx1 = x()[0]; by0 = by1 = y()[0];

	for (int i = 1; i < count(); ++i) {
	    bx0 = min(bx0, float(x()[i]));
	    by0 = min(by0, float(y()[i]));
	    bx1 = max(bx1, float(x()[i]));
	    by1 = max(by1, float(y()[i]));
	}
	tcx = (bx0 + bx1) / 2;
	tcy = (by0 + by1) / 2;
	tol = 0;
	cacheExtent(bx0, by0, tcx, tcy, tol);
    }
    transformRect(bx0, by0, bx1, by1, l, b, dummy1, dummy2, gs);
    transform(tcx, tcy, cx, cy, gs);
}

int Vertices::count() { 
    return _pts ? _pts->count() : 0;
}

Coord* Vertices::x() { 
    return _pts ? _pts->x() : nil; 
}

Coord* Vertices::y() { 
    return _pts ? _pts->y() : nil; 
}
