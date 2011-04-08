/*
 * Copyright (c) 1994-1995 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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

/*
 * ChainViewersCmd and UnchainViewerCmd - commands for chaining and unchaining viewers
 */

#ifndef chainviewer_h
#define chainviewer_h

#include <Unidraw/Commands/command.h>

class Viewer;

class ChainViewersCmd : public Command {
public:
    ChainViewersCmd(Viewer*, ControlInfo*, boolean pan = true, boolean zoom = true);
    ChainViewersCmd(Viewer*, Editor* = nil, boolean pan = true, boolean zoom = true);
    virtual ~ChainViewersCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    Viewer* _viewer;
    boolean _pan;
    boolean _zoom;
};

class UnchainViewersCmd : public Command {
public:
    UnchainViewersCmd(Viewer*, ControlInfo*, boolean pan = true, boolean zoom = true);
    UnchainViewersCmd(Viewer*, Editor* = nil, boolean pan = true, boolean zoom = true);
    virtual ~UnchainViewersCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    Viewer* _viewer;
    boolean _pan;
    boolean _zoom;
};

class ChainViewerCmd : public Command {
public:
    ChainViewerCmd(ControlInfo*, boolean pan = true, boolean zoom = true);
    ChainViewerCmd(Editor* = nil, boolean pan = true, boolean zoom = true);
    virtual ~ChainViewerCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    boolean _pan;
    boolean _zoom;
};

class UnchainViewerCmd : public Command {
public:
    UnchainViewerCmd(ControlInfo*, boolean pan = true, boolean zoom = true);
    UnchainViewerCmd(Editor* = nil, boolean pan = true, boolean zoom = true);
    virtual ~UnchainViewerCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    boolean _pan;
    boolean _zoom;
};

#endif
