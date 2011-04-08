/*
 * Copyright (c) 2004 Scott E. Johnston
 * Copyright (c) 1994-1997,1999 Vectaport Inc.
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
 * DrawKit - object to construct objects for an ComEditor
 */

#ifndef draw_kit_h
#define draw_kit_h

#include <FrameUnidraw/framekit.h>

#include <InterViews/_enter.h>

class FieldEditor;
class NodeDialog;

//: specialized FrameKit for use with drawserv.
class DrawKit : public FrameKit {
public:
    DrawKit();

    virtual void Init(OverlayComp*, const char* name);
    virtual MenuItem *MakeFileMenu();
    virtual Glyph* MakeToolbar();
    virtual MenuItem* MakeToolsMenu();
    virtual MenuItem* MakeViewersMenu();

    static DrawKit* Instance();

    virtual OverlaySelection* MakeSelection(Selection* sel = nil);
    // make Selection of the proper derivation.
protected:
    void toolbar0();
    void toolbar1();
    void launch_drawtool();
    void launch_flipbook();
    void launch_graphdraw();

protected:
    static DrawKit* _comkit;
};

#include <InterViews/_leave.h>

#endif
