/*
 * Copyright (c) 1999 Vectaport Inc.
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
 * Arrowhead implementation.
 */

#include "idarrowhead.h"

#include <InterViews/transformer.h>

#include <OS/math.h>
#include <math.h>

/*****************************************************************************/

static const int COUNT = 4;

static const int BOTLEFT = 0;
static const int TIP = 1;
static const int BOTRIGHT = 2;
static const int BOTCTR = 3;

/*****************************************************************************/

Arrowhead::Arrowhead (
    Coord tx, Coord ty, Coord w, Coord h, Graphic* g
) : SF_Polygon(x(tx, w), y(ty, h), COUNT, g) { }

Arrowhead::Arrowhead (
    Coord* x, Coord* y, Graphic* g
) : SF_Polygon(x, y, COUNT, g) { }

Graphic* Arrowhead::Copy () { return new Arrowhead(Vertices::x(), Vertices::y(), this); }
Graphic& Arrowhead::operator = (Graphic& g) { return Graphic::operator=(g); }
Arrowhead& Arrowhead::operator = (Arrowhead& g) { return (Arrowhead&)Graphic::operator=(g); }

Coord Arrowhead::CorrectedHeight (float t) {
    float w = Vertices::x()[BOTRIGHT] - Vertices::x()[BOTCTR];
    float h = Vertices::y()[TIP] - Vertices::y()[BOTRIGHT];

    float a = -4*h * w*w;
    float radicand = 4*w*w + 4*h*h - t*t;
    float root = (radicand < 0.) ? 0. : sqrt(radicand);
    float b = t*w * root;
    float c = t*t - 4*w*w;

    if (c == 0) return 0;

    Coord h1 = Math::round((a + b) / c);
    Coord h2 = Math::round((a - b) / c);

    return (h1 < h && h1 > 0) ? h1 : h2;
}

void Arrowhead::CorrectedTip (
    Coord& tipx, Coord& tipy, PSBrush* br, Transformer* t
) {
    Transformer total(t);
    Transformer* my_t = GetTransformer();
    concatTransformer(my_t, t, &total);
    
    float thk = UnscaledLength(br->Width(), &total);
    tipx = Vertices::x()[TIP];
    tipy = Vertices::y()[BOTLEFT] + CorrectedHeight(thk);

    if (my_t != nil) my_t->Transform(tipx, tipy);
}

float Arrowhead::UnscaledLength (float length, Transformer* t) {
    Transformer inverse(t);
    inverse.Invert();

    float x0 = 0, y0 = 0, x1 = length, y1 = 0;
    float tx0, ty0, tx1, ty1;

    inverse.Transform(x0, y0, tx0, ty0);
    inverse.Transform(x1, y1, tx1, ty1);

    return hypot(tx0 - tx1, ty0 - ty1);
}

void Arrowhead::draw (Canvas* c, Graphic* gs) {
    PSPattern* pat = gs->GetPattern();
    PSBrush* br = gs->GetBrush();

    if (br->None()) {
        Vertices::y()[BOTCTR] = Vertices::y()[BOTLEFT];
        SF_Polygon::draw(c, gs);

    } else {
        /*  if brush is dashed, disable it for the arrowhead */
	if (br->dashed()) {
	  Ref(br);
	  PSBrush* newbr = new PSBrush(0, br->Width());
	  gs->SetBrush(newbr);
	}

        Coord ytip = Vertices::y()[TIP];
        float thk = UnscaledLength(br->Width(), gs->GetTransformer());
        Coord hcorrect = CorrectedHeight(thk);

        if (pat->None()) {
            Vertices::y()[BOTCTR] = Vertices::y()[TIP] = Vertices::y()[BOTLEFT] + hcorrect;
            SF_Polygon::draw(c, gs);
            Vertices::y()[BOTCTR] = Vertices::y()[TIP] = ytip;

        } else {
            Vertices::y()[BOTCTR] = Vertices::y()[BOTLEFT];
            Vertices::y()[TIP] = Vertices::y()[BOTLEFT] + hcorrect;
            SF_Polygon::draw(c, gs);
            Vertices::y()[TIP] = ytip;
        }

        /*  if brush was dashed, restore it */
	if (br->dashed()) {
	  gs->SetBrush(br);
	  Unref(br);
	}
    }
}

Coord* Arrowhead::x (Coord tipx, Coord w) {
    static Coord px[COUNT];
    
    px[BOTLEFT] = tipx - w/2;
    px[TIP] = px[BOTCTR] = tipx;
    px[BOTRIGHT] = tipx + w/2;

    return px;
}

Coord* Arrowhead::y (Coord tipy, Coord h) {
    static Coord py[COUNT];
    
    py[BOTLEFT] = py[BOTRIGHT] = py[BOTCTR] = tipy - h;
    py[TIP] = tipy;

    return py;
}
