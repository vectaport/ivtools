/*
 * Copyright (c) 1994 Vectaport Inc.
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

#include <Unidraw/Commands/command.h>
#include <Unidraw/Tools/tool.h>
#include <Unidraw/editor.h>
#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovunidraw.h>

CommandDoer::CommandDoer(Command* c) {
    cmd = c;
}

void CommandDoer::Do() {
    Command* command = cmd;
    if (command) {
	if (command->Reversible()) {
	    command = command->Copy();
	    command->Execute();
            if (command->Reversible()) {
                command->Log();
            } else {
                delete command;
            }
	}
	else {
	    command->Execute();
            if (command->Reversible()) {
                command = command->Copy();
                command->Log();
            }
	}
    }
}

CommandPusher::CommandPusher(Command* c) {
    cmd = c;
}

void CommandPusher::Push() {
    ((OverlayUnidraw*)unidraw)->Append(cmd);
}

ToolSelector::ToolSelector(Tool* tl, Editor* ed) {
    tool = tl;
    editor = ed;
}

void ToolSelector::Select() {
    editor->SetCurTool(tool);
}
