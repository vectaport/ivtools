/*
 * Copyright (c) 2004 Scott E. Johnston
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

#include <DrawServ/drawcomps.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawfunc.h>
#include <DrawServ/drawserv.h>

#include <Unidraw/catalog.h>

#include <ComTerp/comterpserv.h>

/*****************************************************************************/

DrawEditor::DrawEditor(OverlayComp* comp, OverlayKit* kit) 
: FrameEditor(false, kit) {
    Init(comp, "DrawEditor");
}

DrawEditor::DrawEditor(const char* file, OverlayKit* kit)
: FrameEditor(false, kit)
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
	    fprintf(stderr, "drawserv: couldn't open %s\n", file);
	}
    }
}

DrawEditor::DrawEditor(boolean initflag, OverlayKit* kit) 
: FrameEditor(initflag, kit) {
}

void DrawEditor::Init (OverlayComp* comp, const char* name) {
  _curr_others = _prev_others = nil;
  _num_curr_others = _num_prev_others = 0;
  _texteditor = nil;
  _autonewframe = false;
  _autonewframe_tts = nil;
  if (!comp) comp = new DrawIdrawComp;
  _terp = new ComTerpServ();
  ((OverlayUnidraw*)unidraw)->comterp(_terp);
  AddCommands(_terp);
  add_comterp("DrawServ", _terp);
  _overlay_kit->Init(comp, name);
  InitFrame();
}

void DrawEditor::InitCommands() {
  FrameEditor::InitCommands();
}

void DrawEditor::AddCommands(ComTerp* comterp) {
  FrameEditor::AddCommands(comterp);

  comterp->add_command("drawserv", new DrawServFunc(comterp, this));
}

