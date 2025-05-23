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
	return "%s([usec]) -- update viewer with optional delay"; }
};

//: command to turn on or off the selection tic marks in comdraw.
// handles(flag) -- enable/disable current selection tic marks and/or highlighting
class HandlesFunc : public UnidrawFunc {
public:
    HandlesFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([flag]) -- disable/enable current selection tic marks and/or highlighting"; }
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
// val=pastemode([flag] :get) -- toggle or set paste mode, default is 0, always paste new graphics
class PasteModeFunc : public UnidrawFunc {
public:
    PasteModeFunc(ComTerp*, Editor*);

    virtual void execute();
    virtual const char* docstring() { 
      return "val=%s([flag] :get) -- toggle or set paste mode, default is 0, always paste new graphics"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":get       return paste mode",
	nil
      };
      return keys;
    }
    static int paste_mode() { return _paste_mode; }
    static void paste_mode(int mode) { _paste_mode = mode; }
 protected:
    static int _paste_mode;
};

//: command to make a graphic read-only in comdraw.
// compview=readonly(compview :clear) -- set or clear the read-only attribute of a graphic component
class ReadOnlyFunc : public UnidrawFunc {
public:
    ReadOnlyFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(compview :clear) -- set or clear the read-only attribute of a graphic component"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":clear     clear read-only attribute",
	nil
      };
      return keys;
    }

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
// compview=import(pathname :popen :next) -- import graphic file from pathname or URL, or from a command if :popen
// (:next imports next in numeric series).
class ImportFunc : public UnidrawFunc {
public:
    ImportFunc(ComTerp*,Editor*);
    OvImportCmd* import(const char* path, boolean popen=false);
    // helper method to import from path
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(pathname :popen :next) -- import graphic file from pathname or URL, or from a command if :popen\n(:next imports next in numeric series)"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":popen     open as pipe",
	":next      import next in numeric series",
	nil
      };
      return keys;
    }

};

//: command to export a graphic file
// export(compview[,compview[,...compview]] [path] :host str :port int :socket :string|:str :eps :idraw) -- export in drawtool (or other) format "; 
class ExportFunc : public UnidrawFunc {
public:
  ExportFunc(ComTerp* c, Editor* e, const char* appname=nil);
  virtual ~ExportFunc() { delete _docstring; }
  virtual void execute();
  virtual const char* docstring();
  virtual const char** dockeys();
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
    virtual const char** dockeys() {
      static const char* keys[] = {
	":keyword value         keyword/value to set attribute on graphic",
	nil
      };
      return keys;
    }

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
	return "pause(:usec usec) -- pause script execution until C/R (or usec)"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":usec      microseconds to pause",
	nil
      };
      return keys;
    }

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

//: command to convert from graphic to drawing coordinates
// dx,dy=gtod(compview gx,gy) -- convert from graphic to drawing coordinates
class GraphicToDrawingFunc : public UnidrawFunc {
public:
    GraphicToDrawingFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "dx,dy=%s(compview gx,gy) -- convert from graphic to drawing coordinates."; }

};

//: command to convert from drawing to graphic coordinates
// gx,gy=dtog(compview dx,dy) -- convert from drawing to graphic coordinates
class DrawingToGraphicFunc : public UnidrawFunc {
public:
    DrawingToGraphicFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "gx,gy=%s(compview dx,dy) -- convert from drawing to graphic coordinates."; }

};

//: command to turn on or off drawing editor gravity
// gravity([flag]) -- enable/disable drawing editor gravity
class GravityFunc : public UnidrawFunc {
public:
    GravityFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([flag]) -- enable/disable drawing editor gravity"; }
};

 //: command to set or get drawing editor grid spacing
// gridspacing([xsize ysize]) -- set/get drawing editor grid spacing
class GridSpacingFunc : public UnidrawFunc {
public:
    GridSpacingFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([xsize ysize]) -- set/get drawing editor grid spacing"; }
};

//: command to return screen size
// sx,sy=ssize() -- size of screen
class ScreenSizeFunc : public UnidrawFunc {
public:
    ScreenSizeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "sx,sy=%s() -- size of screen"; }
};

//: command to return drawing size
// dx,dy=ssize() -- size of drawing
class DrawingSizeFunc : public UnidrawFunc {
public:
    DrawingSizeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "dx,dy=%s() -- size of drawing."; }

};

//: command to return location of last pointer motion
// x,y=pointer() -- x,y location of last pointer motion
class PointerLocFunc : public UnidrawFunc {
public:
    PointerLocFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "x,y=%s() -- x,y location of last pointer motion."; }

};

#endif /* !defined(_unifunc_h) */

