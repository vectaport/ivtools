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
 * ALIterator - an object that marks a position in a data structure (cloned from Iterator)
 */

#ifndef _al_iterator_h
#define _al_iterator_h

#ifdef ALITERATOR
#include <InterViews/enter-scope.h>

#include <InterViews/_enter.h>

class ALIterator {
public:
    ALIterator();
    ALIterator(ALIterator&);
    ALIterator& operator =(ALIterator&);
    virtual void* GetValue();
    virtual void SetValue(void*);
protected:
    void* _value;
};


#else
#define ALIterator _lib_iv(Iterator)
#include <Unidraw/iterator.h>
#endif

#include <InterViews/_leave.h>



#endif
