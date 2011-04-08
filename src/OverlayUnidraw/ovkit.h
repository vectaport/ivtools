/*
 * Copyright (c) 1994-1995 Vectaport Inc.
 * Copyright (c) 1994 Cartoactive Systems
 * Copyright (c) 1993 David B. Hollenbeck
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
 * OverlayKit - object to construct objects for an OverlayEditor
 */

#ifndef overlay_kit_h
#define overlay_kit_h

#include <InterViews/defs.h>
#include <Unidraw/enter-scope.h>
#include <InterViews/_enter.h>

class Color;
class Command;
class Deck;
class Editor;
class Glyph;
class Grid;
class Interactor;
class MenuItem;
class ObservableText;
class OverlayComp;
class OverlayEditor;
class Patch;
class UPage;
class PSBrush;
class PSPattern;
class Viewer;
class TelltaleGroup;
class Tool;
class ToolButton;

class OverlayKit {
public:
    OverlayKit();
    virtual ~OverlayKit();

    void SetEditor(OverlayEditor*);
    OverlayEditor* GetEditor();
    Interactor* Interior();

    virtual void Init(OverlayComp*, const char* name);

    virtual void InitMembers(OverlayComp*);
    virtual void InitViewer();
    virtual void InitLayout(const char* name);

    virtual Glyph* MakeMenus();

    virtual MenuItem* MakeFileMenu();
    virtual MenuItem* MakeEditMenu();
    virtual MenuItem* MakeStructureMenu();
    virtual MenuItem* MakeFontMenu();
    virtual MenuItem* MakeBrushMenu();
    virtual MenuItem* MakePatternMenu();
    virtual MenuItem* MakeFgColorMenu();
    virtual MenuItem* MakeBgColorMenu();
    virtual MenuItem* MakeAlignMenu();
    virtual MenuItem* MakeFrameMenu();
    virtual MenuItem* MakeViewMenu();
    virtual MenuItem* MakeToolsMenu();
    virtual MenuItem* MakeViewersMenu();

    virtual Glyph* MakeStates();
    virtual Glyph* MakeToolbar();

    static OverlayKit* Instance();

    virtual void Annotate(OverlayComp*);
    virtual void AttrEdit(OverlayComp*);

    static int bintest(const char* name);
    static boolean bincheck(const char* name);

    void MouseDoc(const char*);

    static const char mouse_sel[]  = "l-click/drag: Select; m-drag: Move; r-click/drag: Select";
    static const char mouse_mov[]  = "l-drag: Move; m-drag: Move; r-click/drag: Select";
    static const char mouse_scl[]  = "l-drag: Scale; m-drag: Move; r-click/drag: Select";
    static const char mouse_str[]  = "l-drag: Stretch; m-drag: Move; r-click/drag: Select";
    static const char mouse_rot[]  = "l-drag: Rotate; m-drag: Move; r-click/drag: Select";
    static const char mouse_alt[]  = "l-click: Alter; m-m-drag: Move; r-click/drag: Select";
    static const char mouse_mag[]  = "l-drag: Magnify; m-drag: Move; r-click/drag: Select";
    static const char mouse_txt[]  = "l-click: Text; m-drag: Move; r-click/drag: Select";
    static const char mouse_lin[]  = "l-drag: Line; m-drag: Move; r-click/drag: Select";
    static const char mouse_mlin[] = "l-click: Start Multi-Line; m-drag: Move; r-click/drag: Select";
    static const char mouse_ospl[] = "l-click: Start Open Spline; m-drag: Move; r-click/drag: Select";
    static const char mouse_rect[] = "l-drag: Rectangle; m-drag: Move; r-click/drag: Select";
    static const char mouse_ellp[] = "l-drag: Ellipse; m-drag: Move; r-click/drag: Select";
    static const char mouse_poly[] = "l-click: Start Polygon; m-drag: Move; r-click/drag: Select";
    static const char mouse_cspl[] = "l-drag: Start Closed Spline; m-drag: Move; r-click/drag: Select";
    static const char mouse_anno[] = "l-click: Annotate; m-drag: Move; r-click/drag: Select";
    static const char mouse_attr[] = "l-click: Edit Attributes; m-drag: Move; r-click/drag: Select";
    static const char mouse_tack[] = "l-click: Tack Down; m-click: Tack and Finish; r-click: Remove Last Tack";
#ifdef CLIPPOLY
    static const char mouse_clipr[] = "l-drag: Clip MultiLines in Box; m-drag: Move; r-click/drag: Select";
#else
    static const char mouse_clipr[] = "l-drag: Clip MultiLines and Polygons in Box; m-drag: Move; r-click/drag: Select";
#endif    
#ifdef CLIPPOLY
    static const char mouse_clipp[] = "l-drag: Clip MultiLines in Polygon; m-drag: Move; r-click/drag: Select";
#else
    static const char mouse_clipp[] = "l-drag: Clip MultiLines and Polygons in Polygon; m-drag: Move; r-click/drag: Select";
#endif    
    static const char mouse_convexhull[] = "l-click: Start Convex Hull; m-drag: Move; r-click/drag: Select";
    static const char mouse_imgscale[] = "l-drag: Scale Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
    static const char mouse_logscale[] = "l-drag: Logarithmically Scale Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
    static const char mouse_pseudocolor[] = "l-drag: Pseudocolor Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
    static const char mouse_grloc[] = "l-click: Location within Graphic; m-drag: Move; r-click/drag: Select";

    void otherdisplay(const char* display);

protected:
    Glyph* MenuLine(PSBrush*);
    Glyph* MenuArrowLine(boolean tail, boolean head);
    Glyph* MenuRect(Color*);
    Glyph* MenuPatRect(PSPattern*);
    void MakeMenu(MenuItem*, Command*, const char*);
    void MakeMenu(MenuItem*, Command*, Glyph*);
    ToolButton* MakeTool(Tool*, Glyph*, TelltaleGroup*, 
	ObservableText* mousedoc = nil, const char* doc = "");

    const char* otherdisplay();
protected:
    OverlayEditor* _ed;
    Deck* _toolbars;
    Patch* _toolbar;

    char* _otherdisplay;

protected:
    static OverlayKit* _overlaykit;
};

#include <OverlayUnidraw/ovdoer.h>
#include <InterViews/action.h>

declareActionCallback(Command)
declareActionCallback(CommandDoer)
declareActionCallback(CommandPusher)
declareActionCallback(ToolSelector)
declareActionCallback(OverlayKit)

#include <InterViews/_leave.h>

#endif
