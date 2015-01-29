/*
 * Copyright (c) 1997,1999 Vectaport Inc.
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

#if !defined(_assignfunc_h)
#define _assignfunc_h

#include <ComTerp/numfunc.h>

class ComTerp;
class ComValue;

//: = (assign) operator.
class AssignFunc : public ComFunc {
public:
    AssignFunc(ComTerp*);

    virtual boolean post_eval() { return true; }
    virtual void execute();
    virtual const char* docstring() { 
      return "= is the assigment operator"; }

    CLASS_SYMID("AssignFunc");
};

//: %= (mod assign) operator .
class ModAssignFunc : public AssignFunc {
public:
    ModAssignFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "\%= is the mod assignment operator"; }
};

//: *= (multiply assign) operator .
class MpyAssignFunc : public AssignFunc {
public:
    MpyAssignFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "*= is the multiply assignment operator"; }
};

//: *= (add assign) operator .
class AddAssignFunc : public AssignFunc {
public:
    AddAssignFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "+= is the add assignment operator"; }
};

//: += (subtract assign) operator .
class SubAssignFunc : public AssignFunc {
public:
    SubAssignFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "-= is the minus assignment operator"; }
};

//: /= (divide assign) operator .
class DivAssignFunc : public AssignFunc {
public:
    DivAssignFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "/= is the minus assignment operator"; }
};

//: ++(increment before) operator.
class IncrFunc : public AssignFunc {
public:
    IncrFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "++ is the increment-before operator (prefix) and the "; }
};

//: (increment after)++  operator.
class IncrAfterFunc : public AssignFunc {
public:
    IncrAfterFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "increment-after as well (postfix)"; }
};

//: --(decrement before)  operator.
class DecrFunc : public AssignFunc {
public:
    DecrFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "-- is the decrement-before operator (prefix) and the "; }
};

//: (decrement after)--  operator.
class DecrAfterFunc : public AssignFunc {
public:
    DecrAfterFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "decrement-after as well (postfix)"; }
};

#endif /* !defined(_assignfunc_h) */
