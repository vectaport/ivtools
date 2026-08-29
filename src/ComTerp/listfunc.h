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
      return "lst=%s([lst|strm|val] :strmlst :attr :size n :colon) -- create list, copy list, or convert stream (unary $)"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":strmlst   return list inside stream for debug",
	":attr      make attribute list",
	":size n    make list of size n",
	":colon     tag the result coloned(), same as ':' itself builds",
	nil
      };
      return keys;
    }
};

//: list member command for ComTerp; also @ (binary at) operator.
// val=at(lst|attrlst|str n :set val :ins val :del) -- return (or set, insert after, or delete) the nth item in a list or string.
// A nil n means the last item, on every type and in both directions:
// at(lst nil), at(al nil), at(s nil), and the :set/:ins/:del forms of each.
// An attrlst position read returns a detached, single-entry attrlist
// (e.g. al@0 on (:x 10 :y 20) is (:y 20)) rather than a live handle into
// al -- al@n=val therefore can never write through to al (falls through
// to AssignFunc's generic non-writable-lvalue warning, no special-casing
// needed).  attrname()/attrval() (dotfunc.c) accept this shape directly.
class ListAtFunc : public ComFunc {
public:
    ListAtFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "val=%s(lst|attrlst|str n :set val :ins val :del :raw) -- return (or set, insert after, or delete) the nth item in a list, attribute list, or string; a nil n means the last item"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":set val   set val in list",
	":ins val   insert val in list",
	":del       delete val from list, returning the deleted value",
	":raw       currently a no-op -- reserved for opting a coloned colon-list index out of a future dispatch on coloned()",
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


};

//: : (colonlist) operator -- pairs two operands into a coloned list.
// Plain AttributeValueList, tagged via the ComValue-level coloned() flag
// (comvalue.h), so it prints and behaves like any other list; nothing here
// says what the pair means, that's up to whatever reads coloned().  Each
// operand that's a bare identifier is captured as its own unevaluated
// symbol (no lookup) rather than resolved -- lst:foo never fails just
// because foo isn't bound to anything, and a reader can either resolve it
// (symval()) or match it directly against another symbol (e.g. `Dec).
// Anything else (a literal, a parenthesized expression, ...) evaluates
// normally, same as any other operator's operand.
class ColonListFunc : public ComFunc {
public:
    ColonListFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "val=%s(lo hi) -- pair two operands into a coloned list, for the ':' operator; a bare identifier operand is captured as an unevaluated symbol"; }

    CLASS_SYMID("ColonListFunc");
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


//: attrlist command for ComTerp.
// attrlst=attrlist([:<name> [val]] ...) -- create attribute list from keyword/value pairs.
// Keyword-only (no value) sets attribute to true. Missing attribute returns nil.
class AttrListFunc : public ComFunc {
public:
    AttrListFunc(ComTerp*);
    virtual void execute();
    virtual const char* docstring() {
      return "alst=%s([:<name> [val]] ...) -- create attribute list from keyword/value pairs"; }
};

#endif /* !defined(_listfunc_h) */
