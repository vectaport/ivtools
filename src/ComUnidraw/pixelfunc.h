/*
 * Copyright (c) 2001 Scott E. Johnston
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

#if !defined(pixelfunc_h)
#define _pixelfunc_h

#include <ComUnidraw/unifunc.h>

//: command to poke pixel values into raster
// poke(compview x y val) -- poke pixel value into raster
class PixelPokeFunc : public UnidrawFunc {
public:
    PixelPokeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview x y val) -- poke pixel value into raster"; }
};

//: command to peek pixel values from raster
// val=peek(compview x y) -- peek pixel value into raster
class PixelPeekFunc : public UnidrawFunc {
public:
    PixelPeekFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "val=%s(compview x y) -- peek pixel value from raster"; }
};

//: command to return number of columns in a raster
// pcols(compview) -- number of columns in a raster
class PixelColsFunc : public UnidrawFunc {
public:
    PixelColsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview) -- number of columns in a raster"; }
};

//: command to return number of rows in a raster
// pcols(compview) -- number of rows in a raster
class PixelRowsFunc : public UnidrawFunc {
public:
    PixelRowsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview) -- number of rows in a raster"; }
};

//: command to flush pixels poked in a raster
// pflush(compview) -- flush pixels poked into a raster
class PixelFlushFunc : public UnidrawFunc {
public:
    PixelFlushFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview) -- flush pixels poked into a raster"; }
};

//: command to clip raster with polygon
// pclip(compview x1,y1,x2,y2,x3,y3[,...,xn,yn]) -- clip raster with polygon
class PixelClipFunc : public UnidrawFunc {
public:
    PixelClipFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview x1,y1,x2,y2,x3,y3[,...,xn,yn]) -- clip raster with polygon"; }
};

//: command to set/get raster alpha transparency
// alpha(compview [alphaval]) -- set/get alpha transparency value
class AlphaTransFunc : public UnidrawFunc {
public:
    AlphaTransFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview [alphaval]) -- set/get alpha transparency value"; }
};
#endif /* !defined(_pixelfunc_h) */
