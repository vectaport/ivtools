/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#include <ComUnidraw/unifunc.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovselection.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Components/compview.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comvalue.h>

#define TITLE "UnidrawFunc"

/*****************************************************************************/

int UnidrawFunc::_compview_id = -1;

UnidrawFunc::UnidrawFunc(ComTerp* comterp, Editor* ed) : ComFunc(comterp) {
    _ed = ed;
    if (_compview_id<0)
        _compview_id = symbol_add("CompView");
}

void UnidrawFunc::execute_log(Command* cmd) {
    if (cmd != nil) {
	cmd->Execute();
	
	if (cmd->Reversible()) {
	    cmd->Log();
	} else {
	    delete cmd;
	}
    }
}

/*****************************************************************************/

HandlesFunc::HandlesFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void HandlesFunc::execute() {
    ComValue& flag = stack_arg(0);
    if (flag.int_val()) 
	((OverlaySelection*)_ed->GetSelection())->EnableHandles();
    else
	((OverlaySelection*)_ed->GetSelection())->DisableHandles();
    reset_stack();
}

/*****************************************************************************/

PasteFunc::PasteFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PasteFunc::execute() {
    ComValue viewval(stack_arg(0));
    reset_stack();

    ComponentView* view = (ComponentView*)viewval.obj_val();
    OverlayComp* comp = (OverlayComp*)view->GetSubject();

    PasteCmd* cmd = new PasteCmd(_ed, new Clipboard(comp));
    execute_log(cmd);
    push_stack(viewval);
}

