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

#include <ComUnidraw/grdotfunc.h>
#include <ComUnidraw/grfunc.h>
#include <ComUnidraw/grlistfunc.h>
#include <ComUnidraw/groupfunc.h>
#include <ComUnidraw/grstatfunc.h>
#include <ComUnidraw/highlightfunc.h>
#include <ComUnidraw/comeditor.h>
#include <ComUnidraw/comterp-iohandler.h>
#include <ComUnidraw/dialogfunc.h>
#include <ComUnidraw/nfunc.h>
#include <ComUnidraw/plotfunc.h>

#include <ComTerp/ctrlfunc.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/strmfunc.h>

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/creator.h>
#include <Unidraw/iterator.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/unidraw.h>

#include <Unidraw/Commands/command.h>

#include <InterViews/frame.h>

#include <Attribute/attrlist.h>

#include <strstream.h>

/*****************************************************************************/

ComEditor::ComEditor(OverlayComp* comp, OverlayKit* kit) 
: OverlayEditor(false, kit) {
    Init(comp, "ComEditor");
}

ComEditor::ComEditor(const char* file, OverlayKit* kit)
: OverlayEditor(false, kit)
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

ComEditor::ComEditor(boolean initflag, OverlayKit* kit) 
: OverlayEditor(initflag, kit) {
  _terp = nil;
  _whiteboard = -1;
}

void ComEditor::Init (OverlayComp* comp, const char* name) {
    if (!comp) comp = new OverlayIdrawComp;
    _terp = new ComTerpServ();
    ((OverlayUnidraw*)unidraw)->comterp(_terp);
    AddCommands(_terp);
    char buffer[BUFSIZ];
    sprintf(buffer, "Comdraw%d", ncomterp());
    add_comterp(buffer, _terp);
    _overlay_kit->Init(comp, name);
    _whiteboard = -1;
}

void ComEditor::InitCommands() {
    if (!_terp) 
      _terp = new ComTerpServ();
      const char* comdraw_off_str = unidraw->GetCatalog()->GetAttribute("comdraw_off");
    if ((!comterplist() || comterplist()->Number()==1) &&
	(comdraw_off_str ? strcmp(comdraw_off_str, "false")==0 : true))
      _terp_iohandler = new ComTerpIOHandler(_terp, stdin);
    else
      _terp_iohandler = nil;
#if 0
    _terp->add_defaults();
    AddCommands(_terp);
#endif
}

void ComEditor::AddCommands(ComTerp* comterp) {
    ((ComTerpServ*)comterp)->add_defaults();

    comterp->add_command("rect", new CreateRectFunc(comterp, this));
    comterp->add_command("rectangle", new CreateRectFunc(comterp, this));
    comterp->add_command("line", new CreateLineFunc(comterp, this));
    comterp->add_command("arrowline", new CreateLineFunc(comterp, this));
    comterp->add_command("ellipse", new CreateEllipseFunc(comterp, this));
    comterp->add_command("text", new CreateTextFunc(comterp, this));
    comterp->add_command("multiline", new CreateMultiLineFunc(comterp, this));
    comterp->add_command("arrowmultiline", new CreateMultiLineFunc(comterp, this));
    comterp->add_command("openspline", new CreateOpenSplineFunc(comterp, this));
    comterp->add_command("arrowspline", new CreateOpenSplineFunc(comterp, this));
    comterp->add_command("polygon", new CreatePolygonFunc(comterp, this));
    comterp->add_command("closedspline", new CreateClosedSplineFunc(comterp, this));

    comterp->add_command("center", new CenterFunc(comterp, this));
    comterp->add_command("mbr", new MbrFunc(comterp, this));
    comterp->add_command("points", new PointsFunc(comterp, this));

    comterp->add_command("font", new FontFunc(comterp, this));
    comterp->add_command("brush", new BrushFunc(comterp, this));
    comterp->add_command("pattern", new PatternFunc(comterp, this));
    comterp->add_command("colors", new ColorFunc(comterp, this));

    comterp->add_command("nfonts", new NFontsFunc(comterp, this));
    comterp->add_command("nbrushes", new NBrushesFunc(comterp, this));
    comterp->add_command("npatterns", new NPatternsFunc(comterp, this));
    comterp->add_command("ncolors", new NColorsFunc(comterp, this));

    comterp->add_command("setattr", new SetAttrFunc(comterp, this));

    comterp->add_command("select", new SelectFunc(comterp, this));
    comterp->add_command("delete", new DeleteFunc(comterp, this));
    comterp->add_command("move", new MoveFunc(comterp, this));
    comterp->add_command("scale", new ScaleFunc(comterp, this));
    comterp->add_command("rotate", new RotateFunc(comterp, this));

    comterp->add_command("pan", new PanFunc(comterp, this));
    comterp->add_command("smallpanup", new PanUpSmallFunc(comterp, this));
    comterp->add_command("smallpandown", new PanDownSmallFunc(comterp, this));
    comterp->add_command("smallpanleft", new PanLeftSmallFunc(comterp, this));
    comterp->add_command("smallpanright", new PanRightSmallFunc(comterp, this));
    comterp->add_command("largepanup", new PanUpLargeFunc(comterp, this));
    comterp->add_command("largepandown", new PanDownLargeFunc(comterp, this));
    comterp->add_command("largepanleft", new PanLeftLargeFunc(comterp, this));
    comterp->add_command("largepanright", new PanRightLargeFunc(comterp, this));
    
    comterp->add_command("zoom", new ZoomFunc(comterp, this));
    comterp->add_command("zoomin", new ZoomInFunc(comterp, this));
    comterp->add_command("zoomout", new ZoomOutFunc(comterp, this));

    comterp->add_command("tilefile", new TileFileFunc(comterp, this));

    comterp->add_command("update", new UpdateFunc(comterp, this));
    comterp->add_command("ncols", new NColsFunc(comterp, this));
    comterp->add_command("nrows", new NRowsFunc(comterp, this));
    comterp->add_command("handles", new HandlesFunc(comterp, this));

    if (OverlayKit::bincheck("plotmtv"))
      comterp->add_command("barplot", new BarPlotFunc(comterp, this));

    comterp->add_command("import", new ImportFunc(comterp, this));

    comterp->add_command("dot", new GrDotFunc(comterp));
    comterp->add_command("attrlist", new GrAttrListFunc(comterp));
    comterp->add_command("at", new GrListAtFunc(comterp));
    comterp->add_command("size", new GrListSizeFunc(comterp));

    comterp->add_command("acknowledgebox", new AcknowledgeBoxFunc(comterp, this));
    comterp->add_command("confirmbox", new ConfirmBoxFunc(comterp, this));

    comterp->add_command("highlight", new HighlightFunc(comterp, this));
    comterp->add_command("frame", new FrameFunc(comterp, this));

    comterp->add_command("growgroup", new GrowGroupFunc(comterp, this));
    comterp->add_command("trimgroup", new TrimGroupFunc(comterp, this));

    comterp->add_command("pause", new UnidrawPauseFunc(comterp, this));

    comterp->add_command("paste", new PasteFunc(comterp, this));
    comterp->add_command("pastemode", new PasteModeFunc(comterp, this));
    comterp->add_command("addtool", new AddToolButtonFunc(comterp, this));
}

/* virtual */ void ComEditor::ExecuteCmd(Command* cmd) {
  if(!whiteboard()) 
    OverlayEditor::ExecuteCmd(cmd);
  else {
    ostrstream sbuf;
    switch (cmd->GetClassId()) {
    case PASTE_CMD:
      {
      boolean scripted = false;
      Clipboard* cb = cmd->GetClipboard();
      if (cb) {
	Iterator it;
	for (cb->First(it); !cb->Done(it); cb->Next(it)) {
	  OverlayComp* comp = (OverlayComp*)cb->GetComp(it);
	  if (comp) {
	    Creator* creator = unidraw->GetCatalog()->GetCreator();
	    OverlayScript* scripter = (OverlayScript*)
	      creator->Create(Combine(comp->GetClassId(), SCRIPT_VIEW));
	    if (scripter) {
	      scripter->SetSubject(comp);
	      if (scripted) 
		sbuf << ';';
	      else 
		scripted = true;
	      boolean status = scripter->Definition(sbuf);
	      delete scripter;
	    }
	  }
	}
      }
      if (!scripted)
	sbuf << "print(\"Failed attempt to generate script for a PASTE_CMD\\n\" :err)";
      sbuf.put('\0');
      cerr << sbuf.str() << "\n";
      GetComTerp()->run(sbuf.str());
      delete cmd;
      }
      break;
    default:
      sbuf << "print(\"Attempt to convert unknown command (id == %d) to interpretable script\\n\" " << cmd->GetClassId() << " :err)";
      cmd->Execute();
      if (cmd->Reversible()) {
	cmd->Log();
      } else {
	delete cmd;
      }
      break;
    }
  }
}

boolean ComEditor::whiteboard() { 
  if (_whiteboard==-1) {
    Catalog* catalog = unidraw->GetCatalog();
    const char* wbmaster_str = catalog->GetAttribute("wbmaster");
    const char* wbslave_str = catalog->GetAttribute("wbslave");
    if (wbmaster_str && strcmp(wbmaster_str, "true")==0 || 
	wbslave_str && strcmp(wbslave_str, "true")==0) 
      _whiteboard = 1;
    else
      _whiteboard = 0;
  }
  return _whiteboard;
}
