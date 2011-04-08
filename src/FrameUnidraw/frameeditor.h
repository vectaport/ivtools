/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#ifndef frameeditor_h
#define frameeditor_h

#include <ComUnidraw/comeditor.h>
#include <FrameUnidraw/framekit.h>

#include <Unidraw/iterator.h>

class FrameNumberState;
class FrameListState;
class FrameView;

class FrameEditor : public ComEditor {
public:
    FrameEditor(OverlayComp*, OverlayKit* = FrameKit::Instance());
    FrameEditor(const char*, OverlayKit* = FrameKit::Instance());
    FrameEditor(boolean initflag, OverlayKit* = FrameKit::Instance());
    virtual ~FrameEditor();

    void Init (OverlayComp* = nil, const char* = "FrameEditor");
    virtual void UpdateFrame(boolean txtupdate =true);
    virtual void InitFrame();
    virtual void Update();

    EivTextEditor* TextEditor() { return _texteditor; }
    void SetText();
    void ClearText();
    void UpdateText(OverlayComp*, boolean update =true);

    void SetFrame(FrameView* f) { _prevframe = _currframe;_currframe = f; }
    FrameView* GetFrame(int index=-1);

    int OtherFrame(){ return _curr_other; }
    void OtherFrame(int other_frame) 
      { _prev_other = _curr_other; _curr_other = other_frame; }

    FrameNumberState*& framenumstate() { return _framenumstate; }
    FrameListState*& frameliststate() { return _frameliststate; }
protected:
    FrameView* _currframe;
    FrameView* _prevframe;
    FrameNumberState* _framenumstate;
    FrameListState* _frameliststate;
    EivTextEditor* _texteditor;
    int _curr_other;
    int _prev_other;

friend FrameKit;
};

declareActionCallback(FrameEditor)

#endif




