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
 * ArrowLine - a Line with arrows on the ends
 * ArrowMultiLine graphic -- a SF_MultiLine with arrows on the ends
 * ArrowOpenBSpline graphic -- an SF_OpenBSpline with arrows on the ends
 */

#ifndef idarrows_h
#define idarrows_h

#include <Unidraw/Graphic/lines.h>
#include <Unidraw/Graphic/splines.h>

static const float ARROWHEIGHT = 8;            // in points
static const float ARROWWIDTH = ARROWHEIGHT/2;

class Arrowhead;

class ArrowLine : public Line {
public:
    ArrowLine(
	IntCoord x0, IntCoord y0, IntCoord x1, IntCoord y1,
	boolean h, boolean t, float arrow_scale, Graphic* gr = nil
    );
    virtual ~ArrowLine();

    void SetOriginal(IntCoord x0, IntCoord y0, IntCoord x1, IntCoord y1);

    void GetHead(float &x, float &y);
    void GetTail(float &x, float &y);

    void SetArrows(boolean h, boolean t);
    void ScaleArrows(float);
    boolean Head();
    boolean Tail();
    float ArrowScale();

    virtual void SetPattern(PSPattern*);
    virtual PSPattern* GetPattern();

    virtual Graphic* Copy();
    virtual ClassId CompId();
    virtual Graphic& operator = (Graphic&);
    virtual ArrowLine& operator = (ArrowLine&);
protected:
    ArrowLine(
	Coord,Coord,Coord,Coord, Arrowhead*, Arrowhead*, float, Graphic* = nil
    );
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
protected:
    Extent& ArrowheadExtent(Arrowhead*, Graphic* gs);
    boolean ArrowheadContains(Arrowhead*, PointObj&, Graphic* gs);
    boolean ArrowheadIntersects(Arrowhead*, BoxObj&, Graphic* gs);
    void ArrowheadDraw(Arrowhead*, Canvas*, Graphic* gs);
protected:
    PSPattern* _pat;
    Arrowhead* _head;
    Arrowhead* _tail;
    float _arrow_scale;
};

inline boolean ArrowLine::Head () { return _head != nil; }
inline boolean ArrowLine::Tail () { return _tail != nil; }
inline float ArrowLine::ArrowScale () { return _arrow_scale; }

class ArrowMultiLine : public SF_MultiLine {
public:
    ArrowMultiLine(
        IntCoord* x, IntCoord* y, int count, boolean h, boolean t, float arrow_scale,
        Graphic* gr = nil
    );
    virtual ~ArrowMultiLine();

    void SetArrows(boolean h, boolean t);
    void ScaleArrows(float);
    boolean Head();
    boolean Tail();
    float ArrowScale();

    virtual Graphic* Copy();
    virtual ClassId CompId();
    virtual Graphic& operator = (Graphic&);
    virtual ArrowMultiLine& operator = (ArrowMultiLine&);

    virtual void SetOriginal(MultiLineObj*);
protected:
    ArrowMultiLine(
	IntCoord*, IntCoord*, int, Arrowhead*, Arrowhead*, float, Graphic* gr = nil
    );
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
protected:
    Extent& ArrowheadExtent(class Arrowhead*, Graphic* gs);
    boolean ArrowheadContains(Arrowhead*, PointObj&, Graphic* gs);
    boolean ArrowheadIntersects(Arrowhead*, BoxObj&, Graphic* gs);
    void ArrowheadDraw(Arrowhead*, Canvas*, Graphic* gs);
protected:
    Arrowhead* _head;
    Arrowhead* _tail;
    float _arrow_scale;
    unsigned int _arrow_mask;
};

inline boolean ArrowMultiLine::Head () { return _head != nil; }
inline boolean ArrowMultiLine::Tail () { return _tail != nil; }
inline float ArrowMultiLine::ArrowScale () { return _arrow_scale; }

class ArrowOpenBSpline : public SFH_OpenBSpline {
public:
    ArrowOpenBSpline(
        IntCoord* x, IntCoord* y, int count, boolean h, boolean t, float arrow_scale,
        Graphic* gr = nil
    );
    virtual ~ArrowOpenBSpline();

    void SetArrows(boolean h, boolean t);
    void ScaleArrows(float);
    boolean Head();
    boolean Tail();
    float ArrowScale();

    virtual Graphic* Copy();
    virtual ClassId CompId();
    virtual Graphic& operator = (Graphic&);
    virtual ArrowOpenBSpline& operator = (ArrowOpenBSpline&);

    virtual void SetOriginal(MultiLineObj*);
protected:
    ArrowOpenBSpline(
	IntCoord*, IntCoord*, int, Arrowhead*, Arrowhead*, float, Graphic* gr = nil
    );
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
protected:
    Extent& ArrowheadExtent(class Arrowhead*, Graphic* gs);
    boolean ArrowheadContains(Arrowhead*, PointObj&, Graphic* gs);
    boolean ArrowheadIntersects(Arrowhead*, BoxObj&, Graphic* gs);
    void ArrowheadDraw(Arrowhead*, Canvas*, Graphic* gs);
protected:
    Arrowhead* _head;
    Arrowhead* _tail;
    float _arrow_scale;
    unsigned int _arrow_mask;
};

inline boolean ArrowOpenBSpline::Head () { return _head != nil; }
inline boolean ArrowOpenBSpline::Tail () { return _tail != nil; }
inline float ArrowOpenBSpline::ArrowScale () { return _arrow_scale; }

#endif
