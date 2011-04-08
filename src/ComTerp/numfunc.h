/*
 * Copyright (c) 1994,1995,1999,2000 Vectaport Inc.
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

//: base class for all numeric ComTerp commands.
class NumFunc : public ComFunc {
public:
    NumFunc(ComTerp*);

    void promote(ComValue&, ComValue&);
    // method to do C-style promotion of operand types.

};

//: + (plus) operator.
class AddFunc : public NumFunc {
public:
    AddFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "+ is the add operator"; }

};

//: - (subtraction) operator.
class SubFunc : public NumFunc {
public:
    SubFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "- is the minus operator"; }

};

//: - (unary prefix minus) operator.
class MinusFunc : public NumFunc {
public:
    MinusFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return " and the unary prefix minus"; }
};

//: * (multiply) operator.
class MpyFunc : public NumFunc {
public:
    MpyFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "* is the multiply operator"; }

};

//: / (divide) operator.
class DivFunc : public NumFunc {
public:
    DivFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "/ is the divide operator"; }

};

//: modulo command for ComTerp.
class ModFunc : public NumFunc {
public:
    ModFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s is the mod operator"; }

};

//: minimum command for ComTerp.
// n=min(a b) -- return minimum of a and b.
class MinFunc : public NumFunc {
public:
    MinFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "n=%s(a b) -- return minimum of a and b"; }

};

//: maximum command for ComTerp.
// n=max(a b) -- return maximum of a and b.
class MaxFunc : public NumFunc {
public:
    MaxFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "n=%s(a b) -- return maximum of a and b"; }

};

//: absolute-value command for ComTerp.
// n=abs(a) -- return absolute value of a.
class AbsFunc : public NumFunc {
public:
    AbsFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "n=%s(a) -- return absolute value of a"; }

};

//: floor command for ComTerp.
// num=floor(num) -- return closest integer value less than or equal to argument
class FloorFunc : public NumFunc {
public:
    FloorFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "num=%s(num) -- return closest integer value less than or equal to argument"; }

};

//: ceiling command for ComTerp.
// num=ceil(num) -- return closest integer value greater than or equal to argument
class CeilFunc : public NumFunc {
public:
    CeilFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "num=%s(num) -- return closest integer value greater than or equal to argument"; }

};

//: ceiling command for ComTerp.
// num=round(num) -- return closest integer value
class RoundFunc : public NumFunc {
public:
    RoundFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "num=%s(num) -- return closest integer value"; }

};

//: character conversion command for ComTerp.
// c=char(num) -- convert any numeric to a char.
class CharFunc : public ComFunc {
public:
    CharFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "c=%s(num) -- convert any numeric to a char"; }

};

//: short integer conversion command for ComTerp.
// s=short(num) -- convert any numeric to a short.
class ShortFunc : public ComFunc {
public:
    ShortFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "s=%s(num) -- convert any numeric to a short"; }

};

//: integer conversion command for ComTerp.
// i=int(num) -- convert any numeric to an int.
class IntFunc : public ComFunc {
public:
    IntFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "i=%s(num) -- convert any numeric to an int"; }

};

//: long integer conversion command for ComTerp.
// l=long(num) -- convert any numeric to a long.
class LongFunc : public ComFunc {
public:
    LongFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "l=%s(num) -- convert any numeric to a long"; }

};

//: floating-point conversion command for ComTerp.
// f=float(num) -- convert any numeric to a float.
class FloatFunc : public ComFunc {
public:
    FloatFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "f=%s(num) -- convert any numeric to a float"; }

};

//: double-length floating-point conversion command for ComTerp.
// d=double(num) -- convert any numeric to a double.
class DoubleFunc : public ComFunc {
public:
    DoubleFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "d=%s(num) -- convert any numeric to a double"; }

};

#endif /* !defined(_numfunc_h) */
