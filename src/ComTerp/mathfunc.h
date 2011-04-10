/*
 * Copyright (c) 1998,1999 Vectaport Inc.
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
 * This is a collection of mathematical operators.
 */

#if !defined(_mathfunc_h)
#define _mathfunc_h

#include <ComTerp/comfunc.h>

//: exponential command for ComTerp.
// dbl=exp(x) -- returns the value e raised to the power of x.
class ExpFunc : public ComFunc {
public:
    ExpFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the value e raised to the power of x"; }

};

//: logarithmic (natural) command for ComTerp.
// dbl=log(x) -- returns the natural logarithm of x.
class LogFunc : public ComFunc {
public:
    LogFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the natural logarithm of x"; }

};

//: logarithmic (base 10) command for ComTerp.
// dbl=log10(x) -- returns the base-10 logarithm of x.
class Log10Func : public ComFunc {
public:
    Log10Func(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the base-10 logarithm of x"; }

};

//: power command for ComTerp.
// dbl=pow(x y) -- returns the value of x raised to the power of y.
class PowFunc : public ComFunc {
public:
    PowFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x y) -- returns the value of x raised to the power of y"; }

};

//: arc-cosine command for ComTerp.
// dbl=acos(x) -- returns the arc cosine of x.
class ACosFunc : public ComFunc {
public:
    ACosFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the arc cosine of x"; }

};

//: arc-sine command for ComTerp.
// dbl=asin(x) -- returns the arc sine of x.
class ASinFunc : public ComFunc {
public:
    ASinFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the arc sine of x"; }

};

//: arc-tangent command for ComTerp.
// dbl=atan(x) -- returns the arc tangent of x.
class ATanFunc : public ComFunc {
public:
    ATanFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the arc tangent of x"; }

};

//: arc-tangent (of y over x) command for ComTerp.
// dbl=atan2(y x) -- returns the arc tangent of y over x.
class ATan2Func : public ComFunc {
public:
    ATan2Func(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(y x) -- returns the arc tangent of y over x"; }

};

//: cosine command for ComTerp.
// dbl=cos(x) -- returns the cosine of x.
class CosFunc : public ComFunc {
public:
    CosFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the cosine of x"; }

};

//: sine command for ComTerp.
// dbl=sin(x) -- returns the sine of x.
class SinFunc : public ComFunc {
public:
    SinFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the sine of x"; }

};

//: tangent command for ComTerp.
// dbl=tan(x) -- returns the tangent of x.
class TanFunc : public ComFunc {
public:
    TanFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns the tangent of x"; }

};

//: square-root command for ComTerp.
// dbl=sqrt(x) -- returns square root of x.
class SqrtFunc : public ComFunc {
public:
    SqrtFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "dbl=%s(x) -- returns square root of x"; }

};

#endif /* !defined(_mathfunc_h) */
