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
#include <TopoFace/toponode.h>

/****************************************************************************/

TopoNode::TopoNode(void* value) : TopoEdgeList(value) {
}

TopoEdge* TopoNode::next_edge(const TopoEdge* prev_edge, const TopoFace* face) const {
    Iterator i;
    first(i);
    while (!done(i)) {
	TopoEdge* e = edge(elem(i));
	if (e != prev_edge && (e->right_face() == face || e->left_face() == face))
	    return e;
	next(i);
    }
    return nil;
}

FPointObj* TopoNode::point_obj() {
  const float* xp = xpoints();
  const float* yp = ypoints();
  FPointObj* point = new FPointObj(xp[0], yp[0]);
  return point;
}
