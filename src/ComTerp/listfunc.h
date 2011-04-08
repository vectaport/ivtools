/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1999 Vectaport Inc.
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
 * collection of list manipulation functions
 */

#if !defined(_listfunc_h)
#define _listfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class ComValue;

//: create list command for ComTerp.
// lst=list([olst]) -- create an empty list or copy existing one.
class ListFunc : public ComFunc {
public:
    ListFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "lst=%s([olst]) -- create an empty list or copy existing one"; }
};

//: list member command for ComTerp.
// val=at(list|attrlist n) -- return the nth item in a list.
class ListAtFunc : public ComFunc {
public:
    ListAtFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=at(list|attrlist n) -- return the nth item in a list"; }
};

//: list size command for ComTerp.
// num=size(list|attrlist) -- return size of a list.
class ListSizeFunc : public ComFunc {
public:
    ListSizeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=size(list|attrlist) -- return the size of the list"; }
};

#endif /* !defined(_listfunc_h) */
