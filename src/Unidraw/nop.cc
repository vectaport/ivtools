/*
 * Copyright (c) 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Implementation of null command.
 */

#include <Unidraw/classes.h>
#include <Unidraw/Commands/nop.h>

/*****************************************************************************/

ClassId NOPCmd::GetClassId () { return NOP_CMD; }
boolean NOPCmd::IsA (ClassId id) { return NOP_CMD == id || Command::IsA(id); }

NOPCmd::NOPCmd (ControlInfo* c) : Command(c) { }
NOPCmd::NOPCmd (Editor* ed) : Command(ed) { }

Command* NOPCmd::Copy () {
    Command* copy = new NOPCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void NOPCmd::Execute () { }
boolean NOPCmd::Reversible () { return false; }
