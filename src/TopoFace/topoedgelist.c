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

#include <TopoFace/topoedge.h>
#include <TopoFace/topoedgelist.h>

#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>

/****************************************************************************/

TopoEdgeList::TopoEdgeList(void* value) : TopoElement(value) {
    _ulist = new UList;
    _count = 0;
}

TopoEdgeList::~TopoEdgeList() { delete _ulist; }

void TopoEdgeList::append (TopoEdge* v) {
    _ulist->Append(new UList(v));
    ++_count;
}

void TopoEdgeList::prepend (TopoEdge* v) {
    _ulist->Prepend(new UList(v));
    ++_count;
}

void TopoEdgeList::insert_after (Iterator i, TopoEdge* v) {
    elem(i)->Prepend(new UList(v));
    ++_count;
}

void TopoEdgeList::insert_before (Iterator i, TopoEdge* v) {
    elem(i)->Append(new UList(v));
    ++_count;
}

void TopoEdgeList::remove (Iterator& i) {
    UList* doomed = elem(i);

    next(i);
    _ulist->Remove(doomed);
    delete doomed;
    --_count;
}	
    
void TopoEdgeList::remove (TopoEdge* p) {
    UList* temp;

    if ((temp = _ulist->Find(p)) != nil) {
	_ulist->Remove(temp);
        delete temp;
	--_count;
    }
}

TopoEdge* TopoEdgeList::edge (UList* r) const { return (TopoEdge*) (*r)(); }
UList* TopoEdgeList::elem (Iterator i) const { return (UList*) i.GetValue(); }
TopoEdge* TopoEdgeList::get_edge (Iterator i) const { return edge(elem(i)); }

boolean TopoEdgeList::includes (TopoEdge* e) const {
    return _ulist->Find(e) != nil;
}

void TopoEdgeList::first (Iterator& i) const { i.SetValue(_ulist->First()); }
void TopoEdgeList::last (Iterator& i) const { i.SetValue(_ulist->Last()); }
void TopoEdgeList::next (Iterator& i) const { i.SetValue(elem(i)->Next()); }
void TopoEdgeList::prev (Iterator& i) const { i.SetValue(elem(i)->Prev()); }
boolean TopoEdgeList::done (Iterator i) const { return elem(i) == _ulist->End(); }
boolean TopoEdgeList::is_empty () const { return _ulist->IsEmpty(); }
int TopoEdgeList::number () const { return _count; }

