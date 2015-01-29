/*
 * Copyright (c) 1994-1995,1999 Vectaport Inc.
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
 * FixViewCmd - a command for fixing size and location of graphics
 */

#ifndef ov_fixview_h
#define ov_fixview_h

#include <Unidraw/Commands/command.h>

//: command for fixing size or location of a graphic view.
class FixViewCmd : public Command {
public:
    FixViewCmd(ControlInfo*, boolean size = true, boolean location = true);
    FixViewCmd(Editor* = nil, boolean size = true, boolean location = true);
    virtual ~FixViewCmd();

    virtual void Log();
    // log without setting modify flag.
    virtual void Execute();
    // fix location or size of every view in selection.
    virtual void Unexecute();
    // reverse fixing of location or size of every view in stored selection.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Size() { return _size; }
    // return fix size flag.
    boolean Location() { return _location; }
    // return fix location flag.

protected:
    boolean _size;
    boolean _location;


};

//: command for unfixing size or location of a graphic view.
class UnfixViewCmd : public Command {
public:
    UnfixViewCmd(ControlInfo*, boolean size = true, boolean location = true);
    UnfixViewCmd(Editor* = nil, boolean size = true, boolean location = true);
    virtual ~UnfixViewCmd();

    virtual void Log();
    // log without setting modify flag.
    virtual void Execute();
    // unfix location or size of every view in selection.
    virtual void Unexecute();
    // reverse unfixing of location or size of every view in stored selection.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean Size() { return _size; }
    // return unfix size flag.
    boolean Location() { return _location; }
    // return unfix location flag.

protected:
    boolean _size;
    boolean _location;


};

#endif
