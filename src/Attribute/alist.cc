/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 */

/*
 * AList implementation -- cloned from UList
 */

//: flag for separate ALIterator class.
// if defined a separate ALIterator and AList are used that
// are identical to Iterator and UList, otherwise
// ALIterator is really Iterator, and AList is really UList.
#define ALITERATOR

#include <Attribute/alist.h>

/*****************************************************************************/

AList::AList (void* p) { _next = this; _prev = this; _object = p; }

AList::~AList () {
    AList* next = _next;

    if (next != this && next != nil) {
        Remove(this);
        delete next;
    }
}

void AList::Append (AList* e) {
    _prev->_next = e;
    e->_prev = _prev;
    e->_next = this;
    _prev = e;
}

void AList::Prepend (AList* e) {
    _next->_prev = e;
    e->_prev = this;
    e->_next = _next;
    _next = e;
}

void AList::Remove (AList* e) {
    e->_prev->_next = e->_next;
    e->_next->_prev = e->_prev;
    e->_prev = e->_next = nil;
}

void AList::Delete (void* p) {
    register AList* e;

    e = Find(p);
    if (e != nil) {
	Remove(e);
	delete e;
    }
}

AList* AList::Find (void* p) {
    register AList* e;

    for (e = _next; e != this; e = e->_next) {
	if (e->_object == p) {
	    return e;
	}
    }
    return nil;
}

AList* AList::operator[] (int count) {
    AList* pos = First();
    int i;

    for (i = 1; i < count && pos != End(); ++i) {
	pos = pos->Next();
    }
    if (i == count) {
	return pos;
    }
    return nil;
}	
