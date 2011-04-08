/*
 * Copyright (c) 1996-1998 Vectaport Inc.
 * Copyright (c) 1994, 1995 Vectaport Inc., Cartoactive Systems
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
 * FrameKit definitions
 */


#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/framekit.h>
#include <FrameUnidraw/framestates.h>
#include <FrameUnidraw/frameviewer.h>

#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovfixview.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/slctbyattr.h>
#include <OverlayUnidraw/setattrbyexpr.h>

#include <IVGlyph/exportchooser.h>
#include <IVGlyph/saveaschooser.h>
#include <IVGlyph/textform.h>

#include <Unidraw/Commands/catcmds.h>
#include <Unidraw/Commands/transforms.h>

#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/grid.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/statevars.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <UniIdraw/idkybd.h>

#include <InterViews/frame.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/telltale.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>
#include <IV-look/mf_kit.h>
#include <IVGlyph/textedit.h>

#include <OS/string.h>

#include <stdlib.h>

#undef None

/******************************************************************************/

static const char* page_width_attrib = "pagewidth";
static const char* page_height_attrib = "pageheight";
static const char* page_cols_attrib = "pagecols";
static const char* page_rows_attrib = "pagerows";
static const char* grid_x_incr = "gridxincr";
static const char* grid_y_incr = "gridyincr";

/******************************************************************************/

FrameKit* FrameKit::_framekit = nil;

FrameKit::FrameKit () {
}

FrameKit* FrameKit::Instance() {
    if (!_framekit)
	_framekit = new FrameKit();
    return _framekit;
}

void FrameKit::InitViewer () {
    Catalog* catalog = unidraw->GetCatalog();

    const char* page_w = catalog->GetAttribute(page_width_attrib);
    const char* page_h = catalog->GetAttribute(page_height_attrib);
    const char* page_cols = catalog->GetAttribute(page_cols_attrib);
    const char* page_rows = catalog->GetAttribute(page_rows_attrib);
    const char* x_incr = catalog->GetAttribute(grid_x_incr);
    const char* y_incr = catalog->GetAttribute(grid_y_incr);

    GraphicView* view = (GraphicView*)((FrameEditor*)_ed)->_comp->Create(COMPONENT_VIEW);
    ((FrameEditor*)_ed)->_comp->Attach(view);
    view->Update();

    Style* style = Session::instance()->style();
    boolean bookgeom = style->value_is_on("bookgeom");

    float w = bookgeom ? 700 : round(atof(page_w) * ivinches);
    float h = bookgeom ? 906 : round(atof(page_h) * ivinches);
    if (page_cols && page_rows) {
      int ncols = atoi(page_cols);
      int nrows = atoi(page_rows);
      if (ncols>0 && nrows>0) {
	w = ncols;
	h = nrows;
      }
    }

    OverlayPage* page = new OverlayPage(w, h);
    Grid* grid = new Grid(w, h, atof(x_incr), atof(y_incr));
    grid->Visibility(false);

    if (!bookgeom)
	((FrameEditor*)_ed)->_viewer = new FrameViewer(_ed, view, page, grid);
    else 
	((FrameEditor*)_ed)->_viewer = new FrameViewer(_ed, view, page, grid, (int) h+1, (int) w+1, Rotated);
}

void FrameKit::InitLayout(const char* name) {
  InitLayout(this, name);
}

void FrameKit::InitLayout(OverlayKit* kit, const char* name) {
    FrameEditor* ed = (FrameEditor*) kit->GetEditor();
    if (ed->GetWindow() == nil) {
        TextObserver* mousedoc_observer = new TextObserver(ed->MouseDocObservable(), "");
	WidgetKit& wk = *WidgetKit::instance();
	const LayoutKit& layout = *LayoutKit::instance();
	Glyph* menus = kit->MakeMenus();
	Glyph* states = kit->MakeStates();
	Glyph* toolbar = kit->MakeToolbar();
	if (states)
	    menus->append(states);
	Target* viewer = 
	    new Target(new Frame(kit->Interior()), TargetPrimitiveHit);
	Catalog* catalog = unidraw->GetCatalog();
	if (const char* toolbarloca = catalog->GetAttribute("toolbarloc")) {
	  if (strcmp(toolbarloca, "r") == 0) 
	    toolbar->prepend(layout.vcenter(viewer));
	  else /* if (strcmp(toolbarloca, "l") == 0) */
	    toolbar->append(layout.vcenter(viewer));
	} else 
	  toolbar->append(layout.vcenter(viewer));
	menus->append(toolbar);

	
	Style* style = Session::instance()->style();
	boolean bookgeom = style->value_is_on("bookgeom");
	
	PolyGlyph* topbox = layout.vbox();
	ed->body(menus);
	ed->GetKeyMap()->Execute(CODE_SELECT);
	topbox->append(ed);
	if (!bookgeom) {
	    EivTextEditor* texteditor = new EivTextEditor(wk.style());
	    ((FrameEditor*)ed)->_texteditor = texteditor;
	    Button* set = wk.push_button("Set", new ActionCallback(FrameEditor)(
		(FrameEditor*)ed, &FrameEditor::SetText
	    ));
	    Button* clear = wk.push_button("Clear", new ActionCallback(FrameEditor)(
		(FrameEditor*)ed, &FrameEditor::ClearText
	    ));
	    topbox->append(
		wk.outset_frame(
		    layout.hbox(
			layout.vcenter(
			    layout.margin(
				layout.vbox(
				    layout.hcenter(set),
				    layout.vspace(10),
				    layout.hcenter(clear)
				),
				10
			    )
			),
			layout.vcenter(texteditor)
		    )
		)
	    );
	    topbox->append(
		wk.outset_frame(
		    layout.hbox(
			layout.vcenter(mousedoc_observer)
                    )
                )
            );
	}

	ManagedWindow* w = new ApplicationWindow(topbox);
	ed->SetWindow(w);
	Style* s = new Style(Session::instance()->style());
	s->alias(name);
	w->style(s);
    }
}

MenuItem * FrameKit::MakeFileMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("File"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new OvAboutCmd(new ControlInfo("About flipbook", "", "")),
	     "About flipbook   ");
    MakeMenu(mbi, new OvNewCompCmd(new ControlInfo("New", KLBL_NEWCOMP, CODE_NEWCOMP),
				 new FrameIdrawComp),
	     "New   ");
    MakeMenu(mbi, new OvRevertCmd(new ControlInfo("Revert", KLBL_REVERT, CODE_REVERT)),
	     "Revert   ");
    MakeMenu(mbi, new OvOpenCmd(new ControlInfo("Open...", KLBL_VIEWCOMP, CODE_VIEWCOMP)),
	     "Open...   ");
    MakeMenu(mbi, new OvSaveCompCmd(new ControlInfo("Save", KLBL_SAVECOMP, CODE_SAVECOMP),
				    new SaveAsChooser(".", &kit, kit.style())),
	     "Save   ");
    MakeMenu(mbi, new OvSaveCompAsCmd(new ControlInfo("Save As...",
						      KLBL_SAVECOMPAS,
						      CODE_SAVECOMPAS),
				      new SaveAsChooser(".", &kit, kit.style())),
	     "Save As...   ");
    MakeMenu(mbi, new OvPrintCmd(new ControlInfo("Print...", KLBL_PRINT, CODE_PRINT)),
	     "Print...   ");
    MakeMenu(mbi, new OvImportCmd(new ControlInfo("Import Graphic...",
						  KLBL_IMPORT,
						  CODE_IMPORT)),
	     "Import Graphic...   ");
    MakeMenu(mbi, new OvExportCmd(new ControlInfo("Export Graphic...",
						  "^X", "\030")),
	     "Export Graphic...   ");
    MakeMenu(mbi, new OvWindowDumpAsCmd(new ControlInfo("Dump Window As..."
						  )),
	     "Dump Window As...   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvQuitCmd(new ControlInfo("Quit", KLBL_QUIT, CODE_QUIT)),
	     "Quit   ");
    return mbi;
}

MenuItem* FrameKit::MakeFrameMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Frame"));
    mbi->menu(kit.pulldown());

    MoveFrameCmd::default_instance
      (new MoveFrameCmd(new ControlInfo("Move Forward","^F",""), +1));
    MakeMenu(mbi, MoveFrameCmd::default_instance(),
	     "Move Forward   ");

    MakeMenu(mbi, new MoveFrameCmd(new ControlInfo("Move Backward","^B",""), -1),
	     "Move Backward   ");
    MakeMenu(mbi, new FrameBeginCmd(new ControlInfo("Goto First Frame")),
	     "Goto First Frame");
    MakeMenu(mbi, new FrameEndCmd(new ControlInfo("Goto Last Frame")),
	     "Goto Last Frame ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new CreateMoveFrameCmd(new ControlInfo("New Forward","F","F")),
	     "New Forward    ");
    MakeMenu(mbi, new CreateMoveFrameCmd(new ControlInfo("New Backward","B","B"), false),
	     "New Backward   ");
    MakeMenu(mbi, new CopyMoveFrameCmd(new ControlInfo("Copy Forward","X","X")),
	     "Copy Forward   ");
    MakeMenu(mbi, new CopyMoveFrameCmd(new ControlInfo("Copy Backward","Y","Y"), false),
	     "Copy Backward  ");
    MakeMenu(mbi, new DeleteFrameCmd(new ControlInfo("Delete","D","D")),
	     "Delete  ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new ShowOtherFrameCmd(new ControlInfo("Show Prev Frame","",""), -1),
	     "Show Prev Frame");
    MakeMenu(mbi, new ShowOtherFrameCmd(new ControlInfo("Hide Prev Frame","",""), 0),
	     "Hide Prev Frame");

    MenuItem* menu_item;
    menu_item = kit.menu_item(kit.label("Enable Looping"));
    menu_item->action
      (new ActionCallback(MoveFrameCmd)
       (MoveFrameCmd::default_instance(), &MoveFrameCmd::set_wraparound));
    mbi->menu()->append_item(menu_item);

    menu_item = kit.menu_item(kit.label("Disable Looping"));
    menu_item->action
      (new ActionCallback(MoveFrameCmd)
       (MoveFrameCmd::default_instance(), &MoveFrameCmd::clr_wraparound));
    mbi->menu()->append_item(menu_item);

#if 0
    MakeMenu(mbi, new AutoNewFrameCmd(new ControlInfo("Toggle Auto New Frame",
						      "","")),
	     "Toggle Auto New Frame");
#else
    menu_item = kit.check_menu_item(kit.label("Auto New Frame"));
    menu_item->state()->set(TelltaleState::is_chosen, ((FrameEditor*)GetEditor())->AutoNewFrame());
    AutoNewFrameCmd::default_instance(new AutoNewFrameCmd(GetEditor()));
    menu_item->action
      (new ActionCallback(AutoNewFrameCmd)
       (AutoNewFrameCmd::default_instance(), &AutoNewFrameCmd::Execute));
    mbi->menu()->append_item(menu_item);
#endif
    return mbi;
}

MenuItem* FrameKit::MakeStructureMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Structure"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new FrameGroupCmd(new ControlInfo("Group", KLBL_GROUP, CODE_GROUP)),
	     "Group   ");
    MakeMenu(mbi, new FrameUngroupCmd(new ControlInfo("Ungroup", KLBL_UNGROUP, CODE_UNGROUP)),
	     "Ungroup   ");
    MakeMenu(mbi, new FrameFrontCmd(new ControlInfo("Bring to Front",
				       KLBL_FRONT, CODE_FRONT)),
	     "Bring to Front   ");
    MakeMenu(mbi, new FrameBackCmd(new ControlInfo("Send to Back",
				      KLBL_BACK, CODE_BACK)),
	     "Send to Back   ");

    return mbi;
}

MenuItem* FrameKit::MakeEditMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Edit"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new UndoCmd(new ControlInfo("Undo", KLBL_UNDO, CODE_UNDO)),
	     "Undo   ");
    MakeMenu(mbi, new RedoCmd(new ControlInfo("Redo", KLBL_REDO, CODE_REDO)),
	     "Redo   ");
    MakeMenu(mbi, new CutCmd(new ControlInfo("Cut", KLBL_CUT, CODE_CUT)),
	     "Cut   ");
    MakeMenu(mbi, new FrameCopyCmd(new ControlInfo("Copy", KLBL_COPY, CODE_COPY)),
	     "Copy   ");
    MakeMenu(mbi, new PasteCmd(new ControlInfo("Paste", KLBL_PASTE, CODE_PASTE)),
	     "Paste   ");
    MakeMenu(mbi, new DupCmd(new ControlInfo("Duplicate", KLBL_DUP, CODE_DUP)),
	     "Duplicate   ");
    MakeMenu(mbi, new OvDeleteCmd(new ControlInfo("Delete", KLBL_DEL, CODE_DEL)),
	     "Delete   ");
    MakeMenu(mbi, new OvSlctAllCmd(new ControlInfo("Select All", KLBL_SLCTALL, CODE_SLCTALL)),
	     "Select All   ");
    MakeMenu(mbi, new SlctByAttrCmd(new ControlInfo("Select by Attribute", "$", "$")),
	     "Select by Attribute   ");
    MakeMenu(mbi, new SetAttrByExprCmd(new ControlInfo("Compute Attributes ", "#", "#")),
	     "Compute Attributes ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new ScaleCmd(new ControlInfo("Flip Horizontal",
				       KLBL_HFLIP, CODE_HFLIP),
		       -1.0, 1.0),
	     "Flip Horizontal   ");
    MakeMenu(mbi, new ScaleCmd(new ControlInfo("Flip Vertical",
				       KLBL_VFLIP, CODE_VFLIP),
		       1.0, -1.0),
	     "Flip Vertical   ");
    MakeMenu(mbi, new RotateCmd(new ControlInfo("90 Clockwise", KLBL_CW90, CODE_CW90),
			-90.0),
	     "90 Clockwise   ");
    MakeMenu(mbi, new RotateCmd(new ControlInfo("90 CounterCW", KLBL_CCW90, CODE_CCW90),
			90.0),
	     "90 CounterCW   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvPreciseMoveCmd(new ControlInfo("Precise Move",
					     KLBL_PMOVE, CODE_PMOVE)),
	     "Precise Move   ");
    MakeMenu(mbi, new OvPreciseScaleCmd(new ControlInfo("Precise Scale",
					      KLBL_PSCALE, CODE_PSCALE)),
	     "Precise Scale   ");
    MakeMenu(mbi, new OvPreciseRotateCmd(new ControlInfo("Precise Rotate",
					       KLBL_PROTATE, CODE_PROTATE)),
	     "Precise Rotate   ");

    return mbi;
}

// upgrade to copy of OverlayKit::MakeViewMenu 3/24/99
#if 0 

MenuItem* FrameKit::MakeViewMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem* mbi = kit.menubar_item(kit.label("View"));
    mbi->menu(kit.pulldown());

#if 0
    MakeMenu(mbi, new FrameNewViewCmd(new ControlInfo("New View", KLBL_NEWVIEW, CODE_NEWVIEW)),
	     "New View   ");
    MakeMenu(mbi, new OvCloseEditorCmd(new ControlInfo("Close View",
					     KLBL_CLOSEEDITOR,
					     CODE_CLOSEEDITOR)),
	     "Close View   ");
    mbi->menu()->append_item(kit.menu_item_separator());
#endif
    MakeMenu(mbi, new PageCmd(new ControlInfo("Page on/off",
					      "p", "p")),
	     "Page on/off   ");
    MakeMenu(mbi, new NormSizeCmd(new ControlInfo("Normal Size",
					  KLBL_NORMSIZE, CODE_NORMSIZE)),
	     "Normal Size   ");
    MakeMenu(mbi, new RedToFitCmd(new ControlInfo("Reduce to Fit",
					  KLBL_REDTOFIT, CODE_REDTOFIT)),
	     "Reduce to Fit   ");
    MakeMenu(mbi, new CenterCmd(new ControlInfo("Center Page",
					KLBL_CENTER, CODE_CENTER)),
	     "Center Page   ");
    MakeMenu(mbi, new OrientationCmd(new ControlInfo("Orientation",
					     KLBL_ORIENTATION,
					     CODE_ORIENTATION)),
	     "Orientation   ");

    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new GridCmd(new ControlInfo("Grid on/off",
				      KLBL_GRID, CODE_GRID)),
	     "Grid on/off   ");
    MakeMenu(mbi, new GridSpacingCmd(new ControlInfo("Grid Spacing...",
						     KLBL_GRIDSPC, CODE_GRIDSPC)),
	     "Grid Spacing...   ");
    MakeMenu(mbi, new GravityCmd(new ControlInfo("Gravity on/off",
					 KLBL_GRAVITY, CODE_GRAVITY)),
	     "Gravity on/off   ");
    mbi->menu()->append_item(kit.menu_item_separator());

    MenuItem* zoomi = kit.menu_item(kit.label("Zoom             "));
    Menu* zoom = kit.pullright();
    zoomi->menu(zoom);
    MakeMenu(zoomi, new ZoomCmd(new ControlInfo("Zoom In", "Z", "Z"), 2.0),
	     "Zoom In          ");
    MakeMenu(zoomi, new ZoomCmd(new ControlInfo("Zoom Out", "^Z", "\032"), 0.5),
	     "Zoom Out         ");
    MakeMenu(zoomi, new PreciseZoomCmd(new ControlInfo("Precise Zoom")),
	     "Precise Zoom     ");
    mbi->menu()->append_item(zoomi);

    MenuItem* spani = kit.menu_item(kit.label("Small Pan        "));
    Menu* span = kit.pullright();
    spani->menu(span);
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Up"), NO_PAN, PLUS_SMALL_PAN),
	     "Small Pan Up     ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Down"), NO_PAN, MINUS_SMALL_PAN),
	     "Small Pan Down   ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Left"), MINUS_SMALL_PAN, NO_PAN),
	     "Small Pan Left   ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Right"), PLUS_SMALL_PAN, NO_PAN),
	     "Small Pan Right  ");
    mbi->menu()->append_item(spani);

    MenuItem* lpani = kit.menu_item(kit.label("Large Pan        "));
    Menu* lpan = kit.pullright();
    lpani->menu(lpan);
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Up"), NO_PAN, PLUS_LARGE_PAN),
	     "Large Pan Up     ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Down"), NO_PAN, MINUS_LARGE_PAN),
	     "Large Pan Down   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Left"), MINUS_LARGE_PAN, NO_PAN),
	     "Large Pan Left   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Right"), PLUS_LARGE_PAN, NO_PAN),
	     "Large Pan Right  ");
    mbi->menu()->append_item(lpani);

    MakeMenu(mbi, new PrecisePanCmd(new ControlInfo("Precise Pan")),
	     "Precise Pan      ");

    return mbi;
}

#else

MenuItem* FrameKit::MakeViewMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem* mbi = kit.menubar_item(kit.label("View"));
    mbi->menu(kit.pulldown());

#if 0
    OvNewViewCmd::default_instance
      (new OvNewViewCmd
       (new ControlInfo("New View", KLBL_NEWVIEW, CODE_NEWVIEW), 
       "localhost:0.0"));
    MakeMenu(mbi, OvNewViewCmd::default_instance(), "New View   ");

    MakeMenu(mbi, new OvCloseEditorCmd(new ControlInfo("Close View",
					     KLBL_CLOSEEDITOR,
					     CODE_CLOSEEDITOR)),
	     "Close View   ");
#endif
#if 0
    MenuItem* menu_item;
    menu_item = kit.menu_item(kit.label("New Display"));
    menu_item->action
      (new ActionCallback(OvNewViewCmd)
       (OvNewViewCmd::default_instance(), &OvNewViewCmd::set_display));
    mbi->menu()->append_item(menu_item);
    mbi->menu()->append_item(kit.menu_item_separator());
#endif

    MakeMenu(mbi, new PageCmd(new ControlInfo("Page on/off",
					      "p", "p")),
	     "Page on/off   ");
    MakeMenu(mbi, new OvPrecisePageCmd(new ControlInfo("Precise Page",
					     "^P", "\020")),
	     "Precise Page   ");
    MakeMenu(mbi, new NormSizeCmd(new ControlInfo("Normal Size",
					  KLBL_NORMSIZE, CODE_NORMSIZE)),
	     "Normal Size   ");
    MakeMenu(mbi, new RedToFitCmd(new ControlInfo("Reduce to Fit",
					  KLBL_REDTOFIT, CODE_REDTOFIT)),
	     "Reduce to Fit   ");
    MakeMenu(mbi, new CenterCmd(new ControlInfo("Center Page",
					KLBL_CENTER, CODE_CENTER)),
	     "Center Page   ");
    MakeMenu(mbi, new OrientationCmd(new ControlInfo("Orientation",
					     KLBL_ORIENTATION,
					     CODE_ORIENTATION)),
	     "Orientation   ");

    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new GridCmd(new ControlInfo("Grid on/off",
				      KLBL_GRID, CODE_GRID)),
	     "Grid on/off   ");
    MakeMenu(mbi, new GridSpacingCmd(new ControlInfo("Grid Spacing...",
						     KLBL_GRIDSPC, CODE_GRIDSPC)),
	     "Grid Spacing...   ");
    MakeMenu(mbi, new GravityCmd(new ControlInfo("Gravity on/off",
					 KLBL_GRAVITY, CODE_GRAVITY)),
	     "Gravity on/off   ");
    MakeMenu(mbi, new ScribblePointerCmd(new ControlInfo("Scribble pointer on/off",
					 "", "")),
	     "Scribble pointer on/off   ");
    mbi->menu()->append_item(kit.menu_item_separator());

    MenuItem* zoomi = kit.menu_item(kit.label("Zoom             "));
    Menu* zoom = kit.pullright();
    zoomi->menu(zoom);
    MakeMenu(zoomi, new ZoomCmd(new ControlInfo("Zoom In", "Z", "Z"), 2.0),
	     "Zoom In          ");
    MakeMenu(zoomi, new ZoomCmd(new ControlInfo("Zoom Out", "^Z", "\032"), 0.5),
	     "Zoom Out         ");
    MakeMenu(zoomi, new PreciseZoomCmd(new ControlInfo("Precise Zoom")),
	     "Precise Zoom     ");
    mbi->menu()->append_item(zoomi);

    MenuItem* spani = kit.menu_item(kit.label("Small Pan        "));
    Menu* span = kit.pullright();
    spani->menu(span);
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Up"), NO_PAN, PLUS_SMALL_PAN),
	     "Small Pan Up     ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Down"), NO_PAN, MINUS_SMALL_PAN),
	     "Small Pan Down   ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Left"), MINUS_SMALL_PAN, NO_PAN),
	     "Small Pan Left   ");
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Right"), PLUS_SMALL_PAN, NO_PAN),
	     "Small Pan Right  ");
    mbi->menu()->append_item(spani);

    MenuItem* lpani = kit.menu_item(kit.label("Large Pan        "));
    Menu* lpan = kit.pullright();
    lpani->menu(lpan);
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Up"), NO_PAN, PLUS_LARGE_PAN),
	     "Large Pan Up     ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Down"), NO_PAN, MINUS_LARGE_PAN),
	     "Large Pan Down   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Left"), MINUS_LARGE_PAN, NO_PAN),
	     "Large Pan Left   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Right"), PLUS_LARGE_PAN, NO_PAN),
	     "Large Pan Right  ");
    mbi->menu()->append_item(lpani);

    MakeMenu(mbi, new PrecisePanCmd(new ControlInfo("Precise Pan")),
	     "Precise Pan      ");

    mbi->menu()->append_item(kit.menu_item_separator());

#if 0
    MenuItem* grmenu = kit.menu_item(kit.label("Hide/Show Graphics   "));
    grmenu->menu(kit.pullright());
    mbi->menu()->append_item(grmenu);
    
    MakeMenu(grmenu, new HideViewCmd(_ed->GetViewer(), new ControlInfo("Hide Graphic Here", "H", "H")),
	     "Hide Graphic Here");
    MakeMenu(grmenu, new UnhideViewsCmd(new ControlInfo("Unhide Graphics There", "^H", "\010")),
	     "Unhide Graphics There");
    MakeMenu(grmenu, new DesensitizeViewCmd(_ed->GetViewer(), new ControlInfo("Desensitize Graphic Here", "", "")),
	     "Desensitize Graphic Here");
    MakeMenu(grmenu, new SensitizeViewsCmd(new ControlInfo("Sensitize Graphics There", "", "")),
	     "Sensitize Graphics There");
#endif

    MenuItem* fixmenu = kit.menu_item(kit.label("Fix/Unfix Graphics   "));
    fixmenu->menu(kit.pullright());
    mbi->menu()->append_item(fixmenu);
    
    MakeMenu(fixmenu, new FixViewCmd(new ControlInfo("Fix Size", " ", " "), true, false),
	     "Fix Size");
    MakeMenu(fixmenu, new UnfixViewCmd(new ControlInfo("Unfix Size", " ", " "), true, false),
	     "Unfix Size");
    MakeMenu(fixmenu, new FixViewCmd(new ControlInfo("Fix Location", " ", " "), false, true),
	     "Fix Location");
    MakeMenu(fixmenu, new UnfixViewCmd(new ControlInfo("Unfix Location", " ", " "), false, true),
	     "Unfix Location");

#if 0
    MenuItem* chainmenu = kit.menu_item(kit.label("Chain/Unchain Viewers   "));
    chainmenu->menu(kit.pullright());
    mbi->menu()->append_item(chainmenu);
    
    MakeMenu(chainmenu, new ChainViewersCmd(_ed->GetViewer(), new ControlInfo("Chain Panning", " ", " "), true, false),
	     "Chain Panning");
    MakeMenu(chainmenu, new UnchainViewersCmd(_ed->GetViewer(), new ControlInfo("Unchain Panning", " ", " "), true, false),
	     "Unchain Panning");
    MakeMenu(chainmenu, new ChainViewersCmd(_ed->GetViewer(), new ControlInfo("Chain Zooming", " ", " "), false, true),
	     "Chain Zooming");
    MakeMenu(chainmenu, new UnchainViewersCmd(_ed->GetViewer(), new ControlInfo("Unchain Zooming", " ", " "), false, true),
	     "Unchain Zooming");
#endif
    return mbi;
}

#endif


Glyph* FrameKit::MakeStates() {
    ((FrameEditor*)_ed)->_framenumstate = new FrameNumberState();
    ((FrameEditor*)_ed)->_frameliststate = new FrameListState();
    NameView* fnumview = new NameView(((FrameEditor*)_ed)->framenumstate());
    NameView* flistview = new NameView(((FrameEditor*)_ed)->frameliststate());

    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    return kit.inset_frame(
	lk.margin(
	    lk.hbox(fnumview, lk.hspace(40), flistview, lk.hglue()),
	    4, 2
	    )
    );
		
}
