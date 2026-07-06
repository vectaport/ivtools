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

#include <ComUnidraw/comkit.h>
#include <ComUnidraw/comterp-acehandler.h>
#include <ComUnidraw/grdotfunc.h>
#include <ComUnidraw/grfunc.h>
#include <ComUnidraw/grlistfunc.h>
#include <ComUnidraw/groupfunc.h>
#include <ComUnidraw/grstatfunc.h>
#include <ComUnidraw/grstrmfunc.h>
#include <ComUnidraw/highlightfunc.h>
#include <ComUnidraw/comeditor.h>
#include <ComUnidraw/comterp-iohandler.h>
#include <ComUnidraw/dialogfunc.h>
#include <ComUnidraw/nfunc.h>
#include <ComUnidraw/pixelfunc.h>
#include <ComUnidraw/plotfunc.h>
#include <ComUnidraw/soundfunc.h>

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

#include <InterViews/event.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xevent.h>
/* XkbKeycodeToKeysym: bracket the XKB header with Xdefs/Xundefs so its
   Display parameter resolves to X11's XDisplay (see Unidraw/editor.c). */
#include <IV-X11/Xdefs.h>
#include <X11/XKBlib.h>
#include <IV-X11/Xundefs.h>
#include <X11/keysym.h>       /* XK_Up/Down/Left/Right for shift-arrow capture */
#include <sys/time.h>         /* gettimeofday for the shift-arrow watchdog */

static double comeditor_now_seconds() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}
static const double SHIFTARROW_TIMEOUT = 2.0;   // seconds without a poll -> disarm

#include <Unidraw/Commands/command.h>

#include <InterViews/frame.h>

#include <Attribute/attrlist.h>

#include <strstream>
#include <string.h>
#include <fstream>
#include <iostream>

using std::cout;

#include <leakchecker.h>

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
  _keyq_head = _keyq_tail = 0;
  _shiftarrow_on = false;
  _shiftarrow_deadline = 0.0;
}

void ComEditor::Init (OverlayComp* comp, const char* name) {
    _keyq_head = _keyq_tail = 0;
    _shiftarrow_on = false;
    _shiftarrow_deadline = 0.0;
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
      const char* stdin_off_str = unidraw->GetCatalog()->GetAttribute("stdin_off");
#ifndef HAVE_ACE
    if ((!comterplist() || comterplist()->Number()==1) &&
	(stdin_off_str ? strcmp(stdin_off_str, "false")==0 : true))
      _terp_iohandler = new ComTerpIOHandler(_terp, stdin);
    else
#endif
      _terp_iohandler = nil;
#if 0
    _terp->add_defaults();
    AddCommands(_terp);
#endif
}

void ComEditor::AddCommands(ComTerp* comterp) {
    ((ComTerpServ*)comterp)->add_defaults();

    comterp->add_command("depth", new GrDepthFunc(comterp));
    comterp->add_command("parent", new GrParentFunc(comterp));
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
    comterp->add_command("raster", new CreateRasterFunc(comterp, this));
    comterp->add_command("pixmap", new CreateRasterFunc(comterp, this));

    comterp->add_command("center", new CenterFunc(comterp, this));
    comterp->add_command("mbr", new MbrFunc(comterp, this));
    comterp->add_command("points", new PointsFunc(comterp, this));

    comterp->add_command("font", new FontFunc(comterp, this));
    comterp->add_command("brush", new BrushFunc(comterp, this));
    comterp->add_command("pattern", new PatternFunc(comterp, this));
    comterp->add_command("patternmask", new PatternMaskFunc(comterp, this));
    comterp->add_command("colors", new ColorFunc(comterp, this));
    comterp->add_command("fontbyname", new FontByNameFunc(comterp, this));
    comterp->add_command("colorsrgb", new ColorRgbFunc(comterp, this));
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
    comterp->add_command("fliph", new FlipHorizontalFunc(comterp, this));
    comterp->add_command("flipv", new FlipVerticalFunc(comterp, this));

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

    comterp->add_command("update", new UpdateUnidrawFunc(comterp, this));
    comterp->add_command("ncols", new NColsFunc(comterp, this));
    comterp->add_command("nrows", new NRowsFunc(comterp, this));
    comterp->add_command("handles", new HandlesFunc(comterp, this));

#if 0
    if (OverlayKit::bincheck("plotmtv"))
      comterp->add_command("barplot", new BarPlotFunc(comterp, this));
#endif

    comterp->add_command("save", new SaveFileFunc(comterp, this));
    comterp->add_command("import", new ImportFunc(comterp, this));
    comterp->add_command("export", new ExportFunc(comterp, this));

    comterp->add_command("dot", new GrDotFunc(comterp));
    comterp->add_command("attrlist", new GrAttrListFunc(comterp));
    comterp->add_command("at", new GrListAtFunc(comterp));
    comterp->add_command("size", new GrListSizeFunc(comterp));
    comterp->add_command("stream", new GrStreamFunc(comterp));

    comterp->add_command("acknowledgebox", new AcknowledgeBoxFunc(comterp, this));
    comterp->add_command("confirmbox", new ConfirmBoxFunc(comterp, this));

    comterp->add_command("highlight", new HighlightFunc(comterp, this));
    comterp->add_command("frame", new FrameFunc(comterp, this));

    comterp->add_command("growgroup", new GrowGroupFunc(comterp, this));
    comterp->add_command("trimgroup", new TrimGroupFunc(comterp, this));
    comterp->add_command("group", new GroupFunc(comterp, this));
    comterp->add_command("ungroup", new UngroupFunc(comterp, this));
    comterp->add_command("front", new FrontSelectionFunc(comterp, this));
    comterp->add_command("back", new BackSelectionFunc(comterp, this));

    comterp->add_command("pause", new UnidrawPauseFunc(comterp, this));

    comterp->add_command("paste", new PasteFunc(comterp, this));
    comterp->add_command("pastemode", new PasteModeFunc(comterp, this));
    comterp->add_command("addtool", new AddToolButtonFunc(comterp, this));

    comterp->add_command("dtos", new DrawingToScreenFunc(comterp, this));
    comterp->add_command("stod", new ScreenToDrawingFunc(comterp, this));
    comterp->add_command("dtog", new DrawingToGraphicFunc(comterp, this));
    comterp->add_command("gtod", new GraphicToDrawingFunc(comterp, this));
    comterp->add_command("ssize", new ScreenSizeFunc(comterp, this));
    comterp->add_command("dsize", new DrawingSizeFunc(comterp, this));

    comterp->add_command("poke", new PixelPokeFunc(comterp, this));
    comterp->add_command("peek", new PixelPeekFunc(comterp, this));
    comterp->add_command("pokeline", new PixelPokeLineFunc(comterp, this));
    comterp->add_command("pcols", new PixelColsFunc(comterp, this));
    comterp->add_command("prows", new PixelRowsFunc(comterp, this));
    comterp->add_command("pflush", new PixelFlushFunc(comterp, this));
    comterp->add_command("pclip", new PixelClipFunc(comterp, this));
    comterp->add_command("alpha", new AlphaTransFunc(comterp, this));

    comterp->add_command("trans", new TransformerFunc(comterp, this));

    comterp->add_command("gravity", new GravityFunc(comterp, this));
    comterp->add_command("gridspacing", new GridSpacingFunc(comterp, this));

    comterp->add_command("hide", new HideCompFunc(comterp, this));
    comterp->add_command("show", new ShowCompFunc(comterp, this));
    comterp->add_command("desensitize", new DesensitizeCompFunc(comterp, this));
    comterp->add_command("sensitize", new SensitizeCompFunc(comterp, this));

    #if defined(LEAKCHECK) 
    comterp->add_command("compleak", new CompLeakFunc(comterp));
    comterp->add_command("viewleak", new ViewLeakFunc(comterp));
    comterp->add_command("alistleak", new AlistLeakFunc(comterp));
    comterp->add_command("attrvleak", new AttrvLeakFunc(comterp));
    comterp->add_command("mlineleak", new MlineLeakFunc(comterp));
    comterp->add_command("graphicleak", new GraphicLeakFunc(comterp));
    comterp->add_command("commandleak", new CommandLeakFunc(comterp));
    #endif

    comterp->add_command("pointer", new PointerLocFunc(comterp, this));
    comterp->add_command("lastkey", new LastKeyFunc(comterp, this));

    comterp->add_command("beep", new ComdrawBeepFunc(comterp, this));
    comterp->add_command("ding", new ComdrawDingFunc(comterp, this));
}

/* virtual */ void ComEditor::ExecuteCmd(Command* cmd) {
  if(!whiteboard()) 

    /* normal Unidraw command execution */
    OverlayEditor::ExecuteCmd(cmd);

  else {

    /* indirect command execution, all by script */
    std::ostrstream sbuf;
    boolean oldflag = OverlayScript::ptlist_parens();
    OverlayScript::ptlist_parens(false);
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
      cout << sbuf.str() << "\n";
      cout.flush();
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
    OverlayScript::ptlist_parens(oldflag);
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

void ComEditor::stdio_setup(UnidrawComterpHandler* handler) {
  if (handler) {
    SetComTerp(handler->comterp());
    handler->comterp()->outfunc() = (outfuncptr)&stdout_puts;
  }
}

void ComEditor::stdio_prompt(UnidrawComterpHandler* handler) {
  if (handler!=NULL) 
    (*handler->comterp()->outfunc()) (get_command_prompt(), nil);
  else
    fprintf(stdout, "%s", get_command_prompt());
}

/*****************************************************************************/
// keyboard eavesdrop + shift-arrow capture for the comterp lastkey()
// command (see comeditor.h).  The pointer() analog on the keyboard side:
// keystroke() queues every keysym, and optionally routes Shift+arrow to
// the queue while suppressing its viewer-pan.

void ComEditor::keystroke(const Event& e) {
    // shift-arrow capture (opt-in, default off): while on, a MODIFIED arrow
    // or letter -- Shift held OR Caps Lock on -- is routed to the key queue
    // with SHIFTARROW_FLAG and its normal action (arrow pan, letter tool
    // shortcut) is suppressed, so a script owns the keyboard while it drives.
    // Caps Lock is the hands-free enable (and the natural two-player enable:
    // both players just tap their keys).  Bare (unmodified) arrows still pan
    // and bare letters still fire their tool shortcuts.  Use the level-0
    // keysym like the pan dispatch -- XLookupKeysym at the shifted index can
    // be NoSymbol for arrows -- so the queued code is the UNSHIFTED keysym
    // (e.g. Shift+D queues XK_d|FLAG); the script maps those.
    if ((e.shift_is_down() || e.capslock_is_down())
	&& e.rep()->xevent_.type == KeyPress
	&& shiftarrow_capture()) {
	KeySym base = XkbKeycodeToKeysym(e.rep()->display_->rep()->display_,
					 e.rep()->xevent_.xkey.keycode, 0, 0);
	if (base==XK_Up || base==XK_Down || base==XK_Left || base==XK_Right
	    || (base>=XK_a && base<=XK_z)) {
	    enqueue_key((unsigned long)base | SHIFTARROW_FLAG);
	    return;                       // suppress the pan / tool-shortcut
	}
    }
    // pure eavesdrop for lastkey(), then dispatch as usual.  e.keysym()
    // folds in shift, so Shift+d arrives as XK_D.
    enqueue_key(e.keysym());
    OverlayEditor::keystroke(e);
}

// key-event ring buffer.  On overflow the oldest key is dropped -- a game
// poll loop drains faster than a human types, so overflow means the script
// stopped reading, in which case stale keys are the right thing to lose.
void ComEditor::enqueue_key(unsigned long keysym) {
    if (keysym == 0) return;
    int next = (_keyq_tail + 1) % KEYQ_SIZE;
    if (next == _keyq_head)               // full: drop oldest
	_keyq_head = (_keyq_head + 1) % KEYQ_SIZE;
    _keyq[_keyq_tail] = keysym;
    _keyq_tail = next;
}

unsigned long ComEditor::dequeue_key() {
    if (_keyq_head == _keyq_tail) return 0;   // empty
    unsigned long k = _keyq[_keyq_head];
    _keyq_head = (_keyq_head + 1) % KEYQ_SIZE;
    return k;
}

// shift-arrow capture with a self-disarming watchdog (see comeditor.h).
void ComEditor::shiftarrow_capture(boolean on) {
    _shiftarrow_on = on;
    if (on) _shiftarrow_deadline = comeditor_now_seconds() + SHIFTARROW_TIMEOUT;
}

boolean ComEditor::shiftarrow_capture() {
    if (_shiftarrow_on && comeditor_now_seconds() > _shiftarrow_deadline)
	_shiftarrow_on = false;           // watchdog lapsed -> auto-restore
    return _shiftarrow_on;
}

void ComEditor::shiftarrow_poll() {
    if (_shiftarrow_on)
	_shiftarrow_deadline = comeditor_now_seconds() + SHIFTARROW_TIMEOUT;
}

// map a queued key code to a portable name (see comeditor.h).  The X keysym
// stays inside this one function; the returned string is the lastkey()
// surface, so a non-X backend reimplements only this mapping.
const char* ComEditor::keyname(unsigned long code) {
    boolean cap = (code & SHIFTARROW_FLAG) != 0;
    unsigned long ks = code & ~(unsigned long)SHIFTARROW_FLAG;
    const char* base;
    char one[2];
    switch (ks) {
      case XK_Up:        base = "up";    break;
      case XK_Down:      base = "down";  break;
      case XK_Left:      base = "left";  break;
      case XK_Right:     base = "right"; break;
      case XK_Escape:    base = "esc";   break;
      case XK_space:     base = "space"; break;
      case XK_Return:    base = "enter"; break;
      case XK_Tab:       base = "tab";   break;
      case XK_BackSpace: base = "bs";    break;
      case XK_Delete:    base = "del";   break;
      default:
	if ((ks>=XK_a && ks<=XK_z) || (ks>=XK_0 && ks<=XK_9)) {
	    one[0] = (char)ks; one[1] = '\0'; base = one;
	} else {
	    // unmapped: decimal keysym so it's still usable (rarely hit)
	    snprintf(_keyname_buf, sizeof(_keyname_buf), cap ? "S-%lu" : "%lu", ks);
	    return _keyname_buf;
	}
    }
    snprintf(_keyname_buf, sizeof(_keyname_buf), cap ? "S-%s" : "%s", base);
    return _keyname_buf;
}
