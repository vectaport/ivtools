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
 * command for hiding and desensitizing selected views
 */

#ifndef ov_showhide_h
#define ov_showhide_h

#include <Unidraw/Commands/command.h>

class Viewer;

class HideViewCmd : public Command {
public:
    HideViewCmd(Viewer*, ControlInfo*);
    HideViewCmd(Viewer*, Editor* = nil);
    virtual ~HideViewCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    Viewer* _viewer;
};

class UnhideViewsCmd : public Command {
public:
    UnhideViewsCmd(ControlInfo*);
    UnhideViewsCmd(Editor* = nil);
    virtual ~UnhideViewsCmd();

    virtual boolean Reversible();
    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
};

class DesensitizeViewCmd : public Command {
public:
    DesensitizeViewCmd(Viewer*, ControlInfo*);
    DesensitizeViewCmd(Viewer*, Editor* = nil);
    virtual ~DesensitizeViewCmd();

    virtual void Log();
    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
    Viewer* _viewer;
};

class SensitizeViewsCmd : public Command {
public:
    SensitizeViewsCmd(ControlInfo*);
    SensitizeViewsCmd(Editor* = nil);
    virtual ~SensitizeViewsCmd();

    virtual boolean Reversible();
    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


protected:
};

#endif
