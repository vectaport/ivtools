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
 * DrawLinkList - a UList based list of DrawLink's
 */
#ifndef drawlinklist_h
#define drawlinklist_h


//: UList based list of DrawLink's

#include <Unidraw/globals.h>
#include <InterViews/observe.h>

class DrawLink;
class GraphicId;
class Iterator;
class UList;

class DrawLinkList : public Observable, Observer {
public:
    DrawLinkList();
    virtual ~DrawLinkList();

    void add_drawlink(DrawLink*);
    // add DrawLink to list

    DrawLink* find_drawlink(GraphicId*);
    // find DrawLink to the selector of this graphic

    void First(Iterator&);
    // set iterator to point to first DrawLink in list.
    void Last(Iterator&);
    // set iterator to point to last DrawLink in list.
    void Next(Iterator&);
    // set iterator to point to next DrawLink in list.
    void Prev(Iterator&);
    // set iterator to point to previous DrawLink in list.
    boolean Done(Iterator);
    // return true if iterator is pointing off the end of the list.
    // works for forward and backward traversals.
    boolean IsEmpty();
    // true if no DrawLink objects in list.
    int Number();
    // number of DrawLink objects in list.

    void Append(DrawLink*);
    // append DrawLink to end of list.
    void Prepend(DrawLink*);
    // append DrawLink to front of list.
    void InsertAfter(Iterator, DrawLink*);
    // insert DrawLink after position pointed to by iterator.
    void InsertBefore(Iterator, DrawLink*);
    // insert DrawLink before position pointed to by iterator.
    void Remove(DrawLink*);
    // remove DrawLink from list, returning responsibility for freeing the
    // associated memory.

    DrawLink* GetDrawLink(Iterator);
    // get DrawLink pointed to by iterator.
    void SetDrawLink(DrawLink*, Iterator&);
    // set DrawLink pointed to by iterator.
    boolean Includes(DrawLink*);
    // check if list includes DrawLink by pointer-comparison.

    UList* Elem(Iterator); 
    // return UList (UList) pointed to by Iterator (Iterator).
    DrawLink* Link(UList*);
    // return DrawLink pointed to by UList (UList).

    virtual void update(Observable* obs) { notify(); }
    // invoked when DrawLink is updated
protected:
    void Remove(Iterator&);
    // remove DrawLink pointed to by iterator from the list, 
    // returning responsibility for freeing the associated memory.
    // This requires saving a pointer to the DrawLink before calling this method.

    UList* _ulist;
    unsigned int _count;
};

#endif
