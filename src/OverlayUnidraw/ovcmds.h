/*
 * Copyright (c) 1998-1999 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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
 * OverlayIdraw-specific commands.
 */

#ifndef ovcmds_h
#define ovcmds_h

#include <OverlayUnidraw/ovcamcmds.h> // for backward compatibility

#include <OverlayUnidraw/ovcomps.h>

#include <UniIdraw/idcmds.h>

#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/struct.h>

#include <InterViews/action.h>

class OpenFileChooser;
class OverlayViewer;
class PageDialog;
#include <iosfwd>

//: derived new command.
class OvNewCompCmd : public NewCompCmd {
public:
    OvNewCompCmd(ControlInfo*, Component* prototype = nil);
    OvNewCompCmd(Editor* = nil, Component* prototype = nil);

    virtual void Execute();
    // create new (empty) document.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: derived revert command.
class OvRevertCmd : public RevertCmd {
public:
    OvRevertCmd(ControlInfo*);
    OvRevertCmd(Editor* = nil);

    virtual void Execute();
    // revert to disk version of current document.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: derived open command.
class OvViewCompCmd : public ViewCompCmd {
public:
    OvViewCompCmd(ControlInfo*, OpenFileChooser* = nil);
    OvViewCompCmd(Editor* = nil, OpenFileChooser* = nil);

    virtual void Execute();
    // pop-up filechooser and attempt to open document.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    OpenFileChooser* chooser_;
};

//: derived open command with grid support.
class OvOpenCmd : public OvViewCompCmd {
public:
    OvOpenCmd(ControlInfo*, OpenFileChooser* = nil);
    OvOpenCmd(Editor* = nil, OpenFileChooser* = nil);

    virtual void Execute();
    // run base class execute method, then set grid spacing
    // from attributes of just-opened document.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: derived save command.
class OvSaveCompCmd : public SaveCompCmd {
public:
    OvSaveCompCmd(ControlInfo*, OpenFileChooser* = nil);
    OvSaveCompCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvSaveCompCmd();
    void Init();

    virtual void Execute();
    // use catalog to save document in ASCII script format.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    Component* component() { return comp_; }
    // return pointer to saved component.   

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);

    Component* comp_;
};

//: derived save-as command.
class OvSaveCompAsCmd : public SaveCompAsCmd {
public:
    OvSaveCompAsCmd(ControlInfo*, OpenFileChooser* = nil);
    OvSaveCompAsCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvSaveCompAsCmd();
    void Init();

    virtual void Execute();
    // prompt for pathname to save document to.

    virtual Command* Copy();
    virtual ClassId GetClassId();

    Component* component() { return comp_; }
    // return pointer to saved component.   

    void pathname(const char*);
    // set pathname to save file to.

 protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);
    virtual boolean IsA(ClassId);

    char* path_;
    Component* comp_;
};

//: derived quit command.
class OvQuitCmd : public QuitCmd {
public:
    OvQuitCmd(ControlInfo*);
    OvQuitCmd(Editor* = nil);

    virtual void Execute();
    // prompt to save document if modified, then quit application.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: derived delete command that can be made irreversable.
class OvDeleteCmd : public DeleteCmd {
public:
    OvDeleteCmd(ControlInfo*, Clipboard* = nil);
    OvDeleteCmd(Editor* = nil, Clipboard* = nil);
    virtual ~OvDeleteCmd();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual boolean Reversable();
    // changeable value.
    void Reversable(boolean);
    // set reversable value.
protected:
    boolean _reversable;
};

//: derived select-all command.
class OvSlctAllCmd : public SlctAllCmd {
public:
    OvSlctAllCmd(ControlInfo*);
    OvSlctAllCmd(Editor* = nil);

    virtual void Execute();
    // create new OverlaySelection with everything in it.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: derived group command.
class OvGroupCmd : public GroupCmd {
public:
    OvGroupCmd(ControlInfo*, OverlayComp* dest = nil);
    OvGroupCmd(Editor* = nil, OverlayComp* dest = nil);

    virtual void Execute();
    // create new OverlaysComp and stuff selection in it.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual OverlaysComp* MakeOverlaysComp();
    // construct an OverlaysComp.

    OverlayComp* GetGroup(); 
    // get OverlaysComp used by execute method.
    void SetGroup(OverlayComp*); 
    // set OverlaysComp to be used by execute method.
};

inline OverlayComp* OvGroupCmd::GetGroup () { return (OverlayComp*) _group; }
inline void OvGroupCmd::SetGroup (OverlayComp* g) { _group = g; }

//: derived new-view command.
class OvNewViewCmd : public NewViewCmd {
public:
    OvNewViewCmd(ControlInfo*, const char* display=nil);
    OvNewViewCmd(Editor* = nil, const char* display=nil);
    virtual ~OvNewViewCmd(); 

    virtual void Execute();
    // create new view by constructing an OverlayEditor and asking
    // the unidraw object to open it.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void set_display();
    // prompt for alternate X11 display to attempt opening new view upon.
    void clr_display();
    // clear alternate X11 display to attempt opening new view upon.

    const char* display();
    // return alternate X11 display string.
    void display(const char*);
    // set alternate X11 display string.

    static OvNewViewCmd* default_instance() { return _default; }
    // return default instance of this command.
    static void default_instance(OvNewViewCmd* cmd)
      { _default = cmd; }
    // set default instance of this command.
    
protected:
    char * _display;

    static OvNewViewCmd* _default;

};

declareActionCallback(OvNewViewCmd)

//: derived close command.
class OvCloseEditorCmd : public CloseEditorCmd {
public:
    OvCloseEditorCmd(ControlInfo*);
    OvCloseEditorCmd(Editor* = nil);

    virtual void Execute();
    // close current viewer, invoking OvSaveAsCmd if needed.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: command to toggle page graphic visibility.
class PageCmd : public Command {
public:
    PageCmd(ControlInfo*);
    PageCmd(Editor* = nil);

    virtual void Execute();
    // toggle visibility of graphic that shows outline of current page.
    virtual boolean Reversible();
    // returns false.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};


//: command to set page size.
class PrecisePageCmd : public Command {
public:
    PrecisePageCmd(ControlInfo*);
    PrecisePageCmd(Editor* = nil);
    virtual ~PrecisePageCmd();

    virtual void Execute();
    // prompt for page width and height, and then recreate page graphic.
    virtual boolean Reversible();
    // returns false.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PageDialog* _dialog;
};

//: command to enable/disable continuous mode drawing with pointer (mouse).
class ScribblePointerCmd : public Command {
public:
    ScribblePointerCmd(ControlInfo*);
    ScribblePointerCmd(Editor* = nil);

    virtual void Execute();
    // toggle current scribble_pointer value.
    virtual boolean Reversible();
    // returns false.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: command to internally tile a PGM or PPM image.
class TileFileCmd : public Command {
public:
    TileFileCmd(ControlInfo*);
    TileFileCmd();
    TileFileCmd(
        Editor*, const char* ifn, const char* ofn, int twidth, int theight
    );
    TileFileCmd(
        ControlInfo*, const char* ifn, const char* ofn, int twidth, int theight
    );
    virtual ~TileFileCmd();

    virtual void Execute();
    // create a new internally tiled PGM or PPM binary image from an
    // existing untiled PGM or PPM binary image.
    virtual boolean Reversible();
    // returns false.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
private:
    const char* _ifn;
    const char* _ofn;
    int _twidth;
    int _theight;
};

//: command to dump current viewer canvas as .xwd file.
class OvWindowDumpAsCmd : public SaveCompAsCmd {
public:
    OvWindowDumpAsCmd(ControlInfo*, OpenFileChooser* = nil);
    OvWindowDumpAsCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvWindowDumpAsCmd();
    void Init();

    virtual void Execute();
    // prompt for pathname, then write contents of current viewer canvas
    // to that file by invoking "xwd" with the appropriate X window id.

    virtual Command* Copy();

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);
};

//: command to create clickable imagemap from viewer canvas.
class OvImageMapCmd : public SaveCompAsCmd {
public:
    OvImageMapCmd(ControlInfo*, OpenFileChooser* = nil);
    OvImageMapCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvImageMapCmd();
    void Init();

    virtual void Execute();
    // prompt for pathname, then export current viewer canvas as a 
    // clickable imagemap, with URL's written out where-ever "url"
    // attributes exist.

    virtual Command* Copy();

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);

  void DumpViews(OverlayView*, ostream&, ostream&);
  void DumpPolys(OverlayView*, ostream&, ostream&, float* ux, float* uy, int unp,
		     int pwidth, int pheight);
  void GetScreenCoords(OverlayViewer* viewer, Graphic* poly,
		       int nf, float* fx, float* fy,
		       int& ni, int*& ix, int*& iy);
};

//: command to push down one level in the graphical structure
class PushCmd : public BackCmd {
public:
    PushCmd(ControlInfo*);
    PushCmd(Editor* = nil);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: command to pull up one level in the graphical structure
class PullCmd : public FrontCmd {
public:
    PullCmd(ControlInfo*);
    PullCmd(Editor* = nil);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
