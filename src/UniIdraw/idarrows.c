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
 * Implementation of ArrowLine, ArrowMultiLine, and ArrowOpenBSpline graphics.
 */

#include <UniIdraw/idarrowhead.h>
#include <UniIdraw/idarrows.h>
#include <UniIdraw/idclasses.h>

#include <Unidraw/globals.h>

#include <Unidraw/Graphic/pspaint.h>
#include <Unidraw/Graphic/util.h>

#include <InterViews/transformer.h>
#include <InterViews/painter.h>

#include <math.h>

/****************************************************************************/

ArrowLine::ArrowLine (
    Coord x0, Coord y0, Coord x1, Coord y1, boolean h, boolean t, 
    float arrow_scale, Graphic* gr
) : Line(x0, y0, x1, y1, gr) {
    _head = nil;
    _tail = nil;
    _arrow_scale = arrow_scale;

    _pat = nil;
    if (gr != nil) ArrowLine::SetPattern(gr->GetPattern());

    SetArrows(h, t);
}

ArrowLine::ArrowLine (
    Coord x0, Coord y0, Coord x1, Coord y1, Arrowhead* h, Arrowhead* t,
    float arrow_scale, Graphic* gr
) : Line(x0, y0, x1, y1, gr) {
    _head = h;
    _tail = t;
    _arrow_scale = arrow_scale;

    _pat = nil;
    if (gr != nil) ArrowLine::SetPattern(gr->GetPattern());
}

ArrowLine::~ArrowLine () {
    delete _head;
    delete _tail;
}

void ArrowLine::SetOriginal (Coord x0, Coord y0, Coord x1, Coord y1) {
    _x0 = x0;
    _y0 = y0;
    _x1 = x1;
    _y1 = y1;
    uncacheExtent();
    SetArrows(Head(), Tail());
}    

void ArrowLine::SetPattern (PSPattern* pat) {
    if (_pat != pat) {
	Ref(pat);
	Unref(_pat);
	_pat = pat;
	invalidateCaches();
    }
}

PSPattern* ArrowLine::GetPattern () { return _pat; }
Graphic& ArrowLine::operator = (Graphic& g) { return Graphic::operator=(g); }

ArrowLine& ArrowLine::operator = (ArrowLine& aline) {
    Graphic::operator=(aline);
    SetArrows(aline.Head(), aline.Tail());

    if (Head()) *_head = *aline._head; 
    if (Tail()) *_tail = *aline._tail; 
    _arrow_scale = aline.ArrowScale();

    return *this;
}

Graphic* ArrowLine::Copy () {
    Arrowhead* head = Head() ? (Arrowhead*) _head->Copy() : nil;
    Arrowhead* tail = Tail() ? (Arrowhead*) _tail->Copy() : nil;

    return new ArrowLine(_x0, _y0, _x1, _y1, head, tail, _arrow_scale, this);
}

void ArrowLine::ScaleArrows (float mag) {
    if (Head()) _head->Scale(mag, mag, _x0, _y0);
    if (Tail()) _tail->Scale(mag, mag, _x1, _y1);

    _arrow_scale = mag;
    invalidateCaches();
}

void ArrowLine::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    Line::getExtent(l, b, cx, cy, tol, gs);
    Extent e(l, b, cx, cy, tol);

    if (Head()) e.Merge(ArrowheadExtent(_head, gs));
    if (Tail()) e.Merge(ArrowheadExtent(_tail, gs));

    l = e._left;
    b = e._bottom;
    cx = e._cx;
    cy = e._cy;
    tol = e._tol;
}

boolean ArrowLine::contains (PointObj& po, Graphic* gs) {
    return 
        Line::contains(po, gs) || 
        Head() && ArrowheadContains(_head, po, gs) ||
        Tail() && ArrowheadContains(_tail, po, gs);
}

boolean ArrowLine::intersects (BoxObj& bo, Graphic* gs) {
    return 
        Line::intersects(bo, gs) || 
        Head() && ArrowheadIntersects(_head, bo, gs) ||
        Tail() && ArrowheadIntersects(_tail, bo, gs);
}

void ArrowLine::draw (Canvas* c, Graphic* gs) {
    PSBrush* br = gs->GetBrush();

    if (!br->None()) {
        Coord x0 = _x0, y0 = _y0, x1 = _x1, y1 = _y1;

        if (Head()) _head->CorrectedTip(x0, y0, br, gs->GetTransformer());
        if (Tail()) _tail->CorrectedTip(x1, y1, br, gs->GetTransformer());

        update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5
#undef Line
        _p->Line(c, x0, y0, x1, y1);
#define Line _lib_iv(Line)
#else
        _p->Line(c, x0, y0, x1, y1);
#endif /* Line */
    }

    if (Head()) ArrowheadDraw(_head, c, gs);
    if (Tail()) ArrowheadDraw(_tail, c, gs);
}

Extent& ArrowLine::ArrowheadExtent (Arrowhead* arrow, Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;
    static Extent e;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    getExtentGraphic(arrow, e._left, e._bottom, e._cx, e._cy, e._tol, &gstemp);

    return e;
}

boolean ArrowLine::ArrowheadContains (
    Arrowhead* arrow, PointObj& po, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return containsGraphic(arrow, po, &gstemp);
}

boolean ArrowLine::ArrowheadIntersects (
    Arrowhead* arrow, BoxObj& bo, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return intersectsGraphic(arrow, bo, &gstemp);
}

void ArrowLine::ArrowheadDraw (Arrowhead* arrow, Canvas* c, Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    drawGraphic(arrow, c, &gstemp);
}

void ArrowLine::SetArrows (boolean h, boolean t) {
    delete _head;
    delete _tail;

    Coord width = round(ARROWWIDTH*ivpoints);
    Coord height = round(ARROWHEIGHT*ivpoints);

    _head = h ? new Arrowhead(_x0, _y0, width, height) : nil;
    _tail = t ? new Arrowhead(_x1, _y1, width, height) : nil;

    float angle = atan2(_y0-_y1, _x0-_x1)*180/M_PI;

    if (h) _head->Rotate(angle-90, _x0, _y0);
    if (t) _tail->Rotate(angle+90, _x1, _y1);

    ScaleArrows(_arrow_scale);
    invalidateCaches();
}

ClassId ArrowLine::CompId () { return ARROWLINE_COMP; }

/****************************************************************************/

ArrowMultiLine::ArrowMultiLine (
    Coord* x, Coord* y, int count, boolean h, boolean t,
    float arrow_scale, Graphic* gr
) : SF_MultiLine(x, y, count, gr) {
    _head = nil;
    _tail = nil;
    _arrow_scale = arrow_scale;
    if (x && y ) SetArrows(h, t);
    _arrow_mask = 0;
    _arrow_mask |= h ? 0x1 : 0;
    _arrow_mask |= t ? 0x2 : 0;
}

ArrowMultiLine::ArrowMultiLine (
    Coord* x, Coord* y, int count, Arrowhead* h, Arrowhead* t,
    float arrow_scale, Graphic* gr
) : SF_MultiLine(x, y, count, gr) {
    _head = h;
    _tail = t;
    _arrow_scale = arrow_scale;
}

ArrowMultiLine::~ArrowMultiLine () {
    delete _head;
    delete _tail;
}

Graphic& ArrowMultiLine::operator = (Graphic& g) {
    return Graphic::operator=(g);
}

ArrowMultiLine& ArrowMultiLine::operator = (ArrowMultiLine& aml) {
    Graphic::operator=(aml);
    SetArrows(aml.Head(), aml.Tail());

    if (Head()) *_head = *aml._head; 
    if (Tail()) *_tail = *aml._tail; 
    _arrow_scale = aml.ArrowScale();

    return *this;
}

Graphic* ArrowMultiLine::Copy () {
    Arrowhead* head = Head() ? (Arrowhead*) _head->Copy() : nil;
    Arrowhead* tail = Tail() ? (Arrowhead*) _tail->Copy() : nil;

    return new ArrowMultiLine(x(), y(), count(), head, tail, _arrow_scale, this);
}

void ArrowMultiLine::ScaleArrows (float mag) {
    if (Head()) _head->Scale(mag, mag, x()[0], y()[0]);
    if (Tail()) _tail->Scale(mag, mag, x()[count()-1], y()[count()-1]);
    
    _arrow_scale = mag;
    invalidateCaches();
}

void ArrowMultiLine::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    SF_MultiLine::getExtent(l, b, cx, cy, tol, gs);
    Extent e(l, b, cx, cy, tol);

    if (Head()) e.Merge(ArrowheadExtent(_head, gs));
    if (Tail()) e.Merge(ArrowheadExtent(_tail, gs));

    l = e._left;
    b = e._bottom;
    cx = e._cx;
    cy = e._cy;
    tol = e._tol;
}

boolean ArrowMultiLine::contains (PointObj& po, Graphic* gs) {
    return 
        SF_MultiLine::contains(po, gs) || 
        Head() && ArrowheadContains(_head, po, gs) ||
        Tail() && ArrowheadContains(_tail, po, gs);
}

boolean ArrowMultiLine::intersects (BoxObj& bo, Graphic* gs) {
    return 
        SF_MultiLine::intersects(bo, gs) || 
        Head() && ArrowheadIntersects(_head, bo, gs) ||
        Tail() && ArrowheadIntersects(_tail, bo, gs);
}

void ArrowMultiLine::draw (Canvas* c, Graphic* gs) {
    PSBrush* br = gs->GetBrush();

    if (!Head() && !Tail()) {
        SF_MultiLine::draw(c, gs);

    } else if (!br->None()) {
        int l = count()-1;
        Coord x0 = x()[0], y0 = y()[0], x1 = x()[l], y1 = y()[l];
        Coord tx0 = x0, ty0 = y0, tx1 = x1, ty1 = y1;

        if (Head()) _head->CorrectedTip(tx0, ty0, br, gs->GetTransformer());
        if (Tail()) _tail->CorrectedTip(tx1, ty1, br, gs->GetTransformer());

        x()[0] = tx0; y()[0] = ty0;
        x()[l] = tx1; y()[l] = ty1;

        update(gs);
#if __GNUC__>=2 && __GNUC_MINOR__>=5
#undef MultiLine
        _p->MultiLine(c, x(), y(), count());
#define MultiLine _lib_iv(MultiLine)
#else
        _p->MultiLine(c, x(), y(), count());
#endif /* MultiLine */

        x()[0] = x0; y()[0] = y0;
        x()[l] = x1; y()[l] = y1;

        if (Head()) ArrowheadDraw(_head, c, gs);
        if (Tail()) ArrowheadDraw(_tail, c, gs);
    }
}

Extent& ArrowMultiLine::ArrowheadExtent (Arrowhead* arrow, Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;
    static Extent e;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    getExtentGraphic(arrow, e._left, e._bottom, e._cx, e._cy, e._tol, &gstemp);

    return e;
}

boolean ArrowMultiLine::ArrowheadContains (
    Arrowhead* arrow, PointObj& po, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return containsGraphic(arrow, po, &gstemp);
}

boolean ArrowMultiLine::ArrowheadIntersects (
    Arrowhead* arrow, BoxObj& bo, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return intersectsGraphic(arrow, bo, &gstemp);
}

void ArrowMultiLine::ArrowheadDraw (Arrowhead* arrow, Canvas* c, Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    drawGraphic(arrow, c, &gstemp);
}

void ArrowMultiLine::SetArrows (boolean h, boolean t) {
    delete _head;
    delete _tail;

    Coord width = round(ARROWWIDTH*ivpoints);
    Coord height = round(ARROWHEIGHT*ivpoints);

    int k = count()-2;
    int l = count()-1;

    _head = h ? new Arrowhead(x()[0], y()[0], width, height) : nil;
    _tail = t ? new Arrowhead(x()[l], y()[l], width, height) : nil;

    float head_angle = atan2(y()[0]-y()[1], x()[0]-x()[1])*180/M_PI;
    float tail_angle = atan2(y()[k]-y()[l], x()[k]-x()[l])*180/M_PI;

    if (h) _head->Rotate(head_angle-90, x()[0], y()[0]);
    if (t) _tail->Rotate(tail_angle+90, x()[l], y()[l]);

    ScaleArrows(_arrow_scale);
    invalidateCaches();
}

ClassId ArrowMultiLine::CompId () { return ARROWMULTILINE_COMP; }

void ArrowMultiLine::SetOriginal (MultiLineObj* mlo) {
    Vertices::SetOriginal(mlo);
    SetArrows(_arrow_mask&0x1, _arrow_mask&0x2);
}

/****************************************************************************/

ArrowOpenBSpline::ArrowOpenBSpline (
    Coord* x, Coord* y, int count, boolean h, boolean t,
    float arrow_scale, Graphic* gr
) : SFH_OpenBSpline(x, y, count, gr) {
    _head = nil;
    _tail = nil;
    _arrow_scale = arrow_scale;
    if (x && y ) SetArrows(h, t);
    _arrow_mask = 0;
    _arrow_mask |= h ? 0x1 : 0;
    _arrow_mask |= t ? 0x2 : 0;
}

ArrowOpenBSpline::ArrowOpenBSpline (
    Coord* x, Coord* y, int count, Arrowhead* h, Arrowhead* t,
    float arrow_scale, Graphic* gr
) : SFH_OpenBSpline(x, y, count, gr) {
    _head = h;
    _tail = t;
    _arrow_scale = arrow_scale;
}

ArrowOpenBSpline::~ArrowOpenBSpline () {
    delete _head;
    delete _tail;
}

Graphic& ArrowOpenBSpline::operator = (Graphic& g) {
    return Graphic::operator=(g);
}

ArrowOpenBSpline& ArrowOpenBSpline::operator = (ArrowOpenBSpline& aml) {
    Graphic::operator=(aml);
    SetArrows(aml.Head(), aml.Tail());

    if (Head()) *_head = *aml._head; 
    if (Tail()) *_tail = *aml._tail; 
    _arrow_scale = aml.ArrowScale();

    return *this;
}

Graphic* ArrowOpenBSpline::Copy () {
    Arrowhead* head = Head() ? (Arrowhead*) _head->Copy() : nil;
    Arrowhead* tail = Tail() ? (Arrowhead*) _tail->Copy() : nil;
    Coord* x, *y;
    const Coord* cx, * cy;
    int count = GetOriginal(cx, cy);
    x = (Coord*)cx; y = (Coord*)cy;

    return new ArrowOpenBSpline(x, y, count, head, tail, _arrow_scale, this);
}

void ArrowOpenBSpline::ScaleArrows (float mag) {
    if (Head()) _head->Scale(mag, mag, x()[0], y()[0]);
    if (Tail()) _tail->Scale(mag, mag, x()[count()-1], y()[count()-1]);
    
    _arrow_scale = mag;
    invalidateCaches();
}

void ArrowOpenBSpline::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    SFH_OpenBSpline::getExtent(l, b, cx, cy, tol, gs);
    Extent e(l, b, cx, cy, tol);

    if (Head()) e.Merge(ArrowheadExtent(_head, gs));
    if (Tail()) e.Merge(ArrowheadExtent(_tail, gs));

    l = e._left;
    b = e._bottom;
    cx = e._cx;
    cy = e._cy;
    tol = e._tol;
}

boolean ArrowOpenBSpline::contains (PointObj& po, Graphic* gs) {
    return 
        SFH_OpenBSpline::contains(po, gs) || 
        Head() && ArrowheadContains(_head, po, gs) ||
        Tail() && ArrowheadContains(_tail, po, gs);
}

boolean ArrowOpenBSpline::intersects (BoxObj& bo, Graphic* gs) {
    return 
        SFH_OpenBSpline::intersects(bo, gs) || 
        Head() && ArrowheadIntersects(_head, bo, gs) ||
        Tail() && ArrowheadIntersects(_tail, bo, gs);
}

void ArrowOpenBSpline::draw (Canvas* c, Graphic* gs) {
    PSBrush* br = gs->GetBrush();

    if (!Head() && !Tail()) {
        SFH_OpenBSpline::draw(c, gs);

    } else if (!br->None()) {
        int j = count()-3, k = count()-2, l = count()-1;
        Coord x0 = x()[0], y0 = y()[0], x1 = x()[l], y1 = y()[l];
        Coord tx0 = x0, ty0 = y0, tx1 = x1, ty1 = y1;

        if (Head()) _head->CorrectedTip(tx0, ty0, br, gs->GetTransformer());
        if (Tail()) _tail->CorrectedTip(tx1, ty1, br, gs->GetTransformer());

        x()[0] = x()[1] = x()[2] = tx0; y()[0] = y()[1] = y()[2] = ty0;
        x()[l] = x()[k] = x()[j] = tx1; y()[l] = y()[k] = y()[k] = ty1;

        update(gs);
        _p->BSpline(c, x(), y(), count());

        x()[0] = x()[1] = x()[2] = x0; y()[0] = y()[1] = y()[2] = y0;
        x()[l] = x()[k] = x()[j] = x1; y()[l] = y()[k] = y()[j] = y1;

        if (Head()) ArrowheadDraw(_head, c, gs);
        if (Tail()) ArrowheadDraw(_tail, c, gs);
    }
}

Extent& ArrowOpenBSpline::ArrowheadExtent (Arrowhead* arrow, Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;
    static Extent e;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    getExtentGraphic(arrow, e._left, e._bottom, e._cx, e._cy, e._tol, &gstemp);

    return e;
}

boolean ArrowOpenBSpline::ArrowheadContains (
    Arrowhead* arrow, PointObj& po, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return containsGraphic(arrow, po, &gstemp);
}

boolean ArrowOpenBSpline::ArrowheadIntersects (
    Arrowhead* arrow, BoxObj& bo, Graphic* gs
) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    return intersectsGraphic(arrow, bo, &gstemp);
}

void ArrowOpenBSpline::ArrowheadDraw (Arrowhead* arrow,Canvas* c,Graphic* gs) {
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);    
    concatGraphic(arrow, arrow, gs, &gstemp);
    drawGraphic(arrow, c, &gstemp);
}

void ArrowOpenBSpline::SetArrows (boolean h, boolean t) {
    delete _head;
    delete _tail;

    Coord width = round(ARROWWIDTH*ivpoints);
    Coord height = round(ARROWHEIGHT*ivpoints);

    int l = count()-1;
    int k = l-3;

    _head = h ? new Arrowhead(x()[0], y()[0], width, height) : nil;
    _tail = t ? new Arrowhead(x()[l], y()[l], width, height) : nil;

    float head_angle = atan2(y()[0]-y()[3], x()[0]-x()[3])*180/M_PI;
    float tail_angle = atan2(y()[k]-y()[l], x()[k]-x()[l])*180/M_PI;

    if (h) _head->Rotate(head_angle-90, x()[0], y()[0]);
    if (t) _tail->Rotate(tail_angle+90, x()[l], y()[l]);

    ScaleArrows(_arrow_scale);
    invalidateCaches();
}

ClassId ArrowOpenBSpline::CompId () { return ARROWSPLINE_COMP; }

void ArrowOpenBSpline::SetOriginal (MultiLineObj* mlo) {
    Vertices::SetOriginal(mlo);
    SetArrows(_arrow_mask&0x1, _arrow_mask&0x2);
}

