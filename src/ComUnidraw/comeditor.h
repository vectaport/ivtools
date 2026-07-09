/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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

#ifndef comeditor_h
#define comeditor_h

#include <OverlayUnidraw/oved.h>
#include <ComUnidraw/comkit.h>

class ComTerpIOHandler;
class ComTerp;
class ComTerpServ;
class UnidrawComterpHandler;

//: editor that integrates ComTerp into the drawing editor framework.
class ComEditor : public OverlayEditor {
public:
    ComEditor(OverlayComp*, OverlayKit* = ComKit::Instance());
    // constructor for using existing component.
    ComEditor(const char* file, OverlayKit* = ComKit::Instance());
    // constructor for building top-level component from a file.
    ComEditor(boolean initflag, OverlayKit* = ComKit::Instance());
    // constructor for use of derived classes.
    void Init(OverlayComp* = nil, const char* name = "ComEditor");
    virtual void InitCommands();
    // method for running Unidraw Command objects after OverlayEditor
    // is constructed.
    virtual void AddCommands(ComTerp*);
    // method for adding ComFunc objects to the ComTerp associated with
    // this ComEditor.

    virtual ComTerpServ* GetComTerp() { return _terp;}
    // return pointer to associated ComTerp (always a ComTerpServ).
    virtual ComTerpServ* comterp() { return _terp;}
    // return pointer to associated ComTerp (always a ComTerpServ).
    virtual void SetComTerp(ComTerpServ* terp) { _terp = terp;}
    // set pointer to associated ComTerp (always a ComTerpServ).
    virtual void comterp(ComTerpServ* terp) { _terp = terp;}
    // set pointer to associated ComTerp (always a ComTerpServ).

    virtual void ExecuteCmd(Command* cmd);
    // indirect command execution for distributed whiteboard mechanism.

    boolean whiteboard();
    // test for distributed whiteboard mode, which only exists when
    // used with a ComEditor.

    void stdio_setup(UnidrawComterpHandler* handler);
    // initial setup of stdio handler that writes to stdout.
    void stdio_prompt(UnidrawComterpHandler* handler);
    // conditionally generate (comt) prompt for interactive sessions
    // called only once to add the prompt after initial startup
    // after that it happens in ComUtil/_lexscan.c

    // -- keyboard eavesdrop for the comterp lastkey() command --
    // keystroke() (overridden below) queues every keysym the canvas
    // receives; lastkey() dequeues.  This is the keyboard analog of the
    // pointer capture on OverlayViewer (read by pointer()) -- capture
    // state lives here in the comdraw-layer editor, not in base Unidraw.
    // A bare modifier keypress (Shift/Ctrl/CapsLock/Alt/Meta/Super/Hyper,
    // pressed on its own) is never queued at all -- it isn't a "key" a
    // script would want reported; its effect is the shift/capslock/ctrl/
    // alt/super state carried on whatever key comes next (see the _FLAG
    // enum below).
    virtual void keystroke(const Event&);
    void enqueue_key(unsigned long keysym);
    unsigned long dequeue_key();          // 0 when the queue is empty

    // optional shift-capture (default OFF): while on, a modified arrow OR
    // letter (Shift held OR Caps Lock on) is queued with SHIFT_FLAG set
    // and its normal action -- arrow pan, letter tool shortcut -- is
    // suppressed, so a script owns the keyboard while it drives.  Named
    // "shift-capture", not "shift-arrow": it was arrow-only early on, but
    // now covers letters too, and the name should describe the mechanism
    // (capturing a Shift-modified key), not one now-stale example of what
    // it applies to.  Bare (unmodified) arrows/letters are untouched.
    // Caps Lock is the hands-free enable (and the two-player enable).  A
    // watchdog bumped by every lastkey() poll auto-restores the default
    // if the driving script stops polling (exit, crash, ^C) -- so it
    // can't stick.  SHIFT_FLAG itself is orthogonal to capture, though:
    // keystroke() sets it (and CTRL_FLAG/ALT_FLAG/SUPER_FLAG) on ANY key
    // (captured or not) pressed while that modifier is down, so
    // keyname() can name/uppercase keys that have no natural case of
    // their own (see below) -- capture only additionally decides whether
    // the normal pan/shortcut also gets suppressed, and only ever looks
    // at Shift/Caps Lock, never Ctrl/Alt/Super.
    //
    // CTRL_FLAG/ALT_FLAG/SUPER_FLAG are the only three modifier "roles"
    // kept out of the historical five (Symbolics/Sun keyboards also had
    // Meta and Hyper) -- Meta's name is hopelessly overloaded across
    // platforms (Emacs and X11 itself often alias it onto the same bit as
    // Alt; Qt uses "Meta" to mean Super on Linux), and nothing built a
    // dedicated Hyper key after the Lisp-machine era, so neither carries
    // enough of a live, unambiguous convention to be worth naming.  Ctrl/
    // Alt/Super were picked because each maps cleanly onto one physical
    // key across Windows, Linux/X11, and Mac (via XQuartz): the Mac
    // Command key and a PC's Windows-logo key both land on X11's Mod4,
    // i.e. Super -- see keyname()'s docstring for how they combine with a
    // key name.
    // Bits 32-35, not 16-19: the X11 protocol caps every keysym at 32
    // bits, but XF86's vendor-specific multimedia range (0x1008xxxx --
    // volume/brightness/media keys) has bit 19 set as part of the
    // keysym ITSELF, colliding with a flag bit placed there (a real
    // keysym | flags bug, caught by review: pressing a media key with
    // no modifier held wrongly got "Super-" prepended, and lost a real
    // keysym bit besides).  unsigned long is 64-bit on every platform
    // this builds for, so bits 32+ are safely above ANY valid keysym --
    // including the 0x01000000-0x0110FFFF Unicode-mapped range, X11's
    // widest defined keysym space -- with no risk of ever colliding.
    enum { SHIFT_FLAG = 1UL<<32, CTRL_FLAG = 1UL<<33, ALT_FLAG = 1UL<<34, SUPER_FLAG = 1UL<<35 };
    void shiftcapture(boolean on);  // enable/disable + (re)arm
    boolean shiftcapture();         // live state -- NOT a pure query: lazily
                                     // expires (and clears) the watchdog on
                                     // read if the deadline has lapsed, so
                                     // two calls back-to-back can observe
                                     // true then false with no poll between
    void shiftcapture_poll();       // heartbeat: bump the watchdog

    // Portable name for a queued key code: a standard C character literal
    // for keys that have one -- every printable-ASCII key (letters,
    // digits, and all punctuation: brackets, braces, parens, quotes,
    // colon/semicolon, comma/period, every shifted-numeric symbol, etc.)
    // as itself, plus " " space and "\r" enter (always); "\x1b" esc,
    // "\t" tab, "\b" backspace when unshifted, else the fixed name
    // uppercase ("ESC"/"TAB"/"DEL" -- Shift-Tab is an established
    // reverse-focus/indent convention, Shift-Esc/Shift-Backspace get
    // the same uppercasing for uniformity); "\x7f" delete (always --
    // distinct from Backspace, no shifted form); "up"/"down"/"left"/
    // "right"/"ins" for keys with a meaningful shifted form; "F1".."F12"/
    // "Home"/"End"/"PgUp"/"PgDn" fixed, always that capitalization -- no
    // established shifted convention exists for any of these; else the
    // decimal keysym.  If shift or caps lock was down, arrows/ins come
    // back UPPERCASE ("UP", "INS") -- for letters and shifted-symbol
    // punctuation this already falls out of the keysym itself (Shift+d
    // arrives as XK_D, Shift+[ arrives as XK_braceleft i.e. '{'), for
    // arrows/ins/esc/tab/backspace keyname() applies it explicitly,
    // since they have no natural shifted form to fall back on.
    //
    // If Ctrl, Alt, and/or Super was held, the name above is prefixed
    // "Ctrl-"/"Alt-"/"Super-" (that fixed order, chained when more than
    // one is held: "Ctrl-Alt-Delete"), and a single lowercase letter is
    // forced capital after the prefix regardless of whether Shift was
    // literally down -- every OS/toolkit's own documentation always
    // writes "Ctrl-C", never "Ctrl-c", even for a bare Ctrl+c.  Since
    // that spends the letter's own case, Shift held ALONGSIDE a chord is
    // signaled by capitalizing the prefix word(s) instead: "Ctrl-C" is
    // plain Ctrl+c, "CTRL-C" is Ctrl+Shift+c.  Enter/Space/true-Delete,
    // normally shift-blind (raw "\r"/" "/"\x7f"), switch to their
    // readable name ("ENTER"/"SPACE"/"DELETE") whenever chorded, so a
    // chord string is never part text, part raw control byte.  This is
    // the lastkey() surface -- scripts compare names, never raw X
    // keysyms, so a Qt (or other) backend need only map its key codes
    // to the same names.
    //
    // CONTRACT: the returned pointer aliases a single persistent buffer
    // owned by this object (_keyname_buf below) -- it is NOT a copy, and
    // the NEXT call to keyname() overwrites it.  Consume the string (or
    // copy it, e.g. via ComValue's own copy-on-construct) before calling
    // keyname() again; never cache the returned pointer across two calls.
    const char* keyname(unsigned long code);

protected:

    ComTerpServ* _terp;
    ComTerpIOHandler* _terp_iohandler;
    int _whiteboard; // -1 == unitialized, 0 = false, 1 = true
    enum { KEYQ_SIZE = 32 };
    unsigned long _keyq[KEYQ_SIZE];
    int _keyq_head;
    int _keyq_tail;
    boolean _shiftcapture_on;
    double _shiftcapture_deadline;        // seconds; capture expires past this
    char _keyname_buf[32];                // scratch for keyname()
};

#endif
