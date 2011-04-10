/*
 * Copyright (c) 1994-1998 Vectaport Inc.
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
class UPage;
class TextObserver;
class Viewer;

typedef OverlayEditor* (*editor_launcher)();

class EditorLauncherAction : public Action {
public:
  EditorLauncherAction(editor_launcher edlauncher) 
    { _editor_launcher = edlauncher; }
  virtual void execute() { (*_editor_launcher)(); }
protected:
  editor_launcher _editor_launcher;
};


class OverlayEditor : public IdrawEditor {
public:
    OverlayEditor(OverlayComp*, OverlayKit* ok = OverlayKit::Instance());
    OverlayEditor(const char* file, OverlayKit* ok = OverlayKit::Instance());
    OverlayEditor(boolean initflag, OverlayKit* ok = OverlayKit::Instance());
    virtual ~OverlayEditor();

    virtual void Update();
    virtual void InitCommands();

    OverlayViewer* GetOverlayViewer() { return (OverlayViewer*)GetViewer(); }
    virtual Tool* GetCurTool();
    virtual void SetCurTool(Tool*);
    virtual void SetComponent(Component*);

    virtual void Annotate(OverlayComp*);
    virtual void AttrEdit(OverlayComp*);

    void MouseDoc(const char*);
    ObservableText* MouseDocObservable();

    virtual void InformComponents();

    virtual OverlaysView* GetFrame(int index=-1);

    boolean IsClean();

    virtual void ResetStateVars();

    static int nedlauncher();
    static void add_edlauncher(const char* name, editor_launcher);
    static editor_launcher edlauncher(const char *);
    static editor_launcher edlauncher(int symid);
    static AttributeList* edlauncherlist() { return _edlauncherlist; }
    
    static int ncomterp();
    static void add_comterp(const char* name, ComTerpServ* comterp);
    static ComTerpServ* comterp(const char *);
    static ComTerpServ* comterp(int symid);
    static AttributeList* comterplist() { return _comterplist; }
    
protected:
    void Init(OverlayComp* = nil, const char* = "OverlayEditor");

    Interactor* Interior();
    OverlayPanner* make_panner();
    int panner_align();

protected: 
    OverlayKit* _overlay_kit;
    Tool* _curtool;
    ObservableText* _mousedoc;
    static AttributeList* _edlauncherlist;
    static AttributeList* _comterplist;

friend OverlayKit;
};

inline ObservableText* OverlayEditor::MouseDocObservable() { return _mousedoc; }

#endif
