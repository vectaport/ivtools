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

Summary:         The OS's own cooked-mode tty echo displays every pasted
                  character the instant it lands in the kernel input
                  buffer -- before the application ever calls read(), and
                  well before it's ready to interleave that text with each
                  line's own result.  For a multi-line paste, this dumps
                  the whole block to the screen at once, then the
                  interpreter works through it one line at a time
                  afterward, leaving the (comt) prompts and results out of
                  sync with the echoed text (issue #76).

                  tty_echo_off() disables only the OS's ECHO bit (ICANON
                  stays set -- line editing, backspace, Ctrl-U etc. still
                  work exactly as before); the caller is then responsible
                  for echoing each line itself at the moment it becomes
                  known, which is exactly what _lexscan.c does immediately
                  after each successful infunc() read (see tty_echo_is_off()).
                  This keeps echo and execution correctly interleaved
                  regardless of whether the line arrived by typing or by
                  paste, and whether it's read via a blocking fgets loop
                  (plain comterp) or one byte at a time from a reactor
                  callback (comdraw/drawserv's ComterpHandler) -- the OS
                  echo suppression and the self-echo both operate on
                  whole lines either way.

History:         Added for issue #76, July 2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

static int _tty_echo_off = 0;
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
    atexit(tty_echo_restore);
}

int tty_echo_is_off(void) { return _tty_echo_off; }

/* One-shot self-echo suppression for a single internal, not-typed-by-the-
   user eval -- e.g. comdraw/drawserv's startup seed update(1000000).  NOT
   a general "disable prompt for the duration of a call" mechanism: unlike
   disable_prompt()/enable_prompt() (_parser.c), which stay set for as long
   as the caller holds them open, this flag is consumed and cleared the
   instant _lexscan.c reads the very next line -- during read_expr(), which
   runs to completion before eval_expr() (and whatever event-loop pumping
   the evaluated command does) ever starts.  So it never overlaps the
   window where a reentrant stdin event could observe it still set; a
   held-open flag spanning the whole call does (see issue #76 history --
   that's what broke comdraw's own reentrant-paste handling when tried). */
static int _suppress_next_echo = 0;

void tty_echo_suppress_next(void) { _suppress_next_echo = 1; }

int tty_echo_consume_suppress_next(void) {
    int flag = _suppress_next_echo;
    _suppress_next_echo = 0;
    return flag;
}
