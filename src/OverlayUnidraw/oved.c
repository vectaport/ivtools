/*
 * Copyright (c) 2000 Vectaport Inc, IET Inc.
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
 * Overlay editor main class implementation.
 */


#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovpanner.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>

#include <IVGlyph/observables.h>
#include <IVGlyph/textedit.h>

#include <UniIdraw/idcmds.h>
#include <UniIdraw/idkybd.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/iterator.h>
#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/grid.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/uctrls.h>
#include <Unidraw/unidraw.h>

#include <InterViews/box.h>
#include <InterViews/border.h>
#include <InterViews/display.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/menu.h>
#include <InterViews/message.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/tray.h>
#include <InterViews/window.h>
#include <IV-2_6/InterViews/world.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <OS/math.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

implementActionCallback(OverlayEditor)

/*****************************************************************************/

inline void InsertSeparator (PulldownMenu* pdm) {
    pdm->GetScene()->Insert(
        new VBox(
            new VGlue(2, 0, 0),
            new HBorder,
            new VGlue(2, 0, 0)
        )
    );
}

inline PulldownMenu* MakePulldown (const char* name) {
    return new PulldownMenu(
        new HBox(
            new Message(name, Center, Math::round(.1*ivcm)),
            new HGlue(0, 5*strlen(name), 0)
        )
    );
}

/*****************************************************************************/

AttributeList* OverlayEditor::_edlauncherlist = nil;
AttributeList* OverlayEditor::_comterplist = nil;

OverlayEditor::OverlayEditor (OverlayComp* comp, OverlayKit* ok) : IdrawEditor(false) {
    _viewer = nil;
    ok->SetEditor(this);
    _overlay_kit = ok;
    _mousedoc = new ObservableText("");
    Init(comp);
}

OverlayEditor::OverlayEditor (const char* file, OverlayKit* ok) : IdrawEditor(false) {
    _viewer = nil;
    ok->SetEditor(this);
    _overlay_kit = ok;
    _mousedoc = new ObservableText("");

    if (file == nil) {
	Init();
    } else {
	Catalog* catalog = unidraw->GetCatalog();
	OverlayComp* comp;

	((OverlayCatalog*)catalog)->SetEditor(this);
	if (catalog->Retrieve(file, (Component*&) comp)) {
	    Init(comp);

	} else {
	    OverlayIdrawComp* comp = new OverlayIdrawComp;
	    comp->SetPathName(file);
	    catalog->Register(comp, file);
	    Init(comp, file);
	    
	    fprintf(stderr, "drawtool: couldn't open %s\n", file);
	}
    }
}

OverlayEditor::OverlayEditor (boolean initflag, OverlayKit* ok) : IdrawEditor(initflag) {
    _viewer = nil;
    ok->SetEditor(this);
    _overlay_kit = ok;
    _mousedoc = new ObservableText("");
    _texteditor = nil;
}

OverlayEditor::~OverlayEditor() {
    delete _mousedoc;
}

void OverlayEditor::Init (OverlayComp* comp, const char* name) {
    _texteditor = nil;
    if (!comp) comp = new OverlayIdrawComp;
    _overlay_kit->Init(comp, name);
}

void OverlayEditor::Update () { 
    OverlayViewer* v;

    for (int i = 0; (v = (OverlayViewer*)GetViewer(i)) != nil; ++i) {
        v->Update();
    }
#if 0
    // make sure state views get updated right away (e.g. frame number)
    GetWindow()->repair();
    unidraw->GetWorld()->display()->flush();
#endif
}

Interactor* OverlayEditor::Interior () {
    HBorder* hborder = new HBorder;
    VBorder* vborder = new VBorder;
    int gap = Math::round(.1*ivcm);

    HBox* indicators = new HBox(
        new ArrowVarView(_arrows, _brush, _color),
        new VBorder,
        new PatternVarView(_pattern, _color),
	new VBorder
    );
    HBox* status = new HBox(
	new HGlue(gap, 0, 0),
	new ModifStatusVarView(_modifStatus),
	new CompNameVarView(_name, Left),
	new MagnifVarView(_magnif),
	new GravityVarView(_gravity, Right),
	new FontVarView(_font, Right)
    );

    _tray->HBox(_tray, indicators, status, _tray);
    _tray->HBox(_tray, hborder, _tray);
    _tray->HBox(_tray, _viewer, _tray);

    _tray->VBox(_tray, indicators, hborder, _viewer, _tray);
    _tray->VBox(_tray, status, hborder, _viewer, _tray);
    Alignment alignment = BottomRight;

    OverlayPanner* panner = make_panner();
    if (panner) 
      _tray->Align(panner_align(), _viewer, new Frame(panner));
    return _tray;
}

OverlayPanner* OverlayEditor::make_panner() {

  Catalog* catalog = unidraw->GetCatalog();
  
  boolean panner_off = false;
  if (const char* string = catalog->GetAttribute("panner_off"))
    panner_off = strcmp(string, "true") == 0;
  if (const char* string = catalog->GetAttribute("panner_on"))
    panner_off = strcmp(string, "false") == 0;
  boolean zoomer_off = false;
  if (const char* string = catalog->GetAttribute("zoomer_off"))
    zoomer_off = strcmp(string, "true") == 0;
  if (const char* string = catalog->GetAttribute("zoomer_on"))
    zoomer_off = strcmp(string, "false") == 0;
  boolean slider_off = false;
  if (const char* string = catalog->GetAttribute("slider_off"))
    slider_off = strcmp(string, "true") == 0;
  if (const char* string = catalog->GetAttribute("slider_on"))
    slider_off = strcmp(string, "false") == 0;

  if (!panner_off || !zoomer_off || !slider_off) {
    OverlayPanner* panner = 
      new OverlayPanner(_viewer, 0, !panner_off, !zoomer_off, !slider_off); 
    return panner;
  } else
    return nil;
}

int OverlayEditor::panner_align() {
  
  Catalog* catalog = unidraw->GetCatalog();
  
  Alignment alignment = BottomRight;
  if (const char* panner_align = catalog->GetAttribute("panner_align")) {
    const int nalign = 15;
    char *alignmentstr[nalign] = { 
      "tl", "tc", "tr", "cl", "c", "cr", "cl", "bl", "br", 
      "l", "r", "t", "b", "hc", "vc" 
    };
    if (isdigit(*panner_align))
      alignment = atoi(panner_align); 
    else {
      int n=0;
      while (n<nalign) {
	if (strcmp(alignmentstr[n], panner_align)==0) {
	  alignment = n;
	  break;
	}
	n++;
      }
    }
  }
  return alignment;
}
    
void OverlayEditor::InitCommands() { }

Tool* OverlayEditor::GetCurTool() { return _curtool; }

void OverlayEditor::SetCurTool (Tool* tool) {
    _curtool = tool;
}


static void DoInformComponents(Editor* ed, Component* comp) {
    if (comp && comp->IsA(OVERLAY_COMP)) {
        ((OverlayComp*)comp)->Configure(ed);
    }

    if (comp) {
        Iterator i;
        for (comp->First(i); !comp->Done(i); comp->Next(i)) {
            if (comp->IsA(GRAPHIC_COMP)) {
    	        DoInformComponents(ed, ((GraphicComp*)comp)->GetComp(i));
            }
        }
    }
}


void OverlayEditor::InformComponents() {
    DoInformComponents(this, GetComponent());
}

void OverlayEditor::SetComponent(Component* comp) {
    GetSelection()->Clear();
    IdrawEditor::SetComponent(comp);
    DoInformComponents(this, comp);
}

void OverlayEditor::ReplaceComponent(Component* comp) {
    Component* orig = GetComponent();

    SetComponent(comp);

    if (orig != nil && unidraw->FindAny(orig) == nil) {
        Component* root = orig->GetRoot();
        delete root;
    }
}

void OverlayEditor::Annotate(OverlayComp* comp) {
    _overlay_kit->Annotate(comp);
}

void OverlayEditor::AttrEdit(OverlayComp* comp) {
    _overlay_kit->AttrEdit(comp);
}

int OverlayEditor::nedlauncher() { return _edlauncherlist ? _edlauncherlist->Number() : 0; }

void OverlayEditor::add_edlauncher(const char* name, editor_launcher edlauncher) {
  if (!edlauncher) return;
  if (!_edlauncherlist) 
    _edlauncherlist = new AttributeList();
  AttributeValue* av = new AttributeValue(0, (void *)edlauncher);
  _edlauncherlist->add_attr(name, av);
}

editor_launcher OverlayEditor::edlauncher(const char* name) {
  if (_edlauncherlist) {
    AttributeValue* av = _edlauncherlist->find(name);
    return av ? (editor_launcher) av->obj_val() : nil;
  } else
    return nil;
}

editor_launcher OverlayEditor::edlauncher(int symid) {
  if (_edlauncherlist) {
    AttributeValue* av = _edlauncherlist->find(symid);
    return av ? (editor_launcher) av->obj_val() : nil;
  } else
    return nil;
}

int OverlayEditor::ncomterp() { return _comterplist ? _comterplist->Number() : 0; }

void OverlayEditor::add_comterp(const char* name, ComTerpServ* comterp) {
  if (!comterp) return;
  if (!_comterplist) 
    _comterplist = new AttributeList();
  AttributeValue* av = new AttributeValue(0, (void *)comterp);
  _comterplist->add_attr(name, av);
}

ComTerpServ* OverlayEditor::comterp(const char* name) {
  if (_comterplist) {
    AttributeValue* av = _comterplist->find(name);
    return av ? (ComTerpServ*) av->obj_val() : nil;
  } else
    return nil;
}

ComTerpServ* OverlayEditor::comterp(int symid) {
  if (_comterplist) {
    AttributeValue* av = _comterplist->find(symid);
    return av ? (ComTerpServ*) av->obj_val() : nil;
  } else
    return nil;
}

OverlaysView* OverlayEditor::GetFrame(int index) {
  return (OverlaysView*)GetViewer()->GetGraphicView();
}

boolean OverlayEditor::IsClean() {
  ModifStatusVar* mv = (ModifStatusVar*) GetState("ModifStatusVar");
  return (mv != nil && !mv->GetModifStatus());
}

void OverlayEditor::ResetStateVars() {
  return;
}

/* static */ boolean OverlayEditor::opaque_flag() {
  static boolean opflag = unidraw->GetCatalog()->GetAttribute("opaque_off") ?
    strcmp(unidraw->GetCatalog()->GetAttribute("opaque_off"), "true") != 0 :
    false;
  return opflag;
}

/* virtual */ void OverlayEditor::ExecuteCmd(Command* cmd) {
  cmd->Execute();
  if (cmd->Reversible()) {
    cmd->Log();
  } else {
    delete cmd;
  }
}

void OverlayEditor::SetText() {
    GraphicComp* comp = GetFrame()->GetGraphicComp();
    ((OverlayComp*)comp)->SetAnnotation(TextEditor()->text());
    ((ModifStatusVar*)GetState("ModifStatusVar"))->SetModifStatus(true);
}

void OverlayEditor::ClearText() {
    _texteditor->text("");
}

void OverlayEditor::UpdateText(OverlayComp* comp, boolean update) {
    if (_texteditor) {
	const char* txt = comp->GetAnnotation();
	if (!txt)
	    txt = "";
	_texteditor->text(txt, update);
    }
}

