/*
 * Copyright (c) 2001 Scott Johnston
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
 * This is a collection of bitwise operators.  They inherit the 
 * C-like promotion mechanism of NumFunc. 
 */

#if !defined(_bitfunc_h)
#define _bitfunc_h

#include <ComTerp/numfunc.h>

//: & (bitwise-and) operator.
class BitAndFunc : public NumFunc {
public:
    BitAndFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "& is the bitwise-and operator"; }
};

//: ^ (bitwise-xor) operator.
class BitXorFunc : public NumFunc {
public:
    BitXorFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "^ is the bitwise-xor operator"; }

};

//: | (bitwise-or) operator.
class BitOrFunc : public NumFunc {
public:
    BitOrFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "| is the bitwise-or operator"; }

};

//: ~ (bitwise-not) operator.
class BitNotFunc : public NumFunc {
public:
    BitNotFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "~ is the bitwise-not operator"; }

};

//: << (left-shift) operator.
class LeftShiftFunc : public NumFunc {
public:
    LeftShiftFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "<< is the left-shift operator"; }
};

//: >> (right-shift) operator.
class RightShiftFunc : public NumFunc {
public:
    RightShiftFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return ">> is the right-shift operator"; }
};

#endif /* !defined(_bitfunc_h) */






