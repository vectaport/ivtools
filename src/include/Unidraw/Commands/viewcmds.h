/*
 * Copyright (c) 1990, 1991 Stanford University
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
 * View commands.
 */

#ifndef unidraw_commands_viewcmds_h
#define unidraw_commands_viewcmds_h

#include <Unidraw/Commands/command.h>

class ControlInfo;
class GridDialog;

//: normal-size command
// <a href=../man3.1/viewcmds.html>man page</a>
class NormSizeCmd : public Command {
public:
    NormSizeCmd(ControlInfo*);
    NormSizeCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: reduce-to-fit command
// <a href=../man3.1/viewcmds.html>man page</a>
class RedToFitCmd : public Command {
public:
    RedToFitCmd(ControlInfo*);
    RedToFitCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: center command
// <a href=../man3.1/viewcmds.html>man page</a>
class CenterCmd : public Command {
public:
    CenterCmd(ControlInfo*);
    CenterCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: enable/disable grid command
// <a href=../man3.1/viewcmds.html>man page</a>
class GridCmd : public Command {
public:
    GridCmd(ControlInfo*);
    GridCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: grid-spacing command
// <a href=../man3.1/viewcmds.html>man page</a>
class GridSpacingCmd : public Command {
public:
    GridSpacingCmd(ControlInfo*);
    GridSpacingCmd(Editor* = nil);
    virtual ~GridSpacingCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    GridDialog* _dialog;
};

//: enable/disable gravity command
// <a href=../man3.1/viewcmds.html>man page</a>
class GravityCmd : public Command {
public:
    GravityCmd(ControlInfo*);
    GravityCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: landscape/portrait orientation command
// <a href=../man3.1/viewcmds.html>man page</a>
class OrientationCmd : public Command {
public:
    OrientationCmd(ControlInfo*);
    OrientationCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: editor close command
// <a href=../man3.1/viewcmds.html>man page</a>
class CloseEditorCmd : public Command {
public:
    CloseEditorCmd(ControlInfo*);
    CloseEditorCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
