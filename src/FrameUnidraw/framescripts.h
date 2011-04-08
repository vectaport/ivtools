/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994, 1995, 1999 Vectaport
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * FrameScript* and FrameIdrawScript - command-oriented external 
 * representation of frame components
 */

#ifndef frame_scripts_h
#define frame_scripts_h

#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Components/grcomp.h>

class Clipboard;
class FrameOverlaysComp;
class FrameComp;
class FramesComp;
class FrameIdrawComp;
class ostream;

//: serialized view of FrameOverlaysComp.
class FrameOverlaysScript : OverlaysScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    FrameOverlaysScript(FrameOverlaysComp* = nil);

};

//: serialized view of FrameComp.
class FrameScript : public OverlaysScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    FrameScript(FrameComp* = nil);

    virtual boolean Definition(ostream&);
    static int ReadChildren(istream&, void*, void*, void*, void*);
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);

    boolean suppress_frame() { _suppress_frame = true; }
protected:
    boolean _suppress_frame;
};

//: serialized view of FramesComp.
class FramesScript : public FrameScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    FramesScript(FramesComp* = nil);

    virtual boolean Definition(ostream&);
    static int ReadFrames(istream& in, void* addr1, void* addr2, void* addr3, void* addr4);
};

//: serialized view of FrameIdrawComp.
class FrameIdrawScript : public FramesScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    FrameIdrawScript(FrameIdrawComp* = nil);

    virtual ~FrameIdrawScript();

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);

    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    virtual boolean Emit(ostream&);
    virtual Clipboard* GetGSList();
    virtual Clipboard* GetPtsList();
    virtual Clipboard* GetPicList();

    virtual void SetByPathnameFlag(boolean);
    virtual boolean GetByPathnameFlag();

protected:
    Clipboard* _gslist;
    Clipboard* _ptslist;
    Clipboard* _piclist1;
    Clipboard* _piclist2;
    boolean _gs_compacted;
    boolean _pts_compacted;
    boolean _pic_compacted;
    boolean _by_pathname;
};

#endif

