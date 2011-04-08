/*
 * Copyright (c) 1996 Vectaport Inc.
 * Copyright (c) 1994-1995 Vectaport Inc., Cartoactive Systems
 * Copyright (c) 1990, 1991 Stanford University 
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
 * GraphImportCmd implementation.
 */

#include <GraphUnidraw/graphcatalog.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/graphimport.h>
#include <GraphUnidraw/nodecomp.h>

#include <IVGlyph/importchooser.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <string.h>

/*****************************************************************************/

ClassId GraphImportCmd::GetClassId () { return GRAPHIMPORT_CMD; }

boolean GraphImportCmd::IsA (ClassId id) {
    return GRAPHIMPORT_CMD == id || OvImportCmd::IsA(id);
}

GraphImportCmd::GraphImportCmd (ControlInfo* c, ImportChooser* f) : OvImportCmd(c, f) { 
}

GraphImportCmd::GraphImportCmd (Editor* ed, ImportChooser* f) : OvImportCmd(ed, f) { 
}

Command* GraphImportCmd::Copy () {
    GraphImportCmd* copy = new GraphImportCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void GraphImportCmd::Execute () { 
    GraphComp* comps = (GraphComp*)PostDialog();

    Clipboard* cb;
    if (comps != nil) {
	GraphPasteCmd* paste_cmd = new GraphPasteCmd(GetEditor(), new Clipboard(comps));
	paste_cmd->Execute();

	if (chooser_->centered())
	    GetEditor()->GetViewer()->Align(comps, /* Center */ 4);

        if (!chooser_->by_pathname()) {
	    UngroupCmd* ungroup_cmd = new UngroupCmd(GetEditor());
	    ungroup_cmd->Execute();
            MacroCmd* macro_cmd = new MacroCmd(GetEditor(), paste_cmd, ungroup_cmd);
            macro_cmd->Log();
        } else {
  	    paste_cmd->Log();
        }
    } 

}

GraphicComp* GraphImportCmd::Import (const char* pathname) {
    GraphicComp* comp = nil;
    const char* creator = ReadCreator(pathname);
    GraphCatalog* catalog = (GraphCatalog*)unidraw->GetCatalog();

    if (strcmp(creator, "graphdraw") == 0 || 
	strcmp(creator, "netdraw") == 0 || 
	strcmp(creator, "graph-idraw") == 0) {
        catalog->SetImport(true);
        if (catalog->GraphCatalog::Retrieve(pathname, (Component*&) comp)) {
            catalog->SetImport(false);
	    catalog->Forget(comp);
	    
            if (chooser_->by_pathname()) {
	        return new NodeComp((GraphComp*)comp);
            } else
	        return comp;
	}
        catalog->SetImport(false);
	return nil;
    } else 
        return OvImportCmd::Import(pathname);
}
