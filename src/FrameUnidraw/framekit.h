/*
 * Copyright (c) 1996-1999 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cartoactive Systems
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
 * FrameKit - object to construct objects for an FrameEditor
 */

#ifndef frame_kit_h
#define frame_kit_h

#include <OverlayUnidraw/ovkit.h>

#include <InterViews/_enter.h>

class EivTextEditor;
class FrameEditor;

//: derived OverlayKit for use with FrameEditor.
class FrameKit : public OverlayKit {
public:
    FrameKit();

    virtual void InitViewer();
    virtual void InitLayout(const char* name);
    static void InitLayout(OverlayKit* kit, const char* name);

    virtual MenuItem* MakeFileMenu();
    virtual MenuItem* MakeEditMenu();
    virtual MenuItem* MakeStructureMenu();
    virtual MenuItem* MakeViewMenu();
    virtual MenuItem* MakeFrameMenu();
    virtual Glyph* MakeStates();

    static FrameKit* Instance();
protected:
    static FrameKit* _framekit;
};

#include <InterViews/action.h>

#include <InterViews/_leave.h>

#endif
