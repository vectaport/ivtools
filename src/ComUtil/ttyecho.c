/*
 * Copyright (c) 2026 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
ttyecho.c        stdin echo control for interactive comterp/comdraw/drawserv

Summary:         The OS's cooked-mode tty echo displays every pasted character
                  the instant it lands in the kernel input buffer, before the
                  application calls read().  For a multi-line paste that puts
                  the whole block on screen at once, while the interpreter
                  works through it a line at a time afterward, leaving the
                  prompts and results out of sync with the echoed text.

                  tty_echo_off() clears only the ECHO bit -- ICANON stays set,
                  so line editing still works -- and the caller then echoes
                  each line itself as it becomes known, which is what
                  _lexscan.c does after each successful read.  That keeps echo
                  and execution interleaved whether the line was typed or
                  pasted, and whether it is read by a blocking fgets loop or a
                  byte at a time from a reactor callback.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

static int _tty_echo_off = 0;
static int _atexit_registered = 0;
static struct termios _tty_saved_state;

void tty_echo_restore(void) {
    if (_tty_echo_off) {
        /* only clear the flag on success -- if tcsetattr fails (e.g. the fd
           is no longer valid at exit time), leave it set so a later retry
           (atexit and the explicit call in ComTerp::exit() can both reach
           here) doesn't see a false "already restored" and skip trying again */
        if (tcsetattr(fileno(stdin), TCSANOW, &_tty_saved_state) == 0)
            _tty_echo_off = 0;
    }
}

void tty_echo_off(void) {
    if (_tty_echo_off || !isatty(fileno(stdin)))
        return;
    if (tcgetattr(fileno(stdin), &_tty_saved_state) != 0)
        return;
    struct termios raw = _tty_saved_state;
    raw.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSANOW, &raw) != 0)
        return;
    _tty_echo_off = 1;
    if (!_atexit_registered) {
        _atexit_registered = 1;
        atexit(tty_echo_restore);
    }
}

int tty_echo_is_off(void) { return _tty_echo_off; }

/* echo is held off only while a line is executing, not for the whole session:
   ICANON gives the application nothing until Return, so clearing ECHO for the
   duration leaves typing invisible.  Waiting at the prompt runs with ECHO on,
   so the OS shows typing and does its own line editing.

   The cost is that a paste landing at an idle prompt is echoed by the OS as one
   block, and self-echoing those bytes as they are consumed would show them
   twice.  _pre_echoed records how many were already pending, and therefore
   already echoed, when echo went off; they are charged off line by line. */
static long _pre_echoed = 0;
void tty_echo_hold(void);

static int tty_pending_bytes(void) {
    int navail = 0;
    if (!isatty(fileno(stdin))) return 0;
    if (ioctl(fileno(stdin), FIONREAD, &navail) != 0) return 0;
    return navail;
}

/* about to block for input: with nothing buffered a person is about to type,
   so give the OS its echo back.  Input already waiting is the rest of a burst
   -- leave echo off so it stays interleaved with each line's own result. */
void tty_echo_before_read(void) {
    if (tty_pending_bytes() == 0)
        tty_echo_restore();
}

/* a line has been read and execution is about to start: suppress echo so any
   further input arriving meanwhile is not shown ahead of its result.  Returns
   whether this line still needs echoing here -- false when the OS already did
   it, either because echo was on when it arrived or because it was part of a
   block counted in _pre_echoed. */
int tty_echo_after_read(const char* line) {
    int self_echo = _tty_echo_off;
    if (self_echo && _pre_echoed > 0) {
        _pre_echoed -= (long)strlen(line);
        if (_pre_echoed < 0) _pre_echoed = 0;
        self_echo = 0;
    }
    tty_echo_hold();
    return self_echo;
}

/* suppress echo for the execution about to start, remembering how much input
   was already pending -- those bytes the OS has already shown */
void tty_echo_hold(void) {
    if (!_tty_echo_off) {
        _pre_echoed = tty_pending_bytes();
        tty_echo_off();
    }
}


/* one-shot self-echo suppression for a single internal eval the user did not
   type.  Not a general "disable prompt for the duration of a call": unlike
   disable_prompt(), which stays set as long as the caller holds it open, this
   flag is consumed the instant the next line is read, during read_expr(),
   which completes before eval_expr() and any event-loop pumping begins.  So it
   never overlaps the window where a reentrant stdin event could observe it
   still set, which a held-open flag would. */
static int _suppress_next_echo = 0;

void tty_echo_suppress_next(void) { _suppress_next_echo = 1; }

int tty_echo_consume_suppress_next(void) {
    int flag = _suppress_next_echo;
    _suppress_next_echo = 0;
    return flag;
}

/* atexit() and the explicit call in ComTerp::exit() cover an orderly exit;
   neither runs when the process dies by signal, which for an interactive
   session is the common case -- Ctrl-C is SIGINT.  Otherwise a signal-killed
   session leaves the user's shell with ECHO cleared and typing invisible until
   `stty echo`.  Restore, then re-raise with the default disposition, so the
   process still dies of the signal with the right status; only the tty is
   cleaned up first. */
static void tty_echo_signal_handler(int sig) {
    tty_echo_restore();
    signal(sig, SIG_DFL);
    raise(sig);
}

void tty_echo_install_signal_handlers(void) {
    signal(SIGINT, tty_echo_signal_handler);
    signal(SIGTERM, tty_echo_signal_handler);
}
