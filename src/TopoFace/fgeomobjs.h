/*
 * Copyright (c) 1996-1997 Vectaport Inc.
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
 * Interface to geometrical objects with floating point coordinates
 */

#ifndef fgeomobjs_h
#define fgeomobjs_h

#include <IV-2_6/InterViews/defs.h>
#include <Unidraw/enter-scope.h>

class UList;
class ostream;

class FPointObj {
public:
    FPointObj(float = 0, float = 0);
    FPointObj(FPointObj*);

    float Distance(FPointObj&);
public:
    float _x, _y;
};

class FLineObj {
public:
    FLineObj(float = 0, float = 0, float = 0, float = 0);
    FLineObj(FLineObj*);
    ~FLineObj();

    boolean Contains(FPointObj&);
    int Same(FPointObj& p1, FPointObj& p2);
    boolean Intersects(FLineObj&);
    boolean EquationIntersects(FLineObj&, float &x, float&y);
    int Bresenham(int*& xpts, int*& ypts);
    int Extent(float& xmin, float& xmax, float& ymin, float& ymax);
public:
    FPointObj _p1, _p2;
    int* _xpts;
    int* _ypts;
    int _npts;
};

class FBoxObj {
public:
    FBoxObj(float = 0, float = 0, float = 0, float = 0);
    FBoxObj(FBoxObj*);

    boolean Contains(FPointObj&);
    boolean Intersects(FBoxObj&);
    boolean Intersects(FLineObj&);
    FBoxObj operator-(FBoxObj&);
    FBoxObj operator+(FBoxObj&);
    boolean Within(FBoxObj&);
public:
    float _left, _right;
    float _bottom, _top;
};

class FMultiLineObj {
public:
    FMultiLineObj(float* = nil, float* = nil, int = 0);
    virtual ~FMultiLineObj();

    void GetBox(FBoxObj& b);
    boolean Contains(FPointObj&);
    boolean Intersects(FLineObj&);
    boolean Intersects(FBoxObj&);
    boolean Within(FBoxObj&);
    void SplineToMultiLine(float* cpx, float* cpy, int cpcount);
    void ClosedSplineToPolygon(float* cpx, float* cpy, int cpcount);
    void GrowBuf();  // for use of above two methods
    void GrowActualBuf();
    int Bresenham(int*& xpts, int*& ypts);
    void Extent(float& xmin, float& xmax, float& ymin, float& ymax);
protected:
    boolean CanApproxWithLine(
	double x0, double y0, double x2, double y2, double x3, double y3
    );
    void AddLine(double x0, double y0, double x1, double y1);
    void AddBezierArc(
        double x0, double y0, double x1, double y1,
        double x2, double y2, double x3, double y3
    );
    void CalcSection(
	float cminus1x, float cminus1y, float cx, float cy,
	float cplus1x, float cplus1y, float cplus2x, float cplus2y
    );
public:
    float* _x, *_y;
    int _count;
    int _size;
    UList* _ulist;

    float* x() {return _x;}
    float* y() {return _y;}
    int count() const {return _count;}
    int size() const {return _size;}

    virtual boolean operator == (FMultiLineObj&);
    virtual boolean operator != (FMultiLineObj&);

    static FMultiLineObj* make_pts(const float* x, const float*y, int npts);

    static void CompactPoints(boolean flag) {_pts_by_n_enabled=flag;}
protected:
    static UList** _pts_by_n;
    static int _pts_by_n_size;
    static boolean _pts_by_n_enabled;
    int* _xpts;
    int* _ypts;
    int _npts;
    boolean _minmax;
    float _xmin;
    float _xmax;
    float _ymin;
    float _ymax;
};

class FFillPolygonObj : public FMultiLineObj {
public:
    FFillPolygonObj(float* = nil, float* = nil, int = 0);
    virtual ~FFillPolygonObj();

    boolean Contains(FPointObj&);
    boolean Intersects(FLineObj&);
    boolean Intersects(FBoxObj&);
    int Bresenham(int*& xpts, int*& ypts);
    int SortedBorders(int*& ylocs, int*& xbegs, int*& xends, boolean*& xings);
    double PolygonArea();
protected:
    void Normalize();
protected:
    float* _normx, *_normy;
    int _normCount;

    // for SortedBorders
    int _runcnt;      // number of sorted segments of horizontal perimeter
    int* _ylocs;      // y of segment
    int* _xbegs;      // beginning x of segment
    int* _xends;      // ending x of segment
    boolean* _xings;  // true if previous segment on different row from next
};

#endif
