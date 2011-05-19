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
 * OverlayDamage class implementation.
 */

#include <OverlayUnidraw/ovdamage.h>

#include <Unidraw/iterator.h>
#include <Unidraw/Graphic/geomobjs.h>
#include <Unidraw/Graphic/graphic.h>

#include <InterViews/canvas.h>
#include <IV-X11/xcanvas.h>
#include <IV-2_6/InterViews/painter.h>

/*****************************************************************************/

OverlayDamage::OverlayDamage (Canvas* c, Painter* p, Graphic* g) : Damage(c,p,g) {
}

void OverlayDamage::DrawAreas () {
    BoxObj visible(0, 0, _canvas->Width() - 1, _canvas->Height() - 1);
    BoxObj b, *a;
    Iterator i;

    CanvasRep* c = _canvas->rep();
    c->xdrawable_ = c->drawbuffer_;
    for (FirstArea(i); !Done(i); Next(i)) {
        a = GetArea(i); 
        b = *a - visible;
	_output->ClearRect(_canvas, b._left, b._bottom, b._right, b._top);
	_graphic->DrawClipped(_canvas, b._left, b._bottom, b._right, b._top);

    }
    for (FirstArea(i); !Done(i); Next(i)) {
        a = GetArea(i); 
        b = *a - visible;

	Coord x = b._left;
	Coord y = _canvas->Height()-b._top-1;
	Coord w = b._right-b._left+1;
	Coord h = b._top-b._bottom+1;
	XCopyArea(c->dpy(), c->drawbuffer_, c->copybuffer_, c->copygc_,
		  x, y, w, h, x, y);
    }
    c->xdrawable_ = c->copybuffer_;
}    


