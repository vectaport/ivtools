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
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <Unidraw/Graphic/pspaint.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/unidraw.h>

#include <sstream>
#include <uuid/uuid.h>
#if !defined(__APPLE__) && !defined(IV_UUID_STRING_T_DEFINED)
#define IV_UUID_STRING_T_DEFINED
typedef char uuid_string_t[37];  /* Apple-only type; Linux libuuid lacks it */
#endif

/*****************************************************************************/

LinkBrushCmd::LinkBrushCmd(ControlInfo* ci, PSBrush* br) : BrushCmd(ci, br) {}
LinkBrushCmd::LinkBrushCmd(Editor* ed, PSBrush* br) : BrushCmd(ed, br) {}

const char* LinkBrushCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

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
    uint32_t owner_key = 0;
    Iterator it;

    /* Collect the comps whose brush change to propagate.  Originating: comps we
       own (LocallySelected).  Relaying along a chain: comps a remote owner has
       temporarily unlocked through us (grid->unlocked()) via the incoming
       select(:unlock) -- forward those onward so a chain ds1->ds2->ds3 carries
       the change past the first hop.  Stamp the OWNER's key, not ours, so the
       forwarded :unlock/:lock bracket stays valid at the next hop; today that
       key is derivable (selectorkey), but once it becomes a per-owner signature
       (issue #163, the atomic select(:body :sign) follow-on) only the owner
       could mint it, so the relay must forward, never re-derive.  Record the
       owner sid so
       ExecuteCmd can exclude the link back toward the origin. */
    for (sel->First(it); !sel->Done(it); sel->Next(it)) {
        OverlayView* view = (OverlayView*)sel->GetView(it);
        OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
        void* ptr = nil;
        if (comp) drawserv->compidtable()->find(ptr, comp);
        GraphicId* grid = (GraphicId*)ptr;
        if (grid && (grid->selected() == LinkSelection::LocallySelected ||
                     grid->unlocked())) {
            if (!any) {
                sbuf << "s=select();select(grid(";
                any = true;
                /* INTERIM LIMITATION: the owner key+sid are captured from the
                   FIRST matching comp only, so the whole dance relays under one
                   owner's :unlock/:lock key and ExecuteCmd excludes one back-
                   link.  Correct today, when a selection's comps share an owner;
                   a mixed-ownership selection would mis-stamp the rest.  Revisit
                   when per-owner signatures land (issue #163) -- the dance would
                   then have to group comps by owner and emit one bracket each. */
                if (grid->selected() == LinkSelection::LocallySelected) {
                    owner_key = drawserv->sessionidkey();
                    uuid_copy(_dist_owner_sid, drawserv->sessionid());
                } else {
                    owner_key = grid->selectorkey();
                    uuid_copy(_dist_owner_sid, grid->selector());
                }
            } else {
                sbuf << ",grid(";
            }
            sbuf << "\"" << grid->idstr() << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
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

/*****************************************************************************/

/* format a PSColor as "#RRGGBB".  Prefer the color's own name when it is
   already a clean "#" + 6 hex-digit string (the colors("#RRGGBB") path) so
   it round-trips exactly; otherwise derive from intensities (0..1 floats),
   which is the reliable ground truth for the menu path where the name is a
   palette label like "Black".  Intensity derivation can lose a least-
   significant bit through the 8->16->8 bit X11 color path, so the name is
   preferred when it is itself an exact hex spec. */
static boolean is_hex6(const char* s) {
    if (!s || s[0] != '#') return false;
    int i;
    for (i = 1; i <= 6; i++) {
        char c = s[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F')))
            return false;
    }
    return s[7] == '\0';
}

static void color_to_hex(PSColor* c, char* out /* >= 8 bytes */) {
    if (!c || c->None()) { strcpy(out, "None"); return; }
    const char* name = c->GetName();
    if (is_hex6(name)) { strcpy(out, name); return; }
    ColorIntensity r, g, b;
    c->GetIntensities(r, g, b);
    int ri = (int)(r * 255.0 + 0.5);
    int gi = (int)(g * 255.0 + 0.5);
    int bi = (int)(b * 255.0 + 0.5);
    if (ri < 0) ri = 0; if (ri > 255) ri = 255;
    if (gi < 0) gi = 0; if (gi > 255) gi = 255;
    if (bi < 0) bi = 0; if (bi > 255) bi = 255;
    snprintf(out, 8, "#%02x%02x%02x", ri, gi, bi);
}

LinkColorCmd::LinkColorCmd(ControlInfo* ci, PSColor* fg, PSColor* bg, int fgnum, int bgnum)
    : ColorCmd(ci, fg, bg), _fgnum(fgnum), _bgnum(bgnum) {}
LinkColorCmd::LinkColorCmd(Editor* ed, PSColor* fg, PSColor* bg, int fgnum, int bgnum)
    : ColorCmd(ed, fg, bg), _fgnum(fgnum), _bgnum(bgnum) {}

const char* LinkColorCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    Editor* ed = GetEditor();
    if (!ed) return _dist_script_buf.c_str();

    LinkSelection* sel = (LinkSelection*)ed->GetSelection();
    if (!sel) return _dist_script_buf.c_str();

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    std::ostringstream sbuf;
    boolean any = false;
    uint32_t owner_key = 0;
    Iterator it;

    /* same originate-or-relay collection as LinkBrushCmd::dist_script(): own
       (LocallySelected) comps when originating, plus remote-owner comps we hold
       unlocked when relaying onward, stamped with the owner's key. */
    for (sel->First(it); !sel->Done(it); sel->Next(it)) {
        OverlayView* view = (OverlayView*)sel->GetView(it);
        OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
        void* ptr = nil;
        if (comp) drawserv->compidtable()->find(ptr, comp);
        GraphicId* grid = (GraphicId*)ptr;
        if (grid && (grid->selected() == LinkSelection::LocallySelected ||
                     grid->unlocked())) {
            if (!any) {
                sbuf << "s=select();select(grid(";
                any = true;
                /* INTERIM LIMITATION: the owner key+sid are captured from the
                   FIRST matching comp only, so the whole dance relays under one
                   owner's :unlock/:lock key and ExecuteCmd excludes one back-
                   link.  Correct today, when a selection's comps share an owner;
                   a mixed-ownership selection would mis-stamp the rest.  Revisit
                   when per-owner signatures land (issue #163) -- the dance would
                   then have to group comps by owner and emit one bracket each. */
                if (grid->selected() == LinkSelection::LocallySelected) {
                    owner_key = drawserv->sessionidkey();
                    uuid_copy(_dist_owner_sid, drawserv->sessionid());
                } else {
                    owner_key = grid->selectorkey();
                    uuid_copy(_dist_owner_sid, grid->selector());
                }
            } else {
                sbuf << ",grid(";
            }
            sbuf << "\"" << grid->idstr() << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
        sbuf << " :unlock \"" << keystr << "\")";
        /* serialize by RGB intensities so both the menu path and the
           colors("#RRGGBB") path distribute identically, exactly, and
           palette-independently */
        char fghex[8], bghex[8];
        color_to_hex(GetFgColor(), fghex);
        color_to_hex(GetBgColor(), bghex);
        sbuf << ";colors(\"" << fghex << "\" \"" << bghex << "\")";
        sbuf << ";select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkColorCmd::Copy() {
    LinkColorCmd* copy = new LinkColorCmd(CopyControlInfo(),
                                          GetFgColor(), GetBgColor(),
                                          _fgnum, _bgnum);
    InitCopy(copy);
    return copy;
}

ClassId LinkColorCmd::GetClassId() { return LINK_COLOR_CMD; }
boolean LinkColorCmd::IsA(ClassId id) {
    return id == LINK_COLOR_CMD || ColorCmd::IsA(id);
}
