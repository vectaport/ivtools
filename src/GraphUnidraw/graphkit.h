/*
 * Copyright (c) 1994,1999 Vectaport Inc.
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
 * GraphKit - object to construct objects for an GraphEditor
 */

#ifndef graph_kit_h
#define graph_kit_h

#include <OverlayUnidraw/ovkit.h>

#include <InterViews/_enter.h>

class FieldEditor;
class NodeDialog;

//: kit for constructing GraphEditor.
class GraphKit : public OverlayKit {
public:
    GraphKit();

    virtual void Init(OverlayComp*, const char* name);
    // initialize based on optional component and pathname.
    virtual void InitLayout(const char* name);
    // initialize layout of editor/viewer.
    virtual MenuItem* MakeFileMenu();
    virtual MenuItem* MakeEditMenu();
    virtual MenuItem* MakeViewMenu();
    virtual MenuItem* MakeToolsMenu();
    virtual Glyph* MakeToolbar();

    static GraphKit* Instance();
    // return default instance of GraphKit.

    static const char mouse_node[]  = "l-click: Add Node; m-drag: Move; r-click/drag: Select";
    static const char mouse_lnode[]  = "l-click: Add Labeled Node; m-drag: Move; r-click/drag: Select";
    static const char mouse_labl[]  = "l-click: Type In Label, l-,m-,r-click To Finish";
    static const char mouse_edge[]  = "l-drag: Connect; m-drag: Move; r-click/drag: Select";

protected:
    static GraphKit* _graphkit;
};

declareActionCallback(GraphKit)

#include <InterViews/_leave.h>

#endif
