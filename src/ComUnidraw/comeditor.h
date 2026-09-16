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
    // runs Unidraw Command objects after OverlayEditor is constructed.
    virtual void AddCommands(ComTerp*);
    // adds ComFunc objects to the ComTerp associated with this ComEditor.

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
    // tests for distributed whiteboard mode, only possible with a ComEditor.

    void stdio_setup(UnidrawComterpHandler* handler);
    // initial setup of stdio handler that writes to stdout.
    void stdio_prompt(UnidrawComterpHandler* handler);
    // conditionally generates the (comt) prompt for interactive sessions;
    // called once at startup, then ComUtil/_lexscan.c takes over.

    // keyboard eavesdrop for lastkey(): keystroke() queues every keysym,
    // lastkey() dequeues; a bare modifier rides on the next key instead.
    virtual void keystroke(const Event&);
    void enqueue_key(unsigned long keysym);
    unsigned long dequeue_key();          // 0 when the queue is empty

    // shift-capture (opt-in): queues a Shift/CapsLock-modified arrow/letter
    // with SHIFT_FLAG, suppressing it; a watchdog auto-restores default.

    // CTRL_FLAG/ALT_FLAG/SUPER_FLAG carry modifier state on every key;
    // bits 32+ stay clear of the 32-bit keysym space, incl. XF86 media keys.
    enum { SHIFT_FLAG = 1UL<<32, CTRL_FLAG = 1UL<<33, ALT_FLAG = 1UL<<34, SUPER_FLAG = 1UL<<35 };
    void shiftcapture(boolean on);  // enable/disable + (re)arm
    // live state, not pure: reading lazily expires the watchdog, so
    // two calls back-to-back can see true then false with no poll between.
    boolean shiftcapture();
    void shiftcapture_poll();       // heartbeat: bump the watchdog

    // portable name for a queued key code: a C literal for printable-ASCII/
    // space/enter; TAB/arrows/ins/Backspace(->"DEL") case-vary; \x7f Delete
    // never does; F1-PgDn fixed; else decimal.

    // Escape is "Esc"/"ESC" (case-varies like Tab), never the raw \x1b byte.

    // Ctrl/Alt/Super chord as fixed "Ctrl-Alt-Super-<key>" prefix; a chorded
    // letter is always capital; Shift on a chord capitalizes the prefix.

    // CONTRACT: the returned pointer aliases a single persistent buffer
    // (_keyname_buf); next keyname() call overwrites it -- copy before then.
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
