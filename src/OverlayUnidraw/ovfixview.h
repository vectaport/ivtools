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
 * OvHideCmd - a command for hiding selected graphics
 */

#ifndef ov_fixview_h
#define ov_fixview_h

#include <Unidraw/Commands/command.h>

class FixViewCmd : public Command {
public:
    FixViewCmd(ControlInfo*, boolean size = true, boolean location = true);
    FixViewCmd(Editor* = nil, boolean size = true, boolean location = true);
    virtual ~FixViewCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Size() { return _size; }
    boolean Location() { return _location; }

protected:
    boolean _size;
    boolean _location;


};

class UnfixViewCmd : public Command {
public:
    UnfixViewCmd(ControlInfo*, boolean size = true, boolean location = true);
    UnfixViewCmd(Editor* = nil, boolean size = true, boolean location = true);
    virtual ~UnfixViewCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Size() { return _size; }
    boolean Location() { return _location; }

protected:
    boolean _size;
    boolean _location;


};

#endif
