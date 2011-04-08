/*
 * Copyright (c) 2000 IET Inc.
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
class OverlayComp;
class OvImportCmd;
class OvSaveCompCmd;
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
// compview=paste(compview [xscale yscale xoff yoff | a00,a01,a10,a11,a20,a21]) -- paste graphic into the viewer"
class PasteFunc : public UnidrawFunc {
public:
    PasteFunc(ComTerp*,Editor*,OverlayCatalog* = nil);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(compview [xscale yscale xoff yoff | a00,a01,a10,a11,a20,a21]) -- paste graphic into the viewer"; }

protected:
    OverlayCatalog* _catalog;
};

//: command for toggling or setting paste mode
// val=paste([flag] :val) -- toggle or set paste mode, default is 0, always paste new graphics
class PasteModeFunc : public UnidrawFunc {
public:
    PasteModeFunc(ComTerp*, Editor*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s([flag] :val) -- toggle or set paste mode, default is 0, always paste new graphics"; }
    static int paste_mode() { return _paste_mode; }
    static void paste_mode(int mode) { _paste_mode = mode; }
 protected:
    static int _paste_mode;
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

//: command to save document (to pathname)
//error=save([path]) -- save editor document (to pathname). 
class SaveFileFunc : public UnidrawFunc {
public:
    SaveFileFunc(ComTerp*,Editor*);
    Command* save(const char* path);
    // helper method to import from path
    virtual void execute();
    virtual const char* docstring() { 
	return "error=%s([path]) -- save editor document (to pathname)"; }

};

//: command to import a graphic file
// compview=import(pathname :popen) -- import graphic file from pathname or URL, or from a command if :popen.
class ImportFunc : public UnidrawFunc {
public:
    ImportFunc(ComTerp*,Editor*);
    OvImportCmd* import(const char* path, boolean popen=false);
    // helper method to import from path
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(pathname :popen) -- import graphic file from pathname or URL, or from a command if :popen"; }

};

//: command to export a graphic file
// export(compview[,compview[,...compview]] [path] :host host_str :port port_int :socket :string|:str :eps :idraw) -- export in drawtool (or other) format "; 
class ExportFunc : public UnidrawFunc {
public:
  ExportFunc(ComTerp* c, Editor* e, const char* appname=nil);
  virtual ~ExportFunc() { delete _docstring; }
  virtual void execute();
  virtual const char* docstring();
  const char* appname() { return _appname ? _appname : "drawtool"; }
  void appname(const char* name) 
    { _appname = name; delete _docstring; _docstring=nil;}

 protected:
  void compout(OverlayComp*, ostream*);

  const char* _appname;
  char* _docstring;
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

//: command to composite component for a frame, defaults to current
// compview=frame([index]) -- return composite component for a frame, defaults to current
class FrameFunc : public UnidrawFunc {
public:
    FrameFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s([index]) --  return composite component for a frame, defaults to current"; }

};

//: command to pause script execution until C/R
// pause -- pause script execution until C/R
class UnidrawPauseFunc : public UnidrawFunc {
public:
    UnidrawPauseFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "pause -- pause script execution until C/R"; }

};

//: command to add button to custom toolbar
// compview=addtool(pathname) -- add button to toolbar based on zero-centered idraw drawing.
class AddToolButtonFunc : public UnidrawFunc {
public:
    AddToolButtonFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(pathname) -- add button to toolbar based on zero-centered idraw drawing."; }

};

//: command to convert from screen to drawing coordinates
// dx,dy=stod(sx,sy) -- convert from screen to drawing coordinates
class ScreenToDrawingFunc : public UnidrawFunc {
public:
    ScreenToDrawingFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "dx,dy=%s(sx,sy) -- convert from screen to drawing coordinates."; }

};

//: command to convert from drawing to screen coordinates
// sx,sy=dtos(dx,dy) -- convert from drawing to screen coordinates
class DrawingToScreenFunc : public UnidrawFunc {
public:
    DrawingToScreenFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "sx,sy=%s(dx,dy) -- convert from drawing to screen coordinates."; }

};

#endif /* !defined(_unifunc_h) */

