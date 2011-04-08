/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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
 * This is a collection of a numeric operators.  They 
 * automatically promote numeric types as needed, just like C.
 */

#if !defined(_numfunc_h)
#define _numfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class ComValue;

class NumFunc : public ComFunc {
public:
    NumFunc(ComTerp*);

    void promote(ComValue&, ComValue&);

};

class AddFunc : public NumFunc {
public:
    AddFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "+ is the add operator"; }

};

class SubFunc : public NumFunc {
public:
    SubFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "- is the minus operator"; }

};

class MinusFunc : public NumFunc {
public:
    MinusFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return " and the unary prefix minus"; }
};

class MpyFunc : public NumFunc {
public:
    MpyFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "* is the multiply operator"; }

};

class DivFunc : public NumFunc {
public:
    DivFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "/ is the divide operator"; }

};

class ModFunc : public NumFunc {
public:
    ModFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "% is the mod operator"; }

};

class MinFunc : public NumFunc {
public:
    MinFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(a b) -- return minimum of a and b"; }

};

class MaxFunc : public NumFunc {
public:
    MaxFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(a b) -- return maximum of a and b"; }

};

#endif /* !defined(_numfunc_h) */
