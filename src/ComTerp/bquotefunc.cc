/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1998,1999,2000 Vectaport Inc.
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

#include <ComTerp/bquotefunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#define TITLE "BackQuoteFunc"

/*****************************************************************************/

BackQuoteFunc::BackQuoteFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void BackQuoteFunc::execute() {
#if 0  // maybe, maybe not
  ComValue topval(stack_arg(0, true));
  if (topval.is_command())
      topval.assignval(stack_arg_post_eval(0, true /* no symbol lookup */));
  reset_stack();
  topval.bquote(1);
  push_stack(topval);
#else
  ComValue retval(stack_arg(0, true));
  reset_stack();
  retval.bquote(1);
  push_stack(retval);
#endif  
}
