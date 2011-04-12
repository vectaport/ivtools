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

#ifndef topo_edge_h
#define topo_edge_h

#include <TopoFace/topoelt.h>

class FMultiLineObj;
class FPointObj;
class TopoFace;
class TopoNode;

class TopoEdge : public TopoElement {
public:
    TopoEdge(void* value = nil);
    ~TopoEdge();
    
    void attach_nodes(TopoNode* start, TopoNode* end);
    void remove_nodes();

    void attach_faces(TopoFace* left, TopoFace* right);

    void attach_start_node(TopoNode* start);
    void attach_end_node(TopoNode* end);

    TopoNode* start_node() const;
    TopoNode* end_node() const;

    TopoFace* left_face() const;
    TopoFace* right_face() const;

    boolean starts_at(TopoNode*) const;
    boolean ends_at(TopoNode*) const;

    FMultiLineObj* multiline();
    FPointObj* point();
protected:
    TopoNode* _start;
    TopoNode* _end;
    
    TopoFace* _left;
    TopoFace* _right;
};

#endif
