/*
 * Copyright (c) 2004 Scott E. Johnston
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
 * Implementation of LinkSelection class.
 */

#ifdef HAVE_ACE

#include <DrawServ/draweditor.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>
#include <Unidraw/viewer.h>

#include <ComTerp/comhandler.h>
#include <ComTerp/comterpserv.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <IV-2_6/InterViews/world.h>

#include <stdio.h>

GraphicIdList* LinkSelection::_locally_selected = nil;
GraphicIdList* LinkSelection::_waiting_to_be_selected = nil;
const char* LinkSelection::_selected_strings[] =  { "NotSelected", "LocallySelected", "RemotelySelected", "WaitingToBeSelected"};

/*****************************************************************************/

LinkSelection::LinkSelection (DrawEditor* editor, LinkSelection* s) : OverlaySelection(s) {
  Init(editor);
}

LinkSelection::LinkSelection (DrawEditor* editor, Selection* s) : OverlaySelection(s) {
  Init(editor);
}

void LinkSelection::Init(DrawEditor* editor) {
  _editor = editor;
  if (!_locally_selected) {
    _locally_selected = new GraphicIdList;
    _waiting_to_be_selected = new GraphicIdList;
  }
  waiting_count() = granted_count() = 0;
  wtbs_flag() = remote_flag() = paste_in_progress_flag() = false;
}

void LinkSelection::Update(Viewer* viewer) {
#if 0
  fprintf(stderr, "LinkSelection::Update\n");
#endif
  Reserve();
  OverlaySelection::Update(viewer);
}

void LinkSelection::Clear(Viewer* viewer) {
#if 0
  fprintf(stderr, "LinkSelection::Clear\n");
#endif
#if 0
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();
  Iterator it;
  First(it);
  while(!Done(it)) {
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    void* ptr = nil;
    table->find(ptr, (void*)comp);
    if (ptr) {
      GraphicId* grid = (GraphicId*)ptr;
      if (grid->selected()==LocallySelected || grid->selected()==WaitingToBeSelected) 
	grid->selected(NotSelected);
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, "grid(\"%s\" \"%s\" :state %d)%c",
	       grid->idstr(), grid->selectorstr(), grid->selected(), '\0');
      ((DrawServ*)unidraw)->DistributeCmdString(buf);
    }
    Next(it);
  }
#endif
  OverlaySelection::Clear(viewer);
  Reserve();
}

void LinkSelection::Reserve() {
#if 0
  fprintf(stderr, "LinkSelection::Reserve\n");
#endif
  CompIdTable* table = ((DrawServ*)unidraw)->compidtable();
  wtbs_flag() = false;
  remote_flag() = false;

  /* clear anything that was in the previous selection, but not in this one */
  Selection* lastsel = _editor->last_selection();
  #if 0
  fprintf(stderr, "\n\nSELECTION: **********************************\n");
  fprintf(stderr, "SELECTION: last_selection() size==%d\n", _editor->last_selection()->Number());
  fprintf(stderr, "SELECTION: waiting_to_be_selectedection size==%d\n", Number());
  #endif
  if (!lastsel) return;
  Iterator lt;
  lastsel->First(lt);
  Iterator it;
  while (!lastsel->Done(lt)) {
    First(it);
    boolean match = false;
    while (!Done(it) && !match) {
      if(GetView(it)==lastsel->GetView(lt)) 
	match = true;
      else 
	Next(it);
    }
    if (!match) {
      OverlayComp* comp = ((OverlayView*)lastsel->GetView(lt))->GetOverlayComp();
      void* ptr = nil;
      table->find(ptr, (void*)comp);
      if (ptr) {
	GraphicId* grid = (GraphicId*)ptr;
	grid->selected(NotSelected);
	((DrawServ*)unidraw)->grid_message(grid);
      }
    }
    lastsel->Remove(lt);
  }


  /* remove anything from selection that has a remote selector */
  First(it);
  while (!Done(it)) {
    int removed = false;
    OverlayView* view = GetView(it);
    OverlayComp* comp = view ? view->GetOverlayComp() : nil;
    void* ptr = nil;
    table->find(ptr, (void*)comp);
    if (ptr) {
      GraphicId* grid = (GraphicId*)ptr;

      /* self-owned NotSelected: reclaim directly without request */
      if (grid->selected()==NotSelected && 
	  !uuid_is_null(grid->selector()) &&
	  uuid_compare(grid->selector(), ((DrawServ*)unidraw)->sessionid())==0) {
        grid->selected(LocallySelected);
        ((DrawServ*)unidraw)->grid_message(grid);
      }

      /* remote selector */
      else if (!uuid_is_null(grid->selector()) && 
	       uuid_compare(((DrawServ*)unidraw)->sessionid(), grid->selector())) {
	
	Remove(it);
	removed = true;
	
	if (grid->selected()==RemotelySelected) {
	  remote_flag() = true; // immediate refusal, remote deteted
	}

	if (grid->selected()==NotSelected) {
	  /* make a request to select this in the future */
	  /* mostly as a way of confirming it being put into RemotelySelected */
	  grid->selected(WaitingToBeSelected);
	  if (!paste_in_progress_flag()) {
	    waiting_count()++;
	    wtbs_flag() = true;
	  }
	  ((DrawServ*)unidraw)->grid_message(grid);
	} 
	
      } else {
	if (grid->selected()!=LocallySelected) {
	  grid->selected(WaitingToBeSelected);
	  if (!paste_in_progress_flag()) {
	    waiting_count()++;
	    wtbs_flag() = true;
	  }
	  ((DrawServ*)unidraw)->grid_message(grid);
	}
      }
      
    }
    if (!removed) 
      Next(it);
  }
  
  // beep or ding if known immediately
  if (waiting_count() == 0) {
    if (remote_flag() && !wtbs_flag()) {
      Beep(FILELINE);
    }  else if (wtbs_flag()) {
      Ding(FILELINE);
    }
  }
  
  /* store copy of selection */
  First(it);
  while (!Done(it)) {
    lastsel->Append(GetView(it));
    Next(it);
  }
  #if 0
  fprintf(stderr, "SELECTION: **********************************\n\n\n");
  #endif
}

void LinkSelection::AddComp(OverlayComp* comp) {
  OverlayView* ov = comp->FindView(_editor->GetViewer());
  Append(ov);
  Update(_editor->GetViewer());
}

void LinkSelection::Beep(const char* debugstr) {
  ComterpHandler* handler = ((DrawServHandler*)((DrawEditor*)_editor)->GetComTerp()->handler());
  if (handler) {
    int len;
    
    char buffer[BUFSIZ];
    snprintf(buffer, BUFSIZ, "BEEP: remote_flag=%d wtbs_flag=%d waiting=%d granted=%d paste_in_progress_flag=%d",
	     remote_flag(), wtbs_flag(), waiting_count(), granted_count(), paste_in_progress_flag());
    if (debugstr!=NULL) {
      len = strlen(buffer);
      snprintf(buffer+len, BUFSIZ-len, " (%s)", debugstr);
    }
    len = strlen(buffer);
    snprintf(buffer+len, BUFSIZ-len, "\n");
    log_with_timestamp(buffer);
  }
  
  ((OverlayEditor*)_editor)->Beep();
}

void LinkSelection::Ding(const char* debugstr) {
  ComterpHandler* handler = ((DrawServHandler*)((DrawEditor*)_editor)->GetComTerp()->handler());
  if (handler) {
    int len;
    
    char buffer[BUFSIZ];
    snprintf(buffer, BUFSIZ, "DING: remote_flag=%d wtbs_flag=%d waiting=%d granted=%d paste_in_progress_flag=%d",
	     remote_flag(), wtbs_flag(), waiting_count(), granted_count(), paste_in_progress_flag());
    if (debugstr!=NULL) {
      len = strlen(buffer);
      snprintf(buffer+len, BUFSIZ-len, " (%s)", debugstr);
    }
    len = strlen(buffer);
    snprintf(buffer+len, BUFSIZ-len, "\n");
    log_with_timestamp(buffer);
  }
  
  ((OverlayEditor*)_editor)->Ding();
}


// return values:
//  0 = still waiting (more requests in flight)
//  1 = all resolved, at least one granted (ding)
// -1 = all resolved, all denied (beep)
int LinkSelection::all_requests_resolved(boolean granted) {
  if (granted)
    granted_count()++;
  if (waiting_count() > 0)
    waiting_count()--;
  if (waiting_count() == 0) {
    boolean all_denied = granted_count() == 0 && !wtbs_flag();
    wtbs_flag() = false;
    remote_flag() = false;
    granted_count() = 0;
    return all_denied ? -1 : 1;
  }
  return 0;  // still waiting for more responses
}

boolean LinkSelection::request_resolved_check(boolean granted, const char* fileline) {
  if (waiting_count() > 0) {
    int status = all_requests_resolved(granted);
    if (status==-1)
      Beep(fileline);
    else if (status==1)
      Ding(fileline);
    return true;
  } else
    return false;
}

// in LinkSelection override
void LinkSelection::CopyFlags(OverlaySelection* from) {
  LinkSelection* ls = (LinkSelection*)from;
  if (ls) {
    waiting_count() = ls->waiting_count();
    granted_count() = ls->granted_count();
    wtbs_flag() = ls->wtbs_flag();
    remote_flag() = ls->remote_flag();
    paste_in_progress_flag() = ls->paste_in_progress_flag();
  }
}

#endif
