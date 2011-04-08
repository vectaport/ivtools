/*
 * Copyright (c) 1994 Vectaport, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software without 
 * specific, written prior permission.  The copyright holders make no 
 * representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <TopoFace/topoelt.h>

#if !defined(__CYGWIN__)
#include <values.h>
#else
#include <math.h>
#define MAXFLOAT HUGE_VAL
#endif

/****************************************************************************/

TopoElement::TopoElement(void* value) {
    _npts = -1;
    _x = _y = _z = nil;
    _alloc = false;
    _value = value;
}

TopoElement::~TopoElement() {
    delete_points();
}

void TopoElement::insert_points(int npts, float *x, float *y, float *z) {
    delete_points();
    _alloc = true;
    _npts = npts;
    _x = new float[npts];
    _y = new float[npts];
    if (z) _z = new float[npts];
    for (int i=0; i<npts; i++) {
	_x[i] = x[i];
	_y[i] = y[i];
	if (z) _z[i] = z[i];
    }
    compute_minmax();
}

void TopoElement::insert_points(int npts, int *x, int *y, int *z) {
    delete_points();
    _alloc = true;
    _npts = npts;
    _x = new float[npts];
    _y = new float[npts];
    if (z) _z = new float[npts];
    for (int i=0; i<npts; i++) {
	_x[i] = x[i];
	_y[i] = y[i];
	if (z) _z[i] = z[i];
    }
    compute_minmax();
}

void TopoElement::insert_pointers(int npts, float *x, float *y, float *z, boolean freeflag) {
    delete_points();
    _alloc = freeflag;
    _npts = npts;
    _x = x;
    _y = y;
    _z = z;
    compute_minmax();
}

int TopoElement::npts() const { return _npts; }
const float *TopoElement::xpoints() const { return _x; }
const float *TopoElement::ypoints() const { return _y; }
const float *TopoElement::zpoints() const { return _z; }

void TopoElement::delete_points() {
    if (_alloc) {
	delete _x;
	delete _y;
	delete _z;
    }
    _npts = 0;
}

void TopoElement::compute_minmax() {
    _xmin = _ymin = _zmin = MAXFLOAT;
    _xmax = _ymax = _zmax = -MAXFLOAT;
    for (int i=0; i<_npts; i++) {
	if (_x[i]<_xmin)
	    _xmin = _x[i];
	else if (_x[i]>_xmax)
	    _xmax = _x[i];
	if (_y[i]<_ymin)
	    _ymin = _y[i];
	else if (_y[i]>_ymax)
	    _ymax = _y[i];
	if (_z) {
	    if (_z[i]<_zmin)
		_zmin = _z[i];
	    else if (_z[i]>_zmax)
		_zmax = _z[i];
	}
    }
}
