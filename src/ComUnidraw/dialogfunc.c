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

#include <ComUnidraw/dialogfunc.h>
#include <IVGlyph/gdialogs.h>
#include <InterViews/window.h>

#define TITLE "DialogFunc"

/*****************************************************************************/

AcknowledgeBoxFunc::AcknowledgeBoxFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void AcknowledgeBoxFunc::execute() {
  ComValue msgstrv(stack_arg(0));
  reset_stack();
  GAcknowledgeDialog::post(GetEditor()->GetWindow(), msgstrv.symbol_ptr());
}

/*****************************************************************************/

ConfirmBoxFunc::ConfirmBoxFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void ConfirmBoxFunc::execute() {
  ComValue msgstrv(stack_arg(0));
  reset_stack();
  int status = GConfirmDialog::post(GetEditor()->GetWindow(), msgstrv.symbol_ptr());
  ComValue retval(status);
  push_stack(retval);
}


