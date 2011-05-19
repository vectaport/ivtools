/*
 * Copyright (c) 2009 Scott E. Johnston
 * Copyright (c) 1997-2000 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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

#include <DrawServ/drawclasses.h>
#include <DrawServ/drawcmds.h>
#include <DrawServ/draweditor.h>

#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/framecomps.h>

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/nodecomp.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>

/*****************************************************************************/ 
CopyMoveGraphFrameCmd::CopyMoveGraphFrameCmd(ControlInfo* i, boolean after)
: CopyMoveFrameCmd(i, after)
{
}

CopyMoveGraphFrameCmd::CopyMoveGraphFrameCmd(Editor* e, boolean after)
: CopyMoveFrameCmd(e, after)
{
}

ClassId CopyMoveGraphFrameCmd::GetClassId() { return COPYMOVEGRAPHFRAME_CMD; }
boolean CopyMoveGraphFrameCmd::IsA(ClassId id) {
    return id == COPYMOVEGRAPHFRAME_CMD || CopyMoveFrameCmd::IsA(id);
}

Command* CopyMoveGraphFrameCmd::Copy() {
    Command* copy = new CopyMoveGraphFrameCmd(CopyControlInfo(), _after);
    InitCopy(copy);
    return copy;
}

void CopyMoveGraphFrameCmd::Execute() {
  FrameEditor* ed = (FrameEditor*)GetEditor();
  Append(new OvSlctAllCmd(ed));
  Append(new GraphCopyCmd(ed));
  Append(new CreateFrameCmd(ed, _after));
  Append(new MoveFrameCmd(ed, _after ? +1 : -1));
  Append(new GraphPasteCmd(ed));
  MacroCmd::Execute();
}

