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
 * collection of transform functions
 */

#if !defined(_xformfunc_h)
#define _xformfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: ComTerp command to apply an affine-transform to a 2d coordinate.
// point=xform(x,y a00,a01,a10,a11,a20,a21) -- affine transform of x,y coordinates.
class XformFunc : public ComFunc {
public:
    XformFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "point=%s(x,y a00,a01,a10,a11,a20,a21) -- affine transform of x,y coordinates"; }

};

//: ComTerp command to invert an affine transform.
// affine=invert(a00,a01,a10,a11,a20,a21) -- invert affine transform.
class InvertXformFunc : public ComFunc {
public:
    InvertXformFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "affine=%s(a00,a01,a10,a11,a20,a21) -- invert affine transform"; }

};

//: ComTerp command to transpose a matrix
// matrix=xpose(matrix) -- transpose an arbitrary matrix
class XposeFunc : public ComFunc {
public:
    XposeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "matrix=%s(matrix) -- transpose an arbitrary matrix"; }

};

#endif /* !defined(_xformfunc_h) */
