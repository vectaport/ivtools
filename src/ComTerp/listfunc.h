/*
 * Copyright (c) 2011 Wave Semiconductor Inc.
 * Copyright (c) 2001 Scott E. Johnston
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
// lst=list([lst|strm|val] :strmlst :attr :size n) -- create list, copy list, or convert stream
class ListFunc : public ComFunc {
public:
    ListFunc(ComTerp*);

    virtual void execute();
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() { 
      return "lst=%s([lst|strm|val] :strmlst :attr :size n) -- create list, copy list, or convert stream (unary $$)"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":strmlst   return list inside stream for debug",
	":attr      make attribute list",
	":size n    make list of size n",
	nil
      };
      return keys;
    }
};

//: list member command for ComTerp.
// val=at(lst|attrlst|str n :set val :ins val) -- return (or set or insert after) the nth item in a list or string.
class ListAtFunc : public ComFunc {
public:
    ListAtFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=at(lst|attrlst n :set val :ins val) -- return (or set or insert after) the nth item in a list"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":set val   set val in list",
	":ins val   insert val in list",
	nil
      };
      return keys;
    }
};

//: list size command for ComTerp.
// num=size(lst|attrlst|string) -- return size of a list (or string).
class ListSizeFunc : public ComFunc {
public:
    ListSizeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=size(lst|attrlst|string) -- return the size of the list (or string)"; }
};

//: , (tuple) operator.
class TupleFunc : public ComFunc {
public:
    TupleFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return ", is the tuple operator"; }

    CLASS_SYMID("TupleFunc");

};


//: list index command for ComTerp.
// val=index(lst|str val|char|str :last :all :substr) -- return index of value (or char or string) in list (or string), nil if not found.
class ListIndexFunc : public ComFunc {
public:
    ListIndexFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "num=index(lst|str val|char|str :last :all :substr) -- return index of value (or char or string) in list (or string), nil if not found"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":last      find last val or str in list",
	":all       return all matches in list",
	":substr    sub-string match",
	nil
      };
      return keys;
    }
};

#endif /* !defined(_listfunc_h) */
