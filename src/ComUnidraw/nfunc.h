/*
 * Copyright (c) 2001 Scott E. Johnston
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

#if !defined(_comunidraw_nfunc_h)
#define _comunidraw_nfunc_h

#include <ComUnidraw/unifunc.h>

//: command to return number of onscreen columns in comdraw.
// ncols() -- onscreen horizontal extent in pixels or 
class NColsFunc : public UnidrawFunc {
public:
    NColsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- onscreen horizontal extent in pixels"; }

};

//: command to return number of onscreen rows in comdraw.
// nrows() -- onscreen vertical extent in pixels
class NRowsFunc : public UnidrawFunc {
public:
    NRowsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- onscreen vertical extent in pixels"; }

};

//: command to return number of fonts in comdraw menu.
// nfonts() -- return size of font menu
class NFontsFunc : public UnidrawFunc {
public:
    NFontsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- return size of font menu"; }

};

//: command to return number of brushes in comdraw menu.
// nbrushes() -- return size of brush menu
class NBrushesFunc : public UnidrawFunc {
public:
    NBrushesFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- return size of brush menu"; }

};

//: command to return number of patterns in comdraw menu.
// npatterns() -- return size of pattern menu
class NPatternsFunc : public UnidrawFunc {
public:
    NPatternsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- return size of pattern menu"; }

};

//: command to return number of colors in comdraw menus.
// ncolors() -- return size of color menus.
class NColorsFunc : public UnidrawFunc {
public:
    NColorsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- return size of color menus"; }

};

#endif /* !defined(_comunidraw_nfunc_h) */
