//
// Simple Text Editor Buffer Implementation
//
// Copyright (C) 1993 Ellemtel Telecommunication Systems Labratories
//
// Permission is granted to any individual or institution to use, copy,
// modify, and distribute this software, provided that this complete
// copyright and permission notice is maintained, intact, in all copies
// and supporting documentation.
//
// Ellemtel Telecommunication Systems Labratories make no representation
// of the suitability of this software for any purpose. It is provided
// "as is" without any expressed or implied warranty.
//
// Jan Andersson, Torpa Konsult AB
// janne@torpa.se - 1993-08-29

#include <IVGlyph/textwindow.h>
#include <InterViews/selection.h>
#include <InterViews/event.h>
#include <IV-X11/Xlib.h>
#include <IV-X11/Xutil.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xevent.h>
#include <IV-X11/xselection.h>
#include <IV-X11/xwindow.h>

TextEditAppWindow::TextEditAppWindow(Glyph* g)
: ApplicationWindow(g)
{
}

void TextEditAppWindow::receive(const Event& e) 
{
   Atom atom;
   char* name;
   WindowRep& w = *Window::rep();
   DisplayRep& d = *w.display_->rep();
   XEvent& xe = e.rep()->xevent_;
   SelectionManager* s;
   switch (xe.type) {
   case SelectionRequest:
      // check type of selection
      atom = xe.xselectionrequest.selection;
      name = XGetAtomName(d.display_, atom);
      s = w.display_->find_selection(name);
      // request the selecton
      s->rep()->request(s, xe.xselectionrequest);
      XFree(name);
      break;
   case SelectionNotify:
      // check type of selection
      atom = xe.xselectionrequest.selection;
      name = XGetAtomName(d.display_, atom);
      s = w.display_->find_selection(name);
      // notify about the selection
      s->rep()->notify(s, xe.xselection);
      XFree(name);
   default:
      // pass everything else to Window::receive
      Window::receive(e);
      break;
   }
}
