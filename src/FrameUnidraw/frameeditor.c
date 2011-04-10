/*
 * Copyright (c) 1994-1998 Vectaport Inc.
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

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framefunc.h>
#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/frameviews.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovviewer.h>

#include <IVGlyph/textedit.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/Components/text.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Tools/grcomptool.h>

#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/keymap.h>
#include <Unidraw/unidraw.h>

#include <InterViews/box.h>
#include <InterViews/border.h>
#include <InterViews/glue.h>

#include <IV-2_6/InterViews/frame.h>
#include <IV-2_6/InterViews/panner.h>
#include <IV-2_6/InterViews/tray.h>

#include <ComTerp/comterpserv.h>

#include <stdio.h>

implementActionCallback(FrameEditor)

/*****************************************************************************/

FrameEditor::FrameEditor(OverlayComp* gc, OverlayKit* ok)
: ComEditor(false, ok)
{
    Init(gc);
}

FrameEditor::FrameEditor(const char* file, OverlayKit* ok)
: ComEditor(false, ok)
{
    if (file == nil) {
	Init();

    } else {
	Catalog* catalog = unidraw->GetCatalog();
	OverlayComp* comp;

	if (catalog->Retrieve(file, (Component*&) comp)) {
	    Init(comp);

	} else {
	    Init();
	    fprintf(stderr, "flipbook: couldn't open %s\n", file);
	}
    }
}

FrameEditor::FrameEditor(boolean initflag, OverlayKit* ok) 
: ComEditor(initflag, ok) {}

FrameEditor::~FrameEditor() {}

void FrameEditor::Init (OverlayComp* comp, const char* name) {
  _curr_other = _prev_other = 0;
  _texteditor = nil;
  if (!comp) comp = new FrameIdrawComp;
  _terp = new ComTerpServ();
  AddCommands(_terp);
  add_comterp("Flipbook", _terp);
  _overlay_kit->Init(comp, name);
  InitFrame();
  _autonewframe = false;
}

void FrameEditor::InitCommands() {
  int secs = 0;
  Catalog* catalog = unidraw->GetCatalog();
  const char* slideshow_str = catalog->GetAttribute("slideshow");
  if (slideshow_str) secs = atoi(slideshow_str);
  if (!secs) {
    FrameIdrawComp* comp = (FrameIdrawComp*)GetGraphicComp();
    const char* attrname = "slideshow";
    AttributeValue* av = comp->FindValue(attrname);
    if (av) secs = av->int_val();
  }
  if (secs && _terp) {
    MoveFrameCmd::default_instance()->set_wraparound();
    char buffer[BUFSIZ];
    sprintf(buffer, "timeexpr(\"moveframe(1)\" :sec %d)", secs);
    _terp->run(buffer);
  }
}

void FrameEditor::InitFrame() {
    _currframe = nil;
    _prevframe = nil;
    FrameIdrawView* view = (FrameIdrawView*)GetViewer()->GetGraphicView();
    Iterator it;
    view->First(it);
    OverlayView* subview = ((OverlayView*)view->GetView(it));
    if (subview && subview->IsA(FRAME_VIEW)) {			    
      subview->Desensitize();
      view->Next(it);
      if (view->Done(it)) {
	view->First(it);
	if (framenumstate()) framenumstate()->framenumber(0, true);
      } else {
	if (framenumstate()) framenumstate()->framenumber(1, true);
	Iterator i(it);
	view->Next(i);
	while (!view->Done(i)) {
	  OverlayView* v = (OverlayView*)view->GetView(i);
	  v->Hide();
	  view->Next(i);
	}
      }
    }
    SetFrame((FrameView*)view->GetView(it));
    UpdateFrame(false);
}

void FrameEditor::Update() {
  ComEditor::Update();
}

void FrameEditor::UpdateFrame(boolean txtupdate) {
    FrameIdrawView *views = (FrameIdrawView*)GetViewer()->GetGraphicView();
    views->UpdateFrame(_currframe, _prevframe, _curr_other, _prev_other);
    _prev_other = _curr_other;
    if (GetFrame())
      UpdateText((OverlayComp*)GetFrame()->GetGraphicComp(), txtupdate);
    Iterator last;
    views->Last(last);
    if (frameliststate()) frameliststate()->framenumber(views->Index(last)+1);
}

void FrameEditor::SetText() {
    GraphicComp* comp = GetFrame()->GetGraphicComp();
    ((OverlayComp*)comp)->SetAnnotation(TextEditor()->text());
    ((ModifStatusVar*)GetState("ModifStatusVar"))->SetModifStatus(true);
}

void FrameEditor::ClearText() {
    _texteditor->text("");
}

void FrameEditor::UpdateText(OverlayComp* comp, boolean update) {
    if (_texteditor) {
	const char* txt = comp->GetAnnotation();
	if (!txt)
	    txt = "";
	_texteditor->text(txt, update);
    }
}

OverlaysView* FrameEditor::GetFrame(int index) {
  if (index<0) 
    return _currframe;
  else if (index<_frameliststate->framenumber()) {
    FrameIdrawView* views = (FrameIdrawView*)GetViewer()->GetGraphicView();
    Iterator i;
    int count = 0;
    views->First(i);
    while (count++<index && !views->Done(i)) views->Next(i);
    return (OverlaysView*)views->GetView(i);
  } else
    return nil;
}

void FrameEditor::AddCommands(ComTerp* comterp) { 
  ComEditor::AddCommands(comterp);
  comterp->add_command("moveframe", new MoveFrameFunc(comterp, this));
  comterp->add_command("createframe", new CreateFrameFunc(comterp, this));
}

void FrameEditor::DoAutoNewFrame() {
  if (_autonewframe) {
    CreateMoveFrameCmd* cmd = new CreateMoveFrameCmd(this, true);
    cmd->Execute();
    cmd->Log();
  }
}
