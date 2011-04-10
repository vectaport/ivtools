/*
 * Copyright (c) 1994 Vectaport Inc.
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
 * Implementation of OverlayUnidraw class.
 */

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovpainter.h>
#include <OverlayUnidraw/ovstates.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovviewer.h>

#include <Unidraw/Commands/dirty.h>
#include <Unidraw/Commands/macro.h>

#include <Unidraw/Components/component.h>

#include <Unidraw/editor.h>
#include <Unidraw/globals.h>
#include <Unidraw/iterator.h>
#include <Unidraw/ulist.h>
#include <Unidraw/Graphic/graphic.h>

#include <InterViews/display.h>
#include <InterViews/event.h>
#include <InterViews/resource.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <IV-2_6/InterViews/world.h>
#include <IV-X11/xevent.h>
#include <IV-X11/xwindow.h>

#include <ComTerp/comterpserv.h>

#include <iostream.h>

MacroCmd* OverlayUnidraw::_cmdq = nil;
boolean* OverlayUnidraw::_updated_ptr = nil;
ComTerpServ* OverlayUnidraw::_comterp = nil;
int OverlayUnidraw::_npause = nil;

/*****************************************************************************/

OverlayUnidraw::OverlayUnidraw (Catalog* c, int& argc, char** argv, 
				OptionDesc* od, PropertyData* pd) 
: Unidraw(c, argc, argv, od, pd) {
    _cmdq = new MacroCmd();
    _ovviewer = nil;
    _comterp = nil;
    _npause = 0;

    /* replace default Painter with an OverlayPainter */
    OverlayGraphic::new_painter();
}

OverlayUnidraw::OverlayUnidraw (Catalog* c, World* w) 
: Unidraw(c, w) {
    _cmdq = new MacroCmd();
    _ovviewer = nil;
    _comterp = nil;
    _npause = 0;
}

OverlayUnidraw::~OverlayUnidraw () 
{
    delete _cmdq;
    OverlayPainter::FreeCache();
}

void OverlayUnidraw::Append(Command* cmd) {
    _cmdq->Append(cmd);
}

boolean OverlayUnidraw::unidraw_updated() 
{ return *_updated_ptr; }
boolean OverlayUnidraw::npause_lessened() 
{ return _comterp && _npause > _comterp->npause(); }

boolean OverlayUnidraw::unidraw_updated_or_command_pushed() 
{ 
  Iterator it;
  _cmdq->First(it);
  return !_cmdq->Done(it) || unidraw_updated();
}

boolean OverlayUnidraw::unidraw_updated_or_command_pushed_or_npause_lessened() 
{ 
  Iterator it;
  _cmdq->First(it);
  return !_cmdq->Done(it) || unidraw_updated() || npause_lessened();
}

void OverlayUnidraw::Run () {
    Session* session = GetWorld()->session();
    Event e;
    Iterator it;
    alive(true);
    _npause = _comterp ? _comterp->npause() : 0;

    while (alive() && !session->done() && !npause_lessened()) {
	updated(false);

	_updated_ptr = &_updated;
//	session->read(e, &unidraw_updated);
//	session->read(e, &unidraw_updated_or_command_pushed);
	session->read(e, &unidraw_updated_or_command_pushed_or_npause_lessened);
	if (!updated()) {
	    e.handle();
	    session->default_display()->flush();
	}

	for (_cmdq->First(it); !_cmdq->Done(it); _cmdq->First(it)) {
 	    unidraw->ExecuteCmd(_cmdq->GetCommand(it)->Copy());
	    _cmdq->Remove(_cmdq->GetCommand(it));
	}

	Process();
	Sweep();

	if (updated()) {
	    Update(true);
	}
    }
    _npause = _comterp ? _comterp->npause() : 0;

}

void OverlayUnidraw::Log (Command* cmd, boolean dirty) {
    if (cmd->Reversible()) {
        Editor* ed = cmd->GetEditor();
        Component* comp = ed->GetComponent()->GetRoot();

        UList* past, *future;
        GetHistory(comp, past, future);

        Resource::ref(ed);
        ClearHistory(future);

	if (IsClean(ed) && dirty) {
	    DirtyCmd* dc = new DirtyCmd(ed);
	    dc->Execute();
            cmd = new MacroCmd(ed, cmd, dc);
	}
	    
        past->Prepend(new UList(cmd));
        ClearHistory(past, _histlen+1);
    }
}

/* static */ void OverlayUnidraw::pointer_tracker_func(Event& e) {
  if (e.type() != Event::motion || !e.window()) return;
  PixelCoord x = e.rep()->xevent_.xmotion.x;
  PixelCoord y = e.rep()->xevent_.xmotion.y;
  WindowRep& ewr = *e.window()->rep();
  Iterator i;
  for (unidraw->First(i); !unidraw->Done(i); unidraw->Next(i)) {
    OverlayEditor* ed = (OverlayEditor*) unidraw->GetEditor(i);
    Viewer* v; if (!(v = ed->GetViewer())) continue;
    Canvas* c; if (!(c = v->GetCanvas())) continue;
    Window* cw; if (!(cw = c->window())) continue;

    /* if event is associated with the ApplicationWindow */
    if (ed->GetWindow() == e.window()) {

      /* adjust for delta between application window and Unidraw canvas */
      PixelCoord deltax, deltay;
      WindowRep& cwr = *cw->rep();
      if (cwr.xpos_) // this test requires a window setup where xpos_ is zero after Window::bind
      {
	deltax = cwr.xpos_ - ewr.xpos_;
	deltay = cwr.ypos_ - ewr.ypos_;
      } else {
	cw->offset_from_toplevel(deltax, deltay);
      }
      x -= deltax;
      y -= deltay;
      ed->ptrlocstate()->ptrcoords(x, cwr.canvas_->pheight() - y - 1);
      break;
    }

    /* else if event is associated with the Unidraw canvas */
    else if (cw == e.window()) {

      /* coordinates will already be in the canvas coordinate system */
      ed->ptrlocstate()->ptrcoords(x, ewr.canvas_->pheight() - y - 1);
      break;

    }
  }
}








