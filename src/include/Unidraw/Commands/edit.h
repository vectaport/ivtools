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
 * Editing commands.
 */

#ifndef unidraw_commands_edit_h
#define unidraw_commands_edit_h

#include <Unidraw/Commands/macro.h>

class Connector;
class GraphicComp;

//: undo command
// <a href=../man3.1/edit.html>man page</a>
class UndoCmd : public Command {
public:
    UndoCmd(ControlInfo*);
    UndoCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: redo command
// <a href=../man3.1/edit.html>man page</a>
class RedoCmd : public Command {
public:
    RedoCmd(ControlInfo*);
    RedoCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: cut command
// <a href=../man3.1/edit.html>man page</a>
class CutCmd : public Command {
public:
    CutCmd(ControlInfo*, Clipboard* = nil);
    CutCmd(Editor* = nil, Clipboard* = nil);
    virtual ~CutCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    boolean _executed;
};

//: copy command
// <a href=../man3.1/edit.html>man page</a>
class CopyCmd : public Command {
public:
    CopyCmd(ControlInfo*, Clipboard* = nil);
    CopyCmd(Editor* = nil, Clipboard* = nil);
    virtual ~CopyCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: paste command
// <a href=../man3.1/edit.html>man page</a>
class PasteCmd : public Command {
public:
    PasteCmd(ControlInfo*, Clipboard* = nil);
    PasteCmd(Editor* = nil, Clipboard* = nil);
    virtual ~PasteCmd();

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    boolean _executed;
};

//: replace command
// <a href=../man3.1/edit.html>man page</a>
class ReplaceCmd : public MacroCmd {
public:
    ReplaceCmd(ControlInfo*, GraphicComp* = nil);
    ReplaceCmd(Editor* = nil, GraphicComp* = nil);

    GraphicComp* GetReplacement();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    void Init(GraphicComp*);
};

//: duplicate command
// <a href=../man3.1/edit.html>man page</a>
class DupCmd : public Command {
public:
    DupCmd(ControlInfo*, Clipboard* = nil);
    DupCmd(Editor* = nil, Clipboard* = nil);
    virtual ~DupCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    boolean _executed;
};

//: delete command
// <a href=../man3.1/edit.html>man page</a>
class DeleteCmd : public Command {
public:
    DeleteCmd(ControlInfo*, Clipboard* = nil);
    DeleteCmd(Editor* = nil, Clipboard* = nil);
    virtual ~DeleteCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    boolean _executed;
};

//: select-all command
// <a href=../man3.1/edit.html>man page</a>
class SlctAllCmd : public Command {
public:
    SlctAllCmd(ControlInfo*);
    SlctAllCmd(Editor* = nil);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: connect command
// <a href=../man3.1/edit.html>man page</a>
class ConnectCmd : public Command {
public:
    ConnectCmd(ControlInfo*, Connector* = nil, Connector* = nil);
    ConnectCmd(Editor* = nil, Connector* = nil, Connector* = nil);

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    void GetConnectors(Connector*&, Connector*&);

    virtual Command* Copy();
    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    Connector* _source, *_target;
};

//: mobility command
// <a href=../man3.1/edit.html>man page</a>
class MobilityCmd : public Command {
public:
    MobilityCmd(ControlInfo*, Mobility = Fixed);
    MobilityCmd(Editor* = nil, Mobility = Fixed);

    Mobility GetMobility();

    virtual Command* Copy();
    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    Mobility _mobility;
};

#endif
