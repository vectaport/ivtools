/*
 * Copyright (c) 1994 Vectaport Inc.
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

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphdialog.h>
#include <GraphUnidraw/grapheditor.h>

#include <OverlayUnidraw/ovkit.h>
#include <OverlayUnidraw/ovviewer.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idvars.h>

#include <Unidraw/Components/text.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Tools/grcomptool.h>

#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/keymap.h>
#include <Unidraw/unidraw.h>

#include <InterViews/box.h>
#include <InterViews/border.h>
#include <InterViews/glue.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>

#include <IV-2_6/InterViews/frame.h>
#include <IV-2_6/InterViews/panner.h>
#include <IV-2_6/InterViews/tray.h>

#include <ComTerp/comterpserv.h>

#include <stdio.h>
#include <string.h>

/*****************************************************************************/

GraphEditor::GraphEditor(OverlayComp* oc, OverlayKit* ok)
: ComEditor(false, ok)
{
    Init(oc);
}

GraphEditor::GraphEditor(const char* file, OverlayKit* ok)
: ComEditor(false, ok)
{
    if (file == nil) {
	Init();

    } else {
	Catalog* catalog = unidraw->GetCatalog();
	OverlayComp* comp;

	if (catalog->Retrieve(file, (Component*&) comp)) {
	    Init(comp);

	} else {
	    Init();
	    fprintf(stderr, "graphdraw: couldn't open %s\n", file);
	}
    }
}

GraphEditor::GraphEditor(boolean initflag, OverlayKit* ok) 
: ComEditor(initflag, ok) {}

GraphEditor::~GraphEditor() {}

void GraphEditor::Init (OverlayComp* comp, const char* name) {
    if (!comp) comp = new GraphIdrawComp;
    _terp = new ComTerpServ();
    AddCommands(_terp);
    add_comterp("Graphdraw", _terp);
    _overlay_kit->Init(comp, name);
    
    WidgetKit& kit = *WidgetKit::instance();
    Style* s = kit.style();
    _nodedialog = new NodeDialog(&kit, s);
    Resource::ref(_nodedialog);
}

const char* GraphEditor::GetNodeLabel() {
    _nodedialog->clear();
    while (_nodedialog->post_for(GetWindow())) {
	const char* label = _nodedialog->value();
	if (strlen(label) > 0)
	    return label;
    }
    return "";
}
