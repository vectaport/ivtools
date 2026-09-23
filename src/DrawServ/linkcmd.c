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
#include <Unidraw/clipboard.h>
#include <InterViews/transformer.h>

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

    /* collect comps we own or a remote owner has unlocked through us;
       stamp the owner's key and sid, not ours, so ExecuteCmd excludes the origin. */
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
                /* interim limitation: owner key and sid come from the first
                   matching comp, correct only while comps share one owner. */
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
            char gidstr[9];
            snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());
            sbuf << "\"" << gidstr << "\")";
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

LinkTransformCmd::LinkTransformCmd(Editor* ed, Transformer* t) : SetTransformCmd(ed, t) {}

const char* LinkTransformCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    Transformer* delta = GetTransformer();
    if (!delta) return _dist_script_buf.c_str();

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    /* trans() names its target, so the comp comes from the clipboard, not
       the selection, but ownership still comes from the grid either way. */
    Clipboard* cb = GetClipboard();
    if (!cb) return _dist_script_buf.c_str();
    Iterator i;
    cb->First(i);
    if (cb->Done(i)) return _dist_script_buf.c_str();
    OverlayComp* comp = (OverlayComp*)cb->GetComp(i);
    if (!comp) return _dist_script_buf.c_str();

    void* ptr = nil;
    drawserv->compidtable()->find(ptr, comp);
    GraphicId* grid = (GraphicId*)ptr;
    if (!grid) return _dist_script_buf.c_str();   /* not distributed yet */

    /* relay only what this node owns or a remote owner has unlocked through
       it, the gate LinkBrushCmd applies, else two nodes bounce it forever. */
    boolean locally_owned = (grid->selected() == LinkSelection::LocallySelected);
    if (!locally_owned && !grid->unlocked())
        return _dist_script_buf.c_str();

    uint32_t owner_key = 0;
    if (locally_owned) {
        owner_key = drawserv->sessionidkey();
        uuid_copy(_dist_owner_sid, drawserv->sessionid());
    } else {
        /* forward the owner's key rather than re-derive it, so the bracket
           stays valid at the next hop; see LinkBrushCmd for why. */
        owner_key = grid->selectorkey();
        uuid_copy(_dist_owner_sid, grid->selector());
    }

    /* dist_script runs before Execute, so the graphic still holds the
       pre-delta transform; send where it will land, not the nudge. */
    Graphic* gr = comp->GetGraphic();
    if (!gr) return _dist_script_buf.c_str();
    Transformer result;
    Transformer* cur = gr->GetTransformer();
    if (cur) result = *cur;
    result.postmultiply(*delta);

    float a00, a01, a10, a11, a20, a21;
    result.matrix(a00, a01, a10, a11, a20, a21);

    char keystr[9];
    snprintf(keystr, sizeof(keystr), "%08X", owner_key);
    char gidstr[9];
    snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());

    /* the select(:unlock)/select(:lock) bracket carries the owner, not the
       address (trans() already names its graphic), for the far node's dist_script. */
    std::ostringstream sbuf;
    sbuf << "s=select();select(grid(\"" << gidstr << "\")"
	 << " :unlock \"" << keystr << "\")"
	 << ";trans(grid(\"" << gidstr << "\") "
	 << a00 << "," << a01 << "," << a10 << ","
	 << a11 << "," << a20 << "," << a21 << ")"
	 << ";select(s :lock \"" << keystr << "\")";
    _dist_script_buf = sbuf.str();

    return _dist_script_buf.c_str();
}

Command* LinkTransformCmd::Copy() {
    LinkTransformCmd* copy = new LinkTransformCmd(GetEditor(), GetTransformer());
    InitCopy(copy);
    return copy;
}

ClassId LinkTransformCmd::GetClassId() { return LINK_TRANSFORM_CMD; }
boolean LinkTransformCmd::IsA(ClassId id) {
    return id == LINK_TRANSFORM_CMD || SetTransformCmd::IsA(id);
}

/*****************************************************************************/

LinkFontCmd::LinkFontCmd(ControlInfo* ci, PSFont* font, int fontnum, const char* fontname)
  : FontCmd(ci, font), _fontnum(fontnum), _fontname(fontname ? fontname : "") {}
LinkFontCmd::LinkFontCmd(Editor* ed, PSFont* font, int fontnum, const char* fontname)
  : FontCmd(ed, font), _fontnum(fontnum), _fontname(fontname ? fontname : "") {}

const char* LinkFontCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    /* only the call that built this command knows which form to replay */
    if (_fontnum <= 0 && _fontname.empty()) return _dist_script_buf.c_str();

    if (!GetFont()) return _dist_script_buf.c_str();

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

    /* same collection and relay rules as LinkBrushCmd; see its comment. */
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
            char gidstr[9];
            snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());
            sbuf << "\"" << gidstr << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
	sbuf << " :unlock \"" << keystr << "\")";
        if (_fontnum > 0)
            sbuf << ";font(" << _fontnum << ")";
        else
            sbuf << ";fontbyname(\"" << _fontname << "\")";
        sbuf << ";select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkFontCmd::Copy() {
    LinkFontCmd* copy = new LinkFontCmd(CopyControlInfo(), GetFont(), _fontnum,
                                        _fontname.empty() ? nil : _fontname.c_str());
    InitCopy(copy);
    return copy;
}

ClassId LinkFontCmd::GetClassId() { return LINK_FONT_CMD; }
boolean LinkFontCmd::IsA(ClassId id) { return id == LINK_FONT_CMD || FontCmd::IsA(id); }

/*****************************************************************************/

LinkPatternCmd::LinkPatternCmd(ControlInfo* ci, PSPattern* pat, int patnum, const char* maskargs)
  : PatternCmd(ci, pat), _patnum(patnum), _maskargs(maskargs ? maskargs : "") {}
LinkPatternCmd::LinkPatternCmd(Editor* ed, PSPattern* pat, int patnum, const char* maskargs)
  : PatternCmd(ed, pat), _patnum(patnum), _maskargs(maskargs ? maskargs : "") {}

const char* LinkPatternCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    /* the far node needs a menu index or the patternmask bits to resolve
       this the same way; with neither, say nothing rather than guess. */
    if (_patnum <= 0 && _maskargs.empty()) return _dist_script_buf.c_str();

    if (!GetPattern()) return _dist_script_buf.c_str();

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

    /* same collection and relay rules as LinkBrushCmd; see its comment. */
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
            char gidstr[9];
            snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());
            sbuf << "\"" << gidstr << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
	sbuf << " :unlock \"" << keystr << "\")";
        if (_patnum > 0)
            sbuf << ";pattern(" << _patnum << ")";
        else
            sbuf << ";patternmask(" << _maskargs << ")";
        sbuf << ";select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkPatternCmd::Copy() {
    LinkPatternCmd* copy = new LinkPatternCmd(CopyControlInfo(), GetPattern(), _patnum,
                                              _maskargs.empty() ? nil : _maskargs.c_str());
    InitCopy(copy);
    return copy;
}

ClassId LinkPatternCmd::GetClassId() { return LINK_PATTERN_CMD; }
boolean LinkPatternCmd::IsA(ClassId id) { return id == LINK_PATTERN_CMD || PatternCmd::IsA(id); }

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

    /* same originate-or-relay collection as LinkBrushCmd::dist_script(),
       stamped with the owner's key. */
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
                /* interim limitation: owner key and sid come from the first
                   matching comp, correct only while comps share one owner. */
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
            char gidstr[9];
            snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());
            sbuf << "\"" << gidstr << "\")";
        }
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
        sbuf << " :unlock \"" << keystr << "\")";
        /* serialize by RGB intensities so the menu path and colors()
           path distribute identically and palette-independently */
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

/*****************************************************************************/

/* shared by LinkFrontCmd::dist_script() and LinkBackCmd::dist_script():
   collect grid ids from cb this node may relay (owns, or a remote owner
   has unlocked through it), stamping the owner's key/sid so ExecuteCmd
   excludes the link back toward the change's origin.  Unlike the
   graphic-state commands above, front()/back() read this command's own
   clipboard rather than the live selection -- populated by Execute()
   itself (FrontCmd::Execute()/BackCmd::Execute()), from a single
   explicit target set beforehand (FrontSelectionFunc/BackSelectionFunc)
   or, when nothing was set, from the live selection -- so this walks cb
   instead of ed->GetSelection(), and DrawServ::ExecuteCmd calls
   Execute() before dist_script() for these two commands. */
static boolean collect_relayable_grids(Clipboard* cb, DrawServ* drawserv,
                                        std::ostringstream& sbuf,
                                        uint32_t& owner_key,
                                        uuid_t owner_sid) {
    boolean any = false;
    Iterator it;
    for (cb->First(it); !cb->Done(it); cb->Next(it)) {
        OverlayComp* comp = (OverlayComp*)cb->GetComp(it);
        void* ptr = nil;
        if (comp) drawserv->compidtable()->find(ptr, comp);
        GraphicId* grid = (GraphicId*)ptr;
        if (!grid) continue;   /* not distributed yet */
        if (grid->selected() == LinkSelection::LocallySelected || grid->unlocked()) {
            if (!any) {
                sbuf << "s=select();select(grid(";
                any = true;
                if (grid->selected() == LinkSelection::LocallySelected) {
                    owner_key = drawserv->sessionidkey();
                    uuid_copy(owner_sid, drawserv->sessionid());
                } else {
                    owner_key = grid->selectorkey();
                    uuid_copy(owner_sid, grid->selector());
                }
            } else {
                sbuf << ",grid(";
            }
            sbuf << "\"" << grid->idstr() << "\")";
        }
    }
    return any;
}

LinkFrontCmd::LinkFrontCmd(ControlInfo* ci) : FrontCmd(ci) {}
LinkFrontCmd::LinkFrontCmd(Editor* ed) : FrontCmd(ed) {}

const char* LinkFrontCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    Clipboard* cb = GetClipboard();
    if (!cb) return _dist_script_buf.c_str();

    std::ostringstream sbuf;
    uint32_t owner_key = 0;
    boolean any = collect_relayable_grids(cb, drawserv, sbuf, owner_key, _dist_owner_sid);

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
	sbuf << " :unlock \"" << keystr << "\")";
        sbuf << ";front();select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkFrontCmd::Copy() {
    LinkFrontCmd* copy = new LinkFrontCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

ClassId LinkFrontCmd::GetClassId() { return LINK_FRONT_CMD; }
boolean LinkFrontCmd::IsA(ClassId id) { return id == LINK_FRONT_CMD || FrontCmd::IsA(id); }

/*****************************************************************************/

LinkBackCmd::LinkBackCmd(ControlInfo* ci) : BackCmd(ci) {}
LinkBackCmd::LinkBackCmd(Editor* ed) : BackCmd(ed) {}

const char* LinkBackCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    Clipboard* cb = GetClipboard();
    if (!cb) return _dist_script_buf.c_str();

    std::ostringstream sbuf;
    uint32_t owner_key = 0;
    boolean any = collect_relayable_grids(cb, drawserv, sbuf, owner_key, _dist_owner_sid);

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
	sbuf << " :unlock \"" << keystr << "\")";
        sbuf << ";back();select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkBackCmd::Copy() {
    LinkBackCmd* copy = new LinkBackCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

ClassId LinkBackCmd::GetClassId() { return LINK_BACK_CMD; }
boolean LinkBackCmd::IsA(ClassId id) { return id == LINK_BACK_CMD || BackCmd::IsA(id); }

/*****************************************************************************/

LinkMoveCmd::LinkMoveCmd(ControlInfo* ci, float dx, float dy) : MoveCmd(ci, dx, dy) {}
LinkMoveCmd::LinkMoveCmd(Editor* ed, float dx, float dy) : MoveCmd(ed, dx, dy) {}

const char* LinkMoveCmd::dist_script() {
    _dist_script_buf = "";
    uuid_clear(_dist_owner_sid);

    float dx, dy;
    GetMovement(dx, dy);
    if (dx == 0 && dy == 0) return _dist_script_buf.c_str();

    Editor* ed = GetEditor();
    if (!ed) return _dist_script_buf.c_str();

    LinkSelection* sel = (LinkSelection*)ed->GetSelection();
    if (!sel) return _dist_script_buf.c_str();

    DrawServ* drawserv = (DrawServ*)unidraw;
    if (!drawserv->linklist() || drawserv->linklist()->Number() == 0)
        return _dist_script_buf.c_str();

    Transformer delta;
    delta.translate(dx, dy);

    std::ostringstream sbuf, transbuf;
    boolean any = false;
    uint32_t owner_key = 0;
    Iterator it;

    /* one trans() per relayable comp instead of a replayed move(dx,dy) --
       each comp's own resulting position is idempotent, the shared delta
       is not (see class comment). */
    for (sel->First(it); !sel->Done(it); sel->Next(it)) {
        OverlayView* view = (OverlayView*)sel->GetView(it);
        OverlayComp* comp = view ? (OverlayComp*)view->GetSubject() : nil;
        void* ptr = nil;
        if (comp) drawserv->compidtable()->find(ptr, comp);
        GraphicId* grid = (GraphicId*)ptr;
        if (!grid || !(grid->selected() == LinkSelection::LocallySelected ||
                       grid->unlocked()))
            continue;

        Graphic* gr = comp->GetGraphic();
        if (!gr) continue;
        Transformer result;
        Transformer* cur = gr->GetTransformer();
        if (cur) result = *cur;
        result.postmultiply(delta);

        float a00, a01, a10, a11, a20, a21;
        result.matrix(a00, a01, a10, a11, a20, a21);

        char gidstr[9];
        snprintf(gidstr, sizeof(gidstr), "%08X", grid->idkey());

        if (!any) {
            sbuf << "s=select();select(grid(\"" << gidstr << "\")";
            any = true;
            if (grid->selected() == LinkSelection::LocallySelected) {
                owner_key = drawserv->sessionidkey();
                uuid_copy(_dist_owner_sid, drawserv->sessionid());
            } else {
                owner_key = grid->selectorkey();
                uuid_copy(_dist_owner_sid, grid->selector());
            }
        } else {
            sbuf << ",grid(\"" << gidstr << "\")";
        }

        transbuf << ";trans(grid(\"" << gidstr << "\") "
                  << a00 << "," << a01 << "," << a10 << ","
                  << a11 << "," << a20 << "," << a21 << ")";
    }

    if (any) {
        char keystr[9];
        snprintf(keystr, sizeof(keystr), "%08X", owner_key);
        sbuf << " :unlock \"" << keystr << "\")";
        sbuf << transbuf.str();
        sbuf << ";select(s :lock \"" << keystr << "\")";
        _dist_script_buf = sbuf.str();
    }

    return _dist_script_buf.c_str();
}

Command* LinkMoveCmd::Copy() {
    float dx, dy;
    GetMovement(dx, dy);
    LinkMoveCmd* copy = new LinkMoveCmd(CopyControlInfo(), dx, dy);
    InitCopy(copy);
    return copy;
}

ClassId LinkMoveCmd::GetClassId() { return LINK_MOVE_CMD; }
boolean LinkMoveCmd::IsA(ClassId id) { return id == LINK_MOVE_CMD || MoveCmd::IsA(id); }
