/*
 * Copyright (c) 2002 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1998-2000 Vectaport Inc.
 * Copyright (c) 1994-1995 Vectaport Inc., Cartoactive Systems
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
 * OverlayKit definitions
 */

#include <OverlayUnidraw/annotate.h>
#include <OverlayUnidraw/attrtool.h>
#include <OverlayUnidraw/grloctool.h>
#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovchainview.h>
#include <OverlayUnidraw/ovclip.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovctrl.h>
#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovfixview.h>
#include <OverlayUnidraw/ovgdialog.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovhull.h>
#include <OverlayUnidraw/ovipcmds.h>
#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovshowhide.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovstates.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/rastercmds.h>
#include <OverlayUnidraw/setattrbyexpr.h>
#include <OverlayUnidraw/slctbyattr.h>

#include <ComGlyph/attrdialog.h>
#include <ComGlyph/comtextedit.h>
#include <IVGlyph/exportchooser.h>
#include <IVGlyph/idraw.h>
#include <IVGlyph/saveaschooser.h>

#include <UniIdraw/idcatalog.h>

#include <Unidraw/ctrlinfo.h>
#include <Unidraw/editor.h>
#include <Unidraw/grid.h>
#include <Unidraw/iterator.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/statevars.h>
#include <Unidraw/uctrls.h>
#include <Unidraw/viewer.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/brushcmd.h>
#include <Unidraw/Commands/catcmds.h>
#include <Unidraw/Commands/colorcmd.h>
#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/font.h>
#include <Unidraw/Commands/patcmd.h>
#include <Unidraw/Commands/struct.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Commands/viewcmds.h>

#include <Unidraw/Components/text.h>

#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/polygons.h>

#include <Unidraw/Tools/connect.h>
#include <Unidraw/Tools/grcomptool.h>
#include <Unidraw/Tools/magnify.h>
#include <Unidraw/Tools/move.h>
#include <Unidraw/Tools/reshape.h>
#include <Unidraw/Tools/rotate.h>
#include <Unidraw/Tools/scale.h>
#include <Unidraw/Tools/select.h>
#include <Unidraw/Tools/stretch.h>

#define iv2_6_compatible 1
#include <UniIdraw/idcomp.h>
#include <UniIdraw/idcmds.h>
#include <UniIdraw/idkybd.h>
#include <UniIdraw/idarrow.h>
#include <UniIdraw/idarrows.h>
#undef iv2_6_compatible

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/deck.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <InterViews/frame.h>
#include <InterViews/label.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/tray.h>
#include <InterViews/window.h>

#include <IV-look/kit.h>

#include <IV-X11/xdisplay.h>
#include <IV-X11/xcolor.h>
#include <IV-X11/xpattern.h>
#undef None
#include <OS/math.h>

#include <IVGlyph/idraw.h>
#include <IVGlyph/figure.h>
#include <IVGlyph/textform.h>
#include <IVGlyph/toolbutton.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>

/******************************************************************************/

implementActionCallback(Command)
implementActionCallback(CommandDoer)
implementActionCallback(CommandPusher)
implementActionCallback(ToolSelector)
implementActionCallback(OverlayKit)

static const char* brAttrib = "brush";
static const char* fontAttrib = "font";
static const char* patAttrib = "pattern";
static const char* fgAttrib = "fgcolor";
static const char* bgAttrib = "bgcolor";

static const char* page_width_attrib = "pagewidth";
static const char* page_height_attrib = "pageheight";
static const char* page_cols_attrib = "pagecols";
static const char* page_rows_attrib = "pagerows";
static const char* grid_x_incr = "gridxincr";
static const char* grid_y_incr = "gridyincr";
static const char* scribble_pointer_attrib = "scribble_pointer";

static const int unit = 15;

static int xClosed[] = { unit/5, unit, unit, unit*3/5, 0 };
static int yClosed[] = { 0, unit/5, unit*3/5, unit, unit*2/5 };
static Coord fxClosed[] = { unit/5, unit, unit, unit*3/5, 0 };
static Coord fyClosed[] = { 0, unit/5, unit*3/5, unit, unit*2/5 };
static const int nClosed = 5;

static int xOpen[] = { 0, unit/2, unit/2, unit };
static int yOpen[] = { 0, unit/4, unit*3/4, unit };
static Coord fxOpen[] = { 0, unit/2, unit/2, unit };
static Coord fyOpen[] = { 0, unit/4, unit*3/4, unit };
static const int nOpen = 4;

const char* OverlayKit::mouse_sel  = "l-click/drag: Select; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_mov  = "l-drag: Move; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_scl  = "l-drag: Scale; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_str  = "l-drag: Stretch; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_rot  = "l-drag: Rotate; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_alt  = "l-click: Alter; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_mag  = "l-drag: Magnify; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_txt  = "l-click: Text; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_lin  = "l-drag: Line; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_mlin = "l-click: Start Multi-Line; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_ospl = "l-click: Start Open Spline; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_rect = "l-drag: Rectangle; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_ellp = "l-drag: Ellipse; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_poly = "l-click: Start Polygon; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_cspl = "l-drag: Start Closed Spline; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_anno = "l-click: Annotate; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_attr = "l-click: Edit Attributes; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_tack = "l-click: Tack Down; m-click: Tack and Finish; r-click: Remove Last Tack";
#ifdef CLIPPOLY
const char* OverlayKit::mouse_clipr = "l-drag: Clip MultiLines in Box; m-drag: Move; r-click/drag: Select";
#else
const char* OverlayKit::mouse_clipr = "l-drag: Clip MultiLines and Polygons in Box; m-drag: Move; r-click/drag: Select";
#endif    
#ifdef CLIPPOLY
const char* OverlayKit::mouse_clipp = "l-drag: Clip MultiLines in Polygon; m-drag: Move; r-click/drag: Select";
#else
const char* OverlayKit::mouse_clipp = "l-drag: Clip MultiLines and Polygons in Polygon; m-drag: Move; r-click/drag: Select";
#endif    
const char* OverlayKit::mouse_convexhull = "l-click: Start Convex Hull; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_imgscale = "l-drag: Scale Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_logscale = "l-drag: Logarithmically Scale Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_pseudocolor = "l-drag: Pseudocolor Image between Pixel Values on Line; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_grloc = "l-click: Location within Graphic; m-drag: Move; r-click/drag: Select";
const char* OverlayKit::mouse_custom = "l-click: Drop icon; m-drag: Move; r-click/drag: Select";

/*****************************************************************************/

OverlayKit* OverlayKit::_overlaykit = nil;

OverlayKit::OverlayKit () {
    WidgetKit& kit = *WidgetKit::instance();
    _ed = nil;
    _otherdisplay = nil;
    _set_button_flag = false;
    _clr_button_flag = false;
    _tg = nil;
    _toolbar_vbox = nil;
    _appname = nil;
}

OverlayKit::~OverlayKit() {
  delete _otherdisplay;
}

void OverlayKit::SetEditor(OverlayEditor* editor) {
    _ed = editor;
}

OverlayEditor* OverlayKit::GetEditor() {
    return _ed;
}

Interactor* OverlayKit::Interior() {
    return _ed->Interior();
}

void OverlayKit::Init (OverlayComp* comp, const char* name) {
    InitMembers(comp);
    _ed->InitStateVars();
    InitViewer();
    InitLayout(name);
}

void OverlayKit::InitMembers (OverlayComp* comp) {
    _ed->_comp = comp;
    _ed->_keymap = new KeyMap;
    _ed->_curCtrl = new ControlState;
    _ed->_selection = MakeSelection();
    _ed->_tray = new Tray;
    _ed->_tray->Propagate(false);
}

void OverlayKit::InitViewer () {
    Catalog* catalog = unidraw->GetCatalog();

    const char* page_w = catalog->GetAttribute(page_width_attrib);
    const char* page_h = catalog->GetAttribute(page_height_attrib);
    const char* page_cols = catalog->GetAttribute(page_cols_attrib);
    const char* page_rows = catalog->GetAttribute(page_rows_attrib);
    const char* x_incr = catalog->GetAttribute(grid_x_incr);
    const char* y_incr = catalog->GetAttribute(grid_y_incr);
    const char* scribble_pointer = catalog->GetAttribute(scribble_pointer_attrib);

    GraphicView* view = (GraphicView*)_ed->_comp->Create(COMPONENT_VIEW);
    _ed->_comp->Attach(view);
    view->Update();

    /*
     * These statements had to be moved down here to workaround
     * a strange cfront 3.0 bug.
     */
    float w = Math::round(atof(page_w) * ivinches);
    float h = Math::round(atof(page_h) * ivinches);
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

    _ed->_viewer = new OverlayViewer(_ed, view, page, grid);
    if (scribble_pointer) 
        ((OverlayViewer*)_ed->_viewer)->scribble_pointer(strcmp(scribble_pointer, "true")==0);
}

void OverlayKit::InitLayout(const char* name) {
  InitLayout(this, name);
}

void OverlayKit::InitLayout(OverlayKit* kit, const char* name) {
    kit->_appname = name; 
    OverlayEditor* ed = kit->GetEditor();
    Catalog* catalog = unidraw->GetCatalog();
    const char* stripped_string = catalog->GetAttribute("stripped");
    boolean stripped_flag = stripped_string ? strcmp(stripped_string, "true")==0 : false;
    if (ed->GetWindow() == nil) {

      TextObserver* mousedoc_observer = new TextObserver(ed->MouseDocObservable(), "");
      const LayoutKit& lk = *LayoutKit::instance();
      WidgetKit& wk = *WidgetKit::instance();
      PolyGlyph* topbox = lk.vbox();

      Glyph* menus = kit->MakeMenus();
      Glyph* states = kit->MakeStates();
      Glyph* toolbar = kit->MakeToolbar();

      if (stripped_flag) {

	Target* viewer = 
	    new Target(new Frame(ed->Interior()), TargetPrimitiveHit);
	ed->body(viewer);
	topbox->append(ed);

      } else {

	if (states)
	    menus->append(states);
	Target* viewer = 
	    new Target(new Frame(ed->Interior()), TargetPrimitiveHit);
	if (const char* toolbarloca = catalog->GetAttribute("toolbarloc")) {
	  if (strcmp(toolbarloca, "r") == 0) 
	    toolbar->prepend(lk.vcenter(viewer));
	  else /* if (strcmp(toolbarloca, "l") == 0) */
	    toolbar->append(lk.vcenter(viewer));
	} else 
	  toolbar->append(lk.vcenter(viewer));
	menus->append(toolbar);

	ed->body(menus);
	topbox->append(ed);
	topbox->append(
		wk.outset_frame(
		    lk.hbox(
			lk.vcenter(mousedoc_observer)
		    )
		)
	    );
      }


      ed->GetKeyMap()->Execute(CODE_SELECT);
	
      if (ed->comterp()) {
	    boolean set_flag = kit->_set_button_flag;
	    boolean clr_flag = kit->_clr_button_flag;
	    EivTextEditor* texteditor = nil;
	    if(!set_flag && !clr_flag) {
	      texteditor = new ComTextEditor(wk.style(), ed->comterp());
	    }
	    else
	      texteditor = new EivTextEditor(wk.style());
	    ed->_texteditor = texteditor;
	    Button* set = set_flag ? wk.push_button("Set", new ActionCallback(OverlayEditor)(
		(OverlayEditor*)ed, &OverlayEditor::SetText
	    )) : nil;
	    Button* clear = clr_flag ? wk.push_button("Clear", new ActionCallback(OverlayEditor)(
		(OverlayEditor*)ed, &OverlayEditor::ClearText
	    )) : nil;
	    Glyph* buttonbox = nil;

	    if (set && !clear) {
	      buttonbox = 
		lk.vbox(
			    lk.hcenter(set));
	    } else if (!set && clear) { 
	      buttonbox = 
		lk.vbox(
			    lk.hcenter(clear));
	    } else if (set && clear) {
	      buttonbox = 
		lk.vbox(
			    lk.hcenter(set),
			    lk.vspace(10),
			    lk.hcenter(clear)
			    );
	    }
	    if (buttonbox) {
	      topbox->append(
		  wk.outset_frame(
		      lk.hbox(
			  lk.vcenter(
			      lk.margin(
                                  buttonbox,
				  10
			      )
			  ),
			  lk.vcenter(texteditor)
		      )
		  )
	      );
	    } else {
	      topbox->append(
		  wk.outset_frame(
		      lk.hbox(
			  lk.vcenter(
			      lk.margin(
				  lk.vbox(
#if 0
 			              wk.label("type help"),
			              lk.vspace(10),
			              wk.label("to print"),
			              lk.vspace(10),
			              wk.label("info to stdout")
#else
				      kit->appicon()
#endif
			              ),
				  10
			      )
			  ),
			  lk.vcenter(texteditor)
		      )
		  )
	      );
	    }
	}

      ManagedWindow* w = new ApplicationWindow(topbox, kit->_otherdisplay);
      ed->SetWindow(w);
      Style* s = new Style(Session::instance()->style());
      s->alias(name);
      w->style(s);

    }
 }


Glyph* OverlayKit::appicon() {
  const LayoutKit& lk = *LayoutKit::instance();
  WidgetKit& wk = *WidgetKit::instance();
  return lk.vbox(lk.hcenter(wk.label("ivtools")), 
		 lk.hcenter(wk.label(this->appname())),
		 lk.vspace(20),
		 lk.hcenter(wk.label("type help for list of")),
		 lk.hcenter(wk.label("keyboard commands"))
		 );
}

Glyph* OverlayKit::MakeStates() {
  Catalog* catalog = unidraw->GetCatalog();
  const char* ptrlocstr = catalog->GetAttribute("ptrloc");
  if (ptrlocstr && strcmp(ptrlocstr, "true")==0) {
    if (Event::event_tracker() != OverlayUnidraw::pointer_tracker_func)
	Event::event_tracker(OverlayUnidraw::pointer_tracker_func);

    _ed->_ptrlocstate = new PtrLocState(0,0, _ed);
    NameView* ptrlocview = new NameView(_ed->ptrlocstate());

    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    return kit.inset_frame(
	lk.margin(
	    lk.hbox(lk.hglue(), ptrlocview),
	    4, 2
	    )
    );

  } else
    return nil;
}

ToolButton* OverlayKit::MakeTool(Tool* tool, Glyph* pict, TelltaleGroup* tg, 
    ObservableText* mousedoc, const char* doc) {

    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = kit.style();
    ControlInfo* info = tool->GetControlInfo();
    ToolSelector* sel = new ToolSelector(tool, _ed);
    ToolButton* button = new ToolButton(pict, info->GetKeyLabel(), s, tg,
	new ActionCallback(ToolSelector)(sel, &ToolSelector::Select), mousedoc, doc);
    _ed->GetKeyMap()->Register(new ToolControl(info, button));
    return button;
}

Glyph* OverlayKit::MakeToolbar() {
    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = kit.style();

    /* tools shared between pallettes */
    ToolButton* select;
    ToolButton* move;
    ToolButton* scale;
    ToolButton* stretch;
    ToolButton* rotate;
    ToolButton* reshape;
    ToolButton* magnify;

    _toolbars = layout.deck(1);


    PolyGlyph* vb = layout.vbox();
    _toolbar_vbox = new Glyph*[2];
    _toolbar_vbox[0] = vb;

    _tg = new TelltaleGroup();

    Glyph* sel = kit.label("Select");
    Glyph* mov = kit.label("Move");
    Glyph* scl = kit.label("Scale");
    Glyph* str = kit.label("Stretch");
    Glyph* rot = kit.label("Rotate");
    Glyph* alt = kit.label("Alter");
    Glyph* mag = kit.label("Magnify");
    Glyph* txt = kit.label("Text");
    Glyph* glin = new Fig31Line(new Brush(0), kit.foreground(), nil,
				0, 0, unit, unit);
    Glyph* gmlin = new Fig31Polyline(new Brush(0), kit.foreground(), nil,
				     fxOpen, fyOpen, nOpen);
    Glyph* gospl = new Fig31Open_BSpline(new Brush(0), kit.foreground(), nil,
					 fxOpen, fyOpen, nOpen);
    Glyph* grect = new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
				      0, 0, unit, unit*4/5);
    Glyph* gellp = new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
				    0, 0, unit*2/3, unit*2/5);
    Glyph* gpoly = new Fig31Polygon(new Brush(0), kit.foreground(), nil,
				    fxClosed, fyClosed, nClosed);
    Glyph* gcspl = new Fig31Closed_BSpline(new Brush(0), kit.foreground(), nil,
					   fxClosed, fyClosed, nClosed);
    Glyph* anno = kit.label("Annotate");
    Glyph* attr = kit.label("Attribute");
    Glyph* clipr = kit.label("ClipRect");
    Glyph* clipp = kit.label("ClipPoly");
    Glyph* hull = kit.label("ConvexHull");
    Glyph* grloc = kit.label("GraphicLoc");

    _maxwidth = 0;
    Requisition req;
    _maxwidth = Math::max((sel->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((mov->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((scl->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((str->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((rot->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((alt->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((mag->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((txt->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((glin->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((gmlin->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((gospl->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((grect->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((gellp->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((gpoly->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((gcspl->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((anno->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((attr->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((clipr->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((clipp->request(req), req.x_requirement().natural()),
			 _maxwidth);
    _maxwidth = Math::max((hull->request(req), req.x_requirement().natural()),
			 _maxwidth);

    _maxwidth = Math::max((grloc->request(req), req.x_requirement().natural()),
			 _maxwidth);

    vb->append(select = MakeTool(new SelectTool(new ControlInfo("Select", KLBL_SELECT, CODE_SELECT)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(sel)),
			_tg, _ed->MouseDocObservable(), mouse_sel));
    vb->append(move = MakeTool(new MoveTool(new ControlInfo("Move", KLBL_MOVE, CODE_MOVE)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(mov)),
			_tg, _ed->MouseDocObservable(), mouse_mov));
    vb->append(scale = MakeTool(new ScaleTool(new ControlInfo("Scale", KLBL_SCALE, CODE_SCALE)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(scl)), _tg, _ed->MouseDocObservable(), mouse_scl));
    vb->append(stretch = MakeTool(new StretchTool(new ControlInfo("Stretch", KLBL_STRETCH,CODE_STRETCH)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(str)), _tg, _ed->MouseDocObservable(), mouse_str));
    vb->append(rotate = MakeTool(new RotateTool(new ControlInfo("Rotate", KLBL_ROTATE, CODE_ROTATE)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(rot)), _tg, _ed->MouseDocObservable(), mouse_rot));
    vb->append(reshape = MakeTool(new ReshapeTool(new ControlInfo("Alter", KLBL_RESHAPE, CODE_RESHAPE)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(alt)), _tg, _ed->MouseDocObservable(), mouse_alt));
    vb->append(magnify = MakeTool(new MagnifyTool(new ControlInfo("Magnify", KLBL_MAGNIFY,CODE_MAGNIFY)),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(mag)), _tg, _ed->MouseDocObservable(), mouse_mag));
    TextGraphic* text = new TextGraphic("Text", stdgraphic);
    TextOvComp* textComp = new TextOvComp(text);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo("Text", KLBL_TEXT, CODE_TEXT), textComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(txt)), _tg, _ed->MouseDocObservable(), mouse_txt));
    ArrowLine* line = new ArrowLine(
	0, 0, unit, unit, false, false, 1., stdgraphic
    );
    ArrowLineOvComp* arrowLineComp = new ArrowLineOvComp(line);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(arrowLineComp, KLBL_LINE, CODE_LINE), arrowLineComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(glin)),
			_tg, _ed->MouseDocObservable(), mouse_lin));
    ArrowMultiLine* ml = new ArrowMultiLine(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    ml->SetPattern(psnonepat);
    ArrowMultiLineOvComp* mlComp = new ArrowMultiLineOvComp(ml);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(mlComp, KLBL_MULTILINE, CODE_MULTILINE), mlComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(gmlin)),
			_tg, _ed->MouseDocObservable(), mouse_mlin));
    ArrowOpenBSpline* spl = new ArrowOpenBSpline(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    spl->SetPattern(psnonepat);
    ArrowSplineOvComp* splComp = new ArrowSplineOvComp(spl);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(splComp, KLBL_SPLINE, CODE_SPLINE), splComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(gospl)),
			_tg, _ed->MouseDocObservable(), mouse_ospl));
    SF_Rect* rect = new SF_Rect(0, 0, unit, unit*4/5, stdgraphic);
    rect->SetPattern(psnonepat);
    RectOvComp* rectComp = new RectOvComp(rect);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(rectComp, KLBL_RECT, CODE_RECT), rectComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(grect)),
			_tg, _ed->MouseDocObservable(), mouse_rect));
    SF_Ellipse* ellipse = new SF_Ellipse(0, 0, unit*2/3, unit*2/5, stdgraphic);
    ellipse->SetPattern(psnonepat);
    EllipseOvComp* ellipseComp = new EllipseOvComp(ellipse);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(ellipseComp, KLBL_ELLIPSE, CODE_ELLIPSE), ellipseComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(gellp)),
			_tg, _ed->MouseDocObservable(), mouse_ellp));
    SF_Polygon* polygon = new SF_Polygon(xClosed, yClosed, nClosed,stdgraphic);
    polygon->SetPattern(psnonepat);
    PolygonOvComp* polygonComp = new PolygonOvComp(polygon);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(polygonComp, KLBL_POLY, CODE_POLY), polygonComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(gpoly)),
			_tg, _ed->MouseDocObservable(), mouse_poly));
    SFH_ClosedBSpline* cspline = new SFH_ClosedBSpline(
        xClosed, yClosed, nClosed, stdgraphic
    );
    cspline->SetPattern(psnonepat);
    ClosedSplineOvComp* csplineComp = new ClosedSplineOvComp(cspline);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(csplineComp, KLBL_CSPLINE,CODE_CSPLINE), csplineComp),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(gcspl)),
			_tg, _ed->MouseDocObservable(), mouse_cspl));

    _toolbars->append(vb);
    vb = layout.vbox();
    _toolbar_vbox[1] = vb;
    vb->append(select);
    vb->append(move);
    vb->append(scale);
    vb->append(rotate);
    vb->append(reshape);
    vb->append(magnify);
    vb->append(MakeTool(new AttributeTool(new ControlInfo("Attribute", "T", "T")),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(attr)), _tg, _ed->MouseDocObservable(), mouse_attr));
    vb->append(MakeTool(new AnnotateTool(new ControlInfo("Annotate", "A", "A")),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(anno)), _tg, _ed->MouseDocObservable(), mouse_anno));
    vb->append(MakeTool(new GrLocTool(new ControlInfo("GraphicLoc", "", "")),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(grloc)), _tg, _ed->MouseDocObservable(), mouse_grloc));
#ifdef CLIPPOLY
    vb->append(MakeTool(new ClipRectTool(new ControlInfo("ClipRect", "C", "C")),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(clipr)), _tg, _ed->MouseDocObservable(), mouse_clipr));
    vb->append(MakeTool(new ClipPolyTool(new ControlInfo("ClipPoly")),
			layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
				       layout.hcenter(clipp)), _tg, _ed->MouseDocObservable(), mouse_clipp));
#endif
    if (!bintest("qhull"))
      vb->append(MakeTool(new ConvexHullTool(new ControlInfo("ConvexHull")),
			  layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
					 layout.hcenter(hull)), _tg, _ed->MouseDocObservable(), mouse_convexhull));
    _toolbars->append(vb);
    _toolbars->flip_to(0);
    _toolbar = new Patch(_toolbars);

    return layout.hbox(
	layout.vflexible(
		new Background(
		    layout.vcenter(
			_toolbar
		    ),
		    unidraw->GetCatalog()->FindColor("#aaaaaa")
		),
		fil, 0.0
	)
    );
}

Glyph* OverlayKit::MakeMenus() {
    Menu* menubar_ = WidgetKit::instance()->menubar();
    
    MenuItem* m;
    m = MakeFileMenu(); 
    if (m) menubar_->append_item(m);
    m = MakeEditMenu();
    if (m) menubar_->append_item(m);
    m = MakeStructureMenu();
    if (m) menubar_->append_item(m);
    m = MakeFontMenu();
    if (m) menubar_->append_item(m);
    m = MakeBrushMenu();
    if (m) menubar_->append_item(m);
    m = MakePatternMenu();
    if (m) menubar_->append_item(m);
    m = MakeFgColorMenu();
    if (m) menubar_->append_item(m);
    m = MakeBgColorMenu();
    if (m) menubar_->append_item(m);
    m = MakeAlignMenu();
    if (m) menubar_->append_item(m);
    m = MakeFrameMenu();
    if (m) menubar_->append_item(m);
    m = MakeViewMenu();
    if (m) menubar_->append_item(m);
    m = MakeToolsMenu();
    if (m) menubar_->append_item(m);
    m = MakeViewersMenu();
    if (m) menubar_->append_item(m);
    Resource::ref(menubar_);
    return LayoutKit::instance()->vbox(menubar_);
}

void OverlayKit::MouseDoc(const char* doc) {
    _ed->MouseDocObservable()->textvalue(doc);
}

static float MENU_WIDTH = 1.3;   /* in cm */
static float MENU_HEIGHT = 0.5;

Glyph* OverlayKit::MenuLine(PSBrush* br) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    if (br->None())
	return lk.hbox(lk.hglue(), wk.label("None"), lk.hglue());
    else
	return lk.margin(new Fig31Line(br, wk.foreground(), nil,
				       0, 0, MENU_WIDTH*2*ivcm, 0),
			 0.1*MENU_WIDTH*ivcm, 0.4*MENU_HEIGHT*ivcm);
}

Glyph* OverlayKit::MenuArrowLine(boolean tail, boolean head) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    Brush * br    = new Brush(0);
    Coord * x = new Coord[6];
    Coord * y = new Coord[6];
    
    x[0] = 0.1*MENU_WIDTH*2*ivcm;
    y[0] = 0.05*MENU_WIDTH*2*ivcm;
    x[1] = 0.0;
    y[1] = 0.0;
    x[2] = 0.1*MENU_WIDTH*2*ivcm;
    y[2] = -0.05*MENU_WIDTH*2*ivcm;
    
    x[3] = 0.9*MENU_WIDTH*2*ivcm;
    y[3] = 0.05*MENU_WIDTH*2*ivcm;
    x[4] = MENU_WIDTH*2*ivcm;
    y[4] = 0.0;
    x[5] = 0.9*MENU_WIDTH*2*ivcm;
    y[5] = -0.05*MENU_WIDTH*2*ivcm;
    
    Fig31Line* liner = new Fig31Line(br, wk.foreground(), nil,
				     0, 0, MENU_WIDTH*2*ivcm, 0);
    Fig31Polyline* tailer = new Fig31Polyline(br, wk.foreground(), nil,
					      x, y, 3);
    Fig31Polyline* header = new Fig31Polyline(br, wk.foreground(), nil,
					      x+3, y+3, 3);
    
    if (tail == true) {
	if (head == true) {
	    return lk.fixed(lk.vbox(lk.vglue(),
				    lk.hbox(lk.hglue(),
					    lk.overlay(tailer, liner, header),
					    lk.hglue()),
				    lk.vglue()),
			    1.2*MENU_WIDTH*2*ivcm, 0.9*MENU_HEIGHT*ivcm);
	} else {
	    return lk.fixed(lk.vbox(lk.vglue(),
				    lk.hbox(lk.hglue(),
					    lk.overlay(tailer, liner),
					    lk.hglue()),
				    lk.vglue()),
			    1.2*MENU_WIDTH*2*ivcm, 0.9*MENU_HEIGHT*ivcm);
	}
    } else {
	if (head == true) {
	    return lk.fixed(lk.vbox(lk.vglue(),
				    lk.hbox(lk.hglue(),
					    lk.overlay(liner, header),
					    lk.hglue()),
				    lk.vglue()),
			    1.2*MENU_WIDTH*2*ivcm, 0.9*MENU_HEIGHT*ivcm);
	} else {
	    return lk.fixed(lk.vbox(lk.vglue(),
				    lk.hbox(lk.hglue(),
					    lk.overlay(liner),
					    lk.hglue()),
				    lk.vglue()),
			    1.2*MENU_WIDTH*2*ivcm, 0.9*MENU_HEIGHT*ivcm);
	}
    }
}


Glyph* OverlayKit::MenuRect (PSColor * color) {
    Brush * brush = color->None() ? new Brush(0xaaaa, 0.0) : new Brush(0.0);
    Coord w = (MENU_WIDTH*ivcm);
    Coord h = (MENU_HEIGHT*ivcm);
    
    Resource::ref(brush);
    return new Fig31Rectangle(brush, WidgetKit::instance()->foreground(),
			      color->None() ? nil : color, 0, 0, w, h);
}


Glyph* OverlayKit::MenuPatRect (PSPattern * pat) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    Brush * brush = new Brush(0.0);
    Resource::ref(brush);
    Color * color = new Color(*(wk.foreground()));
    Resource::ref(color);
    color->rep(Session::instance()->default_display()->rep()->default_visual_)->stipple_ = pat->rep()->pixmap_;
    Coord w = (MENU_WIDTH*ivcm);
    Coord h = (MENU_HEIGHT*ivcm);
    
// this is totally unbelievable - some part of Xlib defines "None" to be 0L,
// which the preprocessor happily converts here, thus breaking the compiler.
    if (pat->None()) {
	return lk.fixed(lk.vbox(lk.vglue(),
				lk.hbox(lk.hglue(), wk.label("None"), lk.hglue()),
				lk.vglue()),
			1.4*MENU_WIDTH*ivcm,
			1.4*MENU_HEIGHT*ivcm);
    } else {
	return lk.margin(new Fig31Rectangle(brush, wk.foreground(), color,
					    0, 0, w, h),
			 0.2*MENU_WIDTH*ivcm,
			 0.2*MENU_HEIGHT*ivcm);
    }
}

void OverlayKit::MakeMenu(MenuItem* mbi, Command* cmd, const char* label) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    _ed->GetKeyMap()->Register(new CommandControl(cmd->GetControlInfo()));
    cmd->SetEditor(_ed);
    CommandPusher* pusher = new CommandPusher(cmd);
    MenuItem* menu_item = kit.menu_item(lk.hbox(kit.label(label),
						lk.hglue(),
						kit.label(cmd->GetControlInfo()->GetKeyLabel())));
    mbi->menu()->append_item(menu_item);
    menu_item->action(new ActionCallback(CommandPusher)(pusher,
							&CommandPusher::Push));
}

void OverlayKit::MakeMenu(MenuItem* mbi, Command* cmd, Glyph* pict) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    _ed->GetKeyMap()->Register(new CommandControl(cmd->GetControlInfo()));
    cmd->SetEditor(_ed);
    CommandPusher* pusher = new CommandPusher(cmd);
    MenuItem* menu_item = kit.menu_item(pict);
    mbi->menu()->append_item(menu_item);
    menu_item->action(new ActionCallback(CommandPusher)(pusher,
							&CommandPusher::Push));
}

MenuItem * OverlayKit::MakeFileMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("File"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new OvAboutCmd(new ControlInfo("About drawtool", "", "")),
	     "About drawtool   ");
    MakeMenu(mbi, new OvNewCompCmd(new ControlInfo("New", KLBL_NEWCOMP, CODE_NEWCOMP),
				 new OverlayIdrawComp),
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
    MakeMenu(mbi, new OvWindowDumpAsCmd(new ControlInfo("Dump Canvas As..."
						  )),
	     "Dump Canvas As...   ");
    MakeMenu(mbi, new OvImageMapCmd(new ControlInfo("Save ImageMap As..."
						  )),
	     "Save ImageMap As... ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvQuitCmd(new ControlInfo("Quit", KLBL_QUIT, CODE_QUIT)),
	     "Quit   ");
    return mbi;
}

MenuItem* OverlayKit::MakeEditMenu() {
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
    MakeMenu(mbi, new CopyCmd(new ControlInfo("Copy", KLBL_COPY, CODE_COPY)),
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
	     "Select by Attribute");
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
    mbi->menu()->append_item(kit.menu_item_separator());

    MenuItem* imagemenu = kit.menu_item(kit.label("Image Processing   "));
    imagemenu->menu(kit.pullright());
    mbi->menu()->append_item(imagemenu);
    
    MakeMenu(imagemenu, new AlphaTransparentRasterCmd(new ControlInfo("Alpha Transparency",
					     "", "")),
	     "Alpha Transparency ");
    MakeMenu(imagemenu, new ScaleGrayCmd(new ControlInfo("Scale GrayImage",
					     "", "")),
	     "Scale Gray Image   ");
    MakeMenu(imagemenu, new LogScaleCmd(new ControlInfo("LogScale GrayImage",
					     "", "")),
	     "Logscale Gray Image   ");
    MakeMenu(imagemenu, new PseudocolorCmd(new ControlInfo("PseudoColor GrayImage",
					     "", "")),
	     "Pseudocolor Gray Image   ");
#ifdef CLIPPOLY
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new ClipPolyAMinusBCmd(new ControlInfo("ClipPoly A minus B")),
	     "ClipPoly A minus B");
#if 0
    MakeMenu(mbi, new ClipPolyBMinusACmd(new ControlInfo("ClipPoly B minus A")),
	     "ClipPoly B minus A");
#endif
    MakeMenu(mbi, new ClipPolyAAndBCmd(new ControlInfo("ClipPoly A and B")),
	     "ClipPoly A and B ");
#endif

    return mbi;
}

MenuItem* OverlayKit::MakeStructureMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Structure"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new OvGroupCmd(new ControlInfo("Group", KLBL_GROUP, CODE_GROUP)),
	     "Group   ");
    MakeMenu(mbi, new UngroupCmd(new ControlInfo("Ungroup", KLBL_UNGROUP, CODE_UNGROUP)),
	     "Ungroup   ");
    MakeMenu(mbi, new FrontCmd(new ControlInfo("Bring to Front",
				       KLBL_FRONT, CODE_FRONT)),
	     "Bring to Front   ");
    MakeMenu(mbi, new BackCmd(new ControlInfo("Send to Back",
				      KLBL_BACK, CODE_BACK)),
	     "Send to Back   ");
    MakeMenu(mbi, new PullCmd(new ControlInfo("Pull Up One"
                                     )),
           "Pull Up One   ");
    MakeMenu(mbi, new PushCmd(new ControlInfo("Push Down One"
                                    )),
           "Push Down One   ");

    return mbi;
}

MenuItem* OverlayKit::MakeFontMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem* mbi;
    
    Catalog* catalog = unidraw->GetCatalog();
    int i = 1;
    PSFont* font = catalog->ReadFont(fontAttrib, i);
    TextGraphic* text;
    
    mbi = kit.menubar_item(kit.label("Font"));
    mbi->menu(kit.pulldown());
    
    while (font != nil) {
	text = new TextGraphic(font->GetPrintFontAndSize(), stdgraphic);
	text->SetFont(font);
	MakeMenu(mbi, new FontCmd(new ControlInfo(new TextOvComp(text)), font),
		 lk.hbox(lk.hglue(),
			 lk.hcenter(new Label(font->GetPrintFontAndSize(),
					      font,
					      kit.foreground())),
			 lk.hglue())
	     );
	font = catalog->ReadFont(fontAttrib, ++i);
    }
    
    return mbi;
}

MenuItem* OverlayKit::MakeBrushMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("Brush"));
    mbi->menu(kit.pulldown());
    ControlInfo* ctrlInfo;
    ArrowLine* line;
    
    int i = 1;
    Catalog* catalog = unidraw->GetCatalog();
    PSBrush* br = catalog->ReadBrush(brAttrib, i);
    while (br != nil) {
	
	if (br->None()) {
	    ctrlInfo = new ControlInfo("None");
	    
	} else {
	    line = new ArrowLine(0, 0, Math::round(MENU_WIDTH*ivcm), 0, false, false, 1., stdgraphic);
	    line->SetBrush(br);
	    ctrlInfo = new ControlInfo(new ArrowLineComp(line));
	}
	MakeMenu(mbi, new BrushCmd(ctrlInfo, br), MenuLine(br));
	br = catalog->ReadBrush(brAttrib, ++i);
    }
    
    mbi->menu()->append_item(kit.menu_item_separator());
    
    line = new ArrowLine(0, 0, Math::round(MENU_WIDTH*ivcm), 0,
			 false, false, 1., stdgraphic);
    ctrlInfo = new ControlInfo(new ArrowLineComp(line));
    MakeMenu(mbi, new ArrowCmd(ctrlInfo, false, false), MenuArrowLine(false, false));
    line = new ArrowLine(0, 0, Math::round(MENU_WIDTH*ivcm), 0,
			 true, false, 1., stdgraphic);
    ctrlInfo = new ControlInfo(new ArrowLineComp(line));
    MakeMenu(mbi, new ArrowCmd(ctrlInfo, true, false), MenuArrowLine(true, false));
    line = new ArrowLine(0, 0, Math::round(MENU_WIDTH*ivcm), 0,
			 false, true, 1., stdgraphic);
    ctrlInfo = new ControlInfo(new ArrowLineComp(line));
    MakeMenu(mbi, new ArrowCmd(ctrlInfo, false, true), MenuArrowLine(false, true));
    line = new ArrowLine(0, 0, Math::round(MENU_WIDTH*ivcm), 0,
			 true, true, 1., stdgraphic);
    ctrlInfo = new ControlInfo(new ArrowLineComp(line));
    MakeMenu(mbi, new ArrowCmd(ctrlInfo, true, true), MenuArrowLine(true, true));

    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvPreciseBrushCmd(new ControlInfo("Precise Width",
				       "", "")),
	     "Precise Width");
    return mbi;
}

MenuItem* OverlayKit::MakePatternMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("Pattern"));
    mbi->menu(kit.pulldown());
    
    int i = 1;
    Catalog* catalog = unidraw->GetCatalog();
    PSPattern* pat = catalog->ReadPattern(patAttrib, i);
    
    while (pat != nil) {
	ControlInfo* ctrlInfo;
	IntCoord w = Math::round(MENU_WIDTH*ivcm);
	IntCoord h = Math::round(MENU_HEIGHT*ivcm);
	
	if (pat->None()) {
	    ctrlInfo = new ControlInfo("None");
	} else {
	    SF_Rect* sfr = new SF_Rect(0, 0, w, h, stdgraphic);
	    sfr->SetPattern(pat);
	    ctrlInfo = new ControlInfo(new RectOvComp(sfr));
	}
	MakeMenu(mbi, new PatternCmd(ctrlInfo, pat), MenuPatRect(pat));
	pat = catalog->ReadPattern(patAttrib, ++i);
    }
    return mbi;
}

MenuItem* OverlayKit::MakeFgColorMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("FgColor"));
    mbi->menu(kit.pulldown());
    
    int i = 1;
    Catalog* catalog = unidraw->GetCatalog();
    PSColor* color = catalog->ReadColor(fgAttrib, i);
    
    while (color != nil) {
	IntCoord w = Math::round(MENU_WIDTH*ivcm);
	IntCoord h = Math::round(MENU_HEIGHT*ivcm);
	
	SF_Rect* sfr = new SF_Rect(0, 0, w, h, stdgraphic);
	sfr->SetColors(color, color);
	MakeMenu(mbi, new ColorCmd(new ControlInfo(new RectOvComp(sfr), color->GetName()),
			   color, nil),
		 lk.hbox(MenuRect(color),
			 kit.label("  "),
			 kit.label(color->GetName()),
			 lk.hglue())
	     );
	color = catalog->ReadColor(fgAttrib, ++i);
    }
    
    return mbi;
}


MenuItem* OverlayKit::MakeBgColorMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("BgColor"));
    mbi->menu(kit.pulldown());
    
    int i = 1;
    Catalog* catalog = unidraw->GetCatalog();
    PSColor* color = catalog->ReadColor(bgAttrib, i);
    
    while (color != nil) {
	ControlInfo* ctrlInfo;
	IntCoord w = Math::round(MENU_WIDTH*ivcm);
	IntCoord h = Math::round(MENU_HEIGHT*ivcm);

	if (color->None()) {
	  ctrlInfo = new ControlInfo("None");
	} else {
	  SF_Rect* sfr = new SF_Rect(0, 0, w, h, stdgraphic);
	  sfr->SetColors(color, color);
	  ctrlInfo = new ControlInfo(new RectOvComp(sfr), color->GetName());
	}
	MakeMenu(mbi, new ColorCmd( ctrlInfo, nil, color),
		 lk.hbox(MenuRect(color),
			 kit.label("  "),
			 kit.label(color->GetName()),
			 lk.hglue())
		 );
	color = catalog->ReadColor(bgAttrib, ++i);
    }
    
    return mbi;
}

#undef Center
#define Center iv2_6_Center

MenuItem* OverlayKit::MakeAlignMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("Align"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new AlignCmd(new ControlInfo("Left Sides",
				       KLBL_ALGNLEFT,
				       CODE_ALGNLEFT), Left, Left),
	     "Left Sides   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Right Sides",
				       KLBL_ALGNRIGHT,
				       CODE_ALGNRIGHT), Right, Right),
	     "Right Sides   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Tops",
				       KLBL_ALGNTOP,
				       CODE_ALGNTOP), Top, Top),
	     "Tops   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Bottoms",
				       KLBL_ALGNBOT,
				       CODE_ALGNBOT), Bottom, Bottom),
	     "Bottoms   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Horiz Centers",
				       KLBL_ALGNHCTR,
				       CODE_ALGNHCTR), HorizCenter, HorizCenter),
	     "Horiz Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Vert Centers",
				       KLBL_ALGNVCTR,
				       CODE_ALGNVCTR), VertCenter, VertCenter),
	     "Vert Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Centers",
				       KLBL_ALGNCTR,
				       CODE_ALGNCTR), Center, Center),
	     "Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Left",
				       KLBL_ABUTLEFT,
				       CODE_ABUTLEFT), Left, Right),
	     "Abut Left   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Right",
				       KLBL_ABUTRIGHT,
				       CODE_ABUTRIGHT), Right, Left),
	     "Abut Right   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Up",
				       KLBL_ABUTUP,
				       CODE_ABUTUP), Top, Bottom),
	     "Abut Up   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Down",
				       KLBL_ABUTDOWN,
				       CODE_ABUTDOWN), Bottom, Top),
	     "Abut Down   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new AlignToGridCmd(new ControlInfo("Align to Grid",
					     KLBL_ALGNTOGRID,
					     CODE_ALGNTOGRID)),
	     "Align to Grid   ");

    return mbi;
}

MenuItem* OverlayKit::MakeFrameMenu() {
    return nil;
}

MenuItem* OverlayKit::MakeViewMenu() {
    /* for handling special keys */
    char keystr[3];
    keystr[2] = '\0';

    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem* mbi = kit.menubar_item(kit.label("View"));
    mbi->menu(kit.pulldown());

    OvNewViewCmd::default_instance
      (new OvNewViewCmd
       (new ControlInfo("New View", KLBL_NEWVIEW, CODE_NEWVIEW), 
       "localhost:0.0"));
    MakeMenu(mbi, OvNewViewCmd::default_instance(), "New View   ");

    MakeMenu(mbi, new OvCloseEditorCmd(new ControlInfo("Close View",
					     KLBL_CLOSEEDITOR,
					     CODE_CLOSEEDITOR)),
	     "Close View   ");
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
    *(unsigned short*)keystr = XK_Up;
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Up", "", keystr), NO_PAN, PLUS_SMALL_PAN),
	     "Small Pan Up     ");
    *(unsigned short*)keystr = XK_Down;
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Down", "", keystr), NO_PAN, MINUS_SMALL_PAN),
	     "Small Pan Down   ");
    *(unsigned short*)keystr = XK_Left;
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Left", "", keystr), MINUS_SMALL_PAN, NO_PAN),
	     "Small Pan Left   ");
    *(unsigned short*)keystr = XK_Right;
    MakeMenu(spani, new FixedPanCmd(new ControlInfo("Small Pan Right", "", keystr), PLUS_SMALL_PAN, NO_PAN),
	     "Small Pan Right  ");
    mbi->menu()->append_item(spani);

    MenuItem* lpani = kit.menu_item(kit.label("Large Pan        "));
    Menu* lpan = kit.pullright();
    lpani->menu(lpan);
    *(unsigned short*)keystr = XK_Page_Up;
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Up", "PgUp", keystr), NO_PAN, PLUS_LARGE_PAN),
	     "Large Pan Up     ");
    *(unsigned short*)keystr = XK_Page_Down;
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Down", "PgDn", keystr), NO_PAN, MINUS_LARGE_PAN),
	     "Large Pan Down   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Left"), MINUS_LARGE_PAN, NO_PAN),
	     "Large Pan Left   ");
    MakeMenu(lpani, new FixedPanCmd(new ControlInfo("Large Pan Right"), PLUS_LARGE_PAN, NO_PAN),
	     "Large Pan Right  ");
    mbi->menu()->append_item(lpani);

    MakeMenu(mbi, new PrecisePanCmd(new ControlInfo("Precise Pan")),
	     "Precise Pan      ");

    mbi->menu()->append_item(kit.menu_item_separator());

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
    return mbi;
}


MenuItem * OverlayKit::MakeToolsMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Tools"));
    mbi->menu(kit.pulldown());

    MenuItem* menu_item = kit.menu_item(kit.label("Extra Tools"));
    menu_item->action(new ActionCallback(OverlayKit)(this, &OverlayKit::toolbar1));
    mbi->menu()->append_item(menu_item);

    menu_item = kit.menu_item(kit.label("Idraw Tools"));
    menu_item->action(new ActionCallback(OverlayKit)(this, &OverlayKit::toolbar0));
    mbi->menu()->append_item(menu_item);

    menu_item = kit.menu_item(kit.label("Add Custom Tool"));
    menu_item->action(new ActionCallback(OverlayKit)(this, &OverlayKit::add_custom_tool));
    mbi->menu()->append_item(menu_item);

    return mbi;
}

void OverlayKit::toolbar0() {
    _toolbars->flip_to(0);
    _ed->GetKeyMap()->Execute(CODE_SELECT);
    _toolbar->redraw();
}

void OverlayKit::toolbar1() {
    _toolbars->flip_to(1);
    _ed->GetKeyMap()->Execute(CODE_SELECT);
    _toolbar->redraw();
}

void OverlayKit::add_custom_tool() {
    static OpenFileChooser* chooser = nil;
    Editor* ed = GetEditor();
    Style* style = new Style(Session::instance()->style());
    if (!chooser) {
      style->attribute("subcaption", "Open Idraw Icon For Tool Button:");
      style->attribute("open", "Open");
      chooser = new OpenFileChooser(".", WidgetKit::instance(), style);
      Resource::ref(chooser);
    }
    boolean again;
    boolean reset_caption = false;
    const char* name = nil;
    GraphicComp* comp = nil;
    while (again = chooser->post_for(ed->GetWindow())) {
      const String* str = chooser->selected();
      NullTerminatedString ns(*str);
      name = ns.string();
      IdrawCatalog* catalog = (IdrawCatalog*)unidraw->GetCatalog();
      boolean ok = true;
      style->attribute("caption", "                     " );
      chooser->twindow()->repair();
      chooser->twindow()->display()->sync();
      if (catalog->IdrawCatalog::Retrieve(name, (Component*&)comp)) {
	break;
      } else {
	style->attribute("caption", "Open failed!" );
	reset_caption = true;
      }

    }
    chooser->unmap();
    if (reset_caption) {
	style->attribute("caption", "            ");
    }
    add_tool_button(name, (OverlayComp*)comp);
}

OverlayComp* OverlayKit::add_tool_button(const char* path, OverlayComp* comp) {
    LayoutKit& layout = *LayoutKit::instance();
    if (!comp) {
      IdrawCatalog* catalog = (IdrawCatalog*)unidraw->GetCatalog();
      catalog->IdrawCatalog::Retrieve(path, (Component*&)comp);
    }
    _toolbars->flip_to(1);
    Glyph* newbutton = path&&comp ? IdrawReader::load(path) : nil;
    if (newbutton) {
      _toolbar_vbox[1]->append(MakeTool(new GraphicCompTool(new ControlInfo(comp, "", ""), comp),
					layout.overlay(layout.hcenter(layout.hspace(_maxwidth)),
						       layout.hcenter(newbutton)),
					_tg, _ed->MouseDocObservable(), mouse_custom));
    } else {
      delete comp;
      comp = nil;
    }
    _ed->GetKeyMap()->Execute(CODE_SELECT);
    _toolbar->redraw();
    return comp;
}

MenuItem * OverlayKit::MakeViewersMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    AttributeList* edlaunchlist = _ed->edlauncherlist();
    AttributeList* comterplist = _ed->comterplist();

    if (!edlaunchlist /* && !comterplist */) return nil;
    MenuItem *mbi = kit.menubar_item(kit.label("Editors"));
    mbi->menu(kit.pulldown());

    /* make one menu item per registered editor_launcher */
    if (edlaunchlist) {
      Iterator i;
      edlaunchlist->First(i);
      while (!edlaunchlist->Done(i)) {
	Attribute* attr = edlaunchlist->GetAttr(i);
	char buf[BUFSIZ];
	sprintf(buf, "%s Editor", attr->Name());
	MenuItem* menu_item = kit.menu_item(kit.label(buf));
	menu_item->action(new EditorLauncherAction((editor_launcher)attr->Value()->obj_val()));
	mbi->menu()->append_item(menu_item);
	edlaunchlist->Next(i);
      }
    }

    /* make one menu item per registered interpreter */
    if (comterplist) {
      Iterator i;
      comterplist->First(i);
      if (edlaunchlist && !comterplist->Done(i))
	mbi->menu()->append_item(kit.menu_item_separator());
      while (!comterplist->Done(i)) {
	Attribute* attr = comterplist->GetAttr(i);
	AttrDialog* attrdialog = new AttrDialog((ComTerpServ*)attr->Value()->obj_val());
	char buf[BUFSIZ];
	sprintf(buf, "%s Interpreter", attr->Name());
	MakeMenu(mbi, new SetAttrByExprCmd(new ControlInfo(buf, "", ""), attrdialog),
		 buf);
	comterplist->Next(i);
      }
    }

    return mbi;
}

OverlayKit* OverlayKit::Instance() {
    if (!_overlaykit)
	_overlaykit = new OverlayKit();
    return _overlaykit;
}

void OverlayKit::Annotate(OverlayComp* comp) {
    WidgetKit& kit = *WidgetKit::instance();
    const char* anno = comp->GetAnnotation();
    if (!anno)
	anno = "";
    AnnotateDialog* dlog = new AnnotateDialog(anno, &kit, kit.style());
    dlog->ref();
    if (dlog->post_for(_ed->GetWindow())) {
	const char* newtext = dlog->value();
	comp->SetAnnotation(newtext);
	((ModifStatusVar*)_ed->GetState("ModifStatusVar"))->SetModifStatus(true);
    }
}

void OverlayKit::AttrEdit(OverlayComp* comp) {
    WidgetKit& kit = *WidgetKit::instance();
    AttributeDialog* dlog = new AttributeDialog(comp, &kit, kit.style());
    dlog->ref();
#if 0
    if (dlog->post_for(_ed->GetWindow()))
	((ModifStatusVar*)_ed->GetState("ModifStatusVar"))->SetModifStatus(true);	
#else
    dlog->map_for(_ed->GetWindow());
#endif
}

int OverlayKit::bintest(const char* command) {
  char combuf[BUFSIZ];
  // the echo -n $PATH is to workaround a mysterious problem on MacOS X
  // whereby the which sometime returns nothing
  sprintf( combuf, "echo -n $PATH; which %s", command );
  FILE* fptr = popen(combuf, "r");
  char testbuf[BUFSIZ];	
  fgets(testbuf, BUFSIZ, fptr);  
  // fprintf(stderr, "%s\n", testbuf);
  pclose(fptr);
  if (strncmp(testbuf+strlen(testbuf)-strlen(command)-1, 
	      command, strlen(command)) != 0) {
    return -1;
  }
  return 0;
}

boolean OverlayKit::bincheck(const char* command) {
  int status = bintest(command);
  return status==0 ? 1 : 0;
}

const char* OverlayKit::otherdisplay() { return _otherdisplay; }
void OverlayKit::otherdisplay(const char* display) 
{ delete _otherdisplay; _otherdisplay= strdup(display); }


OverlaySelection* OverlayKit::MakeSelection(Selection* sel) {
  return new OverlaySelection(sel);
}
