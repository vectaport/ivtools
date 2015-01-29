/*
 * Copyright (c) 1994,1999 Vectaport Inc.
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

/*
 * Doer for glyph-menu launched commands
 */

#ifndef ovdoer_h
#define ovdoer_h

#include <Unidraw/enter-scope.h>

class Command;
class Editor;
class Tool;

//: helper object for executing command from glyph-based pull-down menus.
class CommandDoer {
public:
    CommandDoer(Command*);

    void Do();
protected:
    Command* cmd;
};

//: helper object for postponing execution of commands until environment is ready.
class CommandPusher {
public:
    CommandPusher(Command*);

    void Push();
protected:
    Command* cmd;
};

//: helper object for selecting tool button when pressed.
class ToolSelector {
public:
    ToolSelector(Tool*, Editor*);

    void Select();
protected:
    Tool* tool;
    Editor* editor;
};

#endif
