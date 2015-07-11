/*
 * Copyright (c) 1997 Vectaport Inc.
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
  MultiLine clipping to a polygon routine
  */
#ifdef CLIPPOLY
#include <poly.h>
#include <primitives.h>
#include <nclip.h>
#endif
#include <OverlayUnidraw/ptinpoly.h>
#include <OS/math.h>
#include <math.h>
#include <string.h>

void add_intersect(Point intp, int& size, int& n, Point*& pts) {
    if (n == size) {
        Point* newpts = new Point[n*2];
	for (int j = 0; j < n; j++)
	    newpts[j] = pts[j];
	delete pts;
	pts = newpts;
	size = n*2;
    }
    pts[n++] = intp;
}

Point* sort_by_dist(Point p, int n, Point* pts) {
    int sn = 0;
    Point* spts = new Point[n];
    double* dists = new double[n];
    int* taken = new int[n];
    memset(taken, 0, n*sizeof(int));
    for (int i = 0; i < n; i++)
        dists[i] = ((p.x() - pts[i].x()) * (p.x() - pts[i].x())) +
	    ((p.y() - pts[i].y()) * (p.y() - pts[i].y()));
    for (int i = 0; i < n; i++) {
        double mindist = HUGE_VAL;
        int mini;
	for (int j = 0; j < n; j++) {
	  if (!taken[j] && dists[j] < mindist) {
	    mini = j;
	    mindist = dists[j];
	  }
	}
	spts[i] = pts[mini];
	taken[mini] = 1;
    }
    delete [] taken;
    delete [] dists;
    return spts;
}

void alloc_int_array(int size, int*& array)
{
    array = new int[size];
    memset(array, 0, size*sizeof(int));
}

void alloc_float_array(int size, float*& array)
{
    array = new float[size];
    memset(array, 0, size*sizeof(float));
}

void alloc_float_float_array(int size, float**& array)
{
    array = new float*[size];
    memset(array, 0, size*sizeof(float*));
}

void insert_int_array(int& size, int i, int*& array, int val)
{
    if (i == size) {
        int* newarray;
	alloc_int_array(i*2, newarray);
	for (int j = 0; j < i; j++)
	    newarray[j] = array[j];
	delete array;
	array = newarray;
	size = i*2;
    }
    array[i] = val;
}

void insert_float_array(int& size, int i, float*& array, float val)
{
    if (i == size) {
        float* newarray;
	alloc_float_array(i*2, newarray);
	for (int j = 0; j < i; j++)
	    newarray[j] = array[j];
	delete array;
	array = newarray;
	size = i*2;
    }
    array[i] = val;
}

void cliplinepoly(int ln, float* lx, float* ly, int pn, float* px, float* py,
		  int& in_nlines, int*& in_ni, float**& in_nx, float**& in_ny,
		  int& out_nlines, int*& out_ni, float**& out_nx, float**& out_ny)
{
#ifdef CLIPPOLY
    // set initial in-out state
    int instate = point_in_poly(lx[0], ly[0], pn, px, py);

    // initialization
    in_nlines = out_nlines = 0;
    int initsize = Math::max(ln, pn);
    int in_size = initsize;
    int in_xsize = initsize;
    int in_ysize = initsize;
    int in_ixsize = initsize;
    int in_iysize = initsize;
    int out_size = initsize;
    int out_xsize = initsize;
    int out_ysize = initsize;
    int out_ixsize = initsize;
    int out_iysize = initsize;
    alloc_int_array(in_size, in_ni);
    alloc_float_float_array(in_xsize, in_nx);
    alloc_float_array(in_ixsize, in_nx[0]);
    alloc_float_float_array(in_ysize, in_ny);
    alloc_float_array(in_iysize, in_ny[0]);
    alloc_int_array(out_size, out_ni);
    alloc_float_float_array(out_xsize, out_nx);
    alloc_float_array(out_ixsize, out_nx[0]);
    alloc_float_float_array(out_ysize, out_ny);
    alloc_float_array(out_iysize, out_ny[0]);

    // insert first line point
    if (instate) {
      insert_float_array(in_ixsize, in_ni[in_nlines], in_nx[in_nlines], lx[0]);
      insert_float_array(in_iysize, in_ni[in_nlines], in_ny[in_nlines], ly[0]);
      in_ni[in_nlines] = 1;
    }
    else {
      insert_float_array(out_ixsize, out_ni[out_nlines], out_nx[out_nlines], lx[0]);
      insert_float_array(out_iysize, out_ni[out_nlines], out_ny[out_nlines], ly[0]);
      out_ni[out_nlines] = 1;
    }

    // for each line segment
    for (int li = 1; li < ln; li++) {
        Point lp1(lx[li - 1], ly[li - 1]);
	Point lp2(lx[li], ly[li]);
	Edge ledge(lp1, lp2);

	// build list of intersections with polygon edges
	int intsize = 10;
	int intn = 0;
	Point* intpts = new Point[intsize];
	for (int pi = 1; pi < pn; pi++) {
	    Point pp1(px[pi - 1], py[pi - 1]);
	    Point pp2(px[pi], py[pi]);
	    Edge pedge(pp1, pp2);
	    Point resp1, resp2;
	    int inters = intersect(ledge, pedge, resp1, resp2);
	    switch(inters) {
	    case 0:
	        break;
	    case 1:
	        add_intersect(resp1, intsize, intn, intpts);
	        break;
	    case 2:
	        add_intersect(resp1, intsize, intn, intpts);
		add_intersect(resp2, intsize, intn, intpts);
	        break;
	    }
	}

	if (intn > 0) { // intersected some polygon edges
	    // sort intersect list by distance from lp1
	    Point* sorted_intpts = sort_by_dist(lp1, intn, intpts);

	    // walk through list of intersections, breaking up the segment
	    // as necessary
	    for (int i = 0; i < intn; i++) {
	      Point intpt = sorted_intpts[i];
	      if (instate) {
		// add intersection point to current in-line
	        insert_float_array(in_ixsize, in_ni[in_nlines], in_nx[in_nlines],
				 intpt.x());
	        insert_float_array(in_iysize, in_ni[in_nlines], in_ny[in_nlines],
				 intpt.y());
		in_ni[in_nlines]++;

		// end that line
		in_nlines++;

		// allocate next in-line
		in_ixsize = initsize;
		alloc_float_array(in_ixsize, in_nx[in_nlines]);
		in_iysize = initsize;
		alloc_float_array(in_iysize, in_ny[in_nlines]);

		// switch to outside the polygon
		instate = 0;

		// add intersection point to start a new out-line
		insert_float_array(out_ixsize, out_ni[out_nlines],
				 out_nx[out_nlines],
				 intpt.x());
		insert_float_array(out_iysize, out_ni[out_nlines],
				 out_ny[out_nlines],
				 intpt.y());
		out_ni[out_nlines]++;
	      }
	      else {
		// add intersection point to current out-line
	        insert_float_array(out_ixsize, out_ni[out_nlines],
				 out_nx[out_nlines],
				 intpt.x());
	        insert_float_array(out_iysize, out_ni[out_nlines],
				 out_ny[out_nlines],
				 intpt.y());
		out_ni[out_nlines]++;

		// end that line
		out_nlines++;

		// allocate next out-line
		out_ixsize = initsize;
		alloc_float_array(out_ixsize, out_nx[out_nlines]);
		out_iysize = initsize;
		alloc_float_array(out_iysize, out_ny[out_nlines]);

		// switch to inside the polygon
		instate = 1;

		// add intersection point to start a new in-line
		insert_float_array(in_ixsize, in_ni[in_nlines], in_nx[in_nlines],
				 intpt.x());
		insert_float_array(in_iysize, in_ni[in_nlines], in_ny[in_nlines],
				 intpt.y());
		in_ni[in_nlines]++;
	      }
	    }
	    // add final segment point
	    if (instate) {
	        insert_float_array(in_ixsize, in_ni[in_nlines], in_nx[in_nlines],
				 lp2.x());
	        insert_float_array(in_iysize, in_ni[in_nlines], in_ny[in_nlines],
				 lp2.y());
		in_ni[in_nlines]++;
	    }
	    else {
	        insert_float_array(out_ixsize, out_ni[out_nlines], out_nx[out_nlines],
				 lp2.x());
	        insert_float_array(out_iysize, out_ni[out_nlines], out_ny[out_nlines],
				 lp2.y());
		out_ni[out_nlines]++;
	    }
	}
	else { // no intersects: add segment to current mline, keeping instate
	    if (instate) {
	        insert_float_array(in_ixsize, in_ni[in_nlines], in_nx[in_nlines],
				 lp2.x());
	        insert_float_array(in_iysize, in_ni[in_nlines], in_ny[in_nlines],
				 lp2.y());
		in_ni[in_nlines]++;
	    }
	    else {
	        insert_float_array(out_ixsize, out_ni[out_nlines],
				 out_nx[out_nlines],
				 lp2.x());
	        insert_float_array(out_iysize, out_ni[out_nlines],
				 out_ny[out_nlines],
				 lp2.y());
		out_ni[out_nlines]++;
	    }
	}
    }
    // fix final line count
    if (instate)
      in_nlines++;
    else
      out_nlines++;
#endif
}
