/*
 * Copyright (c) 2026 Scott E. Johnston
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

#include <ComTerp/soundfunc.h>

#define TITLE "SoundFunc"

/*****************************************************************************/

BeepFunc::BeepFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void BeepFunc::execute() {
    reset_stack();
    static boolean afplay = bincheck("afplay");
    if (afplay)
      system("afplay /System/Library/Sounds/Pop.aiff &");
    else {
      FILE* tty = fopen("/dev/tty", "w");
      if (tty) {
        fputs("\a", tty);
        fflush(tty);
        fclose(tty);
      }
    }
}


/*****************************************************************************/

DingFunc::DingFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DingFunc::execute() {
    reset_stack();
    static boolean afplay = bincheck("afplay");
    if (afplay)
      system("afplay /System/Library/Sounds/Funk.aiff &");
    else {
      FILE* tty = fopen("/dev/tty", "w");
      if (tty) {
        fputs("\a\a\a", tty);
        fflush(tty);
        fclose(tty);
      }
    }
}
    
