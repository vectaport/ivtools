/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#if !defined(_unifunc_h)
#define _unifunc_h

#include <Unidraw/editor.h>
#include <ComTerp/comfunc.h>

class Command;
class ComTerp;
class OverlayCatalog;

class UnidrawFunc : public ComFunc {
public:
    UnidrawFunc(ComTerp*,Editor*);

    void execute_log(Command*);

    Editor* editor() { return _ed; }
protected:
    Editor* _ed;

static int _compview_id;

};

class HandlesFunc : public UnidrawFunc {
public:
    HandlesFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(flag) -- enable/disable current selection tic marks and/or highlighting"; }
};

class PasteFunc : public UnidrawFunc {
public:
    PasteFunc(ComTerp*,Editor*,OverlayCatalog* = nil);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(grcomp [xscale yscale xoff yoff | a00,a01,a10,a11,a20,a21]) -- paste graphic component into the viewer"; }

protected:
    OverlayCatalog* _catalog;
};

class ReadOnlyFunc : public UnidrawFunc {
public:
    ReadOnlyFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(grcomp :clear) -- set or clear the readonly attribute of a graphic component"; }

protected:
    int _clear_symid;
};

class BarPlotFunc : public UnidrawFunc {
public:
    BarPlotFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([var_str value_float] [...] :title title_str :xtitle xtitle_str :ytitle ytitle_str :valtitle valtitle_str) -- display a barplot"; }
protected:
  int _title_symid;
  int _xtitle_symid;
  int _ytitle_symid;
  int _valtitle_symid;
};

#endif /* !defined(_unifunc_h) */
