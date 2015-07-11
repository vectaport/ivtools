/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * Rubberbanding primitives.
 */

#include <IV-2_6/InterViews/rubband.h>
#include <IV-2_6/InterViews/painter.h>
#include <OS/math.h>
#include <math.h>

float Rubberband::Angle (IntCoord x0, IntCoord y0, IntCoord x1, IntCoord y1) {
    return degrees(atan2f(y1 - y0, x1 - x0));
}

float Rubberband::Distance(
    IntCoord x0, IntCoord y0, IntCoord x1, IntCoord y1
) {
    return hypotf(x0-x1, y0-y1);
}

Rubberband::Rubberband (Painter* p, Canvas* c, IntCoord x, IntCoord y) {
    if (p == nil) {
	output = nil;
    } else {
	output = new Painter(p);
	output->Reference();
	output->Begin_xor();
    }
    canvas = c;
    drawn = false;
    offx = x;
    offy = y;
}

void Rubberband::Draw () {
    /* shouldn't be called */
}

void Rubberband::Redraw () {
    drawn = false;
    Draw();
}

void Rubberband::Erase () {
    if (drawn) {
        drawn = false;
        Draw();
        drawn = false;
    }
}

void Rubberband::Track (IntCoord x, IntCoord y) {
    if (x != trackx || y != tracky) {
        Erase();
	trackx = x;
	tracky = y;
	Draw();
    }
}

Rubberband::~Rubberband () {
    Unref(output);
}

void Rubberband::SetPainter (Painter* p) {
    if (p != output) {
	p->Reference();
	Unref(output);
	output = p;
	output->Begin_xor();
    }
}

void Rubberband::SetCanvas (Canvas* c) {
    canvas = c;
}
