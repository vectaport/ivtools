/*
 * Copyright (c) 1998 Vectaport Inc.
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

#if !defined(_iuefunc_h)
#define _iuefunc_h

#include <ComTerp/comfunc.h>

class ComTerp;
class IueImageComp;

class IueFunc : public ComFunc {
public:
    IueFunc(ComTerp*);

protected:
    static IueImageComp* image_comp(ComValue&);
};

class IueImageFunc : public IueFunc {
public:
    IueImageFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "img=%s(path|img :mem :adrg) -- load or convert image"; }

};

class IueGetPixelFunc : public IueFunc {
public:
    IueGetPixelFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s(img x,y) -- get image pixel value of arbitrary type"; }

};

class IueNcolsFunc : public IueFunc {
public:
    IueNcolsFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s(img) -- get number of columns in image"; }

};

class IueNrowsFunc : public IueFunc {
public:
    IueNrowsFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s(img) -- get number of rows in image"; }

};

class IuePixTypeFunc : public IueFunc {
public:
    IuePixTypeFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "str=%s(img) -- get type of image pixels"; }

};

#endif /* !defined(_iuefunc_h) */
