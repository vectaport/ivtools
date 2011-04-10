/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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
class OvImportCmd;
class OverlayCatalog;

//: base class for interpreter commands in comdraw.
class UnidrawFunc : public ComFunc {
public:
    UnidrawFunc(ComTerp*,Editor*);

    void execute_log(Command*);

    Editor* GetEditor() { return editor(); }
    Editor* editor() { return _ed; }
protected:
    void menulength_execute(const char* kind);
    Editor* _ed;

static int _compview_id;

};

//: command to update Unidraw from comdraw.
// update() -- update viewers
class UpdateFunc : public UnidrawFunc {
public:
    UpdateFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- update viewers"; }
};

//: command to turn on or off the selection tic marks in comdraw.
// handles(flag) -- enable/disable current selection tic marks and/or highlighting
class HandlesFunc : public UnidrawFunc {
public:
    HandlesFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(flag) -- enable/disable current selection tic marks and/or highlighting"; }
};

//: command to paste a graphic in comdraw.
// paste(compview [xscale yscale xoff yoff | a00,a01,a10,a11,a20,a21]) -- paste graphic into the viewer"
class PasteFunc : public UnidrawFunc {
public:
    PasteFunc(ComTerp*,Editor*,OverlayCatalog* = nil);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview [xscale yscale xoff yoff | a00,a01,a10,a11,a20,a21]) -- paste graphic into the viewer"; }

protected:
    OverlayCatalog* _catalog;
};

//: command to make a graphic read-only in comdraw.
// compview=readonly(compview :clear) -- set or clear the readonly attribute of a graphic component
class ReadOnlyFunc : public UnidrawFunc {
public:
    ReadOnlyFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(compview :clear) -- set or clear the readonly attribute of a graphic component"; }

};

//: command to import a graphic file
// import(pathname) -- import graphic file from pathname or URL.
class ImportFunc : public UnidrawFunc {
public:
    ImportFunc(ComTerp*,Editor*);
    OvImportCmd* import(const char* path);
    // helper method to import from path
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(pathname) -- import graphic file from pathname or URL"; }

};

//: command to set attributes on a graphic
// compview=setattr(compview [:keyword value [:keyword value [...]]]) -- set attributes of a graphic component.
class SetAttrFunc : public UnidrawFunc {
public:
    SetAttrFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(compview [:keyword value [:keyword value [...]]]) -- set attributes of a graphic component"; }

};

#endif /* !defined(_unifunc_h) */
