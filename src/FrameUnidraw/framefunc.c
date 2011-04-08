/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framefunc.h>
#include <FrameUnidraw/frameviews.h>
#include <Unidraw/iterator.h>
#include <Unidraw/viewer.h>
#include <ComTerp/comvalue.h>

/*****************************************************************************/

MoveFrameFunc::MoveFrameFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void MoveFrameFunc::execute() {
  ComValue deltav(stack_arg(0, false, ComValue::oneval()));
  static int abs_symid = symbol_find("abs");
  ComValue absflag(stack_key(abs_symid));
  reset_stack();

  if (editor() && deltav.is_num()) {
    int deltaframes = 0;
    if (absflag.is_true()) {
      FramesView* fv = (FramesView*)GetEditor()->GetViewer()->GetGraphicView();
      Iterator it;
      fv->SetView(((FrameEditor*)GetEditor())->GetFrame(), it);
      int currframe = fv->Index(it);
      deltaframes = deltav.int_val() - currframe;
    }
    else
      deltaframes = deltav.int_val();
    MoveFrameCmd* cmd = new MoveFrameCmd(GetEditor(), deltaframes);
    cmd->wraparound(MoveFrameCmd::default_instance()->wraparound());
    execute_log(cmd);
    ComValue retval(cmd->actualmotion(), ComValue::IntType);
    push_stack(retval);
  }
}

/*****************************************************************************/

CreateFrameFunc::CreateFrameFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void CreateFrameFunc::execute() {
  static int before_symval = symbol_add("before");
  ComValue beforev(stack_key(before_symval));
  reset_stack();

  if (editor()) {
    CreateMoveFrameCmd* cmd = 
      new CreateMoveFrameCmd(editor(), beforev.is_false());
    execute_log(cmd);
    ComValue retval(cmd->moveframecmd()->actualmotion(), ComValue::IntType);
    push_stack(retval);
  }
}

