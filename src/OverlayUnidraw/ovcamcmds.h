/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cider Press
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
 * OverlayIdraw-specific commands.
 */

#ifndef ovcamcmds_h
#define ovcamcmds_h

#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Commands/edit.h>

class PanDialog;
class Perspective;
class ZoomDialog;

//: base class for pan and zoom commands.
class CameraMotionCmd : public Command {
public:
    CameraMotionCmd(ControlInfo*);
    CameraMotionCmd(Editor* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

#if 0
    void MoveCamera(IntCoord, IntCoord);
#endif
};

//: command for viewer zoom/de-zoom.
class ZoomCmd : public CameraMotionCmd {
public:
    ZoomCmd(ControlInfo*, float zf =1.0);
    ZoomCmd(Editor* = nil, float zf =1.0);

    virtual void Execute();
    // same as viewer zoom mechanism.
    virtual void Unexecute();
    virtual boolean Reversible();
    // returns false, but why?

    virtual Command* Copy();
    virtual void Read(istream&); // archaic read method.
    virtual void Write(ostream&); // archaic write method.
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    float _zf;
};

//: command for zooming other than by powers of 2.
// Although this command works, it has yet to be useful
// because other parts of the framework constrain the
// resultant zoom to a power of 2 anyways.
class PreciseZoomCmd : public CameraMotionCmd {
public:
    PreciseZoomCmd(ControlInfo*);
    PreciseZoomCmd(Editor* = nil);
    virtual ~PreciseZoomCmd();

    virtual void Execute();
    // prompt for precise zoom value, then execute ZoomCmd.
    virtual boolean Reversible();
    // returns false, but why?

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    ZoomDialog* _dialog;
};

//: command for panning viewer.
class PanCmd : public CameraMotionCmd {
public:
    PanCmd(ControlInfo*, IntCoord px =0, IntCoord py =0);
    PanCmd(Editor* = nil, IntCoord px =0, IntCoord py =0);

    virtual void Execute();
    // same as viewer pan mechanism.
    virtual void Unexecute();
    virtual boolean Reversible();
    // returns false, but why?

    virtual Command* Copy();
    virtual void Read(istream&); // archaic read method.
    virtual void Write(ostream&); // archaic write method.
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    IntCoord _px;
    IntCoord _py;
};

//: pan amount for FixedPanCmd
enum PanAmount { NO_PAN, PLUS_SMALL_PAN, PLUS_LARGE_PAN, MINUS_SMALL_PAN,
		 MINUS_LARGE_PAN };

//: command for panning by fixed amount.
// Used in a set of pull-down menu items under the "View" menu of a drawing editor.
class FixedPanCmd : public CameraMotionCmd {
public:
    FixedPanCmd(ControlInfo*, PanAmount xpan =NO_PAN, PanAmount ypan =NO_PAN);
    FixedPanCmd(Editor* = nil, PanAmount xpan =NO_PAN, PanAmount ypan =NO_PAN);

    virtual void Execute();
    // figure pan amount and invoke PanCmd.
    virtual boolean Reversible();
    // returns false, but why?

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PanAmount _xpan;
    PanAmount _ypan;
};

//: command for precision panning of the viewer.
class PrecisePanCmd : public CameraMotionCmd {
public:
    PrecisePanCmd(ControlInfo*);
    PrecisePanCmd(Editor* = nil);
    virtual ~PrecisePanCmd();

    virtual void Execute();
    // prompt for precise amount to pan, then invoke PanCmd.
    virtual boolean Reversible();
    // returns false, but why?

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PanDialog* _dialog;
};

#endif
