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

#ifndef topo_entity_h
#define topo_entity_h

#include <InterViews/resource.h>
#include <OS/types.h>

class TopoElement : public Resource {
public:

    virtual ~TopoElement();
    void* value() const { return _value; }
    void value(void* value) { _value = value; }

    void insert_points(int npts, float *x, float *y, float *z=nil);
    void insert_points(int npts, int *x, int *y, int *z=nil);
    void insert_pointers(int npts, float *x, float *y, float *z=nil, boolean freeflag = false);

    virtual int npts() const;
    virtual const float *xpoints() const;
    virtual const float *ypoints() const;
    virtual const float *zpoints() const;

    float xmin() const { return _xmin; }
    float xmax() const { return _xmax; }
    float ymin() const { return _ymin; }
    float ymax() const { return _ymax; }
    float zmin() const { return _zmin; }
    float zmax() const { return _zmax; }

protected:
    TopoElement(void* value = nil);

    void delete_points();
    void compute_minmax();

protected:
    boolean _alloc;
    float *_x;
    float *_y;
    float *_z;
    int _npts;

    float _xmin;
    float _ymin;
    float _zmin;

    float _xmax;
    float _ymax;
    float _zmax;

    void* _value;
};

#endif
