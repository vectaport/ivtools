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
 * Implementation of geometrical objects with floating point coordinates.
 */

#include <TopoFace/fgeomobjs.h>
#include <Unidraw/Graphic/util.h>
#include <Unidraw/ulist.h>

#include <OS/memory.h>
#include <OS/math.h>

#include <math.h>
#include <iostream.h>

/*****************************************************************************/

static const int NUMPOINTS = 200;	// must be > 1
static const double SMOOTHNESS = 1.0;
static int mlsize = 0;
static int mlcount = 0;
static float* mlx, *mly;

/*****************************************************************************/

FPointObj::FPointObj (float x, float y) { _x = x; _y = y; }
FPointObj::FPointObj (FPointObj* p) { _x = p->_x; _y = p->_y; }

float FPointObj::Distance (FPointObj& p) {
    return sqrt(float(square(_x - p._x) + square(_y - p._y)));
}

/*****************************************************************************/

FLineObj::FLineObj (float x0, float y0, float x1, float y1) {
    _p1._x = x0; _p1._y = y0; _p2._x = x1; _p2._y = y1;
    _xpts = _ypts = nil;
}

FLineObj::FLineObj (FLineObj* l) {
    _p1._x = l->_p1._x; _p1._y = l->_p1._y; 
    _p2._x = l->_p2._x; _p2._y = l->_p2._y;
    _xpts = _ypts = nil;
}

FLineObj::~FLineObj() {
  delete _xpts;
  delete _ypts;
}

boolean FLineObj::Contains (FPointObj& p) {
    return
	(p._x >= min(_p1._x, _p2._x)) && (p._x <= max(_p1._x, _p2._x)) &&
	(p._y >= min(_p1._y, _p2._y)) && (p._y <= max(_p1._y, _p2._y)) && (
            (p._y - _p1._y)*(_p2._x - _p1._x) - 
            (_p2._y - _p1._y)*(p._x - _p1._x)
        ) == 0;
}

inline int signum (float a) {
    if (a < 0.0) {
        return -1;
    } else if (a > 0.0) {
        return 1;
    } else {
        return 0;
    }
}

int FLineObj::Same (FPointObj& p1, FPointObj& p2) {
    float dx, dx1, dx2;
    float dy, dy1, dy2;
    
    dx = _p2._x - _p1._x;
    dy = _p2._y - _p1._y;
    dx1 = p1._x - _p1._x;
    dy1 = p1._y - _p1._y;
    dx2 = p2._x - _p2._x;
    dy2 = p2._y - _p2._y;

    return signum(dx*dy1 - dy*dx1) * signum(dx*dy2 - dy*dx2);
}

boolean FLineObj::Intersects (FLineObj& l) {  // from Sedgewick, p. 313
    FBoxObj b1 (_p1._x, _p1._y, _p2._x, _p2._y);
    FBoxObj b2 (l._p1._x, l._p1._y, l._p2._x, l._p2._y);
    
    return
        b1.Intersects(b2) && Same(l._p1, l._p2) <= 0 && l.Same(_p1, _p2) <= 0;
}

boolean FLineObj::EquationIntersects (FLineObj& l, float& x, float& y) {
  float m1 = (l._p2._y-l._p1._y)/(l._p2._x-l._p1._x);
  float m2 = (_p2._y-_p1._y)/(_p2._x-_p1._x);
  float b1 = l._p1._y-m1*l._p1._x;
  float b2 = _p1._y-m2*_p1._x;
  if (m1==m2) return false;
  x = (b2-b1)/(m1-m2);
  y = m1*x+b1;
  return true;
}

int FLineObj::Bresenham(int*& xpts, int*& ypts) {
  if (_xpts) {
    xpts = _xpts;
    ypts = _ypts;
  } else {
    int x1 = Math::round(_p1._x);
    int y1 = Math::round(_p1._y);
    int x2 = Math::round(_p2._x);
    int y2 = Math::round(_p2._y);
    int dx = Math::abs(x1-x2);
    int dy = Math::abs(y1-y2);
    int dirx = x1<x2 ? 1 : -1;
    int diry = y1<y2 ? 1 : -1;
    _npts = Math::max(dx, dy)+1;
    _xpts = new int[_npts];
    _ypts = new int[_npts];
    int count = -(_npts-1)/2;  /* place to set a phase */
    int curx = x1;
    int cury = y1;
    for (int i = 0; i< _npts; i++) {
      _xpts[i] = curx;
      _ypts[i] = cury;
      if (dx>dy) {
	curx += dirx;
	count += dy;
	if (count>0) {
	  count -= dx;
	  cury += diry;
	}
      } else {
	cury += diry;
	count += dx;
	if (count>0) {
	  count -= dy;
	  curx += dirx;
	}
      }
    }
  }
  return _npts;
}

/*****************************************************************************/

FBoxObj::FBoxObj (float x0, float y0, float x1, float y1) {
    _left = min(x0, x1); _bottom = min(y0, y1); 
    _right = max(x0, x1); _top = max(y0, y1);
}

FBoxObj::FBoxObj (FBoxObj* b) {
    _left = b->_left; _bottom = b->_bottom; _right = b->_right; _top = b->_top;
}

boolean FBoxObj::Contains (FPointObj& p) {
    return
        (p._x >= _left) && (p._x <= _right) &&
        (p._y >= _bottom) && (p._y <= _top);
}

boolean FBoxObj::Intersects (FBoxObj& b) {
    return (
        (_left <= b._right) && (b._left <= _right) && 
	(_bottom <= b._top) && (b._bottom <= _top) 
    );
}

boolean FBoxObj::Intersects (FLineObj& l) {
    float x1 = min(l._p1._x, l._p2._x);
    float x2 = max(l._p1._x, l._p2._x);
    float y1 = min(l._p1._y, l._p2._y);
    float y2 = max(l._p1._y, l._p2._y);
    FBoxObj lbox(x1, y1, x2, y2);
    boolean intersects = false;

    if (Intersects(lbox)) {
	intersects = Contains(l._p1) || Contains(l._p2);
        if (!intersects) {
            FLineObj l0 (_left, _bottom, _right, _bottom);
            FLineObj l1 (_right, _bottom, _right, _top);
            FLineObj l2 (_right, _top, _left, _top);
            FLineObj l3 (_left, _top, _left, _bottom);
            intersects =
	        l.Intersects(l0) || l.Intersects(l1) || 
	        l.Intersects(l2) || l.Intersects(l3);
	}
    }
    return intersects;
}

FBoxObj FBoxObj::operator- (FBoxObj& b) {
    FBoxObj i;

    if (Intersects(b)) {
        i._left = max(_left, b._left);
	i._bottom = max(_bottom, b._bottom);
	i._right = min(_right, b._right);
	i._top = min(_top, b._top);
    }
    return i;
}

FBoxObj FBoxObj::operator+ (FBoxObj& b) {
    FBoxObj m;
    
    m._left = min(_left, b._left);
    m._bottom = min(_bottom, b._bottom);
    m._right = max(_right, b._right);
    m._top = max(_top, b._top);
    return m;
}

boolean FBoxObj::Within (FBoxObj& b) {
    return (
        (_left >= b._left) && (_bottom >= b._bottom) && 
        (_right <= b._right) && (_top <= b._top) 
    );
}

/*****************************************************************************/

UList** FMultiLineObj::_pts_by_n = nil;
int FMultiLineObj::_pts_by_n_size = 1024;
boolean FMultiLineObj::_pts_by_n_enabled = false;

FMultiLineObj::FMultiLineObj (float* x, float* y, int count) {
  _x = x; _y = y; _count = count; _size = count;
  _ulist = nil;
  _xpts = _ypts = nil;  
  _minmax = false;
}

FMultiLineObj::~FMultiLineObj() {
  if (_ulist) {
    UList* head = _pts_by_n[count()];
    head->Remove(_ulist);
    delete _ulist;
    delete _x;
    delete _y;
  }
  delete _xpts;
  delete _ypts;
}

void FMultiLineObj::GrowBuf () {
    float* newx, *newy;
    int newsize;

    if (mlsize == 0) {
        mlsize = NUMPOINTS;
	mlx = new float[NUMPOINTS];
	mly = new float[NUMPOINTS];
    } else {
	newsize = mlsize * 2;
	newx = new float[newsize];
	newy = new float[newsize];
	Memory::copy(mlx, newx, newsize * sizeof(float));
	Memory::copy(mly, newy, newsize * sizeof(float));
	delete mlx;
	delete mly;
	mlx = newx;
	mly = newy;
	mlsize = newsize;
    }
}

void FMultiLineObj::GrowActualBuf () {
    float* newx, *newy;
    int newsize;

    if (_size == 0) {
        _size = NUMPOINTS;
	_x = new float[NUMPOINTS];
	_y = new float[NUMPOINTS];
    } else {
	newsize = _size * 2;
	newx = new float[newsize];
	newy = new float[newsize];
	Memory::copy(_x, newx, newsize * sizeof(float));
	Memory::copy(_y, newy, newsize * sizeof(float));
	delete _x;
	delete _y;
	_x = newx;
	_y = newy;
	_size = newsize;
    }
}

boolean FMultiLineObj::CanApproxWithLine (
    double x0, double y0, double x2, double y2, double x3, double y3
) {
    double triangleArea, sideSquared, dx, dy;
    
    triangleArea = x0*y2 - x2*y0 + x2*y3 - x3*y2 + x3*y0 - x0*y3;
    triangleArea *= triangleArea;	// actually 4 times the area
    dx = x3 - x0;
    dy = y3 - y0;
    sideSquared = dx*dx + dy*dy;
    return triangleArea <= SMOOTHNESS * sideSquared;
}

void FMultiLineObj::AddLine (double x0, double y0, double x1, double y1) {
    if (mlcount >= mlsize) {
	GrowBuf();
    } 
    if (mlcount == 0) {
	mlx[mlcount] = x0;
	mly[mlcount] = y0;
	++mlcount;
    }
    mlx[mlcount] = x1;
    mly[mlcount] = y1;
    ++mlcount;
    if (_minmax) {
	if (x0<_xmin) _xmin = x0;
	if (x0>_xmax) _xmax = x0;
	if (y0<_ymin) _ymin = y0;
	if (y0>_ymax) _ymax = y0;
	if (x1<_xmin) _xmin = x1;
	if (x1>_xmax) _xmax = x1;
	if (y1<_ymin) _ymin = y1;
	if (y1>_ymax) _ymax = y1;
    }
}

void FMultiLineObj::AddBezierArc (
     double x0, double y0, double x1, double y1,
     double x2, double y2, double x3, double y3
) {
    double midx01, midx12, midx23, midlsegx, midrsegx, cx,
    	   midy01, midy12, midy23, midlsegy, midrsegy, cy;
    
    Midpoint(x0, y0, x1, y1, midx01, midy01);
    Midpoint(x1, y1, x2, y2, midx12, midy12);
    Midpoint(x2, y2, x3, y3, midx23, midy23);
    Midpoint(midx01, midy01, midx12, midy12, midlsegx, midlsegy);
    Midpoint(midx12, midy12, midx23, midy23, midrsegx, midrsegy);
    Midpoint(midlsegx, midlsegy, midrsegx, midrsegy, cx, cy);    

    if (CanApproxWithLine(x0, y0, midlsegx, midlsegy, cx, cy)) {
        AddLine(x0, y0, cx, cy);
    } else if (
        (midx01 != x1) || (midy01 != y1) || (midlsegx != x2) ||
	(midlsegy != y2) || (cx != x3) || (cy != y3)
    ) {    
        AddBezierArc(x0, y0, midx01, midy01, midlsegx, midlsegy, cx, cy);
    }

    if (CanApproxWithLine(cx, cy, midx23, midy23, x3, y3)) {
        AddLine(cx, cy, x3, y3);
    } else if (
        (cx != x0) || (cy != y0) || (midrsegx != x1) || (midrsegy != y1) ||
	(midx23 != x2) || (midy23 != y2)
    ) {        
        AddBezierArc(cx, cy, midrsegx, midrsegy, midx23, midy23, x3, y3);
    }
}

void FMultiLineObj::CalcSection (
    float cminus1x, float cminus1y, float cx, float cy,
    float cplus1x, float cplus1y, float cplus2x, float cplus2y
) {
    double p0x, p1x, p2x, p3x, tempx,
	   p0y, p1y, p2y, p3y, tempy;
    
    ThirdPoint(
        double(cx), double(cy), double(cplus1x), double(cplus1y), p1x, p1y
    );
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cx), double(cy), p2x, p2y
    );
    ThirdPoint(
        double(cx), double(cy), double(cminus1x), double(cminus1y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p1x, p1y, p0x, p0y);
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cplus2x), double(cplus2y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p2x, p2y, p3x, p3y);
    AddBezierArc(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
}

void FMultiLineObj::SplineToMultiLine (float* cpx, float* cpy, int cpcount) {
    register int cpi;

    if (cpcount < 3) {
        _x = cpx;
	_y = cpy;
	_count = cpcount;
    } else {
        mlcount = 0;

        CalcSection(
            cpx[0], cpy[0], cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1]
        );
        CalcSection(
            cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1], cpx[2], cpy[2]
        );

        for (cpi = 1; cpi < cpcount - 2; ++cpi) {
            CalcSection(
                cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
                cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
            );
        }

        CalcSection(
            cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
            cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
        );
        CalcSection(
            cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
            cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
        );
        _x = mlx;
        _y = mly;
        _count = mlcount;
    }
}

void FMultiLineObj::ClosedSplineToPolygon (float* cpx, float* cpy, int cpcount){
    register int cpi;

    if (cpcount < 3) {
        _x = cpx;
	_y = cpy;
	_count = cpcount;
    } else {
        mlcount = 0;
        CalcSection(
	    cpx[cpcount - 1], cpy[cpcount - 1], cpx[0], cpy[0], 
	    cpx[1], cpy[1], cpx[2], cpy[2]
        );

        for (cpi = 1; cpi < cpcount - 2; ++cpi) {
	    CalcSection(
	        cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	        cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
            );
        }

        CalcSection(
	    cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	    cpx[cpi + 1], cpy[cpi + 1], cpx[0], cpy[0]
        );
        CalcSection(
	    cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
	    cpx[0], cpy[0], cpx[1], cpy[1]
        );
        _x = mlx;
        _y = mly;
        _count = mlcount;
    }
}

void FMultiLineObj::GetBox (FBoxObj& b) {
    b._left = b._right = _x[0];
    b._bottom = b._top = _y[0];

    for (int i = 1; i < _count; ++i) {
	b._left = min(b._left, _x[i]);
	b._bottom = min(b._bottom, _y[i]);
	b._right = max(b._right, _x[i]);
	b._top = max(b._top, _y[i]);
    }
}


boolean FMultiLineObj::Contains (FPointObj& p) {
    register int i;
    FBoxObj b;
    
    GetBox(b);
    if (b.Contains(p)) {
	for (i = 1; i < _count; ++i) {
	    FLineObj l (_x[i-1], _y[i-1], _x[i], _y[i]);
	    if (l.Contains(p)) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean FMultiLineObj::Intersects (FLineObj& l) {
    register int i;
    FBoxObj b;
    
    GetBox(b);
    if (b.Intersects(l)) {
	for (i = 1; i < _count; ++i) {
            FLineObj test(_x[i-1], _y[i-1], _x[i], _y[i]);

	    if (l.Intersects(test)) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean FMultiLineObj::Intersects (FBoxObj& userb) {
    register int i;
    FBoxObj b;
    
    GetBox(b);
    if (b.Intersects(userb)) {
	for (i = 1; i < _count; ++i) {
            FLineObj test(_x[i-1], _y[i-1], _x[i], _y[i]);

	    if (userb.Intersects(test)) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean FMultiLineObj::Within (FBoxObj& userb) {
    FBoxObj b;
    
    GetBox(b);
    return b.Within(userb);
}

inline void FArrayDup (
    const float* x, const float* y, int n, float*& newx, float*& newy
) {
    newx = new float[n];
    newy = new float[n];
    Memory::copy(x, newx, n * sizeof(float));
    Memory::copy(y, newy, n * sizeof(float));
}

FMultiLineObj* FMultiLineObj::make_pts(const float* x, const float* y, int npts) {
    if (!_pts_by_n_enabled) {
        float *copyx, *copyy;
	FArrayDup(x, y, npts, copyx, copyy);
	FMultiLineObj* fmlo = new FMultiLineObj(copyx, copyy, npts);
	return fmlo;
    }

    if (!_pts_by_n) {
	_pts_by_n = new UList*[_pts_by_n_size];
	for (int i=0; i<_pts_by_n_size; i++) 
	    _pts_by_n[i] = nil;
    }
    if (npts>=_pts_by_n_size) {
	int new_size = max(_pts_by_n_size*2, npts+1);
	UList** new_pts_by_n = new UList*[new_size];
	int i = 0;
	for (;i<_pts_by_n_size; i++) 
	    new_pts_by_n[i] = _pts_by_n[i];
	for (;i<new_size; i++) 
	    new_pts_by_n[i] = nil;
	delete _pts_by_n;
	_pts_by_n = new_pts_by_n;
	_pts_by_n_size = new_size;
    }

    if (_pts_by_n[npts]) {

	FMultiLineObj temp_mlo((float*)x, (float*)y, npts);
	UList* ptr = _pts_by_n[npts]->First();
	while (ptr != _pts_by_n[npts]->End()) {
	    if (*(FMultiLineObj*)(*ptr)() == temp_mlo) 
		return (FMultiLineObj*)(*ptr)();
	    ptr = ptr->Next();
	}
    } else 
	_pts_by_n[npts] = new UList();
    
    float *copyx, *copyy;
    FArrayDup(x, y, npts, copyx, copyy);
    FMultiLineObj* mlo = new FMultiLineObj(copyx, copyy, npts);
    _pts_by_n[npts]->Append(mlo->_ulist = new UList(mlo));
    return mlo;
}

boolean FMultiLineObj::operator == (FMultiLineObj& ml) {
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

boolean FMultiLineObj::operator != (FMultiLineObj& ml) { return !(*this == ml); }

int FMultiLineObj::Bresenham(int*& xpts, int*& ypts) {
  if (_xpts) {
    xpts = _xpts;
    ypts = _ypts;
  } else {
    FLineObj* edges[_count-1];
    int total = 0;
    int *temp_xpts, *temp_ypts;
    for (int i=0; i<_count-1; i++) {
      edges[i] = new FLineObj(_x[i], _y[i], _x[i+1], _y[i+1]);
      total += edges[i]->Bresenham(temp_xpts, temp_ypts) - 1;
    }
    total++;
    _xpts = new int[total];
    _ypts = new int[total];
    xpts = _xpts;
    ypts = _ypts;
    int curpt = 0;
    int npts;
    for (int k=0; k<_count-1; k++) {
      npts = edges[k]->Bresenham(temp_xpts, temp_ypts);
      for (int j=0; j<npts-1; j++) {
	_xpts[curpt] = temp_xpts[j];
	_ypts[curpt] = temp_ypts[j];
	curpt++;
      }
    }
    _xpts[curpt] = temp_xpts[npts-1];
    _ypts[curpt] = temp_ypts[npts-1];
    _npts = curpt+1;
    for (int j=0; j<_count-1; j++) 
      delete edges[j];
  }
  return _npts;
}

void FMultiLineObj::Extent(float& xmin, float& xmax, float& ymin, float& ymax) {
  if (!_minmax) {
    if (_count) {
      _minmax = true;
      _xmin = _xmax = _x[0];
      _ymin = _ymax = _y[0];
      for (int i=1; i<_count; i++) {
	if (_x[i]<_xmin) _xmin = _x[i];
	if (_x[i]>_xmax) _xmax = _x[i];
	if (_y[i]<_ymin) _ymin = _y[i];
	if (_y[i]>_ymax) _ymax = _y[i];
      }
    }
  }
  xmin = _xmin;
  xmax = _xmax;
  ymin = _ymin;
  ymax = _ymax;
}

/*****************************************************************************/

FFillPolygonObj::FFillPolygonObj (
    float* x, float* y, int n
) : FMultiLineObj(x, y, n) {
    _normCount = 0;
    _normx = _normy = nil;
    _runcnt = 0;
    _ylocs = _xbegs = _xends = nil;
    _xings = nil;
}

FFillPolygonObj::~FFillPolygonObj () {
    delete _normx;
    delete _normy;
    delete _ylocs;
    delete _xbegs;
    delete _xends;
    delete _xings;
}

static int LowestLeft (float* x, float* y, int count) {
    register int i;
    int lowestLeft = 0;
    float lx = *x;
    float ly = *y;

    for (i = 1; i < count; ++i) {
        if (y[i] < ly || (y[i] == ly && x[i] < lx)) {
	    lowestLeft = i;
	    lx = x[i];
	    ly = y[i];
	}
    }
    return lowestLeft;
}

void FFillPolygonObj::Normalize () {
    if (_count != 0) {
        register int i, newcount = 1;
        int lowestLeft, limit = _count;

	if (*_x == _x[_count - 1] && *_y == _y[_count - 1]) {
	    --limit;
	}
	lowestLeft = LowestLeft(_x, _y, limit);
	_normCount = limit + 2;
	_normx = new float[_normCount];
	_normy = new float[_normCount];

	for (i = lowestLeft; i < limit; ++i, ++newcount) {
	    _normx[newcount] = _x[i];
	    _normy[newcount] = _y[i];
	}
	for (i = 0; i < lowestLeft; ++i, ++newcount) {
	    _normx[newcount] = _x[i];
	    _normy[newcount] = _y[i];
	}

	_normx[newcount] = _normx[1];
	_normy[newcount] = _normy[1];
	--newcount;
	_normx[0] = _normx[newcount];
	_normy[0] = _normy[newcount];
    }
}

boolean FFillPolygonObj::Contains (FPointObj& p) { // derived from A. Glassner,
  if (_normCount == 0) {                         // "An Introduction to
      Normalize();                               // Ray Tracing", p. 53,
  }                                              // courtesy R. Cooperman

  int count = 0;
  FPointObj p0(0.0, 0.0);
  boolean cur_y_sign = _normy[0] >= p._y;

  for (int i = 0; i < _normCount - 2; ++i) {
      FLineObj l (
          _normx[i] - p._x, _normy[i] - p._y,
          _normx[i+1] - p._x, _normy[i+1] - p._y
      );

      if (l.Contains(p0)) {
          return true;
      }

      boolean next_y_sign = l._p2._y >= 0.0;

      if (next_y_sign != cur_y_sign) {
          boolean cur_x_sign = l._p1._x >= 0.0;
          boolean next_x_sign = l._p2._x >= 0.0;

          if (cur_x_sign && next_x_sign) {
              ++count;

          } else if (cur_x_sign || next_x_sign) {
              float dx = l._p2._x - l._p1._x;
              float dy = l._p2._y - l._p1._y;

              if (dy >= 0.0) {
                  if (l._p1._x * dy > l._p1._y * dx) {
                      ++count;
                  }
              } else {
                  if (l._p1._x * dy < l._p1._y * dx) {
                      ++count;
                  }
              }
          }
      }
      cur_y_sign = next_y_sign;
  }
  return count % 2 == 1;
}

boolean FFillPolygonObj::Intersects (FLineObj& l) {
    FBoxObj b;
    boolean intersects = false;
    
    if (_normCount == 0) {
        Normalize();
    }

    GetBox(b);

    if (b.Intersects(l)) {
        FMultiLineObj ml (_normx, _normy, _normCount - 1);
	intersects = ml.Intersects(l) || Contains(l._p1) || Contains(l._p2);
    }

    return intersects;
}

boolean FFillPolygonObj::Intersects (FBoxObj& ub) {
    FBoxObj b;
    
    GetBox(b);
    if (!b.Intersects(ub)) {
	return false;
    }
    if (b.Within(ub)) {
	return true;
    }
    FLineObj bottom(ub._left, ub._bottom, ub._right, ub._bottom);

    if (Intersects(bottom)) {
	return true;
    }

    FLineObj right(ub._right, ub._bottom, ub._right, ub._top);

    if (Intersects(right)) {
	return true;
    }

    FLineObj top(ub._right, ub._top, ub._left, ub._top);

    if (Intersects(top)) {
	return true;
    }

    FLineObj left(ub._left, ub._top, ub._left, ub._bottom);

    return Intersects(left);
}

int FFillPolygonObj::Bresenham(int*& xpts, int*& ypts) {
  if (_xpts) {
    xpts = _xpts;
    ypts = _ypts;
  } else {
    FLineObj* edges[_count];
    int total = 0;
    int *temp_xpts, *temp_ypts;
    for (int i=0; i<_count-1; i++) {
      edges[i] = new FLineObj(_x[i], _y[i], _x[i+1], _y[i+1]);
      total += edges[i]->Bresenham(temp_xpts, temp_ypts) - 1;
    }
    edges[_count-1] = new FLineObj(_x[_count-1], _y[_count-1], _x[0], _y[0]);
    total += edges[_count-1]->Bresenham(temp_xpts, temp_ypts);
    _xpts = new int[total];
    _ypts = new int[total];
    xpts = _xpts;
    ypts = _ypts;
    int curpt = 0;
    int npts;
    for (int k=0; k<_count; k++) {
      npts = edges[k]->Bresenham(temp_xpts, temp_ypts);
      for (int j=0; j<npts-1; j++) {
	_xpts[curpt] = temp_xpts[j];
	_ypts[curpt] = temp_ypts[j];
	curpt++;
      }
    }
    _npts = curpt;
    for (int j=0; j<_count; j++) 
      delete edges[j];
  }
  return _npts;
}

int FFillPolygonObj::SortedBorders
(int*& ylocs, int*& xbegs, int*& xends, boolean*& xings) {

  if (_ylocs) {
    ylocs = _ylocs;
    xbegs = _xbegs;
    xends = _xends;
    xings = _xings;
    return _runcnt;
  }

  /* walk the perimeter a pixel at a time */
  int *xpts, *ypts;
  int npts = Bresenham(xpts, ypts);
  if (npts<=1) return 0;

  /* start up list for sorting contiguous horizontal runs by row */
  UList* list = new UList();

  /* find the start of the first run */
  int begin = 0;
  if (ypts[0] == ypts[npts-1]) {
    while (begin<npts-2 && ypts[begin]==ypts[begin+1]) begin++;
    begin++;
    if (begin==npts-1) return 0;
  }

  /* loop through all runs */
  int runbeg = begin;
  enum { yloc, xbeg, xend, xing };
  const int nval = xing + 1;
  int runend;
  _runcnt = 0;
  do {
    runend = runbeg==npts-1 ? 0 : runbeg+1;
    while (ypts[runend]==ypts[runbeg] && runend != begin)
      runend = runend==npts-1 ? 0 : runend+1;

    /* new run found, enter into sorted list */
    int* vals = new int[nval];
    vals[yloc] = ypts[runbeg];
    vals[xbeg] = Math::min(xpts[runbeg], xpts[runend ? runend-1 : npts-1]);
    vals[xend] = Math::max(xpts[runbeg], xpts[runend ? runend-1 : npts-1]);
    vals[xing] = ypts[runbeg ? runbeg-1 : npts-1] != ypts[runend];
    UList* curr = list;
    UList* next = curr->Next();
    while (next != list) {
      int* nextvals = (int *) (*next) ();
      if (vals[yloc] < nextvals[yloc]) break;
      if (vals[yloc] == nextvals[yloc] && vals[xbeg] < nextvals[xbeg]) break;
      curr = next;
      next = curr->Next();
    }
    curr->Prepend(new UList(vals));
    _runcnt++;
    
    runbeg = runend;
  } while (runend != begin);
  /* done looping through all runs */

  /* save into arrays */
  _ylocs = new int[_runcnt];
  _xbegs = new int[_runcnt];
  _xends = new int[_runcnt];
  _xings = new boolean[_runcnt];
  UList* curr = list->Next();
  for (int i=0; i<_runcnt; i++) {
    int* currvals = (int *) (*curr) ();
    _ylocs[i] = currvals[yloc];
    _xbegs[i] = currvals[xbeg];
    _xends[i] = currvals[xend];
    _xings[i] = currvals[xing];
    delete currvals;
    UList* tmp = curr;
    curr = curr->Next();
    curr->Remove(tmp);
    delete tmp;
  }
  delete list;

  ylocs = _ylocs;
  xbegs = _xbegs;
  xends = _xends;
  xings = _xings;
  return _runcnt;
}

double FFillPolygonObj::PolygonArea() {
  int i,j;
  double area = 0;

  for (i=0;i<_npts;i++) {
    j = (i + 1) % _npts;
    area += _x[i] * _y[j];
    area -= _y[i] * _x[j];
  }

  area /= 2;
  return(area < 0 ? -area : area);
}				    

/*****************************************************************************/
#if 0
Extent::Extent (float x0, float y0, float x1, float y1, float t) {
    _left = x0; _bottom = y0; _cx = x1; _cy = y1; _tol = t;
}

Extent::Extent (Extent& e) {
    _left = e._left; _bottom = e._bottom;
    _cx = e._cx; _cy = e._cy; _tol = e._tol;
}

boolean Extent::Within (Extent& e) {
    float l = _left - _tol, b = _bottom - _tol;
    float el = e._left - _tol, eb = e._bottom - _tol;

    return 
	l >= el && b >= eb && 2*_cx - l <= 2*e._cx - el && 
	2*_cy - b <= 2*e._cy - eb;
    }

void Extent::Merge (Extent& e) {
    float nl = min(_left, e._left);
    float nb = min(_bottom, e._bottom);

    if (Undefined()) {
	_left = e._left; _bottom = e._bottom; _cx = e._cx; _cy = e._cy;
    } else if (!e.Undefined()) {
	_cx = (nl + max(2*_cx - _left, 2*e._cx - e._left)) / 2;
	_cy = (nb + max(2*_cy - _bottom, 2*e._cy - e._bottom)) / 2;
	_left = nl;
	_bottom = nb;
    }
    _tol = max(_tol, e._tol);
}
#endif
