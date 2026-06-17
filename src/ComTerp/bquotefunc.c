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
  ComValue retval(stack_arg(0, true));
  reset_stack();

  /* `StreamObj is the obsolete name for `StreamType (the printed name of
     a StreamType value, as returned by class()).  Warn once per session
     so scripts still carrying the old back-quoted name are nudged forward
     without flooding stderr from a loop.  To silence: comment this out. */
  static int streamobj_symid = symbol_add("StreamObj");
  static boolean streamobj_warned = false;
  if (!streamobj_warned &&retval.type() == ComValue::SymbolType &&
      retval.symbol_val() == streamobj_symid) {
    fprintf(stderr, "warning: `StreamObj is obsolete; use `StreamType\n");
    streamobj_warned = true;
  }
  
  retval.bquote(1);
  push_stack(retval);
}
