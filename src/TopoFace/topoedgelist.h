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

#ifndef topo_edgelist_h
#define topo_edgelist_h

#include <TopoFace/topoelt.h>
#include <Unidraw/_defines.h>
#include <Unidraw/iterator.h>

class Iterator;
class TopoEdge;
class UList;

class TopoEdgeList : public TopoElement {
public:
    TopoEdgeList(void* value = nil);
    virtual ~TopoEdgeList();

    void append(TopoEdge*);
    void prepend(TopoEdge*);
    void insert_after(Iterator, TopoEdge*);
    void insert_before(Iterator, TopoEdge*);
    void remove(TopoEdge*);
    void remove(Iterator&);

    TopoEdge* edge(UList*) const;
    UList* elem(Iterator) const;
    TopoEdge* get_edge(Iterator) const;
    boolean includes(TopoEdge*) const;

    void first(Iterator&) const;
    void last(Iterator&) const;
    void next(Iterator&) const;
    void prev(Iterator&) const;
    boolean done(Iterator) const;
    boolean is_empty() const;
    int number() const;

protected:
    UList* _ulist;
    int _count;
};

#endif
