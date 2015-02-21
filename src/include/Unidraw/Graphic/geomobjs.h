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
 * Interface to geometrical objects used to implement Graphic primitives.
 */

#ifndef unidraw_graphic_geomobjs_h
#define unidraw_graphic_geomobjs_h

#include <cstdio>

#include <IV-2_6/InterViews/defs.h>
#include <Unidraw/enter-scope.h>
#include <InterViews/resource.h>

#include <IV-2_6/_enter.h>

#define GEOMOBJS_DEFINED
#include <leakchecker.h>

class UList;
#include <iosfwd>

//: point geometric object
// <a href=../man3.1/geomobjs.html>man page</a>
class PointObj {
public:
    PointObj(Coord = 0, Coord = 0);
    PointObj(PointObj*);

    float Distance(PointObj&);
public:
    Coord _x, _y;
};

//: line geometric object
// <a href=../man3.1/geomobjs.html>man page</a>
class LineObj {
public:
    LineObj(Coord = 0, Coord = 0, Coord = 0, Coord = 0);
    LineObj(LineObj*);

    boolean Contains(PointObj&);
    int Same(PointObj& p1, PointObj& p2);
    boolean Intersects(LineObj&);
public:
    PointObj _p1, _p2;
};

//: box geometric object
// <a href=../man3.1/geomobjs.html>man page</a>
class BoxObj {
public:
    BoxObj(Coord = 0, Coord = 0, Coord = 0, Coord = 0);
    BoxObj(BoxObj*);

    boolean Contains(PointObj&);
    boolean Intersects(BoxObj&);
    boolean Intersects(LineObj&);
    BoxObj operator-(BoxObj&);
    BoxObj operator+(BoxObj&);
    boolean Within(BoxObj&);
public:
    Coord _left, _right;
    Coord _bottom, _top;
};

//: multi-line geometric object
// <a href=../man3.1/geomobjs.html>man page</a>
class MultiLineObj : public Resource {
public:
    MultiLineObj(Coord* = nil, Coord* = nil, int = 0);
    virtual ~MultiLineObj();

    void GetBox(BoxObj& b);
    boolean Contains(PointObj&);
    boolean Intersects(LineObj&);
    boolean Intersects(BoxObj&);
    boolean Within(BoxObj&);
    void SplineToMultiLine(Coord* cpx, Coord* cpy, int cpcount);
    void ClosedSplineToPolygon(Coord* cpx, Coord* cpy, int cpcount);
protected:
    void GrowBuf();
    boolean CanApproxWithLine(
	double x0, double y0, double x2, double y2, double x3, double y3
    );
    void AddLine(double x0, double y0, double x1, double y1);
    void AddBezierArc(
        double x0, double y0, double x1, double y1,
        double x2, double y2, double x3, double y3
    );
    void CalcSection(
	Coord cminus1x, Coord cminus1y, Coord cx, Coord cy,
	Coord cplus1x, Coord cplus1y, Coord cplus2x, Coord cplus2y
    );
public:
    Coord* _x, *_y;
    int _count;
    UList* _ulist;
    int _pts_made;

    Coord* x() {return _x;}
    Coord* y() {return _y;}
    int count() {return _count;}

    virtual boolean operator == (MultiLineObj&);
    virtual boolean operator != (MultiLineObj&);

    static MultiLineObj* make_pts(const Coord* x, const Coord*y, int npts);

    static void CompactPoints(boolean flag) {_pts_by_n_enabled=flag;}
protected:
    static UList** _pts_by_n;
    static int _pts_by_n_size;
    static boolean _pts_by_n_enabled;

#ifdef LEAKCHECK
 public:
    static LeakChecker* _leakchecker;
#endif
};

//: filled polygon geometric object
// <a href=../man3.1/geomobjs.html>man page</a>
class FillPolygonObj : public MultiLineObj {
public:
    FillPolygonObj(Coord* = nil, Coord* = nil, int = 0);
    virtual ~FillPolygonObj();

    boolean Contains(PointObj&);
    boolean Intersects(LineObj&);
    boolean Intersects(BoxObj&);
protected:
    void Normalize();
protected:
    Coord* _normx, *_normy;
    int _normCount;
};

//: geometric extent object
// <a href=../man3.1/geomobjs.html>man page</a>
class Extent {
public:
    Extent(float = 0, float = 0, float = 0, float = 0, float = 0);
    Extent(Extent&);

    boolean Undefined();
    boolean Within(Extent& e);
    void Merge(Extent&);
public:
    /* defines lower left and center of an object */
    float _left, _bottom, _cx, _cy, _tol;
};

/*
 * inlines
 */

inline boolean Extent::Undefined () { return _left == _cx && _bottom == _cy; }

#include <IV-2_6/_leave.h>

#endif
