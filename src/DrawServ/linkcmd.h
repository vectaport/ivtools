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

/*
 * linkcmd.h - DrawServ mixin and Link* command variants for distributed
 * graphic state changes.
 */

#ifndef linkcmd_h
#define linkcmd_h

#include <Unidraw/Commands/brushcmd.h>
#include <Unidraw/Commands/colorcmd.h>
#include <Unidraw/Commands/font.h>
#include <Unidraw/Commands/patcmd.h>
#include <OverlayUnidraw/ovcmds.h>
#include <string>
#include <uuid/uuid.h>
#if !defined(__APPLE__) && !defined(IV_UUID_STRING_T_DEFINED)
#define IV_UUID_STRING_T_DEFINED
typedef char uuid_string_t[37];  /* Apple-only type; Linux libuuid lacks it */
#endif

//: mixin for Commands that generate distributed scripts in DrawServ
// Mix into any Command subclass to provide dist_script() for use
// by DrawServ::ExecuteCmd when distributing commands to remote drawservs.
class DrawServCmd {
public:
    DrawServCmd() { uuid_clear(_dist_owner_sid); }
    virtual const char* dist_script() = 0;
    // return ComTerp script to distribute, or empty string if none.

    const uuid_t& dist_owner_sid() { return _dist_owner_sid; }
    // session id of the owner the most recent dist_script() was generated for
    // (cleared if none).  DrawServ::ExecuteCmd excludes the link toward this
    // session so a relayed change flows onward along a chain without looping
    // back to its origin.
protected:
    uuid_t _dist_owner_sid;
};

//: BrushCmd with distributed script generation for DrawServ
// Mixes BrushCmd with DrawServCmd to provide dist_script() that
// serializes the brush change for distribution to remote drawservs.
class LinkBrushCmd : public BrushCmd, public DrawServCmd {
public:
    LinkBrushCmd(ControlInfo*, PSBrush* = nil);
    LinkBrushCmd(Editor* = nil, PSBrush* = nil);

    virtual const char* dist_script();
    // return "s=select();select(grid(uuid),...);brush(linepat,width);select(s)"
    // for all LocallySelected graphics, or empty string if none.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    std::string _dist_script_buf;
};

//: SetTransformCmd with distributed script generation for DrawServ
// The odd one of the relays, in a way that makes it simpler rather than
// harder.  Brush, colour, pattern and font all apply to whatever is selected,
// so their scripts have to carry a select(grid(...) :unlock key) bracket to
// tell the far node what to apply them to.  trans() names its target, and the
// command carries it in its clipboard, so the script is just the call.
//
// What goes on the wire is the ABSOLUTE resulting matrix, not the delta this
// command holds.  A delta accumulates -- deliver it twice and the graphic has
// moved twice -- while an absolute transform is idempotent, so a repeat or an
// out-of-order arrival is harmless.  That is the same reasoning that put
// absolute perspectives on the wire for subscribed viewers, and it is why
// trans() was worth relaying before move/scale/rotate.
class LinkTransformCmd : public SetTransformCmd, public DrawServCmd {
public:
    LinkTransformCmd(Editor* = nil, Transformer* = nil);

    virtual const char* dist_script();
    // return "trans(grid(uuid) a00,a01,a10,a11,a20,a21)" for the graphic this
    // command targets, or empty string if it has no distributed identity yet.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    std::string _dist_script_buf;
};

//: FontCmd with distributed script generation for DrawServ
// Mixes FontCmd with DrawServCmd to provide dist_script() that serializes
// the font change for distribution to remote drawservs.
//
// fontnum is the menu index from font(), fontname the name given to
// fontbyname() -- whichever one made this command is what dist_script()
// replays, same as LinkPatternCmd and LinkColorCmd.
//
// Sending the font's own X name instead, for both paths, looks tempting: it is
// self-describing where a menu index depends on both nodes enumerating the
// same font resources.  It is also lossy.  A wildcarded X name carries no
// FONT_NAME or POINT_SIZE property, so FindFont defaults the print font and
// size, and the far node ends up with the right screen font but a blank
// PostScript name and a different line height -- text that lays out
// differently.  Replaying the original call keeps all three.
class LinkFontCmd : public FontCmd, public DrawServCmd {
public:
    LinkFontCmd(ControlInfo*, PSFont* = nil, int fontnum = 0, const char* fontname = nil);
    LinkFontCmd(Editor* = nil, PSFont* = nil, int fontnum = 0, const char* fontname = nil);

    virtual const char* dist_script();
    // return "s=select();select(grid(uuid),... :unlock key);font(n);select(s :lock key)"
    // for all LocallySelected graphics, or empty string if none.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    std::string _dist_script_buf;
    int _fontnum;
    std::string _fontname;
};

//: PatternCmd with distributed script generation for DrawServ
// Mixes PatternCmd with DrawServCmd to provide dist_script() that
// serializes the pattern change for distribution to remote drawservs.
// patnum is the menu index from pattern(), maskargs the literal argument
// text from patternmask() -- whichever one made this command is what
// dist_script() replays, so the far node runs the same call, same idea as
// LinkColorCmd carrying fgnum/bgnum.
class LinkPatternCmd : public PatternCmd, public DrawServCmd {
public:
    LinkPatternCmd(ControlInfo*, PSPattern* = nil, int patnum = 0, const char* maskargs = nil);
    LinkPatternCmd(Editor* = nil, PSPattern* = nil, int patnum = 0, const char* maskargs = nil);

    virtual const char* dist_script();
    // return "s=select();select(grid(uuid),... :unlock key);pattern(patnum);select(s :lock key)"
    // for all LocallySelected graphics, or empty string if none.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    std::string _dist_script_buf;
    int _patnum;
    std::string _maskargs;
};

//: ColorCmd with distributed script generation for DrawServ
// Mixes ColorCmd with DrawServCmd to provide dist_script() that
// serializes the color change for distribution to remote drawservs.
// fgnum/bgnum are the menu indices from colors() -- carried through
// from ColorFunc::execute() so dist_script() can emit colors(fgn bgn)
// matching the local command exactly, same pattern as LinkBrushCmd.
class LinkColorCmd : public ColorCmd, public DrawServCmd {
public:
    LinkColorCmd(ControlInfo*, PSColor* fg, PSColor* bg, int fgnum, int bgnum);
    LinkColorCmd(Editor* = nil, PSColor* fg = nil, PSColor* bg = nil, int fgnum = 0, int bgnum = 0);

    virtual const char* dist_script();
    // return "s=select();select(grid(uuid),...:unlock key);colors(fgnum bgnum);select(s :lock key)"
    // for all LocallySelected graphics, or empty string if none.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    std::string _dist_script_buf;
    int _fgnum, _bgnum;
};

#endif
