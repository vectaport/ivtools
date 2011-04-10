/*
 * Copyright (c) 1997,1999 Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc., Cartoactive Systems
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
 * DrawKit definitions
 */

#include <DrawServ/drawkit.h>
#include <DrawServ/drawcomps.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawlinklist.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/linkselection.h>
#include <DrawServ/rcdialog.h>

#include <FrameUnidraw/frameeditor.h>

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/graphimport.h>
#include <GraphUnidraw/graphkit.h>

#include <OverlayUnidraw/annotate.h>
#include <OverlayUnidraw/attrtool.h>
#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovctrl.h>
#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/slctbyattr.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idkybd.h>

#include <Unidraw/Commands/transforms.h>
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
#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/unidraw.h>

#include <IVGlyph/figure.h>
#include <IVGlyph/toolbutton.h>
#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/brush.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <OS/math.h>

declareActionCallback(DrawKit)
implementActionCallback(DrawKit)

static const int unit = 15;
static const int xradius = 35;
static const int yradius = 20;

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

static Coord xhead[] = { unit, unit-4, unit-7 };
static Coord yhead[] = { unit, unit-7, unit-4 };

/******************************************************************************/

DrawKit* DrawKit::_comkit = nil;

DrawKit::DrawKit () {
}

void DrawKit::Init (OverlayComp* comp, const char* name) {
    FrameKit::Init(comp, name);
}

DrawKit* DrawKit::Instance() {
    if (!_comkit)
	_comkit = new DrawKit();
    return _comkit;
}

MenuItem * DrawKit::MakeFileMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("File"));
    mbi->menu(kit.pulldown());
    MakeMenu(mbi, new OvAboutCmd(new ControlInfo("About drawserv", KLBL_ABOUT, CODE_ABOUT)),
	     "About drawserv   ");
    MakeMenu(mbi, new OvNewCompCmd(new ControlInfo("New", KLBL_NEWCOMP, CODE_NEWCOMP),
				 new DrawIdrawComp),
	     "New   ");
    MakeMenu(mbi, new OvRevertCmd(new ControlInfo("Revert", KLBL_REVERT, CODE_REVERT)),
	     "Revert   ");
    MakeMenu(mbi, new OvOpenCmd(new ControlInfo("Open...", KLBL_VIEWCOMP, CODE_VIEWCOMP)),
	     "Open...   ");
    MakeMenu(mbi, new OvSaveCompCmd(new ControlInfo("Save", KLBL_SAVECOMP, CODE_SAVECOMP)),
	     "Save   ");
    MakeMenu(mbi, new OvSaveCompAsCmd(new ControlInfo("Save As...",
						      KLBL_SAVECOMPAS, CODE_SAVECOMPAS)),
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
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvQuitCmd(new ControlInfo("Quit", KLBL_QUIT, CODE_QUIT)),
	     "Quit   ");
    return mbi;
}

MenuItem* DrawKit::MakeEditMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Edit"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new UndoCmd(new ControlInfo("Undo", KLBL_UNDO, CODE_UNDO)),
	     "Undo   ");
    MakeMenu(mbi, new RedoCmd(new ControlInfo("Redo", KLBL_REDO, CODE_REDO)),
	     "Redo   ");
    MakeMenu(mbi, new GraphCutCmd(new ControlInfo("Cut", KLBL_CUT, CODE_CUT)),
	     "Cut   "); // overrides FrameCutCmd
    MakeMenu(mbi, new GraphCopyCmd(new ControlInfo("Copy", KLBL_COPY, CODE_COPY)),
	     "Copy   ");
    MakeMenu(mbi, new GraphPasteCmd(new ControlInfo("Paste", KLBL_PASTE, CODE_PASTE)),
	     "Paste   ");
    MakeMenu(mbi, new GraphDupCmd(new ControlInfo("Duplicate", KLBL_DUP, CODE_DUP)),
	     "Duplicate   ");
    MakeMenu(mbi, new GraphDeleteCmd(new ControlInfo("Delete", KLBL_DEL, CODE_DEL)),
	     "Delete   ");
    MakeMenu(mbi, new OvSlctAllCmd(new ControlInfo("Select All", KLBL_SLCTALL, CODE_SLCTALL)),
	     "Select All   ");
    MakeMenu(mbi, new SlctByAttrCmd(new ControlInfo("Select by Attribute", "$", "$")),
	     "Select by Attribute   ");
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

MenuItem * DrawKit::MakeToolsMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Tools"));
    mbi->menu(kit.pulldown());

    MenuItem* menu_item = kit.menu_item(kit.label("Graph Tools"));
    menu_item->action(new ActionCallback(DrawKit)(this, &DrawKit::toolbar1));
    mbi->menu()->append_item(menu_item);

    menu_item = kit.menu_item(kit.label("Idraw Tools"));
    menu_item->action(new ActionCallback(DrawKit)(this, &DrawKit::toolbar0));
    mbi->menu()->append_item(menu_item);

    return mbi;
}

Glyph* DrawKit::MakeToolbar() {
    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = kit.style();

    ToolButton* select;
    ToolButton* move;
    ToolButton* reshape;
    ToolButton* magnify;

    _toolbars = layout.deck(2);

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
    Glyph* gedge = layout.overlay(
	new Fig31Line(new Brush(0), kit.foreground(), nil,
		      0, 0, unit, unit),
	new Fig31Polygon(new Brush(0), kit.foreground(), kit.foreground(),
			 xhead, yhead, 3)
    );
    Glyph* gnod1 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
			 0, 0, unit, unit*3/5),
	layout.center(kit.label("___")));
    Glyph* gnod2 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
			 0, 0, unit, unit*3/5),
	layout.center(kit.label("abc")));

    Coord maxwidth = 0;
    Requisition req;
    maxwidth = Math::max((sel->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((mov->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((scl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((str->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((rot->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((alt->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((mag->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((txt->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((glin->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gmlin->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gospl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((grect->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gellp->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gpoly->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gcspl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((anno->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gedge->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod1->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod2->request(req), req.x_requirement().natural()),
			 maxwidth);

    vb->append(select = MakeTool(new SelectTool(new ControlInfo("Select", KLBL_SELECT, CODE_SELECT)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(sel)),
			_tg, _ed->MouseDocObservable(), mouse_sel));
    vb->append(move = MakeTool(new MoveTool(new ControlInfo("Move", KLBL_MOVE, CODE_MOVE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(mov)),
			_tg, _ed->MouseDocObservable(), mouse_mov));
    vb->append(MakeTool(new ScaleTool(new ControlInfo("Scale", KLBL_SCALE, CODE_SCALE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(scl)), _tg, _ed->MouseDocObservable(), mouse_scl));
    vb->append(MakeTool(new StretchTool(new ControlInfo("Stretch", KLBL_STRETCH,CODE_STRETCH)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(str)), _tg, _ed->MouseDocObservable(), mouse_str));
    vb->append(MakeTool(new RotateTool(new ControlInfo("Rotate", KLBL_ROTATE, CODE_ROTATE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(rot)), _tg, _ed->MouseDocObservable(), mouse_rot));
    vb->append(reshape = MakeTool(new ReshapeTool(new ControlInfo("Alter", KLBL_RESHAPE, CODE_RESHAPE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(alt)), _tg, _ed->MouseDocObservable(), mouse_alt));
    vb->append(magnify = MakeTool(new MagnifyTool(new ControlInfo("Magnify", KLBL_MAGNIFY,CODE_MAGNIFY)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(mag)), _tg, _ed->MouseDocObservable(), mouse_mag));
    TextGraphic* text = new TextGraphic("Text", stdgraphic);
    TextOvComp* textComp = new TextOvComp(text);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo("Text", KLBL_TEXT, CODE_TEXT), textComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(txt)), _tg, _ed->MouseDocObservable(), mouse_txt));
    ArrowLine* line = new ArrowLine(
	0, 0, unit, unit, false, false, 1., stdgraphic
    );
    ArrowLineOvComp* arrowLineComp = new ArrowLineOvComp(line);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(arrowLineComp, KLBL_LINE, CODE_LINE), arrowLineComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(glin)),
			_tg, _ed->MouseDocObservable(), mouse_lin));
    ArrowMultiLine* ml = new ArrowMultiLine(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    ml->SetPattern(psnonepat);
    ArrowMultiLineOvComp* mlComp = new ArrowMultiLineOvComp(ml);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(mlComp, KLBL_MULTILINE, CODE_MULTILINE), mlComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gmlin)),
			_tg, _ed->MouseDocObservable(), mouse_mlin));
    ArrowOpenBSpline* spl = new ArrowOpenBSpline(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    spl->SetPattern(psnonepat);
    ArrowSplineOvComp* splComp = new ArrowSplineOvComp(spl);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(splComp, KLBL_SPLINE, CODE_SPLINE), splComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gospl)),
			_tg, _ed->MouseDocObservable(), mouse_ospl));
    SF_Rect* rect = new SF_Rect(0, 0, unit, unit*4/5, stdgraphic);
    rect->SetPattern(psnonepat);
    RectOvComp* rectComp = new RectOvComp(rect);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(rectComp, KLBL_RECT, CODE_RECT), rectComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(grect)),
			_tg, _ed->MouseDocObservable(), mouse_rect));
    SF_Ellipse* ellipse = new SF_Ellipse(0, 0, unit*2/3, unit*2/5, stdgraphic);
    ellipse->SetPattern(psnonepat);
    EllipseOvComp* ellipseComp = new EllipseOvComp(ellipse);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(ellipseComp, KLBL_ELLIPSE, CODE_ELLIPSE), ellipseComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gellp)),
			_tg, _ed->MouseDocObservable(), mouse_ellp));
    SF_Polygon* polygon = new SF_Polygon(xClosed, yClosed, nClosed,stdgraphic);
    polygon->SetPattern(psnonepat);
    PolygonOvComp* polygonComp = new PolygonOvComp(polygon);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(polygonComp, KLBL_POLY, CODE_POLY), polygonComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gpoly)),
			_tg, _ed->MouseDocObservable(), mouse_poly));
    SFH_ClosedBSpline* cspline = new SFH_ClosedBSpline(
        xClosed, yClosed, nClosed, stdgraphic
    );
    cspline->SetPattern(psnonepat);
    ClosedSplineOvComp* csplineComp = new ClosedSplineOvComp(cspline);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(csplineComp, KLBL_CSPLINE,CODE_CSPLINE), csplineComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gcspl)),
			_tg, _ed->MouseDocObservable(), mouse_cspl));
    vb->append(MakeTool(new AnnotateTool(new ControlInfo("Annotate", "A", "A")),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(anno)), _tg, _ed->MouseDocObservable(), mouse_anno));
    vb->append(MakeTool(new AttributeTool(new ControlInfo("Attribute", "T", "T")),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(attr)), _tg, _ed->MouseDocObservable(), mouse_attr));

    _toolbars->append(vb);
    vb = layout.vbox();
    _toolbar_vbox[1] = vb;
    vb->append(select);
    vb->append(move);
    vb->append(reshape);
    vb->append(magnify);
    ArrowLine* aline = new ArrowLine(
	0, 0, unit, unit, true, false, 1., stdgraphic
    );
    EdgeComp* protoedge = new EdgeComp(aline);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protoedge, "E","E"),
					    protoedge),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gedge)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_edge));
    SF_Ellipse* nellipse = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    nellipse->SetPattern(psnonepat);
    TextGraphic* ntext = new TextGraphic("___", stdgraphic);
    nellipse->Align(4, ntext, 4); // same as Center in IV-2_6/InterViews/alignment.h
    NodeComp* protonode = new NodeComp(nellipse, ntext);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode, "N","N"),
					    protonode),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod1)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_node));
    SF_Ellipse* nellipse2 = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    nellipse2->SetPattern(psnonepat);
    TextGraphic* ntext2 = new TextGraphic("abc", stdgraphic);
    nellipse2->Align(4, ntext2, 4); // same as Center in IV-2_6/InterViews/alignment.h
    NodeComp* protonode2 = new NodeComp(nellipse2, ntext2, true);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode2, "L","L"),
					    protonode2),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod2)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    _toolbars->append(vb);

    _toolbars->flip_to(0);
    _toolbar = new Patch(_toolbars);

    return layout.hbox(
	layout.vflexible(
	    layout.vnatural(
		new Background(
		    layout.vcenter(
			_toolbar
		    ),
		    unidraw->GetCatalog()->FindColor("#aaaaaa")
		),
		550
	    )
	)
    );
}

void DrawKit::toolbar0() {
    _toolbars->flip_to(0);
    _ed->GetKeyMap()->Execute(CODE_SELECT);
    _toolbar->redraw();
}

void DrawKit::toolbar1() {
    _toolbars->flip_to(1);
    _ed->GetKeyMap()->Execute("L");
    _toolbar->redraw();
}

void DrawKit::launch_drawtool() {
  OverlayEditor* ed = new OverlayEditor((const char*)nil);
  unidraw->Open(ed);
}

void DrawKit::launch_flipbook() {
  FrameEditor* ed = new FrameEditor((const char*)nil);
  unidraw->Open(ed);
}

void DrawKit::launch_graphdraw() {
  GraphEditor* ed = new GraphEditor((const char*)nil);
  unidraw->Open(ed);
}

OverlaySelection* DrawKit::MakeSelection(Selection* sel) {
#ifdef HAVE_ACE
  return new LinkSelection((DrawEditor*)GetEditor(), sel);
#else
  return FrameKit::MakeSelection(sel);
#endif
}

MenuItem * DrawKit::MakeViewersMenu() {
#ifdef HAVE_ACE
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();

    MenuItem *mbi = kit.menubar_item(kit.label("Connections"));
    mbi->menu(kit.pulldown());

    MenuItem* menu_item = kit.menu_item(kit.label("Connect"));
    menu_item->action(new RemoteConnectPopupAction(GetEditor()));
    mbi->menu()->append_item(menu_item);

    return mbi;
#else
    return nil;
#endif
}


