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

#include <DrawServ/draweditor.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/grid.h>
#include <DrawServ/gridlist.h>
#include <DrawServ/linkselection.h>

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>

#include <Unidraw/iterator.h>
#include <Unidraw/viewer.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <stdio.h>

GraphicIdList* LinkSelection::_locally_selected = nil;
GraphicIdList* LinkSelection::_waiting_to_be_selected = nil;
char* LinkSelection::_selected_strings[] =  { "NotSelected", "LocallySelected", "RemotelySelected", "WaitingToBeSelected"};

/*****************************************************************************/

LinkSelection::LinkSelection (DrawEditor* editor, LinkSelection* s) : OverlaySelection(s) {
  _editor = editor;
  if (!_locally_selected) {
    _locally_selected = new GraphicIdList;
    _waiting_to_be_selected = new GraphicIdList;
  }
}

LinkSelection::LinkSelection (DrawEditor* editor, Selection* s) : OverlaySelection(s) {
  _editor = editor;
  if (!_locally_selected) {
    _locally_selected = new GraphicIdList;
    _waiting_to_be_selected = new GraphicIdList;
  }
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
      snprintf(buf, BUFSIZ, "grid(chgid(0x%08x) chgid(0x%08x) :state %d)%c",
	       grid->id(), grid->selector(), grid->selected(), '\0');
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

  /* clear anything that was in the previous selection, but not in this one */
  Selection* lastsel = _editor->last_selection();
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
      if (grid->selector() && 
	  ((DrawServ*)unidraw)->sessionid()!=grid->selector()) {
	
	Remove(it);
	removed = true;
	
	if (grid->selected()==NotSelected) {
	  
	  /* make a request to select this in the future */
	  grid->selected(WaitingToBeSelected);
	  ((DrawServ*)unidraw)->grid_message(grid);
	} 
	
      } else {
	if (grid->selected()!=LocallySelected) {
	  grid->selected(LocallySelected);
	  ((DrawServ*)unidraw)->grid_message(grid);
	}
      }
      
    }
    if (!removed) 
      Next(it);
  }

  /* store copy of selection */
  First(it);
  while (!Done(it)) {
    lastsel->Append(GetView(it));
    Next(it);
  }
}

void LinkSelection::AddComp(OverlayComp* comp) {
  OverlayView* ov = comp->FindView(_editor->GetViewer());
  Append(ov);
  Update(_editor->GetViewer());
}
