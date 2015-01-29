/*
 * Copyright (c) 2004 Scott E. Johnston
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

#ifndef draweditor_h
#define draweditor_h

#include <FrameUnidraw/frameeditor.h>

class DrawKit;
class Selection;

//: editor for DrawServ application
class DrawEditor : public FrameEditor {
public:
    DrawEditor(OverlayComp*, OverlayKit* = OverlayKit::Instance());
    // constructor for using existing component.
    DrawEditor(const char* file, OverlayKit* = OverlayKit::Instance());
    // constructor for building top-level component from a file.
    DrawEditor(boolean initflag, OverlayKit* = OverlayKit::Instance());
    // constructor for use of derived classes.
    void Init(OverlayComp* = nil, const char* name = "DrawEditor");
    virtual void InitCommands();
    // method for running Unidraw Command objects after OverlayEditor
    // is constructed.
    virtual void AddCommands(ComTerp*);
    // method for adding ComFunc objects to the ComTerp associated with
    // this DrawEditor.

    Selection* last_selection() { return _last_selection; }
    // return point to Selection that shadows the last setting.
protected:
    Selection* _last_selection;

friend class DrawKit;

};

#endif
