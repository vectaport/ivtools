/*
 * Copyright (c) 1998 VectaporT Inc.
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
class ostream;

class OvNewCompCmd : public NewCompCmd {
public:
    OvNewCompCmd(ControlInfo*, Component* prototype = nil);
    OvNewCompCmd(Editor* = nil, Component* prototype = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class OvRevertCmd : public RevertCmd {
public:
    OvRevertCmd(ControlInfo*);
    OvRevertCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class OvViewCompCmd : public ViewCompCmd {
public:
    OvViewCompCmd(ControlInfo*, OpenFileChooser* = nil);
    OvViewCompCmd(Editor* = nil, OpenFileChooser* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    OpenFileChooser* chooser_;
};

class OvOpenCmd : public OvViewCompCmd {
public:
    OvOpenCmd(ControlInfo*, OpenFileChooser* = nil);
    OvOpenCmd(Editor* = nil, OpenFileChooser* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class OvSaveCompCmd : public SaveCompCmd {
public:
    OvSaveCompCmd(ControlInfo*, OpenFileChooser* = nil);
    OvSaveCompCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvSaveCompCmd();
    void Init();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);
};

class OvSaveCompAsCmd : public SaveCompAsCmd {
public:
    OvSaveCompAsCmd(ControlInfo*, OpenFileChooser* = nil);
    OvSaveCompAsCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvSaveCompAsCmd();
    void Init();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);
    virtual boolean IsA(ClassId);
};

class OvQuitCmd : public QuitCmd {
public:
    OvQuitCmd(ControlInfo*);
    OvQuitCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class OvDeleteCmd : public DeleteCmd {
public:
    OvDeleteCmd(ControlInfo*, Clipboard* = nil);
    OvDeleteCmd(Editor* = nil, Clipboard* = nil);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual boolean Reversable();
    void Reversable(boolean);
protected:
    boolean _reversable;
};

class OvSlctAllCmd : public SlctAllCmd {
public:
    OvSlctAllCmd(ControlInfo*);
    OvSlctAllCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class OvGroupCmd : public GroupCmd {
public:
    OvGroupCmd(ControlInfo*, OverlayComp* dest = nil);
    OvGroupCmd(Editor* = nil, OverlayComp* dest = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual OverlaysComp* MakeOverlaysComp();

    OverlayComp* GetGroup();
    void SetGroup(OverlayComp*);
};

inline OverlayComp* OvGroupCmd::GetGroup () { return (OverlayComp*) _group; }
inline void OvGroupCmd::SetGroup (OverlayComp* g) { _group = g; }

class OvNewViewCmd : public NewViewCmd {
public:
    OvNewViewCmd(ControlInfo*, const char* display=nil);
    OvNewViewCmd(Editor* = nil, const char* display=nil);
    virtual ~OvNewViewCmd(); 

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void set_display();
    void clr_display();

    const char* display();
    void display(const char*);

    static OvNewViewCmd* default_instance() { return _default; }
    static void OvNewViewCmd::default_instance(OvNewViewCmd* cmd)
      { _default = cmd; }
    
protected:
    char * _display;

    static OvNewViewCmd* _default;

};

declareActionCallback(OvNewViewCmd)

class OvCloseEditorCmd : public CloseEditorCmd {
public:
    OvCloseEditorCmd(ControlInfo*);
    OvCloseEditorCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class PageCmd : public Command {
public:
    PageCmd(ControlInfo*);
    PageCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class PrecisePageCmd : public Command {
public:
    PrecisePageCmd(ControlInfo*);
    PrecisePageCmd(Editor* = nil);
    virtual ~PrecisePageCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PageDialog* _dialog;
};

class ScribblePointerCmd : public Command {
public:
    ScribblePointerCmd(ControlInfo*);
    ScribblePointerCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

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
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
private:
    const char* _ifn;
    const char* _ofn;
    int _twidth, _theight;
};

class OvWindowDumpAsCmd : public SaveCompAsCmd {
public:
    OvWindowDumpAsCmd(ControlInfo*, OpenFileChooser* = nil);
    OvWindowDumpAsCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvWindowDumpAsCmd();
    void Init();

    virtual void Execute();

    virtual Command* Copy();

protected:
    OpenFileChooser* chooser_;

    void Init(OpenFileChooser*);
};

class OvImageMapCmd : public SaveCompAsCmd {
public:
    OvImageMapCmd(ControlInfo*, OpenFileChooser* = nil);
    OvImageMapCmd(Editor* = nil, OpenFileChooser* = nil);
    ~OvImageMapCmd();
    void Init();

    virtual void Execute();

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

#endif
