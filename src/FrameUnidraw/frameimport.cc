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
 * FrameImportCmd implementation.
 */

#include <FrameUnidraw/framecatalog.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/frameeditor.h>
#include <FrameUnidraw/frameimport.h>
#include <FrameUnidraw/framestates.h>

#include <IVGlyph/importchooser.h>

#include <Unidraw/clipboard.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <string.h>

/*****************************************************************************/

ClassId FrameImportCmd::GetClassId () { return FRAMEIMPORT_CMD; }

boolean FrameImportCmd::IsA (ClassId id) {
    return FRAMEIMPORT_CMD == id || OvImportCmd::IsA(id);
}

FrameImportCmd::FrameImportCmd (ControlInfo* c, ImportChooser* f) : OvImportCmd(c, f) { 
}

FrameImportCmd::FrameImportCmd (Editor* ed, ImportChooser* f) : OvImportCmd(ed, f) { 
}

Command* FrameImportCmd::Copy () {
    FrameImportCmd* copy = new FrameImportCmd(CopyControlInfo(), chooser_);
    InitCopy(copy);
    return copy;
}

void FrameImportCmd::Execute () { 
  GraphicComp* comps = PostDialog();
  
  Clipboard* cb;
  if (comps != nil) {
    FrameImportPasteCmd* paste_cmd = new FrameImportPasteCmd(GetEditor(), new Clipboard(comps));
    paste_cmd->Execute();
    paste_cmd->Log();
    
    if (!comps->IsA(FRAME_IDRAW_COMP)) {
      if(chooser_->centered())
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

GraphicComp* FrameImportCmd::Import (const char* pathname) {
    GraphicComp* comp = nil;
    const char* creator = ReadCreator(pathname);
    if(!creator) return nil;
    FrameCatalog* catalog = (FrameCatalog*)unidraw->GetCatalog();

    if (strcmp(creator, "flipbook") == 0 || 
	strcmp(creator, "frame-idraw") == 0) {
        catalog->SetImport(true);
        if (catalog->FrameCatalog::Retrieve(pathname, (Component*&) comp)) {
            catalog->SetImport(false);
	    catalog->Forget(comp);
	    return comp;
	}
        catalog->SetImport(false);
	return nil;
    } else 
        return OvImportCmd::Import(pathname);
}

/*****************************************************************************/

ClassId FrameImportPasteCmd::GetClassId () { return FRAMEIMPORTPASTE_CMD; }

boolean FrameImportPasteCmd::IsA (ClassId id) { 
    return FRAMEIMPORTPASTE_CMD==id || MacroCmd::IsA(id);
}

FrameImportPasteCmd::FrameImportPasteCmd (ControlInfo* c, Clipboard* cb) : MacroCmd(c) {
    SetClipboard(cb);
    _executed = 0;
}

FrameImportPasteCmd::FrameImportPasteCmd (Editor* ed, Clipboard* cb) : MacroCmd(ed) {
    SetClipboard(cb);
    _executed = 0;
}

FrameImportPasteCmd::~FrameImportPasteCmd () {
}

Command* FrameImportPasteCmd::Copy () {
    Command* copy = new FrameImportPasteCmd(CopyControlInfo(), DeepCopyClipboard());
    InitCopy(copy);
    return copy;
}

void FrameImportPasteCmd::Execute () {
  if(!_executed) {
    Clipboard* cb = GetClipboard();
    Iterator it;
    cb->First(it);
    GraphicComp* gcomp = cb->GetComp(it);
    cb->Next(it);
    if(cb->Done(it) && gcomp->IsA(FRAME_IDRAW_COMP))
      {
	gcomp->First(it);

	/* move to background frame */
	FrameEditor* ed = (FrameEditor*)GetEditor();
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
    
    else  
      Append(new PasteCmd(GetEditor(), cb->Copy()));
  }
  MacroCmd::Execute();
  _executed = 1;
}

