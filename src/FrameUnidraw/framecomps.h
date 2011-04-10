/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

#ifndef framecomps_h
#define framecomps_h

#include <OverlayUnidraw/ovcomps.h>

class FrameCatalog;
class FrameKit;
class FrameScript;
class FramesScript;
class FrameIdrawScript;

class FrameOverlaysComp : public OverlaysComp {
public:
    FrameOverlaysComp(OverlayComp* parent = nil);
    FrameOverlaysComp(Graphic*, OverlayComp* parent = nil);
    FrameOverlaysComp(istream&, OverlayComp* parent = nil);

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _frame_ovcomps_params;

};

class FrameComp : public OverlaysComp {
public:
    FrameComp(OverlayComp* parent = nil);
    FrameComp(Graphic* g, OverlayComp* parent = nil);
    FrameComp(istream&, OverlayComp* parent = nil);

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _frame_comp_params;

};

class FramesComp : public FrameComp {
public:
    FramesComp(OverlayComp* parent = nil);
    FramesComp(Graphic* g, OverlayComp* parent = nil);
    FramesComp(istream&, OverlayComp* parent = nil);
    
    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);
    
    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _frame_comps_params;
    
};

class FrameIdrawComp : public FramesComp {
public:
    FrameIdrawComp(boolean add_bg = true, const char* pathname = nil, OverlayComp* parent = nil);
    FrameIdrawComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);
    virtual ~FrameIdrawComp();

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void SetPathName(const char*);
    virtual const char* GetPathName();

    virtual void GrowIndexedGS(Graphic*);
    virtual Graphic* GetIndexedGS(int);
    virtual void GrowIndexedPts(MultiLineObj*);
    virtual MultiLineObj* GetIndexedPts(int);
    virtual void GrowIndexedPic(OverlaysComp*);
    virtual OverlaysComp* GetIndexedPic(int);

    virtual void ResetIndexedGS();
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    virtual const char* GetBaseDir();

protected:
    static ParamList* _frame_idraw_params;
    float _xincr, _yincr;
    Picture* _gslist;
    MultiLineObj** _ptsbuf;
    int _ptsnum;
    int _ptslen;
    OverlaysComp** _picbuf;
    int _picnum;
    int _piclen;
    char* _pathname;
    char* _basedir;

};

#endif
