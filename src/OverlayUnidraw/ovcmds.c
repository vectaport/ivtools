/*
 * Copyright (c) 1998 Vectaport Inc.
 * Copyright (c) 1994-1995 Vectaport Inc.
 * Copyright (c) 1995 Cider Press
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
 * Implementation of overlay specific commands.
 */

#include <OverlayUnidraw/clippoly.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovdialog.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovimport.h>

#include <IVGlyph/saveaschooser.h>
#include <IVGlyph/gdialogs.h>
#include <IVGlyph/stredit.h>

#include <Unidraw/Components/grview.h>
#include <Unidraw/Graphic/geomobjs.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/grid.h>
#include <Unidraw/iterator.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>
#include <Unidraw/globals.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>

#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/session.h>
#include <InterViews/transformer.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <IV-X11/xwindow.h>

#include <IV-2_6/InterViews/perspective.h>


#include <OS/math.h>
#include <OS/string.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <TopoFace/fgeomobjs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>

/*****************************************************************************/

static const float GRID_XINCR = 8;                 // default grid spacing
static const float GRID_YINCR = 8;

static void UpdateCompNameVars () {
    Iterator i;

    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
        Editor* ed = unidraw->GetEditor(i);
        CompNameVar* compNameVar = (CompNameVar*) ed->GetState("CompNameVar");
        if (compNameVar != nil) compNameVar->UpdateName();
    }
}

static boolean Writable (Component* comp) {
    Catalog* catalog = unidraw->GetCatalog();
    const char* name = catalog->GetName(comp);
    return name == nil || (catalog->Exists(name) && catalog->Writable(name));
}

static boolean OnlyOneEditorOf (Component* c) {
    Component* comp = c->GetRoot();
    Iterator i;
    int count = 0;

    for (unidraw->First(i); !unidraw->Done(i) && count < 2; unidraw->Next(i)) {
        Component* test_comp = unidraw->GetEditor(i)->GetComponent();

        if (test_comp != nil && test_comp->GetRoot() == comp) {
            ++count;
        }
    }
    return count == 1;
}

static boolean ReadyToClose (Editor* ed) {
    ModifStatusVar* mv = (ModifStatusVar*) ed->GetState("ModifStatusVar");

    if (mv != nil && Writable(mv->GetComponent()) && mv->GetModifStatus()) {
        GConfirmDialog* dialog = new GConfirmDialog("Save changes?");
	Resource::ref(dialog);

	boolean result = dialog->post_for(ed->GetWindow());
	if (result) {
            OvSaveCompCmd saveComp(ed);
            saveComp.Execute();

            if (mv->GetModifStatus()) {
                return false;                   // save dialog was aborted
	    }
	}
	else if (dialog->cancel()) {
	    return false;
	}
	Resource::unref(dialog);
    }
    return true;
}

/*****************************************************************************/

ClassId OvNewCompCmd::GetClassId () { return OVNEWCOMP_CMD; }

boolean OvNewCompCmd::IsA (ClassId id) {
    return OVNEWCOMP_CMD == id || NewCompCmd::IsA(id);
}

OvNewCompCmd::OvNewCompCmd (ControlInfo* c, Component* p) : NewCompCmd(c, p) { 
}

OvNewCompCmd::OvNewCompCmd (Editor* ed, Component* p) : NewCompCmd(ed, p) { 
}

Command* OvNewCompCmd::Copy () {
    Command* copy = new OvNewCompCmd(CopyControlInfo(), prototype_->Copy());
    InitCopy(copy);
    return copy;
}

void OvNewCompCmd::Execute () {
    Editor* ed = GetEditor();
    Component* orig = ed->GetComponent();
    Component* comp = prototype_->Copy();
    CompNameVar* compNameVar = (CompNameVar*) ed->GetState("CompNameVar");
    ModifStatusVar* modifVar = (ModifStatusVar*)ed->GetState("ModifStatusVar");

    if (OnlyOneEditorOf(orig) && !ReadyToClose(ed)) {
        return;
    }

    if (compNameVar != nil) compNameVar->SetComponent(comp);
    if (modifVar != nil) modifVar->SetComponent(comp);

    ((OverlayEditor*)ed)->ResetStateVars();
    ed->SetComponent(comp);
    ed->Update();

    if (orig != nil && unidraw->FindAny(orig) == nil) {
        Component* root = orig->GetRoot();
        delete root;
    }
}

/*****************************************************************************/

ClassId OvRevertCmd::GetClassId () { return OVREVERT_CMD; }

boolean OvRevertCmd::IsA (ClassId id) {
    return OVREVERT_CMD == id || RevertCmd::IsA(id);
}

OvRevertCmd::OvRevertCmd (ControlInfo* c) : RevertCmd(c) { }
OvRevertCmd::OvRevertCmd (Editor* ed) : RevertCmd(ed) { }

Command* OvRevertCmd::Copy () {
    Command* copy = new OvRevertCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvRevertCmd::Execute () {
    Editor* ed = GetEditor();
    Component* comp = ed->GetComponent();
    Catalog* catalog = unidraw->GetCatalog();
    const char* name = catalog->GetName(comp);
    ModifStatusVar* mv = (ModifStatusVar*) ed->GetState("ModifStatusVar");

    if (name != nil && (mv == nil || mv->GetModifStatus())) {
        char buf[CHARBUFSIZE];
        strcpy(buf, name);

        GConfirmDialog* dialog = new GConfirmDialog("Really revert to last version saved?");
	Resource::ref(dialog);
	boolean result = dialog->post_for(ed->GetWindow());
	if (result) {
            Component* orig = comp;
            catalog->Forget(orig);
	   
	    GetEditor()->GetWindow()->cursor(hourglass);
            if (unidraw->GetCatalog()->Retrieve(buf, comp)) {
                ed->SetComponent(comp);
                unidraw->CloseDependents(orig);
                unidraw->Update();

                CompNameVar* cv = (CompNameVar*) ed->GetState("CompNameVar");

                if (cv != nil) cv->SetComponent(comp);
                if (mv != nil) mv->SetComponent(comp);

                Component* root = orig->GetRoot();
                delete root;

            } else {
		GetEditor()->GetWindow()->cursor(arrow);
		GConfirmDialog* dialog2 = new GConfirmDialog(
		    "Couldn't revert! (File nonexistent?)," "Save changes?");
		Resource::ref(dialog2);

                UpdateCompNameVars();
                if (mv != nil) mv->Notify();

		boolean result = dialog2->post_for(ed->GetWindow());
		if (result) {
                    OvSaveCompAsCmd saveCompAs(ed);
                    saveCompAs.Execute();
                }
		Resource::unref(dialog2);
            }
        }
	Resource::unref(dialog);
    }
}

/*****************************************************************************/

ClassId OvViewCompCmd::GetClassId () { return OVVIEWCOMP_CMD; }

boolean OvViewCompCmd::IsA (ClassId id) {
    return OVVIEWCOMP_CMD == id || ViewCompCmd::IsA(id);
}

OvViewCompCmd::OvViewCompCmd (ControlInfo* c, OpenFileChooser* fc) : ViewCompCmd(c, nil) {
    chooser_ = fc;
    Resource::ref(chooser_);
}

OvViewCompCmd::OvViewCompCmd (Editor* ed, OpenFileChooser* fc) : ViewCompCmd(ed, nil) {
    chooser_ = fc;
    Resource::ref(chooser_);
}

Command* OvViewCompCmd::Copy () {
    Command* copy = new OvViewCompCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvViewCompCmd::Execute () {
    Editor* ed = GetEditor();

    if (OnlyOneEditorOf(ed->GetComponent()) && !ReadyToClose(ed)) {
        return;
    }

    Style* style;
    boolean reset_caption = false;
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style);
	Resource::ref(chooser_);
	char buf[CHARBUFSIZE];
	const char* domain = unidraw->GetCatalog()->GetAttribute("domain");
	domain = (domain == nil) ? "component" : domain;
	sprintf(buf, "Select a %s to open:", domain);
	style->attribute("caption", "            ");
	style->attribute("subcaption", buf);
    } else {
	style = chooser_->style();
    }
    boolean again;
    while ( again = chooser_->post_for(ed->GetWindow())) {
	
	style->attribute("caption", "            ");
	chooser_->twindow()->repair();
	chooser_->twindow()->display()->sync();

        const String* s = chooser_->selected();
	NullTerminatedString ns(*s);
	const char* name = ns.string();
        Catalog* catalog = unidraw->GetCatalog();
        GraphicComp* comp;

	ed->GetWindow()->cursor(hourglass);
	chooser_->twindow()->cursor(hourglass);
        if (catalog->Retrieve(name, (Component*&) comp)) {
	    ModifStatusVar* modif = (ModifStatusVar*) ed->GetState(
		"ModifStatusVar"
	    );
            Component* orig = ed->GetComponent();
            ed->SetComponent(comp);
            unidraw->Update();

            StateVar* sv = ed->GetState("CompNameVar");
            CompNameVar* cnv = (CompNameVar*) sv;

            if (cnv != nil) cnv->SetComponent(comp);
            if (modif != nil) modif->SetComponent(comp);

            if (orig != nil && unidraw->FindAny(orig) == nil) {
                Component* root = orig->GetRoot();
                delete root;
            }
	    break;
        } else {
	    style->attribute("caption", "Open failed!" );
	    reset_caption = true;
	    ed->GetWindow()->cursor(arrow);
	    chooser_->twindow()->cursor(arrow);
        }
    }
    chooser_->unmap();
    if (reset_caption) {
	style->attribute("caption", "            ");
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
}

/*****************************************************************************/

ClassId OvOpenCmd::GetClassId () { return OVOPEN_CMD; }

boolean OvOpenCmd::IsA (ClassId id) {
    return OVOPEN_CMD == id || OvViewCompCmd::IsA(id);
}

OvOpenCmd::OvOpenCmd (ControlInfo* c, OpenFileChooser* fc) : OvViewCompCmd(c, fc) { }
OvOpenCmd::OvOpenCmd (Editor* ed, OpenFileChooser* fc) : OvViewCompCmd(ed, fc) { }

Command* OvOpenCmd::Copy () {
    Command* copy = new OvOpenCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvOpenCmd::Execute () {
    Editor* ed = GetEditor();
    Component* orig_comp = ed->GetComponent();
    OvViewCompCmd::Execute();
    Component* new_comp = ed->GetComponent();

    if (new_comp != orig_comp) {
        Grid* grid = ed->GetViewer()->GetGrid();
        grid->SetSpacing(GRID_XINCR, GRID_YINCR);
    }
}

/*****************************************************************************/

ClassId OvSaveCompCmd::GetClassId () { return OVSAVECOMP_CMD; }

boolean OvSaveCompCmd::IsA (ClassId id) {
    return OVSAVECOMP_CMD == id || SaveCompCmd::IsA(id);
}

OvSaveCompCmd::OvSaveCompCmd (ControlInfo* c, OpenFileChooser* f) : SaveCompCmd(c) { Init(f);}
OvSaveCompCmd::OvSaveCompCmd (Editor* ed, OpenFileChooser* f) : SaveCompCmd(ed) {Init(f);}
OvSaveCompCmd::~OvSaveCompCmd() {
    Resource::unref(chooser_);
}

void OvSaveCompCmd::Init (OpenFileChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvSaveCompCmd::Copy () {
    Command* copy = new OvSaveCompCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvSaveCompCmd::Execute () {
    Editor* ed = GetEditor();
    ModifStatusVar* modifVar = (ModifStatusVar*)ed->GetState("ModifStatusVar");
    CompNameVar* compNameVar = (CompNameVar*) ed->GetState("CompNameVar");
    const char* name = (compNameVar == nil) ? nil : compNameVar->GetName();

    if (name == nil) {
        OvSaveCompAsCmd saveCompAs(ed, chooser_);
        saveCompAs.Execute();

    } else if (modifVar == nil || modifVar->GetModifStatus()) {
        Catalog* catalog = unidraw->GetCatalog();
        Component* comp;

        if (catalog->Retrieve(name, comp) && catalog->Save(comp, name)) {
            if (modifVar != nil) modifVar->SetModifStatus(false);
            unidraw->ClearHistory(comp);

        } else {
            OvSaveCompAsCmd saveCompAs(ed, chooser_);
            saveCompAs.Execute();
        }
    }
}

/*****************************************************************************/

ClassId OvSaveCompAsCmd::GetClassId () { return OVSAVECOMPAS_CMD; }

boolean OvSaveCompAsCmd::IsA (ClassId id) {
    return OVSAVECOMPAS_CMD == id || SaveCompAsCmd::IsA(id);
}

OvSaveCompAsCmd::OvSaveCompAsCmd (ControlInfo* c, OpenFileChooser* fc) : SaveCompAsCmd(c) {
    Init(fc);
}

OvSaveCompAsCmd::OvSaveCompAsCmd (Editor* ed, OpenFileChooser* fc) : SaveCompAsCmd(ed) {
    Init(fc);
}

OvSaveCompAsCmd::~OvSaveCompAsCmd() {
    Resource::unref(chooser_);
}

void OvSaveCompAsCmd::Init (OpenFileChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvSaveCompAsCmd::Copy () {
    Command* copy = new OvSaveCompAsCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvSaveCompAsCmd::Execute () {
    Editor* ed = GetEditor();

    char buf[CHARBUFSIZE];
    const char* domain = unidraw->GetCatalog()->GetAttribute("domain");
    domain = (domain == nil) ? "component" : domain;
    sprintf(buf, "Save this %s as:", domain);

    boolean reset_caption = false;
    Style* style = new Style(Session::instance()->style());
    style->attribute("subcaption", buf);
    style->attribute("open", "Save");
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Save to file:");
	style->attribute("open", "Save");
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style);
	Resource::ref(chooser_);
    }
    boolean again;
    while (again = chooser_->post_for(ed->GetWindow())) {
	const String* str = chooser_->selected();
	NullTerminatedString ns(*str);
        const char* name = ns.string();
        OverlayCatalog* catalog = (OverlayCatalog*)unidraw->GetCatalog();
        boolean ok = true;
        style->attribute("caption", "                     " );
	chooser_->twindow()->repair();
	chooser_->twindow()->display()->sync();

        if (catalog->Exists(name) && catalog->Writable(name)) {
            char buf[CHARBUFSIZE];
            sprintf(buf, "\"%s\" already exists.", name);
	    GConfirmDialog* dialog = new GConfirmDialog(buf, "Overwrite?");
	    Resource::ref(dialog);

	    ok = dialog->post_for(ed->GetWindow());
	    Resource::unref(dialog);
        }
        if (ok) {
            CompNameVar* cnv = (CompNameVar*) ed->GetState("CompNameVar");
            const char* oldname = (cnv == nil) ? nil : cnv->GetName();
            Component* comp = ed->GetComponent();

            if (catalog->Exists(name) && !catalog->Writable(name)) {
		style->attribute("caption", "Couldn't save to file!" );
            } else {
                if (oldname == nil) {
                    comp = comp->GetRoot();
                } else {
                    catalog->Retrieve(oldname, comp);
                    catalog->Forget(comp);
                }

                StateVar* sv = ed->GetState("ModifStatusVar");
                ModifStatusVar* mv = (ModifStatusVar*) sv;

		if (chooser_->saveas_chooser()) {
		    SaveAsChooser* sac = (SaveAsChooser*)chooser_;
		    catalog->SetCompactions(sac->gs_compacted(), 
					     sac->pts_compacted(),
					     sac->pic_compacted());
		}

		ed->GetWindow()->cursor(hourglass);
		chooser_->twindow()->cursor(hourglass);
                if (catalog->Save(comp, name)) {
                    if (mv != nil) mv->SetModifStatus(false);
                    unidraw->ClearHistory(comp);
                    UpdateCompNameVars();
		    ed->GetWindow()->cursor(arrow);
		    break;
                } else {
                    if (mv != nil) mv->Notify();
                    UpdateCompNameVars();
		    style->attribute("caption", "Couldn't save to file" );
		    reset_caption = true;
		    ed->GetWindow()->cursor(arrow);
		    chooser_->twindow()->cursor(arrow);
                }
            } 
        }
    }
    chooser_->unmap();
    if (reset_caption) {
        style->attribute("caption", "                     " );
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
}

/*****************************************************************************/

ClassId OvQuitCmd::GetClassId () { return OVQUIT_CMD; }
boolean OvQuitCmd::IsA (ClassId id) { return OVQUIT_CMD == id || QuitCmd::IsA(id);}
OvQuitCmd::OvQuitCmd (ControlInfo* c) : QuitCmd(c) { }
OvQuitCmd::OvQuitCmd (Editor* ed) : QuitCmd(ed) { }

Command* OvQuitCmd::Copy () {
    Command* copy = new OvQuitCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvQuitCmd::Execute () {
    Editor* ed = GetEditor();
    
    if (ReadyToClose(ed)) {
        Component* comp = ed->GetComponent();

        if (comp == nil) {
            unidraw->Close(ed);
        } else {
            unidraw->CloseDependents(comp->GetRoot());
        }
        Iterator i;

        for (;;) {
            unidraw->First(i);

            if (unidraw->Done(i)) {
                break;
            }

            ed = unidraw->GetEditor(i);

            if (ReadyToClose(ed)) {
                comp = ed->GetComponent();

                if (comp == nil) {
                    unidraw->Close(ed);
                } else {
                    unidraw->CloseDependents(comp->GetRoot());
                }
            } else {
                return;
            }
        }
        unidraw->Quit();
    }
}

/*****************************************************************************/

ClassId OvDeleteCmd::GetClassId () { return OV_DELETE_CMD; }

boolean OvDeleteCmd::IsA (ClassId id) {
    return OV_DELETE_CMD == id || DeleteCmd::IsA(id);
}

OvDeleteCmd::OvDeleteCmd (ControlInfo* c, Clipboard* cb) : DeleteCmd(c,cb) {
    _reversable = true;
}

OvDeleteCmd::OvDeleteCmd (Editor* ed, Clipboard* cb) : DeleteCmd(ed, cb) {
    _reversable = true;
}

Command* OvDeleteCmd::Copy () {
    Command* copy = new OvDeleteCmd(CopyControlInfo());
    InitCopy(copy);
    ((OvDeleteCmd*)copy)->Reversable(Reversable());
    return copy;
}

boolean OvDeleteCmd::Reversable() { return _reversable; }

void OvDeleteCmd::Reversable(boolean flag) { _reversable = flag;  }

/*****************************************************************************/

ClassId OvSlctAllCmd::GetClassId () { return OVSLCTALL_CMD; }

boolean OvSlctAllCmd::IsA (ClassId id) {
    return OVSLCTALL_CMD == id || SlctAllCmd::IsA(id);
}

OvSlctAllCmd::OvSlctAllCmd (ControlInfo* c) : SlctAllCmd(c) { }
OvSlctAllCmd::OvSlctAllCmd (Editor* ed) : SlctAllCmd(ed) { }

Command* OvSlctAllCmd::Copy () {
    Command* copy = new OvSlctAllCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvSlctAllCmd::Execute () {
    Editor* editor = GetEditor();
    OverlaySelection* newSel = new OverlaySelection;
    Selection* s;
    Viewer* viewer;

    s = editor->GetSelection();
    delete s;
    for (int i = 0; (viewer = editor->GetViewer(i)) != nil; ++i) {
        s = viewer->GetGraphicView()->SelectAll();
        newSel->Merge(s);
        delete s;
    }
    editor->SetSelection(newSel);
}

/*****************************************************************************/

ClassId OvGroupCmd::GetClassId () { return OVGROUP_CMD; }

boolean OvGroupCmd::IsA (ClassId id) {
    return OVGROUP_CMD == id || GroupCmd::IsA(id);
}

OvGroupCmd::OvGroupCmd (ControlInfo* c, OverlayComp* d) : GroupCmd(c) { }
OvGroupCmd::OvGroupCmd (Editor* ed, OverlayComp* d) : GroupCmd(ed) { }

Command* OvGroupCmd::Copy () {
    OverlayComp* dest = (_group == nil) ? nil : (OverlayComp*) _group->Copy();
    Command* copy = new OvGroupCmd(CopyControlInfo(), dest);
    InitCopy(copy);
    return copy;
}

void OvGroupCmd::Execute () {
    Clipboard* cb = GetClipboard();

    if (cb == nil) {
        SetClipboard(cb = new Clipboard);
        Editor* ed = GetEditor();
        Selection* s = ed->GetSelection();

        if (s->Number() > 1) {
            Iterator i;
            GraphicView* views = ed->GetViewer()->GetGraphicView();
            s->Sort(views);

            for (s->First(i); !s->Done(i); s->Next(i)) {
                s->GetView(i)->Interpret(this);
            }
        }

    } else {
        Clipboard* oldcb = cb;
        SetClipboard(cb = new Clipboard);

        Iterator i;
        for (oldcb->First(i); !oldcb->Done(i); oldcb->Next(i)) {
            oldcb->GetComp(i)->Interpret(this);
        }
        delete oldcb;
    }

    if (!cb->IsEmpty()) {
        if (_group == nil) {
            SetGroup(MakeOverlaysComp());
        }
        _group->Interpret(this);
        _executed = true;
    }
}

OverlaysComp* OvGroupCmd::MakeOverlaysComp() {
    return new OverlaysComp;
}

/*****************************************************************************/

OvNewViewCmd* OvNewViewCmd::_default = nil;

ClassId OvNewViewCmd::GetClassId () { return OVNEWVIEW_CMD; }

boolean OvNewViewCmd::IsA (ClassId id) {
    return OVNEWVIEW_CMD == id || NewViewCmd::IsA(id);
}

OvNewViewCmd::OvNewViewCmd (ControlInfo* c, const char* display) 
: NewViewCmd(c) 
{ 
  _display = display ? strdup(display) : nil; 
}

OvNewViewCmd::OvNewViewCmd (Editor* ed, const char* display) 
: NewViewCmd(ed) 
{ 
  _display = display ?  strdup(display) : nil;
}

OvNewViewCmd::~OvNewViewCmd() { 
  delete _display;
}


Command* OvNewViewCmd::Copy () {
    OvNewViewCmd* copy = new OvNewViewCmd(CopyControlInfo());
    InitCopy(copy);
    copy->display(_display);
    return copy;
}

void OvNewViewCmd::Execute () {
    Editor* ed = GetEditor();
    OverlayKit kit;
    if (_display) 
      kit.otherdisplay(_display);
    Editor* newEd = new OverlayEditor((OverlayComp*)GetGraphicComp(), &kit);

    *newEd->GetState("ModifStatusVar") = *ed->GetState("ModifStatusVar");

    ed->GetWindow()->cursor(hourglass);
    unidraw->Open(newEd);
    ed->GetWindow()->cursor(arrow);
}

void OvNewViewCmd::clr_display() {
  delete _display;
  _display = nil;
}

void OvNewViewCmd::set_display() {
  char* newdisplay =
    StrEditDialog::post
    (GetEditor()->GetWindow(), 
     "Enter display name, i.e. \"hostname:0.0\"",  
     _display ? _display : "localhost:0.0");
  delete _display;
  _display = newdisplay;
}

const char* OvNewViewCmd::display() { return _display; }
void OvNewViewCmd::display(const char* display ) 
{ delete _display; 
  _display = display ? strdup(display) : nil; }

implementActionCallback(OvNewViewCmd)

/*****************************************************************************/

ClassId OvCloseEditorCmd::GetClassId () { return OVCLOSEEDITOR_CMD; }

boolean OvCloseEditorCmd::IsA (ClassId id) {
    return OVCLOSEEDITOR_CMD == id || CloseEditorCmd::IsA(id);
}

OvCloseEditorCmd::OvCloseEditorCmd (ControlInfo* c) : CloseEditorCmd(c) { }
OvCloseEditorCmd::OvCloseEditorCmd (Editor* ed) : CloseEditorCmd(ed) { }

Command* OvCloseEditorCmd::Copy () {
    Command* copy = new OvCloseEditorCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

static boolean FoundAnyExcept (Editor* ed) {
    Component* comp = ed->GetComponent()->GetRoot();
    Iterator i;

    for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
        Editor* test_ed = unidraw->GetEditor(i);

        if (test_ed != ed) {
            Component* test_comp = test_ed->GetComponent();

            if (test_comp != nil && test_comp->GetRoot() == comp) {
                return true;
            }
        }
    }
    return false;
}

void OvCloseEditorCmd::Execute () {
    Editor* ed = GetEditor();
    Iterator i;
    unidraw->First(i);
    unidraw->Next(i);

    if (!unidraw->Done(i)) {
	ModifStatusVar* mv = (ModifStatusVar*) ed->GetState("ModifStatusVar");

	if (mv != nil && mv->GetModifStatus() && !FoundAnyExcept(ed)) {
	    GConfirmDialog* dialog = new GConfirmDialog("Save changes?");
	    Resource::ref(dialog);

	    boolean result = dialog->post_for(ed->GetWindow());
	    if (result) {
		OvSaveCompCmd saveComp(ed);
		saveComp.Execute();

                if (mv->GetModifStatus()) {
                    return;                         // save dialog was aborted
                }
	    }
	    else if (dialog->cancel()) {
		return;
	    }
	    Resource::unref(dialog);
	}
        unidraw->Close(ed);
    }
}

/*****************************************************************************/

ClassId PageCmd::GetClassId () { return PAGE_CMD; }

boolean PageCmd::IsA (ClassId id) {
    return PAGE_CMD == id || Command::IsA(id);
}

PageCmd::PageCmd (ControlInfo* c) : Command(c) { }
PageCmd::PageCmd (Editor* ed) : Command(ed) { }

Command* PageCmd::Copy () {
    Command* copy = new PageCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void PageCmd::Execute () {
    Viewer* viewer;

    for (int i = 0; (viewer = GetEditor()->GetViewer(i)) != nil; ++i) {
	OverlayPage* ovpage = (OverlayPage*)viewer->GetPage();

	if (ovpage != nil) ovpage->Visibility(!ovpage->IsVisible());
	viewer->Draw();
    }
}

boolean PageCmd::Reversible () { return false; }

/*****************************************************************************/

ClassId PrecisePageCmd::GetClassId () { return PRECISEPAGE_CMD; }

boolean PrecisePageCmd::IsA (ClassId id) {
    return PRECISEPAGE_CMD == id || Command::IsA(id);
}

PrecisePageCmd::PrecisePageCmd (ControlInfo* c) : Command(c) { _dialog = nil;}
PrecisePageCmd::PrecisePageCmd (Editor* ed) : Command(ed) { _dialog = nil;}

PrecisePageCmd::~PrecisePageCmd () { 
    if (_dialog)
        delete _dialog; 
}

Command* PrecisePageCmd::Copy () {
    Command* copy = new PrecisePageCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void PrecisePageCmd::Execute () {
    float x = 0.0, y = 0.0;
    Editor* ed = GetEditor();

    if (_dialog == nil) {
 	_dialog = new PageDialog();
    }

    ed->InsertDialog(_dialog);
    boolean accepted = _dialog->Accept();
    ed->RemoveDialog(_dialog);

    if (accepted) {
	_dialog->GetValues(x, y);
	Viewer* viewer = GetEditor()->GetViewer();
	viewer->SetPage(new OverlayPage(x, y, true));
	viewer->Update();
    }
}

boolean PrecisePageCmd::Reversible () { return false; }

/*****************************************************************************/

ClassId ScribblePointerCmd::GetClassId () { return SCRIBBLE_POINTER_CMD; }

boolean ScribblePointerCmd::IsA (ClassId id) {
    return SCRIBBLE_POINTER_CMD == id || Command::IsA(id);
}

ScribblePointerCmd::ScribblePointerCmd (ControlInfo* c) : Command(c) { }
ScribblePointerCmd::ScribblePointerCmd (Editor* ed) : Command(ed) { }

Command* ScribblePointerCmd::Copy () {
    Command* copy = new ScribblePointerCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void ScribblePointerCmd::Execute () {
    OverlayViewer* viewer =  (OverlayViewer*)GetEditor()->GetViewer();
    viewer->scribble_pointer(!viewer->scribble_pointer());
}

boolean ScribblePointerCmd::Reversible () { return false; }

/*****************************************************************************/

ClassId TileFileCmd::GetClassId () { return TILEFILE_CMD; }

boolean TileFileCmd::IsA (ClassId id) {
    return TILEFILE_CMD == id || Command::IsA(id);
}

TileFileCmd::TileFileCmd (ControlInfo* c) 
    : Command(c), _ifn(nil), _ofn(nil) { 
}

TileFileCmd::TileFileCmd () 
    : Command((Editor*)nil), _ifn(nil), _ofn(nil) { 
}

TileFileCmd::TileFileCmd (
    Editor* ed, const char* ifn, const char* ofn, int twidth, int theight
) 
    : Command(ed), _ifn(nil), _ofn(nil), _twidth(twidth), _theight(theight) 
{ 
    _ifn = strdup(ifn);
    _ofn = strdup(ofn);
}

TileFileCmd::TileFileCmd (
    ControlInfo* c, const char* ifn, const char* ofn, int twidth, int theight
) 
    : Command(c), _ifn(nil), _ofn(nil), _twidth(twidth), _theight(theight) 
{ 
    _ifn = strdup(ifn);
    _ofn = strdup(ofn);
}

TileFileCmd::~TileFileCmd() {
    delete _ifn;
    _ifn = nil;
    delete _ofn;
    _ofn = nil;
}

Command* TileFileCmd::Copy () {
    TileFileCmd* copy = new TileFileCmd(
        CopyControlInfo(), _ifn, _ofn, _twidth, _theight
    );
    InitCopy(copy);
    return copy;
}

void TileFileCmd::Execute () {
    if (!_ifn || !_ofn) {
        // post a dialog to get the file name
    }
    else {
        const char* err = OvImportCmd::Create_Tiled_File(
            _ifn, _ofn, _twidth, _theight
        );

        if (err) {
            cerr << "unable to create tiled image: " << err << "\n";
        }
    }
}

boolean TileFileCmd::Reversible () { return false; }

/*****************************************************************************/

OvWindowDumpAsCmd::OvWindowDumpAsCmd (ControlInfo* c, OpenFileChooser* fc) : SaveCompAsCmd(c) {
    Init(fc);
}

OvWindowDumpAsCmd::OvWindowDumpAsCmd (Editor* ed, OpenFileChooser* fc) : SaveCompAsCmd(ed) {
    Init(fc);
}

OvWindowDumpAsCmd::~OvWindowDumpAsCmd() {
    Resource::unref(chooser_);
}

void OvWindowDumpAsCmd::Init (OpenFileChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvWindowDumpAsCmd::Copy () {
    Command* copy = new OvWindowDumpAsCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvWindowDumpAsCmd::Execute () {
    Editor* ed = GetEditor();

    char buf[CHARBUFSIZE];
    sprintf(buf, "Dump canvas in .xwd format to:");

    boolean reset_caption = false;
    Style* style = new Style(Session::instance()->style());
    style->attribute("subcaption", buf);
    style->attribute("open", "Dump");
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Dump window to file:");
	style->attribute("open", "Dump");
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style);
	Resource::ref(chooser_);
    }
    boolean again;
    while (again = chooser_->post_for(ed->GetWindow())) {
	const String* str = chooser_->selected();
	NullTerminatedString ns(*str);
        const char* name = ns.string();
        OverlayCatalog* catalog = (OverlayCatalog*)unidraw->GetCatalog();
        boolean ok = true;

        if (catalog->Exists(name) && catalog->Writable(name)) {
            char buf[CHARBUFSIZE];
            sprintf(buf, "\"%s\" already exists.", name);
	    GConfirmDialog* dialog = new GConfirmDialog(buf, "Overwrite?");
	    Resource::ref(dialog);

	    ok = dialog->post_for(ed->GetWindow());
	    Resource::unref(dialog);
        }
        if (ok) {
            CompNameVar* cnv = (CompNameVar*) ed->GetState("CompNameVar");
            const char* oldname = (cnv == nil) ? nil : cnv->GetName();
            Component* comp = ed->GetComponent();

            if (catalog->Exists(name) && !catalog->Writable(name)) {
		style->attribute("caption", "");
		style->attribute("caption", "Couldn't save to file!" );
            } else {
	      char cmdbuf[CHARBUFSIZE];
	      sprintf(cmdbuf, "xwd -id %ld -out %s",
		     ed->GetViewer()->GetCanvas()->window()->rep()->xwindow_,
		     name);
	      ed->GetWindow()->cursor(hourglass);
	      chooser_->twindow()->cursor(hourglass);
	      system(cmdbuf);
	      ed->GetWindow()->cursor(arrow);
	      chooser_->twindow()->cursor(arrow);
	      break;
            }
        }
    }
    chooser_->unmap();
    if (reset_caption) {
	style->attribute("caption", "");
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
}

/*****************************************************************************/

OvImageMapCmd::OvImageMapCmd (ControlInfo* c, OpenFileChooser* fc) : SaveCompAsCmd(c) {
    Init(fc);
}

OvImageMapCmd::OvImageMapCmd (Editor* ed, OpenFileChooser* fc) : SaveCompAsCmd(ed) {
    Init(fc);
}

OvImageMapCmd::~OvImageMapCmd() {
    Resource::unref(chooser_);
}

void OvImageMapCmd::Init (OpenFileChooser* f) {
    chooser_ = f;
    Resource::ref(chooser_);
}

Command* OvImageMapCmd::Copy () {
    Command* copy = new OvImageMapCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void OvImageMapCmd::Execute () {
    Editor* ed = GetEditor();

    char buf[CHARBUFSIZE];
    sprintf(buf, "Save ImageMap template to file:");

    boolean reset_caption = false;
    Style* style = new Style(Session::instance()->style());
    style->attribute("subcaption", buf);
    style->attribute("open", "Save");
    if (chooser_ == nil) {
	style = new Style(Session::instance()->style());
	style->attribute("title", "Save ImageMap");
	style->attribute("subcaption", "Save ImageMap template to file:");
	style->attribute("open", "Save");
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style);
	Resource::ref(chooser_);
    }
    boolean again;
    while (again = chooser_->post_for(ed->GetWindow())) {
	const String* str = chooser_->selected();
	NullTerminatedString ns(*str);
        const char* name = ns.string();
        OverlayCatalog* catalog = (OverlayCatalog*)unidraw->GetCatalog();
        boolean ok = true;

        if (catalog->Exists(name) && catalog->Writable(name)) {
            char buf[CHARBUFSIZE];
            sprintf(buf, "\"%s\" already exists.", name);
	    GConfirmDialog* dialog = new GConfirmDialog(buf, "Overwrite?");
	    Resource::ref(dialog);

	    ok = dialog->post_for(ed->GetWindow());
	    Resource::unref(dialog);
        }
        if (ok) {
            CompNameVar* cnv = (CompNameVar*) ed->GetState("CompNameVar");
            const char* oldname = (cnv == nil) ? nil : cnv->GetName();
            Component* comp = ed->GetComponent();

            if (catalog->Exists(name) && !catalog->Writable(name)) {
		style->attribute("caption", "");
		style->attribute("caption", "Couldn't save to file!" );
            } else {
	      OverlayViewer* viewer = (OverlayViewer*)ed->GetViewer();
	      OverlaysView* views = (OverlaysView*)viewer->GetOverlayView();
	      ofstream outfile(name);
	      char* drname = new char[strlen(name)+20];
	      strcpy(drname, name);
	      strcat(drname, ".drawtool");
	      ofstream drfile(drname);
	      outfile << "# Default" << "\n";
	      outfile << "default default_url" << "\n";
	      drfile << "drawtool(\n";
	      DumpViews(views, outfile, drfile);
	      outfile.close();
	      drfile << ")\n";
	      drfile.close();
	      break;
            }
        }
    }
    chooser_->unmap();
    if (reset_caption) {
	style->attribute("caption", "");
    }
    if (!again)
	ed->GetWindow()->cursor(arrow);
}

void OvImageMapCmd::DumpViews(OverlayView* view, ostream& outs, ostream& drs) {
  if (view->IsA(OVERLAYS_VIEW)) {
    Iterator i;
    OverlaysView* views = (OverlaysView*)view;
    for (views->Last(i); !views->Done(i); views->Prev(i)) {
      OverlayView* vi = (OverlayView*)views->GetView(i);
      DumpViews(vi, outs, drs);
    }
  }
  else if (view->IsA(OVRECT_VIEW)) {
    AttributeList* al = view->GetOverlayComp()->GetAttributeList();
    outs << "# " << *al << "\n";
    outs << "rect ";
    if (al->GetAttr("url")) {
      AttributeValue* av = al->GetAttr("url")->Value();
      if (av && av->type() == AttributeValue::StringType) {
	outs << av->string_ptr();
      }
      else
	outs << "null_url";
    }
    else {
      outs << "null_url";
    }
    int x[4], y[4];
    SF_Rect* rect = (SF_Rect*)view->GetGraphic();
    int x1, y1, x2, y2;
    rect->GetOriginal(x1, y1, x2, y2);
    int sx1, sy1, sx2, sy2;
    OverlayViewer* viewer = (OverlayViewer*)view->GetViewer();
    viewer->GraphicToScreen(rect, x1, y1, sx1, sy1);
    viewer->GraphicToScreen(rect, x2, y2, sx2, sy2);
    Canvas* canvas = viewer->GetCanvas();
    int ht = canvas->pheight();
    outs << " " << sx1 << "," << int(ht-sy1-1);
    outs << " " << sx2 << "," << int(ht-sy2-1) << "\n";
  }
  else if (view->IsA(OVPOLYGON_VIEW)) {
    // 
    OverlayViewer* viewer = (OverlayViewer*)view->GetViewer();
    Canvas* canvas = viewer->GetCanvas();
    int* x;
    int* y;
    int np;
    SF_Polygon* poly = (SF_Polygon*)view->GetGraphic();
    np = poly->GetOriginal(x, y);
    float* fx = new float[np];
    float* fy = new float[np];
    for (int i = 0; i < np; i++) {
      fx[i] = x[i];
      fy[i] = y[i];
    }

    FFillPolygonObj fpo(fx, fy, np);

    float xg0, yg0, xg1, yg1;
#if 0
    viewer->ScreenToGraphic(0, 0, poly, xg0, yg0);
    viewer->ScreenToGraphic(canvas->pwidth(), canvas->pheight(),
			    poly, xg1, yg1);
#endif
    // screen to graphic for regular graphics
    float dx0, dy0, dx1, dy1;
    viewer->ScreenToDrawing(0, 0, dx0, dy0);
    poly->GetTransformer()->InvTransform(dx0, dy0, xg0, yg0);
    viewer->ScreenToDrawing(canvas->pwidth(), canvas->pheight(), dx1, dy1);
    poly->GetTransformer()->InvTransform(dx1, dy1, xg1, yg1);

    FBoxObj viewbox(xg0, yg0, xg1, yg1);

    if (fpo.Intersects(viewbox)) {
      ((OverlayComp*)view->GetGraphicComp())->GetAttributeList()->dump();
#ifdef CLIPPOLY
      float vbx[4];
      float vby[4];
      vbx[0] = xg0;
      vby[0] = yg0;
      vbx[1] = xg0;
      vby[1] = yg1;
      vbx[2] = xg1;
      vby[2] = yg1;
      vbx[3] = xg1;
      vby[3] = yg0;
      float* px = new float[np];
      float* py = new float[np];
      for (int i = 0; i < np; i++) {
	px[i] = x[i];
	py[i] = y[i];
      }
      ClipOperation op = A_AND_B;
      int npolys;
      int* ni;
      float** cx;
      float** cy;
      //clippoly(op, 4, vbx, vby, unp, px, py, npolys, ni, cx, cy);
      clippoly(op, 4, vbx, vby, np, px, py, npolys, ni, cx, cy);
      if (npolys > 0) {
	for (int j = 0; j < npolys; j++) {
	  DumpPolys(view, outs, drs, cx[j], cy[j], ni[j], canvas->pwidth(), canvas->pheight());
	}
	clippoly_delete(npolys, ni, cx, cy);
      }
      delete [] px;
      delete [] py;
#else
      //DumpPolys(view, outs, drs, ux, uy, unp, canvas->pwidth(), canvas->pheight());
#endif
    }
  }
}

void OvImageMapCmd::DumpPolys(OverlayView* view, ostream& outs, ostream& drs,
				   float* ux, float* uy, int unp,
				   int pwidth, int pheight)
{
  int ni;
  int* ix;
  int* iy;
  GetScreenCoords((OverlayViewer*)view->GetViewer(), view->GetGraphic(),
		  unp, ux, uy, ni, ix, iy);
  if (ni > 2 && ni <= 100) {
      AttributeList* al = view->GetOverlayComp()->GetAttributeList();
      outs << "# ";
      if (al)
	outs << *al;
      outs << "\n";
      outs << "poly ";
      if (al && al->GetAttr("url")) {
	AttributeValue* av = al->GetAttr("url")->Value();
	if (av && av->type() == AttributeValue::StringType) {
	  outs << av->string_ptr();
	}
	else
	  outs << "null_url";
      }
      else {
	outs << "null_url";
      }
      drs << "polygon(";
      for (int i = 0; i < ni; i++) {
	outs << " " << ix[i] << "," << pheight-iy[i]-1;
	if (i > 0)
	  drs << ",";
	drs << "(" << ix[i] << "," << /*pheight-iy[i]-1*/ iy[i] << ")";
      }
      outs << "\n";
      drs << " :fillbg 0 :brush 65535,1 :fgcolor \"black\",0,0,0 :bgcolor \"black\",0,0,0 :graypat 0.75";
      drs << *al << ")\n";
      //}
  }
  else if (ni > 3) {
#ifdef CLIPPOLY
    // clip in half
    FBoxObj bbox;
    FFillPolygonObj fpo(ux, uy, unp);
    fpo.GetBox(bbox);
    FBoxObj box1, box2;
    if ((bbox._right - bbox._left) > (bbox._top - bbox._bottom)) {
      // clip left/right halves
      box1._left = bbox._left;
      box1._bottom = bbox._bottom;
      box1._right = bbox._left+((bbox._right-bbox._left)/2);
      box1._top = bbox._top;
      box2._left = bbox._left+((bbox._right-bbox._left)/2);
      box2._bottom = bbox._bottom;
      box2._right = bbox._right;
      box2._top = bbox._top;
    }
    else {
      // clip bottom/top halves
      box1._left = bbox._left;
      box1._bottom = bbox._bottom;
      box1._right = bbox._right;
      box1._top = bbox._bottom+((bbox._top-bbox._bottom)/2);
      box2._left = bbox._left;
      box2._bottom = bbox._bottom+((bbox._top-bbox._bottom)/2);
      box2._right = bbox._right;
      box2._top = bbox._top;
    }
    int n1 = 4;
    float* x1 = new float[n1];
    float* y1 = new float[n1];
    x1[0] = box1._left;
    y1[0] = box1._bottom;
    x1[1] = box1._left;
    y1[1] = box1._top;
    x1[2] = box1._right;
    y1[2] = box1._top;
    x1[3] = box1._right;
    y1[3] = box1._bottom;
    int n2 = unp;
    ClipOperation op = A_AND_B;
    int npolys;
    int* ni;
    float** cx;
    float** cy;
    clippoly(op, n1, x1, y1, n2, ux, uy, npolys, ni, cx, cy);
    if (npolys > 0) {
      for (int j = 0; j < npolys; j++) {
	DumpPolys(view, outs, drs, cx[j], cy[j], ni[j], pwidth, pheight);
      }
      clippoly_delete(npolys, ni, cx, cy);
    }
      
    x1[0] = box2._left;
    y1[0] = box2._bottom;
    x1[1] = box2._left;
    y1[1] = box2._top;
    x1[2] = box2._right;
    y1[2] = box2._top;
    x1[3] = box2._right;
    y1[3] = box2._bottom;
    clippoly(op, n1, x1, y1, n2, ux, uy, npolys, ni, cx, cy);
    if (npolys > 0) {
      for (int j = 0; j < npolys; j++) {
	DumpPolys(view, outs, drs, cx[j], cy[j], ni[j], pwidth, pheight);
      }
      clippoly_delete(npolys, ni, cx, cy);
    }
    delete [] x1;
    delete [] y1;
#endif
  }
}

void OvImageMapCmd::GetScreenCoords(OverlayViewer* viewer, Graphic* poly, int nf, float* fx, float* fy,
				    int& ni, int*& ix, int*& iy)
{
    int lastx, lasty, lastlastx, lastlasty;
    int unp = 0;
    for (int i = 0; i < nf; i++) {
      int sx, sy;
      viewer->GraphicToScreen(poly, fx[i], fy[i], sx, sy);
      if (i == 0) {
	lastx = sx;
	lasty = sy;
	unp++;
      }
      else if (i == 1) {
	if (sx == lastx && sy == lasty)
	  continue;
	lastlastx = lastx;
	lastlasty = lasty;
	lastx = sx;
	lasty = sy;
	unp++;
      }
      else {
	if ((sx == lastx && sy == lasty) ||
	    (sx == lastlastx && sy == lastlasty))
	  continue;

	lastlastx = lastx;
	lastlasty = lasty;
	lastx = sx;
	lasty = sy;
	unp++;
      }
    }
    ni = unp;
    ix = new int[unp];
    iy = new int[unp];
    unp = 0;
    for (int i = 0; i < nf; i++) {
      int sx, sy;
      viewer->GraphicToScreen(poly, fx[i], fy[i], sx, sy);
      if (i == 0) {
	lastx = sx;
	lasty = sy;
	ix[unp] = sx;
	iy[unp++] = sy;
      }
      else if (i == 1) {
	if (sx == lastx && sy == lasty)
	  continue;
	lastlastx = lastx;
	lastlasty = lasty;
	lastx = sx;
	lasty = sy;
	ix[unp] = sx;
	iy[unp++] = sy;
      }
      else {
	if ((sx == lastx && sy == lasty) ||
	    (sx == lastlastx && sy == lastlasty))
	  continue;

	lastlastx = lastx;
	lastlasty = lasty;
	lastx = sx;
	lasty = sy;
	ix[unp] = sx;
	iy[unp++] = sy;
      }
    }
}
