/*
 * Copyright (c) 1998 R.B. Kissh & Associates
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


#include <stdlib.h>
#include <assert.h>

#include "Xd.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "Xud.h"

#include <stream.h>

#include <IV-X11/Xlib.h>
#include <InterViews/window.h>
#include <InterViews/event.h>
#include <InterViews/display.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xevent.h>
#include <IV-X11/xwindow.h>
#include <IV-X11/xcanvas.h>
#include <Dispatch/dispatcher.h>

#include "xtsession.h"


XtSession::XtSession(
    const char* name, int& argc, char** argv, const OptionDesc* options,
    const PropertyData* prop, XtAppContext app, Display* display
):
    Session(name, argc, argv, options, prop, display), _app_context(app)
{
#if 1
    // this is a bit of a hack here
    // we don't want IV to get called back for the X connection events
    // let Xt handle this
    Dispatcher::instance().unlink(
      ConnectionNumber(default_display()->rep()->display_)
    );
#endif

    xt_init();

    XtAppAddWorkProc(_app_context, &XtSession::doWorkProc, this);
}


XtfBoolean XtSession::doWorkProc(XtPointer p) {
    XtSession* s = (XtSession*)p;
    s->workProc();
    return FALSE;
}


void XtSession::workProc() {
    default_display()->repair();
}


XtAppContext XtSession::get() const {
    return _app_context;
}


void XtSession::set(
  XtAppContext app
) {
    _app_context = app;
}


void XtSession::loopHook() {
}


int XtSession::run() {
    Event e;
    do {
        read(e);
        e.handle();

        loopHook();

    } while (!done());
    return 0;
}


static XtEventDispatchProc orig_procs[LASTEvent];

Boolean XtDispatchProc(
  XEvent* xe 
) {

  // if the event is an IV event then process else let Xt process 

  Session& session = *Session::instance();
  Display* dpy = session.default_display();
  DisplayRep* dpy_rep = dpy->rep();
  XDisplay* xdpy = dpy_rep->display_;

  if (WindowRep::find(xe->xany.window, dpy_rep->wtable_)) {
    XPutBackEvent(xdpy, xe);

    // should always succeed, since there is an event pending
    Event e;
    if (dpy->get(e)) {
      e.handle();
    }
  }
  else {
    // let the orig Xt routine handle it
    assert( (xe->type > 0) && (xe->type < LASTEvent) );

    (*orig_procs[xe->type])(xe);
  }
}


void XtSession::xt_init() {
  Display* dpy = default_display();
  DisplayRep* dpy_rep = dpy->rep();
  XDisplay* xdpy = dpy_rep->display_;

  int i;
  for( i = 0; i < LASTEvent; i++ ) { 
    orig_procs[i] = XtSetEventDispatcher(xdpy, i, XtDispatchProc);
  }
}


// ### too bad about this
declarePtrList(DamageList,Window)

void XtSession::read(
  Event& e, boolean (*test)()
) {
  // #### ignoring test

  XEvent xe;
  Display* dpy = default_display();
  DisplayRep* dpy_rep = dpy->rep();
  XDisplay* xdpy = dpy_rep->display_;

  boolean found = false;

  // note that we don't set rep_->readinput_ to false here because we are 
  // assuming that the dispatcher only does timers and not input ready on a fd

  while (!done() && !found) {

    // check this to see if it really needs to be here
    if (dpy_rep->damaged_->count() != 0 && QLength(xdpy) == 0) {
        dpy->repair();
    }

    XtAppNextEvent(_app_context, &xe);

    if (WindowRep::find(xe.xany.window, dpy_rep->wtable_)) {
      XPutBackEvent(xdpy, &xe);
      if (dpy->get(e)) {
        found = true;
      }
    }
    else {
      XtDispatchEvent(&xe);
    }
  }
}


