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
    virtual void keystroke(const Event&);
    void enqueue_key(unsigned long keysym);
    unsigned long dequeue_key();          // 0 when the queue is empty

    // optional shift-arrow capture (default OFF): while on, a modified arrow
    // OR letter (Shift held OR Caps Lock on) is queued as (unshifted_keysym
    // | SHIFTARROW_FLAG) and its normal action -- arrow pan, letter tool
    // shortcut -- is suppressed, so a script owns the keyboard while it
    // drives.  Bare (unmodified) arrows/letters are untouched.  Caps Lock is
    // the hands-free enable (and the two-player enable).  A watchdog bumped
    // by every lastkey() poll auto-restores the default if the driving
    // script stops polling (exit, crash, ^C) -- so it can't stick.
    enum { SHIFTARROW_FLAG = 0x10000 };
    void shiftarrow_capture(boolean on);  // enable/disable + (re)arm
    boolean shiftarrow_capture();         // live state (false once expired)
    void shiftarrow_poll();               // heartbeat: bump the watchdog

    // Portable name for a queued key code: "up"/"down"/"left"/"right",
    // "esc"/"space"/"enter"/"tab"/"bs"/"del", a single char for letters and
    // digits, else the decimal keysym.  A captured (shift/caps) key gets an
    // "S-" prefix ("S-up", "S-d").  This is the lastkey() surface -- scripts
    // compare names, never raw X keysyms, so a Qt (or other) backend need
    // only map its key codes to the same names.
    const char* keyname(unsigned long code);

protected:

    ComTerpServ* _terp;
    ComTerpIOHandler* _terp_iohandler;
    int _whiteboard; // -1 == unitialized, 0 = false, 1 = true
    enum { KEYQ_SIZE = 32 };
    unsigned long _keyq[KEYQ_SIZE];
    int _keyq_head;
    int _keyq_tail;
    boolean _shiftarrow_on;
    double _shiftarrow_deadline;          // seconds; capture expires past this
    char _keyname_buf[32];                // scratch for keyname()
};

#endif
