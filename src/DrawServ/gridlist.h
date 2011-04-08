/*
 * Copyright (c) 2004 Scott E. Johnston
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
 * GraphicIdList - a UList based list of GraphicId's
 */
#ifndef gridlist_h
#define gridlist_h


//: UList based list of GraphicId's

#include <Unidraw/globals.h>
#include <InterViews/resource.h>

class GraphicId;
class Iterator;
class UList;

class GraphicIdList : public Resource {
public:
    GraphicIdList();
    virtual ~GraphicIdList();

    void add_grid(GraphicId*);
    // add GraphicId to list

    void First(Iterator&);
    // set iterator to point to first GraphicId in list.
    void Last(Iterator&);
    // set iterator to point to last GraphicId in list.
    void Next(Iterator&);
    // set iterator to point to next GraphicId in list.
    void Prev(Iterator&);
    // set iterator to point to previous GraphicId in list.
    boolean Done(Iterator);
    // return true if iterator is pointing off the end of the list.
    // works for forward and backward traversals.
    boolean IsEmpty();
    // true if no GraphicId objects in list.
    int Number();
    // number of GraphicId objects in list.

    void Append(GraphicId*);
    // append GraphicId to end of list.
    void Prepend(GraphicId*);
    // append GraphicId to front of list.
    void InsertAfter(Iterator, GraphicId*);
    // insert GraphicId after position pointed to by iterator.
    void InsertBefore(Iterator, GraphicId*);
    // insert GraphicId before position pointed to by iterator.
    void Remove(GraphicId*);
    // remove GraphicId from list, returning responsibility for freeing the
    // associated memory.

    GraphicId* GetGraphicId(Iterator);
    // get GraphicId pointed to by iterator.
    void SetGraphicId(GraphicId*, Iterator&);
    // set GraphicId pointed to by iterator.
    boolean Includes(GraphicId*);
    // check if list includes GraphicId by pointer-comparison.

    UList* Elem(Iterator); 
    // return UList (UList) pointed to by Iterator (Iterator).
    GraphicId* Id(UList*);
    // return GraphicId pointed to by UList (UList).

protected:
    void Remove(Iterator&);
    // remove GraphicId pointed to by iterator from the list, 
    // returning responsibility for freeing the associated memory.
    // This requires saving a pointer to the GraphicId before calling this method.

    UList* _ulist;
    unsigned int _count;

friend class DrawServ;
friend class LinkSelection;
};

#endif
