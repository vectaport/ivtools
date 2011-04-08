/*
 * Copyright (c) 1994,1998 Vectaport, Inc.
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

/****************************************************************************/

TopoEdge::TopoEdge(void* value) : TopoElement(value) {
    _start = _end = nil;
    _left = _right = nil;
}

TopoEdge::~TopoEdge() {
}

void TopoEdge::attach_nodes(TopoNode* start, TopoNode* end) {
    if (_start) 
	_start->remove(this);
    if (_end) 
	_end->remove(this);
    if (start) 
	start->append(this);
    if (end)
	end->append(this);
    _start = start;
    _end = end;
}

void TopoEdge::attach_start_node(TopoNode* start) {
    if (_start) 
	_start->remove(this);
    if (start) 
	start->append(this);
    _start = start;
}

void TopoEdge::attach_end_node(TopoNode* end) {
    if (_end) 
	_end->remove(this);
    if (end)
	end->append(this);
    _end = end;
}

void TopoEdge::attach_faces(TopoFace* left, TopoFace* right) {
    if (_left) 
	_left->remove(this);
    if (_right) 
	_right->remove(this);
    if (left) 
	left->append(this);
    if (right)
	right->append(this);
    _left = left;
    _right = right;
}

TopoNode* TopoEdge::start_node() const { return _start; }
TopoNode* TopoEdge::end_node() const { return _end; }

TopoFace* TopoEdge::left_face() const { return _left; }
TopoFace* TopoEdge::right_face() const { return _right; }

boolean TopoEdge::starts_at(TopoNode* node) const { return node==_start; }
boolean TopoEdge::ends_at(TopoNode* node) const { return node==_end; }

FMultiLineObj* TopoEdge::multiline() {
  int npt = npts();
  float* xp = (float*)xpoints();
  float* yp = (float*)ypoints();
  FMultiLineObj* mline = new FMultiLineObj(xp, yp, npt);
  return mline;
}

FPointObj* TopoEdge::point() {
  int npt = npts();
  const float* xp = xpoints();
  const float* yp = ypoints();
  FPointObj* pt = new FPointObj(xp[0], yp[0]);
  return pt;
}
