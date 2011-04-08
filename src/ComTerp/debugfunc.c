/*
 * Copyright (c) 2000 IET Inc.
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

#include <ComTerp/debugfunc.h>
#include <ComTerp/comterp.h>

#define TITLE "DebugFunc"

/*****************************************************************************/

TraceFunc::TraceFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TraceFunc::execute() {
  static int get_symid = symbol_add("get");
  boolean get_flag = stack_key(get_symid).is_true();
  if (get_flag) {
    reset_stack();
    int mode = comterp()->trace_mode();
    ComValue retval(mode, ComValue::IntType);
    push_stack(retval);
  } else {
    if (nargs()==0) {
      reset_stack();
      int mode = !comterp()->trace_mode();
      comterp()->trace_mode(mode);
      ComValue retval(mode, ComValue::IntType);
      push_stack(retval);
    } else {
      ComValue retval(stack_arg(0));
      reset_stack();
      comterp()->trace_mode(retval.int_val());
      push_stack(retval);
    }
  }
}

