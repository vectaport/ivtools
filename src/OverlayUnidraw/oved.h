/*
 * Copyright (c) 1994-1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
` * Main class for overlay editor, derived from idraw Editor class.
 */

#ifndef oved_h
#define oved_h

#include <UniIdraw/ided.h>
#include <OverlayUnidraw/ovkit.h>
#include <InterViews/action.h>

class AttributeList;
class ComTerpServ;
class Editor;
class GraphicView;
class Grid;
class ObservableText;
class OverlayComp;
class OverlayPanner;
class OverlayViewer;
class OverlaysView;
class PtrLocState;
class UPage;
class TextObserver;
class Viewer;

//: function pointer to method that constructs an OverlayEditor
// for use in EditorLauncherAction.
typedef OverlayEditor* (*editor_launcher)();

//: action to construct and launch an OverlayEditor.
// can be generalized to constructing and launching an Editor in the future,
// if anyone so desires.
class EditorLauncherAction : public Action {
public:
  EditorLauncherAction(editor_launcher edlauncher) 
    { _editor_launcher = edlauncher; }
  virtual void execute() { (*_editor_launcher)(); }
protected:
  editor_launcher _editor_launcher;
};


//: editor derived from IdrawEditor.
class OverlayEditor : public IdrawEditor {
public:
    OverlayEditor(OverlayComp*, OverlayKit* ok = OverlayKit::Instance());
    // construct an editor for editing a given component, using the 'ok'
    // OverlayKit to build the surrounding menus and tools.
    OverlayEditor(const char* file, OverlayKit* ok = OverlayKit::Instance());
    // construct an editor for opening and editing a given component, specified
    // the pathname 'file, using the 'ok'  OverlayKit to build the surrounding 
    // menus and tools.
    OverlayEditor(boolean initflag, OverlayKit* ok = OverlayKit::Instance());
    // constructor for use of derived classes.  Probably could be protected.
    virtual ~OverlayEditor();

    virtual void Update();
    // update every viewer associated with this editor (usually only one).
    virtual void InitCommands();
    // to be filled in by derived classes, for any kind of initialization
    // that needs to occur after the environment has been fully constructed,
    // i.e. after the viewer has been displayed on the screen (after a call
    // to OverlayViewer::Resize()).

    OverlayViewer* GetOverlayViewer() { return (OverlayViewer*)GetViewer(); }
    // return pointer to default viewer.
    virtual Tool* GetCurTool();
    // return pointer to current default tool.
    virtual void SetCurTool(Tool*);
    // set current default tool.
    virtual void SetComponent(Component*);
    // set new component tree, informing them of their new editor.
    virtual void ReplaceComponent(Component*);
    // set new component tree, informing them of their new editor, and delete old one.

    virtual void Annotate(OverlayComp*);
    // invoke annotation dialog box for this component.
    virtual void AttrEdit(OverlayComp*);
    // invoke dialog box for editing attribute list (property list) associated
    // with this component.

    ObservableText* MouseDocObservable();
    // return pointer to observable text that contains current mouse documentation
    // (a text string associated with the current tool).

    virtual void InformComponents();
    // inform components in the tree of the current editor.

    virtual OverlaysView* GetFrame(int index=-1);
    // return current frame number.  -1 unless multi-frame editor.

    boolean IsClean();
    // checks modified status flag.

    virtual void ResetStateVars();
    // to be filled in by derived classses.

    static int nedlauncher();
    // number of editor launching function pointers in static list.
    static void add_edlauncher(const char* name, editor_launcher);
    // add editor launching function pointer to list.
    static editor_launcher edlauncher(const char *);
    // get editor launching function pointer by 'name'.
    static editor_launcher edlauncher(int symid);
    // get editor launching function pointer by 'symid'.
    static AttributeList* edlauncherlist() { return _edlauncherlist; }
    // return pointer to static list of editor launching function pointers.
    
    static int ncomterp();
    // number of ComTerpServ objects in static list.
    static void add_comterp(const char* name, ComTerpServ* comterp);
    // add ComTerpServ to static list.
    static ComTerpServ* comterp(const char *);
    // get ComTerpServ by 'name' from static list.
    static ComTerpServ* comterp(int symid);
    // get ComTerpServ by 'symid' from static list.
    static AttributeList* comterplist() { return _comterplist; }
    // return pointer to static list of ComTerpServ objects.
    
    virtual void DoAutoNewFrame() { };
    // empty method for use by multi-frame editors for creating a new frame
    // when auto-new-frame is enabled.

    static boolean opaque_flag();
    // return global flag indicating whether opaque tranformations are enabled.

    PtrLocState* ptrlocstate() { return _ptrlocstate; }
    // state variable for displaying pointer location within document.

    void ptrlocstate(PtrLocState* ptrlocstate) { _ptrlocstate = ptrlocstate; }
    // set state variable for displaying pointer location within document.

    virtual void ExecuteCmd(Command* cmd);
    // indirect command execution for distributed whiteboard mechanism.
    // actual mechanism implemented in ComEditor.

protected:
    void Init(OverlayComp* = nil, const char* = "OverlayEditor");
    // construct empty component tree if necessary, and pass to
    // OverlayKit for the rest of initialization process.

    Interactor* Interior();
    // lay out the Interactor based interior of a drawing editor.
    OverlayPanner* make_panner();
    // make panner/zoomer/slider buttons, paying attention to 
    // command line arguments/default resources for positioning and enable/disable 
    // hints.
    int panner_align();
    // handle -panner_align or -pal command line argument.

protected: 
    OverlayKit* _overlay_kit;
    Tool* _curtool;
    ObservableText* _mousedoc;
    PtrLocState* _ptrlocstate;
    static AttributeList* _edlauncherlist;
    static AttributeList* _comterplist;

friend OverlayKit;
};

inline ObservableText* OverlayEditor::MouseDocObservable() { return _mousedoc; }

#endif
