/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

//: ComEditor specialized for multi-frame use.
class FrameEditor : public ComEditor {
public:
    FrameEditor(OverlayComp*, OverlayKit* = FrameKit::Instance());
    // construct based on pre-existing FrameIdrawComp.
    FrameEditor(const char*, OverlayKit* = FrameKit::Instance());
    // construct from pathname of a flipbook document.
    FrameEditor(boolean initflag, OverlayKit* = FrameKit::Instance());
    // constructor for use of derived classes.
    virtual ~FrameEditor();

    void Init (OverlayComp* = nil, const char* = "FrameEditor");
    virtual void UpdateFrame(boolean txtupdate =true);
    // update state variables and text-editor display for current frame.
    virtual void InitFrame();
    // initialize multi-frame display mechanism.
    virtual void Update();
    // pass to ComEditor.
    virtual void AddCommands(ComTerp*);
    // add interpreter commands to ComTerp associated with this ComEditor.
    virtual void InitCommands();
    // execute Unidraw commands as needed after FrameEditor is constructed.

    EivTextEditor* TextEditor() { return _texteditor; }
    // return pointer to text-editor that holds current frame annotation.
    void SetText();
    // set contents of text-editor as frame annotation.
    void ClearText();
    // clear contents of text-editor.
    void UpdateText(OverlayComp*, boolean update =true);
    // update contents of text-editor with frame annotation.

    void SetFrame(FrameView* f) { _prevframe = _currframe;_currframe = f; }
    // set current frame.
    virtual OverlaysView* GetFrame(int index=-1);
    // return current frame.

    int OtherFrame(){ return _curr_other; }
    // return index of previous (or secondary) frame.
    void OtherFrame(int other_frame) 
      { _prev_other = _curr_other; _curr_other = other_frame; }
    // set index of previous (or secondary) frame.

    FrameNumberState*& framenumstate() { return _framenumstate; }
    // return reference to pointer to current-frame-number state variable
    FrameListState*& frameliststate() { return _frameliststate; }
    // return reference to pointer to current-frame-count state variable

    void ToggleAutoNewFrame() { _autonewframe = !_autonewframe; }
    // toggle flag which indicates a new frame should be automatically
    // created whenever anything is imported.
    boolean AutoNewFrame() { return _autonewframe; }
    // return flag which indicates a new frame should be automatically
    // created whenever anything is imported.
    virtual void DoAutoNewFrame();
    // virtual method which does the work of creating a new frame
    // if flag is set.

protected:
    FrameView* _currframe;
    FrameView* _prevframe;
    FrameNumberState* _framenumstate;
    FrameListState* _frameliststate;
    EivTextEditor* _texteditor;
    int _curr_other;
    int _prev_other;
    boolean _autonewframe;

friend FrameKit;
};

declareActionCallback(FrameEditor)

#endif




