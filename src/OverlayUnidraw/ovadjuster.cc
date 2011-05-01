/*
 * Copyright (c) 1995 Vectaport Inc.
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
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
 * Implementation of Adjuster derived classes.
 */
#include <OverlayUnidraw/ovadjuster.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>

#include <InterViews/bitmap.h>

#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/perspective.h>
#include <IV-2_6/InterViews/sensor.h>
#include <IV-2_6/InterViews/shape.h>

#include <InterViews/Bitmaps/enlargeHit.bm>
#include <InterViews/Bitmaps/enlargeMask.bm>
#include <InterViews/Bitmaps/enlarge.bm>
#include <InterViews/Bitmaps/reducerHit.bm>
#include <InterViews/Bitmaps/reducerMask.bm>
#include <InterViews/Bitmaps/reducer.bm>
#include <InterViews/Bitmaps/lmoverHit.bm>
#include <InterViews/Bitmaps/lmoverMask.bm>
#include <InterViews/Bitmaps/lmover.bm>
#include <InterViews/Bitmaps/rmoverHit.bm>
#include <InterViews/Bitmaps/rmoverMask.bm>
#include <InterViews/Bitmaps/rmover.bm>
#include <InterViews/Bitmaps/umoverHit.bm>
#include <InterViews/Bitmaps/umoverMask.bm>
#include <InterViews/Bitmaps/umover.bm>
#include <InterViews/Bitmaps/dmoverHit.bm>
#include <InterViews/Bitmaps/dmoverMask.bm>
#include <InterViews/Bitmaps/dmover.bm>

#include <OS/math.h>

#include <IV-2_6/_enter.h>

enum MoveType { 
    MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN, MOVE_UNDEF
};

inline Bitmap* MakeBitmap(void* bits, int width, int height) {
    Bitmap* b = new Bitmap(bits, width, height);
    b->Reference();
    return b;
}

OvMover::OvMover(Interactor* i, int delay, int mt) : Adjuster(i, delay) {
    Init(mt);
}

OvMover::OvMover(
    const char* name, Interactor* i, int delay, int mt
) : Adjuster(name, i, delay) {
    Init(mt);
}

OvMover::~OvMover() { }

void OvMover::Init(int mt) {
    SetClassName("OvMover");
    moveType = mt;
}

void OvMover::AdjustView(Event& e) {
    register Perspective* basep = shown;
    int amtx, amty;

    *basep = *view->GetPerspective();
    amtx = e.shift ? basep->lx : basep->sx;
    amty = e.shift ? basep->ly : basep->sy;

    IntCoord dx = 0, dy = 0;
    switch (moveType) {
	case MOVE_LEFT:	    dx = -amtx; break;
	case MOVE_RIGHT:    dx = amtx; break;
	case MOVE_UP:	    dy = amty; break;
	case MOVE_DOWN:	    dy =  -amty; break;
	default:	    break;
    }

    basep->curx += dx;
    basep->cury += dy;
    view->Adjust(*basep);
}

static Bitmap* lmoverMask;
static Bitmap* lmoverPlain;
static Bitmap* lmoverHit;

OvLeftMover::OvLeftMover(Interactor* i, int delay) : OvMover(i, delay, MOVE_LEFT) {
    Init();
}

OvLeftMover::OvLeftMover(
    const char* name, Interactor* i, int delay
) : OvMover(name, i, delay, MOVE_LEFT) {
    Init();
}

OvLeftMover::~OvLeftMover() { }

void OvLeftMover::Init() {
    SetClassName("OvLeftMover");
    if (lmoverMask == nil) {
        lmoverMask = MakeBitmap(
            lmover_mask_bits, lmover_mask_width, lmover_mask_height
        );
        lmoverPlain = MakeBitmap(
            lmover_plain_bits, lmover_plain_width, lmover_plain_height
        );
        lmoverHit = MakeBitmap(
            lmover_hit_bits, lmover_hit_width, lmover_hit_height
        );
    }
    mask = lmoverMask;
    plain = lmoverPlain;
    hit = lmoverHit;
    shape->Rigid(shape->width/2, 0, shape->height/2, vfil);
}

static Bitmap* rmoverMask;
static Bitmap* rmoverPlain;
static Bitmap* rmoverHit;

OvRightMover::OvRightMover(Interactor* i, int d) : OvMover(i, d, MOVE_RIGHT) {
    Init();
}

OvRightMover::OvRightMover(
    const char* name, Interactor* i, int d
) : OvMover(name, i, d, MOVE_RIGHT) {
    Init();
}

OvRightMover::~OvRightMover() { }

void OvRightMover::Init() {
    SetClassName("OvRightMover");
    if (rmoverMask == nil) {
        rmoverMask = MakeBitmap(
            rmover_mask_bits, rmover_mask_width, rmover_mask_height
        );
        rmoverPlain = MakeBitmap(
            rmover_plain_bits, rmover_plain_width, rmover_plain_height
        );
        rmoverHit = MakeBitmap(
            rmover_hit_bits, rmover_hit_width, rmover_hit_height
        );
    }
    mask = rmoverMask;
    plain = rmoverPlain;
    hit = rmoverHit;
    shape->Rigid(shape->width/2, 0, shape->height/2, vfil);
}

static Bitmap* umoverMask;
static Bitmap* umoverPlain;
static Bitmap* umoverHit;

OvUpMover::OvUpMover(Interactor* i, int d) : OvMover(i, d, MOVE_UP) {
    Init();
}

OvUpMover::OvUpMover(
    const char* name, Interactor* i, int d
) : OvMover(name, i, d, MOVE_UP) {
    Init();
}

OvUpMover::~OvUpMover() { }

void OvUpMover::Init() {
    SetClassName("OvUpMover");
    if (umoverMask == nil) {
        umoverMask = MakeBitmap(
            umover_mask_bits, umover_mask_width, umover_mask_height
        );
        umoverPlain = MakeBitmap(
            umover_plain_bits, umover_plain_width, umover_plain_height
        );
        umoverHit = MakeBitmap(
            umover_hit_bits, umover_hit_width, umover_hit_height
        );
    }
    mask = umoverMask;
    plain = umoverPlain;
    hit = umoverHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}

static Bitmap* dmoverMask;
static Bitmap* dmoverPlain;
static Bitmap* dmoverHit;

OvDownMover::OvDownMover(Interactor* i, int d) : OvMover(i, d, MOVE_DOWN) {
    Init();
}

OvDownMover::OvDownMover(
    const char* name, Interactor* i, int d
) : OvMover(name, i, d, MOVE_DOWN) {
    Init();
}

OvDownMover::~OvDownMover() { }

void OvDownMover::Init() {
    SetClassName("OvDownMover");
    if (dmoverMask == nil) {
        dmoverMask = MakeBitmap(
            dmover_mask_bits, dmover_mask_width, dmover_mask_height
        );
        dmoverPlain = MakeBitmap(
            dmover_plain_bits, dmover_plain_width, dmover_plain_height
        );
        dmoverHit = MakeBitmap(
            dmover_hit_bits, dmover_hit_width, dmover_hit_height
        );
    }
    mask = dmoverMask;
    plain = dmoverPlain;
    hit = dmoverHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}
