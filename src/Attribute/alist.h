/*
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Alist - list object.
 */

#ifndef _alist_h
#define _alist_h

#include <InterViews/enter-scope.h>

#include <InterViews/_enter.h>

#ifdef ALITERATOR

class AList {
public:
    AList(void* = nil);
    virtual ~AList();

    boolean IsEmpty();
    void Append(AList*);
    void Prepend(AList*);
    void Remove(AList*);
    void Delete(void*);
    AList* Find(void*);
    AList* First();
    AList* Last();
    AList* End();
    AList* Next();
    AList* Prev();

    void* operator()();
    AList* operator[](int count);
protected:
    void* _object;
    AList* _next;
    AList* _prev;
};

inline boolean AList::IsEmpty () { return _next == this; }
inline AList* AList::First () { return _next; }
inline AList* AList::Last () { return _prev; }
inline AList* AList::End () { return this; }
inline AList* AList::Next () { return _next; }
inline AList* AList::Prev () { return _prev; }
inline void* AList::operator() () { return _object; }

#else
#define AList _lib_iv(UList)
#include <Unidraw/ulist.h>
#endif

#include <InterViews/_leave.h>

#endif
