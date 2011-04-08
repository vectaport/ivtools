/*
 * Copyright (c) 1994, 1995, 1999 Vectaport Inc.
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

#ifndef framecmds_h
#define framecmds_h

#include <OverlayUnidraw/ovcmds.h>

#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/datas.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/macro.h>
#include <Unidraw/Commands/struct.h>

#include <InterViews/action.h>

//: command to create a frame.
// commmand to create a frame, before or after the current frame.
class CreateFrameCmd : public Command {
public:
    CreateFrameCmd(ControlInfo*, boolean after = true);
    CreateFrameCmd(Editor* =nil, boolean after = true);
    boolean After() { return _after; }

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
protected:
    boolean _after;
};

//: data object for DeleteFrameCmd.
class DeleteFrameData : public VoidData {
public:
    DeleteFrameData(void*, boolean restore_after);
    boolean RestoreAfter() { return _after; }

protected:
    boolean _after;
};

//: command to delete a frame.
class DeleteFrameCmd : public Command {
public:
    DeleteFrameCmd(ControlInfo*);
    DeleteFrameCmd(Editor* = nil);

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
};

//: command to change current frame.
class MoveFrameCmd : public Command {
public:
    MoveFrameCmd(ControlInfo*, int motion = +1, boolean allowbg = true);
    MoveFrameCmd(Editor* =nil, int motion = +1, boolean allowbg = true);
    void init(int motion, boolean allowbg);
    
    void wraparound(boolean flag) { _wraparound = flag; }
    // set flag to indicate forward wraparound during replay.
    boolean wraparound() { return _wraparound; }
    // get flag to indicate forward wraparound during replay.

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();
    virtual void Log();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();

    boolean AllowBg() { return _allowbg; }
    // get flag that indicates whether to allow a background frame.
    void AllowBg(boolean abg) { _allowbg = abg; }
    // set flag that indicates whether to allow a background frame.

    static const char* MoveFuncFormat(); 
    // return format string for generating interpreted move command.
    static const char* AbsMoveFuncFormat(); 
    // return format string for generating interpreted move command with
    // absolute frame arguments.
    static void FuncEnable(const char* movefunc=nil, const char* absmovefunc=nil); 
    // enable execution of an interpreter command on each move.
    static void FuncDisable() { _func_on=false; }
    // disable execution of an interpreter command on each move.

    int requestmotion() { return _requestmotion; }
    // requested motion in frames, negative means backward.
    int plannedmotion() { return _plannedmotion; }
    // planned motion in frames, negative means backward.
    int actualmotion() { return _actualmotion; }
    // actual motion in frames, negative means backward.

    void set_wraparound();
    // set forward wraparound flag.
    void clr_wraparound();
    // clear forward wraparound flag.

    static MoveFrameCmd* default_instance() { return _default; }
    // return a default instance of the MoveFrameCmd.
    static void default_instance(MoveFrameCmd* cmd)
      { _default = cmd; }
    // set a default instance of the MoveFrameCmd.
    
protected:
    int _requestmotion, _actualmotion, _plannedmotion;
    boolean _allowbg;
    boolean _wraparound;

    static boolean _func_on;
    static char* _move_func;
    static char* _absmove_func;

    static MoveFrameCmd* _default;

friend class MoveFrameFunc;
};

declareActionCallback(MoveFrameCmd)

//: command to move to the first frame.
class FrameBeginCmd : public MoveFrameCmd {
public:
    FrameBeginCmd(ControlInfo*);
    FrameBeginCmd(Editor* =nil);

    virtual void Execute();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
};

//: command to move to the last frame.
class FrameEndCmd : public MoveFrameCmd {
public:
    FrameEndCmd(ControlInfo*);
    FrameEndCmd(Editor* =nil);

    virtual void Execute();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
};

//: command to create a frame and move to it.
class CreateMoveFrameCmd : public MacroCmd {
public:
    CreateMoveFrameCmd(ControlInfo*, boolean after = true);
    CreateMoveFrameCmd(Editor* = nil, boolean after = true);
    boolean After() { return _after; }

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();

    CreateFrameCmd* createframecmd();
    MoveFrameCmd* moveframecmd();
protected:
    boolean _after;
};

//: command to copy contents of current frame to create a new frame.
class CopyMoveFrameCmd : public MacroCmd {
public:
    CopyMoveFrameCmd(ControlInfo*, boolean after = true);
    CopyMoveFrameCmd(Editor* = nil, boolean after = true);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
    virtual void Execute();
protected:
    boolean _after;
};

//: OvGroupCmd specialized for FrameUnidraw use.
class FrameGroupCmd : public OvGroupCmd {
public:
    FrameGroupCmd(ControlInfo*, OverlayComp* dest = nil);
    FrameGroupCmd(Editor* = nil, OverlayComp* dest = nil);

    virtual void Execute ();
    virtual OverlaysComp* MakeOverlaysComp();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};


//: UngroupCmd specialized for FrameUnidraw use.
class FrameUngroupCmd : public UngroupCmd {
public:
    FrameUngroupCmd(ControlInfo*);
    FrameUngroupCmd(Editor* = nil);
    virtual ~FrameUngroupCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: FrontCmd specialized for FrameUnidraw use.
class FrameFrontCmd : public FrontCmd {
public:
    FrameFrontCmd(ControlInfo*);
    FrameFrontCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: BackCmd specialized for FrameUnidraw use.
class FrameBackCmd : public BackCmd {
public:
    FrameBackCmd(ControlInfo*);
    FrameBackCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};
 
//: CopyCmd specialized for FrameUnidraw use.
class FrameCopyCmd : public CopyCmd {
public:
    FrameCopyCmd(ControlInfo*, Clipboard* = nil);
    FrameCopyCmd(Editor* = nil, Clipboard* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: OvNewViewCmd specialized for FrameUnidraw use.
class FrameNewViewCmd : public OvNewViewCmd {
public:
    FrameNewViewCmd(ControlInfo*);
    FrameNewViewCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: command to enable/disable display of previous frame
class ShowOtherFrameCmd : public Command {
public:
    ShowOtherFrameCmd(ControlInfo*, int offset = -1);
    ShowOtherFrameCmd(Editor* =nil, int offset = -1);

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();
    virtual void Log();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();

protected:
    int _offset;
    int _old_offset;
};

//: command to enable/disable auto-new-frame on import.
class AutoNewFrameCmd : public MacroCmd {
public:
    AutoNewFrameCmd(ControlInfo*);
    AutoNewFrameCmd(Editor* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual Command* Copy();
    virtual void Execute();
    virtual void Unexecute();
    virtual void Log();
    virtual boolean Reversible();

    static AutoNewFrameCmd* default_instance() { return _default; }
    static void default_instance(AutoNewFrameCmd* cmd)
      { _default = cmd; }
    
protected:
    static AutoNewFrameCmd* _default;

};

declareActionCallback(AutoNewFrameCmd)

#endif
