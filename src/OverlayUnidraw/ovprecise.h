/*
 * Copyright (c) 1998,1999 Vectaport Inc.
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
 * Precise move commands for OverlayUnidraw
 */

#ifndef ovprecise_h
#define ovprecise_h

#include <UniIdraw/idcmds.h>
#include <OverlayUnidraw/ovcmds.h>

class Glyph;
class ObservableEnum;

//: glyphified PreciseMoveCmd.
class OvPreciseMoveCmd : public PreciseMoveCmd {
public:
    OvPreciseMoveCmd(ControlInfo*);
    OvPreciseMoveCmd(Editor* = nil);
    virtual ~OvPreciseMoveCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    Glyph* unit_buttons();

protected:
    ObservableEnum* _unit_enum;
    static char* _default_movestr;
    static int _default_enumval;
};

//: glyphified PreciseScaleCmd.
class OvPreciseScaleCmd : public PreciseScaleCmd {
public:
    OvPreciseScaleCmd(ControlInfo*);
    OvPreciseScaleCmd(Editor* = nil);
    virtual ~OvPreciseScaleCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: glyphified PreciseRotateCmd.
class OvPreciseRotateCmd : public PreciseRotateCmd {
public:
    OvPreciseRotateCmd(ControlInfo*);
    OvPreciseRotateCmd(Editor* = nil);
    virtual ~OvPreciseRotateCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: glyphified PrecisePageCmd.
class OvPrecisePageCmd : public PrecisePageCmd {
public:
    OvPrecisePageCmd(ControlInfo*);
    OvPrecisePageCmd(Editor* = nil);
    virtual ~OvPrecisePageCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: precise brush-width (line-width) command.
class OvPreciseBrushCmd : public Command {
public:
    OvPreciseBrushCmd(ControlInfo*);
    OvPreciseBrushCmd(Editor* = nil);
    virtual ~OvPreciseBrushCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
