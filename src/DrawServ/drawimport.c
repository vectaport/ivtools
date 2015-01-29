/*
 * Copyright (c) 2009 Scott E. Johnston
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
 * DrawImportCmd implementation.
 */

#include <DrawServ/drawcatalog.h>
#include <DrawServ/drawcmds.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawimport.h>
#include <DrawServ/drawclasses.h>
#include <DrawServ/drawcatalog.h>

#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/framestates.h>

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphcatalog.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcreator.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/nodecomp.h>

#include <IVGlyph/importchooser.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <string.h>

/*****************************************************************************/

ClassId DrawImportCmd::GetClassId () { return GRAPHIMPORT_CMD; }

boolean DrawImportCmd::IsA (ClassId id) {
    return DRAWIMPORT_CMD == id || FrameImportCmd::IsA(id);
}

DrawImportCmd::DrawImportCmd (ControlInfo* c, ImportChooser* f) : FrameImportCmd(c, f) { 
}

DrawImportCmd::DrawImportCmd (Editor* ed, ImportChooser* f) : FrameImportCmd(ed, f) { 
}

Command* DrawImportCmd::Copy () {
    DrawImportCmd* copy = new DrawImportCmd(CopyControlInfo(), chooser_);
    InitCopy(copy);
    return copy;
}

void DrawImportCmd::Execute () { 
    GraphicComp* comps = PostDialog();

    Clipboard* cb;
    if (comps != nil) {
      DrawImportPasteCmd* paste_cmd = new DrawImportPasteCmd(GetEditor(), new Clipboard(comps));
      paste_cmd->Execute();
      paste_cmd->Log();
      
      if(!comps->IsA(FRAME_IDRAW_COMP) && !comps->IsA(DRAW_IDRAW_COMP)) {
	if (chooser_->centered())
	  GetEditor()->GetViewer()->Align(comps, /* Center */ 4);
	
	if (!chooser_->by_pathname()) {
	  FrameUngroupCmd* ungroup_cmd = new FrameUngroupCmd(GetEditor());
	  ungroup_cmd->Execute();
	  MacroCmd* macro_cmd = new MacroCmd(GetEditor(), paste_cmd, ungroup_cmd);
	  macro_cmd->Log();
	} else {
	  paste_cmd->Log();
        }
      } else 
	delete comps;
    }
} 



GraphicComp* DrawImportCmd::Import (const char* pathname) {
  GraphicComp* comp = nil;
  const char* creator = ReadCreator(pathname);
  DrawCatalog* catalog = (DrawCatalog*)unidraw->GetCatalog();
  
  if (strcmp(creator, "drawserv") == 0) {
    catalog->SetImport(true);
    if (catalog->DrawCatalog::Retrieve(pathname, (Component*&) comp)) {
      catalog->SetImport(false);
      catalog->Forget(comp);
      return comp;
    }
    catalog->SetImport(false);
    return nil;
  }
  else if (strcmp(creator, "graphdraw") == 0 || 
	   strcmp(creator, "netdraw") == 0 || 
	   strcmp(creator, "graph-idraw") == 0) {
    static GraphCatalog *graphcatalog = new GraphCatalog("GraphCatalog", new GraphCreator());;
    graphcatalog->SetImport(true);
    if (graphcatalog->Retrieve(pathname, (Component*&) comp)) {
      graphcatalog->SetImport(false);
      graphcatalog->Forget(comp);
      
      if (chooser_->by_pathname()) {
	return new NodeComp((GraphComp*)comp);
      } else
	return comp;
    }
    catalog->SetImport(false);
    return nil;
  } else 
    return FrameImportCmd::Import(pathname);
}

/*****************************************************************************/

static NodeComp* node (Clipboard* cb, int index) {
    if (index == -1)
	return nil;

    Iterator i;
    int count = -1;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicComp* gcomp = cb->GetComp(i);
	if (gcomp->IsA(NODE_COMP)) {
	    count++;
	    if (count == index)
	        return (NodeComp*)gcomp;
        }
    }
    return nil;
}

ClassId DrawImportPasteCmd::GetClassId () { return DRAWIMPORTPASTE_CMD; }

boolean DrawImportPasteCmd::IsA (ClassId id) { 
    return DRAWIMPORTPASTE_CMD==id || FrameImportPasteCmd::IsA(id);
}

DrawImportPasteCmd::DrawImportPasteCmd (ControlInfo* c, Clipboard* cb) : FrameImportPasteCmd(c, cb) {
}

DrawImportPasteCmd::DrawImportPasteCmd (Editor* ed, Clipboard* cb) : FrameImportPasteCmd(ed, cb) {
}

DrawImportPasteCmd::~DrawImportPasteCmd () {
}

Command* DrawImportPasteCmd::Copy () {
    Command* copy = new DrawImportPasteCmd(CopyControlInfo(), DeepCopyClipboard());
    InitCopy(copy);
    return copy;
}

void DrawImportPasteCmd::Execute () {
  if(!_executed) {
    Clipboard* cb = GetClipboard();
    Iterator it;
    cb->First(it);
    GraphicComp* gcomp = cb->GetComp(it);
    cb->Next(it);
    if(cb->Done(it) && gcomp->IsA(DRAW_IDRAW_COMP) || gcomp->IsA(FRAME_IDRAW_COMP))
      {
	gcomp->First(it);

	/* move to background frame */
	DrawEditor* ed = (DrawEditor*)GetEditor();
	FrameNumberState* fnumstate = ed->framenumstate();
	int origfnum = fnumstate->framenumber();
	int currfnum = 0;
	Append(new MoveFrameCmd(ed, -origfnum, true /* allowbg */));
	
	/* paste contents of background frame */
	FrameComp* fcomp = (FrameComp*) (gcomp->GetComp(it)->IsA(FRAME_COMP) ? gcomp->GetComp(it) : nil);
	if (fcomp) {

	  while(!gcomp->Done(it)) {
	    gcomp->Remove(it);
	    Clipboard* newcb = new Clipboard();
	    Iterator jt;
	    fcomp->First(jt);
	    while(!fcomp->Done(jt)) {
	      newcb->Append(fcomp->GetComp(jt));
	      fcomp->Remove(jt);
	    }
	    Append(new PasteCmd(ed, newcb));
	    delete fcomp;
	  
	  /* while more frames move to next frame and paste (create new frame if necessary) */
	    if(!gcomp->Done(it)) {
	      currfnum++;
	      fcomp = (FrameComp*) (gcomp->GetComp(it)->IsA(FRAME_COMP) ? gcomp->GetComp(it) : nil);
	      if(currfnum>=ed->NumFrames()) 
		Append(new CreateMoveFrameCmd(ed));
	      else
		Append(new MoveFrameCmd(ed, 1, true /* allowbg */));
	    }
	  }
	}

	/* move to original frame */
	Append(new MoveFrameCmd(ed, origfnum-currfnum, true /* allowbg */));
      }
    
    else {
      Append(new PasteCmd(GetEditor(), cb->Copy()));
      Iterator i;
      for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        GraphicComp* gcomp = cb->GetComp(i);
        if (gcomp->IsA(EDGE_COMP)) {
	  EdgeComp* comp = (EdgeComp*)gcomp;
	  NodeComp* start = node(cb, comp->GetStartNode());
	  NodeComp* end = node(cb, comp->GetEndNode());
	  EdgeConnectCmd* cmd = new EdgeConnectCmd(GetEditor(), comp, start, end);
	  Append(cmd);
        }
      }
    }
  }
  MacroCmd::Execute();
  _executed = 1;
}

