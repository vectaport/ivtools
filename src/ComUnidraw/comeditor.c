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
#include <IV-X11/xevent.h>
#include <X11/keysym.h>       /* XK_Up/Down/Left/Right, modifier keysyms, etc. */
#include <time.h>              /* clock_gettime for the shift-arrow watchdog */
#include <ctype.h>              /* toupper for keyname()'s shift signaling */

/* CLOCK_MONOTONIC, not wall-clock time: an NTP correction or a manual clock
   change must never fire (or extend) the watchdog early/late. */
static double comeditor_now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}
static const double SHIFTCAPTURE_TIMEOUT = 2.0;   // seconds without a poll -> disarm

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
  _shiftcapture_on = false;
  _shiftcapture_deadline = 0.0;
}

void ComEditor::Init (OverlayComp* comp, const char* name) {
    _keyq_head = _keyq_tail = 0;
    _shiftcapture_on = false;
    _shiftcapture_deadline = 0.0;
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
    comterp->add_command("keyname_test", new KeynameTestFunc(comterp, this), nil, nil, true /* hidden: test-only, see keyname_test's docstring */);

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
    KeySym ks = e.keysym();

    // a bare modifier keypress -- Shift/Ctrl/CapsLock/Alt/Meta/Super/Hyper
    // pressed on its own, XK_Shift_L..XK_Hyper_R is the whole contiguous
    // block in keysymdef.h -- is never a "key" lastkey() should report; its
    // effect is already carried as SHIFT_FLAG on whatever key comes next.
    if (ks >= XK_Shift_L && ks <= XK_Hyper_R) {
	OverlayEditor::keystroke(e);
	return;
    }

    boolean shifted = e.shift_is_down() || e.capslock_is_down();

    // Ctrl/Alt/Super ride along as informational bits on every key, just
    // like SHIFT_FLAG -- orthogonal to :shiftcapture below, which
    // only decides whether Shift's normal pan/tool-shortcut is ALSO
    // suppressed.  meta_is_down() tests Mod1Mask, which on essentially
    // every current keyboard IS Alt (Alt and Meta share the same bit; a
    // keyboard with a dedicated Meta key hasn't shipped since the
    // Lisp-machine era).  Event has no super_is_down() of its own, so
    // Mod4 (the Windows-logo key on a PC, or Cmd under XQuartz on a Mac)
    // is tested directly via keymask().
    unsigned long flags = (shifted ? SHIFT_FLAG : 0)
	| (e.control_is_down()      ? CTRL_FLAG  : 0)
	| (e.meta_is_down()         ? ALT_FLAG   : 0)
	| ((e.keymask() & Mod4Mask) ? SUPER_FLAG : 0);

    // shift-capture (opt-in, default off): while on, a MODIFIED arrow or
    // letter -- Shift held OR Caps Lock on -- is routed to the key queue
    // with SHIFT_FLAG and its normal action (arrow pan, letter tool
    // shortcut) is suppressed, so a script owns the keyboard while it
    // drives.  Caps Lock is the hands-free enable (and the natural
    // two-player enable: both players just tap their keys).  Bare
    // (unmodified) arrows still pan and bare letters still fire their tool
    // shortcuts.  e.keysym() already folds shift in (Shift+d arrives as
    // XK_D), so the queued code carries its natural case; keyname() only
    // has to invent an uppercase form for keys that don't have one already.
    if (shifted && e.rep()->xevent_.type == KeyPress && shiftcapture()) {
	if (ks==XK_Up || ks==XK_Down || ks==XK_Left || ks==XK_Right
	    || (ks>=XK_a && ks<=XK_z) || (ks>=XK_A && ks<=XK_Z)) {
	    enqueue_key((unsigned long)ks | flags);
	    return;                       // suppress the pan / tool-shortcut
	}
    }
    // pure eavesdrop for lastkey(), then dispatch as usual.
    enqueue_key((unsigned long)ks | flags);
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

// shift-capture with a self-disarming watchdog (see comeditor.h).
void ComEditor::shiftcapture(boolean on) {
    _shiftcapture_on = on;
    if (on) _shiftcapture_deadline = comeditor_now_seconds() + SHIFTCAPTURE_TIMEOUT;
}

// impure by design (see comeditor.h): this is the one place the watchdog
// actually gets checked, so the getter itself has to do the expiring --
// there is no separate poll/tick callback that could do it instead.
boolean ComEditor::shiftcapture() {
    if (_shiftcapture_on && comeditor_now_seconds() > _shiftcapture_deadline)
	_shiftcapture_on = false;           // watchdog lapsed -> auto-restore
    return _shiftcapture_on;
}

void ComEditor::shiftcapture_poll() {
    if (_shiftcapture_on)
	_shiftcapture_deadline = comeditor_now_seconds() + SHIFTCAPTURE_TIMEOUT;
}

// map a queued key code to a portable name (see comeditor.h).  The X keysym
// stays inside this one function; the returned string is the lastkey()
// surface, so a non-X backend reimplements only this mapping.
/* core_keyname() is everything keyname() did before Ctrl/Alt/Super
   existed -- the bare (possibly shift-varied) name for a keysym, with
   all modifier flag bits already stripped off by the caller.  Split out
   as a free function so keyname() can compute this into a scratch
   buffer, then optionally wrap it with a "Ctrl-"/"Alt-"/"Super-" prefix,
   without the two concerns tangled into one control flow.

   `chorded` is true when the caller is about to prefix the result with
   Ctrl/Alt/Super.  It only changes anything for the six control-
   character keys: Esc/Tab/Backspace already had a readable uppercase
   name for real Shift; Enter/Space/true-Delete didn't (no established
   Shift+Enter/Space/Delete convention -- see keyname()'s docstring).
   But gluing a raw \r/space/\x7f byte onto a "Ctrl-" prefix would leave
   an unprintable, hard-to-compare chord string, so `chorded` borrows the
   same readable-name treatment for those three too, without changing
   their BARE (unchorded, unshifted) behavior at all. */
static const char* core_keyname(unsigned long ks, boolean shifted, boolean chorded,
				 char* buf, size_t bufsz) {
    boolean textual = shifted || chorded;
    switch (ks) {
      case XK_Escape:    return textual ? "ESC"    : "\x1b";
      case XK_space:     return chorded ? "SPACE"  : " ";
      case XK_Return:    return chorded ? "ENTER"  : "\r";
      case XK_Tab:       return textual ? "TAB"    : "\t";
      case XK_BackSpace: return textual ? "DEL"    : "\b";
      case XK_Delete:    return chorded ? "DELETE" : "\x7f";
    }

    /* function keys, and Home/End/PgUp/PgDn, use a fixed capitalized
       name always, ignoring shift/caps-lock -- ordinary keyboard-label
       spelling ("F1", "Home", "PgUp"), not curses' abbreviations
       (KEY_PPAGE/KEY_NPAGE -> "ppage"/"npage" seemed like a good idea
       at the time, but nobody actually calls Page Up "previous page").
       No established "shifted" convention exists for any of these the
       way Shift+arrow or Shift+letter do, so there's no case to vary --
       grouped with the char-literal keys above: a fixed return, shifted
       unconsulted. */
    switch (ks) {
      case XK_F1:    return "F1";
      case XK_F2:    return "F2";
      case XK_F3:    return "F3";
      case XK_F4:    return "F4";
      case XK_F5:    return "F5";
      case XK_F6:    return "F6";
      case XK_F7:    return "F7";
      case XK_F8:    return "F8";
      case XK_F9:    return "F9";
      case XK_F10:   return "F10";
      case XK_F11:   return "F11";
      case XK_F12:   return "F12";
      case XK_Home:  return "Home";
      case XK_End:   return "End";
      case XK_Prior: return "PgUp";  // curses KEY_PPAGE
      case XK_Next:  return "PgDn";  // curses KEY_NPAGE
    }

    const char* base;
    char one[2];
    switch (ks) {
      case XK_Up:        base = "up";    break;
      case XK_Down:      base = "down";  break;
      case XK_Left:      base = "left";  break;
      case XK_Right:     base = "right"; break;
      /* Insert has no character-literal form either, and Shift+Insert
	 (paste, on Windows/Linux/many X11 apps) IS an established
	 convention -- unlike the fixed keys above -- so it stays here,
	 case-varying like the arrows. */
      case XK_Insert:    base = "ins";   break;  // curses KEY_IC; "ins" reads better
      default:
	/* X11's Latin-1 keysyms are numerically identical to their ASCII
	   codepoint across the whole printable range (space 0x20 through
	   tilde 0x7e) -- verified directly, not just for letters/digits:
	   [ ] { } ( ) < > ` ' " : ; , . and every shifted-numeric symbol
	   (! @ # $ % ^ & *) all match too.  So any printable-ASCII keysym
	   just IS its own character.  keystroke() folds shift into ks
	   itself for letters (Shift+d arrives as XK_D) and X11 already
	   resolves shifted symbols to their own distinct keysym (Shift+[
	   is XK_braceleft, not XK_bracketleft), so this already carries
	   the right character either way -- the uppercasing loop below is
	   a harmless no-op for anything that isn't a lowercase letter. */
	if (ks>=0x20 && ks<=0x7e) {
	    one[0] = (char)ks; one[1] = '\0'; base = one;
	} else {
	    // unmapped: decimal keysym so it's still usable (rarely hit)
	    snprintf(buf, bufsz, "%lu", ks);
	    return buf;
	}
    }

    /* arrows and ins have no shifted form of their own, so shift/caps-lock
       is signaled by uppercasing the whole name instead ("up" -> "UP") --
       the same convention letters already get for free from their
       keysym.  One vocabulary, no "S-" prefix. */
    if (shifted) {
	int i = 0;
	for (const char* p = base; *p && i < (int)bufsz-1; p++, i++)
	    buf[i] = toupper((unsigned char)*p);
	buf[i] = '\0';
    } else {
	snprintf(buf, bufsz, "%s", base);
    }
    return buf;
}

// returns a pointer into _keyname_buf, a single persistent member -- see
// the CONTRACT note on the declaration (comeditor.h): valid only until the
// next keyname() call.  Both current callers (LastKeyFunc, KeynameTestFunc
// in unifunc.c) are safe because they immediately hand the pointer to
// ComValue's string constructor, which copies/interns it before any
// second call could happen -- a future caller that holds onto the raw
// pointer across two calls would silently read stale data.
const char* ComEditor::keyname(unsigned long code) {
    boolean shifted = (code & SHIFT_FLAG) != 0;
    boolean ctrl    = (code & CTRL_FLAG)  != 0;
    boolean alt     = (code & ALT_FLAG)   != 0;
    boolean super_  = (code & SUPER_FLAG) != 0;
    unsigned long ks = code & ~(unsigned long)(SHIFT_FLAG|CTRL_FLAG|ALT_FLAG|SUPER_FLAG);
    boolean chorded = ctrl || alt || super_;

    char corebuf[16];
    const char* core = core_keyname(ks, shifted, chorded, corebuf, sizeof(corebuf));
    if (!chorded) {
	/* core may point into corebuf, a LOCAL array -- copy through
	   _keyname_buf (a persistent member, outlives the call) before
	   returning, same as every other return in this function.
	   Returning `core` directly here was the bug: for the common
	   case (any letter/digit/punctuation/arrow with no Ctrl/Alt/
	   Super held), it silently handed the caller a dangling pointer
	   into a stack frame that no longer existed. */
	snprintf(_keyname_buf, sizeof(_keyname_buf), "%s", core);
	return _keyname_buf;
    }

    /* Ctrl/Alt/Super chords: fixed "Ctrl-Alt-Super-<key>" order (Ctrl
       first, matching Emacs/GNOME/Windows documentation convention),
       each word capitalized when Shift is ALSO held -- that's the one
       remaining channel to signal Shift on a chord, because a single
       letter is ALWAYS shown capital right after a modifier prefix
       regardless of whether Shift was literally down: nobody documents
       "Ctrl-c", every OS/toolkit writes "Ctrl-C" even for a bare
       Ctrl+c with no Shift -- which spends the letter's own case, so
       Shift has to show up on the prefix word instead. */
    char keypart[8];
    if (core[0] && !core[1] && core[0]>='a' && core[0]<='z') {
	keypart[0] = toupper((unsigned char)core[0]);
	keypart[1] = '\0';
	core = keypart;
    }

    _keyname_buf[0] = '\0';
    if (ctrl)
	strncat(_keyname_buf, shifted ? "CTRL-" : "Ctrl-", sizeof(_keyname_buf)-strlen(_keyname_buf)-1);
    if (alt)
	strncat(_keyname_buf, shifted ? "ALT-" : "Alt-", sizeof(_keyname_buf)-strlen(_keyname_buf)-1);
    if (super_)
	strncat(_keyname_buf, shifted ? "SUPER-" : "Super-", sizeof(_keyname_buf)-strlen(_keyname_buf)-1);
    strncat(_keyname_buf, core, sizeof(_keyname_buf)-strlen(_keyname_buf)-1);
    return _keyname_buf;
}
