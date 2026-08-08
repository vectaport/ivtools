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
// sym|lst=type(val [val ...]) -- return type symbol(s) for value(s)
class TypeSymbolFunc : public ComFunc {
public:
    TypeSymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() { 
      return "sym|lst=%s(val [ ...]) -- return type symbol(s) for value(s)"; }
};

//: command to return class symbols for values of object type
// sym|lst=type(val [val ...]) -- return type symbol(s) for value(s)
class ClassSymbolFunc : public ComFunc {
public:
    ClassSymbolFunc(ComTerp*);
    virtual void execute();

    virtual const char* docstring() {
      return "sym|lst=%s(val [ ...]) -- return class symbol(s) for value(s) of object type"; }
};

//: command to test a variable's type without ever evaluating it.
// flag=istype(var [typesym]) -- true if var's type equals typesym, without
// firing var if it names a command or holds a FuncObj.  With one argument,
// true if var is a plain/regular value type (not ObjectType).
class IsTypeFunc : public ComFunc {
public:
    IsTypeFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var [typesym]) -- test var's type without evaluating it"; }
};

//: command to test a variable's class without ever evaluating it.
// flag=isclass(var [classsym]) -- true if var's class equals classsym,
// without firing var if it names a command or holds a FuncObj.  With one
// argument, true if var is of ObjectType at all (there's a class to ask).
class IsClassFunc : public ComFunc {
public:
    IsClassFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var [classsym]) -- test var's class without evaluating it"; }
};

//: shortcut for istype(var CommandType), without evaluating var.
// flag=iscomm(var) -- true if var names a command, without invoking it
class IsCommFunc : public ComFunc {
public:
    IsCommFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var) -- true if var names a command, without evaluating it"; }
};

//: shortcut for isclass(var FuncObj), without evaluating var.
// flag=isfunc(var) -- true if var holds a func(), without invoking it
class IsFuncFunc : public ComFunc {
public:
    IsFuncFunc(ComTerp*);
    virtual void execute();

    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "flag=%s(var) -- true if var holds a func(), without evaluating it"; }
};

#endif /* !defined(_typefunc_h) */



