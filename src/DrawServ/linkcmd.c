/*
 * Copyright (c) 2025 Scott E. Johnston
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
 */

#include <DrawServ/linkcmd.h>
#include <DrawServ/linkselection.h>
#include <DrawServ/drawclasses.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/linkselection.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <Unidraw/Graphic/pspaint.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/unidraw.h>

#include <sstream>
#include <uuid/uuid.h>

/*****************************************************************************/

LinkBrushCmd::LinkBrushCmd(ControlInfo* ci, PSBrush* br) : BrushCmd(ci, br) {}
LinkBrushCmd::LinkBrushCmd(Editor* ed, PSBrush* br) : BrushCmd(ed, br) {}

const char* LinkBrushCmd::dist_script() {
    _dist_script_buf = "";

    PSBrush* brush = GetBrush();
    if (!brush) return _dist_script_buf.c_str();

    Editor* ed = GetEditor();
    if (!ed) return _dist_script_buf.c_str();

    LinkSelection* sel = (LinkSelection*)ed->GetSelection();
    if (!sel) return _dist_script_buf.c_str();

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    std::ostringstream sbuf;
    boolean any = false;
    Iterator it;

    /* collect all LocallySelected comps */
    for (sel->First(it); !sel->Done(it); sel->Next(it)) {
        OverlayView* view = (OverlayView*)sel->GetView(it);
        OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
        void* ptr = nil;
        if (comp) drawserv->compidtable()->find(ptr, comp);
        GraphicId* grid = (GraphicId*)ptr;
        if (grid && grid->selected() == LinkSelection::LocallySelected) {
	     if (!any) {
                sbuf << "s=select();select(grid(";
                any = true;
            } else {
                sbuf << ",grid(";
            }
            sbuf << "\"" << grid->idstr() << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", drawserv->sessionidkey());
	sbuf << " :unlock \"" << keystr << "\")";
        if (brush->None())
            sbuf << ";brush(:none);select(s :lock \"" << keystr << "\")";
        else
            sbuf << ";brush(" << brush->GetLinePattern() << ","
                 << brush->Width() << ");select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkBrushCmd::Copy() {
    LinkBrushCmd* copy = new LinkBrushCmd(CopyControlInfo(), GetBrush());
    InitCopy(copy);
    return copy;
}

ClassId LinkBrushCmd::GetClassId() { return LINK_BRUSH_CMD; }
boolean LinkBrushCmd::IsA(ClassId id) { return id == LINK_BRUSH_CMD || BrushCmd::IsA(id); }
