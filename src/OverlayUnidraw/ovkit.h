/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1995-1999 Vectaport Inc.
 * Copyright (c) 1994 Vectaport Inc., Cartoactive Systems
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
class OverlaySelection;
class PSBrush;
class PSColor;
class PSPattern;
class Patch;
class Selection;
class TelltaleGroup;
class Tool;
class ToolButton;
class UPage;
class Viewer;

//: helper class for constructing an OverlayEditor.
// this class (or something derived from it) is supplied to the constructor
// of an OverlayEditor (or any class derived from an OverlayEditor).
// The idea behind OverlayKit is it facilitates a much more flexible construction
// of an OverlayEditor, the central object in any ivtools drawing editor, by
// allow the use of virtual functions during the construction.  Normally
// virtual functions can't be used by a constructor, but virtual functions on
// a pre-constructed helper class, like OverlayKit, can be used.
// <p>
// In this way other classes can be derived from OverlayKit with different
// virtual methods for initializing the pull-down menus and toolbars, and can
// completely change the appearance and functionality of essentially the same
// OverlayEditor.
// <p>
// Another feature of the OverlayKit idea is it solves the problem of what to
// do when a particular vertical application wants to use the features of two
// different derivations of an OverlayEditor.  Prior to OverlayKit you would need
// to use multiple-inheritance, which can have problems, or permanently decide
// on the ordering of one derived editor relative to the other, which may be
// undesirable in future circumstances.  With an OverlayKit it is easier to mix
// and match the desirable features of one lineage of derived OverlayEditor classes
// with the features of another lineage, pushing capability as needed from the 
// editor to the kit.
// <p>
// There is support for a default OverlayKit for use when constructing an
// OverlayEditor.  In general any editor-specific state should reside in
// the editor, not the kit, but this is not a hard-and-fast rule.
class OverlayKit {
public:
    OverlayKit();
    virtual ~OverlayKit();

    void SetEditor(OverlayEditor*);
    // set editor currently associated with this kit.
    OverlayEditor* GetEditor();
    // return pointer to editor currently associated with this kit.
    Interactor* Interior();
    // construct the Interactor based interior of an editor.

    virtual void Init(OverlayComp* comp, const char* name);
    // initialize the editor, then the associated viewer, using an optional 'comp'.    

    virtual void InitMembers(OverlayComp*);
    // initialize member variables of the editor.
    virtual void InitViewer();
    // initialize viewer to go with the editor.
    virtual void InitLayout(const char* name);
    // initialize chrome that goes around the viewer.
    static void InitLayout(OverlayKit* kit, const char* name);
    // static method that implements virtual method.

    virtual Glyph* MakeMenus();
    // make all the pull-down menus and their menu bar.

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
    // make state views in the chrome.
    virtual Glyph* MakeToolbar();
    // make tool palette.

    static OverlayKit* Instance();
    // default instance of OverlayKit.

    virtual void Annotate(OverlayComp*);
    // utility method for popping an annotation dialog box.
    virtual void AttrEdit(OverlayComp*);
    // utility method for popping an attribute editing dialog box.

    static int bintest(const char* name);
    // static method for testing for URL prefix on pathname.  0 if found, otherwise -1.
    static boolean bincheck(const char* name);
    // static method for testing for URL prefix on pathname.  Returns true if found.

    virtual Glyph* appicon();
    // glyph to use for application icon.
    virtual const char* appname() { return _appname; }
    // get name of application.
    virtual void appname(const char* name) { _appname=name; }
    // set name of application.

    void MouseDoc(const char*);
    // set current mouse documentation string on the editor.

    static const char* mouse_sel;
    static const char* mouse_mov;
    static const char* mouse_scl;
    static const char* mouse_str;
    static const char* mouse_rot;
    static const char* mouse_alt;
    static const char* mouse_mag;
    static const char* mouse_txt;
    static const char* mouse_lin;
    static const char* mouse_mlin;
    static const char* mouse_ospl;
    static const char* mouse_rect;
    static const char* mouse_ellp;
    static const char* mouse_poly;
    static const char* mouse_cspl;
    static const char* mouse_anno;
    static const char* mouse_attr;
    static const char* mouse_tack;
#ifdef CLIPPOLY
    static const char* mouse_clipr;
#else
    static const char* mouse_clipr;
#endif    
#ifdef CLIPPOLY
    static const char* mouse_clipp;
#else
    static const char* mouse_clipp;
#endif    
    static const char* mouse_convexhull;
    static const char* mouse_imgscale;
    static const char* mouse_logscale;
    static const char* mouse_pseudocolor;
    static const char* mouse_grloc;
    static const char* mouse_custom;

    void otherdisplay(const char* display);
    // set possible alternate X display string for constructing viewer.  
    // Not yet working.

    boolean& set_button_flag() { return _set_button_flag; }
    // flag to add setr button to text editor
    boolean& clr_button_flag() { return _clr_button_flag; }
    // flag to add clear button to text editor

    OverlayComp* add_tool_button(const char* path, OverlayComp* comp=nil);
    // low-level routine used by ::add_custom_tool and others

    virtual OverlaySelection* MakeSelection(Selection* sel = nil);
    // make Selection of the proper derivation.
protected:
    Glyph* MenuLine(PSBrush*);
    // create line to put in a pulldown menu.
    Glyph* MenuArrowLine(boolean tail, boolean head);
    // create arrow line to put in a pulldown menu.
    Glyph* MenuRect(PSColor*);
    // create color rectangle to put in a pulldown menu.
    Glyph* MenuPatRect(PSPattern*);
    // create patterned rectangle to put in a pulldown menu.
    void MakeMenu(MenuItem*, Command*, const char*);
    // helper method for making a pull-down menu entry.
    void MakeMenu(MenuItem*, Command*, Glyph*);
    // helper method for making a pull-down menu entry.
    ToolButton* MakeTool(Tool*, Glyph*, TelltaleGroup*, 
	ObservableText* mousedoc = nil, const char* doc = "");
    // helper method for making a tool for the toolbar.

    const char* otherdisplay();
    // returns string that might specify an alternate X display.

public:
    void toolbar0();
    // switch to default toolbar
    void toolbar1();
    // switch to alternate toolbar
    void add_custom_tool();
    // import idraw-format tool button

protected:
    OverlayEditor* _ed;
    Deck* _toolbars;
    Patch* _toolbar;
    Glyph** _toolbar_vbox;
    TelltaleGroup* _tg;
    FloatCoord _maxwidth;

    char* _otherdisplay;
    boolean _set_button_flag;
    boolean _clr_button_flag;
    const char* _appname;

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
