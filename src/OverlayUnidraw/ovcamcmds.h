/*
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


class ZoomCmd : public CameraMotionCmd {
public:
    ZoomCmd(ControlInfo*, float zf =1.0);
    ZoomCmd(Editor* = nil, float zf =1.0);

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    float _zf;
};

class PreciseZoomCmd : public CameraMotionCmd {
public:
    PreciseZoomCmd(ControlInfo*);
    PreciseZoomCmd(Editor* = nil);
    virtual ~PreciseZoomCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    ZoomDialog* _dialog;
};

class PanCmd : public CameraMotionCmd {
public:
    PanCmd(ControlInfo*, IntCoord px =0, IntCoord py =0);
    PanCmd(Editor* = nil, IntCoord px =0, IntCoord py =0);

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual void Read(istream&);
    virtual void Write(ostream&);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    IntCoord _px, _py;
};

enum PanAmount { NO_PAN, PLUS_SMALL_PAN, PLUS_LARGE_PAN, MINUS_SMALL_PAN,
		 MINUS_LARGE_PAN };

class FixedPanCmd : public CameraMotionCmd {
public:
    FixedPanCmd(ControlInfo*, PanAmount xpan =NO_PAN, PanAmount ypan =NO_PAN);
    FixedPanCmd(Editor* = nil, PanAmount xpan =NO_PAN, PanAmount ypan =NO_PAN);

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PanAmount _xpan, _ypan;
};

class PrecisePanCmd : public CameraMotionCmd {
public:
    PrecisePanCmd(ControlInfo*);
    PrecisePanCmd(Editor* = nil);
    virtual ~PrecisePanCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    PanDialog* _dialog;
};

#endif
