/*
 * Copyright (c) 2000 IET Inc.
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
 * collection of type functions
 */

#if !defined(_typefunc_h)
#define _typefunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: command to return type symbols for values
// sym|lst=type(val [val ...] :all) -- return type symbol(s) for value(s),
// or with :all the complete list of type symbols this language has.
class TypeSymbolFunc : public ComFunc {
public:
    TypeSymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "sym|lst=%s(val [ ...] :all) -- return type symbol(s) for value(s), blank for no value at all"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":all       return the complete list of type symbols, ignoring any value",
	nil
      };
      return keys;
    }
};

//: command to return class symbols for values of object type
// sym|lst=class(val [val ...] :all :comps) -- return class symbol(s) for
// value(s), or the classes this binary linked.
class ClassSymbolFunc : public ComFunc {
public:
    ClassSymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() {
      return "sym|lst=%s(val [ ...] :all :comps) -- return class symbol(s) for value(s) of object type"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":all       return every class symbol this binary linked, sorted by name",
	":comps     narrow :all to the component classes",
	nil
      };
      return keys;
    }
};

//: command to test a variable's type without ever evaluating it.
// flag=istype(var [typesym] :sym) -- true if var's type equals typesym,
// without firing var if it names a command or holds a FuncObj.  With one
// argument, true if var is a plain/regular value type (not ObjectType).
// With :sym, returns var's own type symbol (nil if var is unbound)
// instead of a flag, ignoring typesym if also given -- the post_eval
// counterpart to what type() returns for the fired value.
class IsTypeFunc : public ComFunc {
public:
    IsTypeFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var [typesym] :sym) -- test var's type without evaluating it -- a compound expression reports CommandType, the unevaluated call at its head, not the type its result would have"; }
};

//: command to test a variable's class without ever evaluating it.
// flag=isclass(var [classsym] :sym) -- true if var's class equals
// classsym, without firing var if it names a command or holds a FuncObj.
// With one argument, true if var is of ObjectType at all (there's a class
// to ask).  With :sym, returns var's own class symbol (nil if var isn't
// of ObjectType) instead of a flag, ignoring classsym if also given --
// the post_eval counterpart to what class() returns for the fired value.
class IsClassFunc : public ComFunc {
public:
    IsClassFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var [classsym] :sym) -- test var's class without evaluating it -- a compound expression has no class to report, being an unevaluated call, not a value"; }
};

//: shortcut for istype(var CommandType), without evaluating var.
// flag=iscomm(var :sym) -- true if var names a command, without
// invoking it.  With :sym, returns `CommandType (nil if false) instead
// of a flag.
class IsCommFunc : public ComFunc {
public:
    IsCommFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var :sym) -- true if var names a command, without evaluating it"; }
};

//: shortcut for isclass(var FuncObj), without evaluating var.
// flag=isfunc(var :sym) -- true if var holds a func(), without
// invoking it.  With :sym, returns `FuncObj (nil if false) instead of a
// flag.
class IsFuncFunc : public ComFunc {
public:
    IsFuncFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var :sym) -- true if var holds a func(), without evaluating it"; }
};

#endif /* !defined(_typefunc_h) */



