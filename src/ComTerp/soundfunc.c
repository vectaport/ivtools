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

int BeepFunc::_beep_count = 0;
int DingFunc::_ding_count = 0;

/*****************************************************************************/

BeepFunc::BeepFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void BeepFunc::execute() {
  static int count_sym = symbol_add("count");
  ComValue countv(stack_key(count_sym));
  reset_stack();
  if (countv.is_true()) {
    ComValue retval(_beep_count, ComValue::IntType);
    push_stack(retval);
    return;
  }
  static boolean afplay = bincheck("afplay");
  if (afplay)
    // best-effort sound: consume system()'s result (a (void) cast doesn't
    // suppress -Wunused-result for it) -- a failed beep must not disturb anyone
    { if (system("afplay /System/Library/Sounds/Pop.aiff &") != 0) { } }
  else {
    FILE* tty = fopen("/dev/tty", "w");
    if (tty) {
      fputs("\a", tty);
      fflush(tty);
      fclose(tty);
    }
  }
  _beep_count++;
}


/*****************************************************************************/

DingFunc::DingFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DingFunc::execute() {
  static int count_sym = symbol_add("count");
  ComValue countv(stack_key(count_sym));
  reset_stack();
  if (countv.is_true()) {
    ComValue retval(_ding_count, ComValue::IntType);
    push_stack(retval);
    return;
  }
  static boolean afplay = bincheck("afplay");
  if (afplay)
      // best-effort sound (see BellFunc above); consume the result so
      // -Wunused-result is satisfied, but ignore a failed beep
      { if (system("afplay /System/Library/Sounds/Funk.aiff &") != 0) { } }
  else {
    FILE* tty = fopen("/dev/tty", "w");
    if (tty) {
      fputs("\a\a\a", tty);
      fflush(tty);
      fclose(tty);
    }
  }
  _ding_count++;
}
    
