/*
 * Copyright (c) 2020 Scott E. Johnston
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
 * ComKit definitions
 */

#include <ComUnidraw/comkit.h>
#include <ComUnidraw/pixelcmds.h>

#include <OverlayUnidraw/ovclip.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovipcmds.h>
#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/setattrbyexpr.h>
#include <OverlayUnidraw/slctbyattr.h>
#include <OverlayUnidraw/rastercmds.h>

#include <UniIdraw/idkybd.h>

#include <Unidraw/ctrlinfo.h>
#include <Unidraw/kybd.h>

#include <Unidraw/Commands/edit.h>
#include <Unidraw/Commands/transforms.h>

#include <IV-look/kit.h>
#include <InterViews/layout.h>

/*****************************************************************************/

ComKit* ComKit::_comkit = nil;

ComKit::ComKit () {
}

ComKit::~ComKit() {
}

ComKit* ComKit::Instance() {
    if (!_comkit)
	_comkit = new ComKit();
    return _comkit;
}

MenuItem* ComKit::MakeEditMenu() {
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
    #ifdef CLIPPOLY
    MakeMenu(imagemenu, new PolyClipRasterCmd(new ControlInfo("Poly Clip Raster",
					     "", "")),
	     "Poly Clip Raster ");
    #endif
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

