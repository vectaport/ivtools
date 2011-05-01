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

#include <TopoFace/fgeomobjs.h>
#include <TopoFace/topoedge.h>
#include <TopoFace/topoface.h>
#include <TopoFace/toponode.h>

#include <Unidraw/ulist.h>

/****************************************************************************/

TopoFace::TopoFace(void* value) : TopoEdgeList(value) {
}

int TopoFace::npts() const { 
  if (_npts < 0  && !is_empty()) {
    Iterator i;
    int counter = 0;
    first(i);
    TopoEdge* first_edge = edge(elem(i));
    TopoEdge* curr_edge = first_edge;
    int numedges = number();
    int edgei = 0;
    do {
      int edge_npts = curr_edge->npts();
      counter += edge_npts;
      const TopoNode* next_node = clockwise(curr_edge) ?
	curr_edge->end_node() : curr_edge->start_node();
      curr_edge = next_node->next_edge(curr_edge, this);
      edgei++;
    } while (curr_edge && curr_edge != first_edge && edgei < numedges);
    ((TopoFace*)this)->_npts = counter;
  }
  return _npts; 
}

const float *TopoFace::xpoints() const { 
    if (!_x && !is_empty()) ((TopoFace*)this)->load_points();
    return _x; 
}

const float *TopoFace::ypoints() const { 
    if (!_y && !_x && is_empty()) ((TopoFace*)this)->load_points();
    return _y; 
}
const float *TopoFace::zpoints() const { 
    if (!_z && !_y && !_x && !is_empty()) ((TopoFace*)this)->load_points();
    return _z; 
}


void TopoFace::load_points() { 
    Iterator i;
    float *x = new float[npts()];
    float *y = new float[npts()];
    int counter = 0;
    first(i);
    TopoEdge* first_edge = edge(elem(i));
    TopoEdge* curr_edge = first_edge;
    int numedges = number();
    int edgei = 0;
    do {
	const float *xptr = curr_edge->xpoints();
	const float *yptr = curr_edge->ypoints();
	int edge_npts = curr_edge->npts();
	if (clockwise(curr_edge)) {
	    for (int j=0; j<edge_npts; j++) {
		x[counter] = xptr[j];
		y[counter] = yptr[j];
		counter++;
	    }
	} else {
	    for (int j=edge_npts-1; j>=0; j--) {
		x[counter] = xptr[j];
		y[counter] = yptr[j];
		counter++;
	    }
	}
	const TopoNode* next_node = clockwise(curr_edge) ? curr_edge->end_node() : curr_edge->start_node();
	curr_edge = next_node->next_edge(curr_edge, this);
	edgei++;
    } while (curr_edge && curr_edge != first_edge && edgei < numedges);

    insert_pointers(npts(), x, y, nil, true);
}

boolean TopoFace::clockwise(TopoEdge* edge) const {
    return edge->right_face() == this;
}

double TopoFace::area() {
   int i,j;
   double area = 0;
   int N = npts();
   const float* x = xpoints();
   const float* y = ypoints();

   for (i=0;i<N;i++) {
      j = (i + 1) % N;
      area += x[i] * y[j];
      area -= y[i] * x[j];
   }

   area /= 2;
   return(area < 0 ? -area : area);
}

FFillPolygonObj* TopoFace::polygon() {
  int npt = npts();
  float* xp = (float*)xpoints();
  float* yp = (float*)ypoints();
  FFillPolygonObj* poly = new FFillPolygonObj(xp, yp, npt);
  return poly;
}
