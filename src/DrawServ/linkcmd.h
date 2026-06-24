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
#include <string>
#include <uuid/uuid.h>

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
