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

// ### needed for Display constructor, we need to derive a subclass
#define protected public

#include "Xd.h"
#include <X11/Intrinsic.h>
#include "Xud.h"

#include <IV-X11/Xlib.h>
#include <IV-X11/xdisplay.h>

#include "widgetwindow.h"
#include "glyphwidgetP.h"
#include "xtudsession.h"

#include <Unidraw/ulist.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/Commands/macro.h>

#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovcreator.h>

#include <InterViews/resource.h>
#include <InterViews/display.h>

#include <IV-2_6/InterViews/sensor.h>

// ###
// the way the World class was designed it makes no sense that _session and 
// _display are private :-(
#define private public
#include <IV-2_6/InterViews/world.h>


class XtWorld : public World {
public:
    XtWorld(Session*, Display*);
    virtual ~XtWorld();
};


XtWorld::XtWorld(
    Session* s, Display* d
)
    : World()
{
    session_ = s;
    display_ = d;
    make_current();
    Sensor::init();
}

XtWorld::~XtWorld() {
    session_ = nil;
    display_ = nil;
}


// --------------------------------------------------------------------------


XtUnidraw::XtUnidraw(
    Catalog* c, World* w
)
    : OverlayUnidraw(c, w)
{
}


XtUnidraw& XtUnidraw::instance() {
    return *((XtUnidraw*)unidraw);
}


void XtUnidraw::Open(Editor* ed) {
    // we need a parent widget?
    // we should create a simple top level widget and use that
}


void XtUnidraw::Open(Editor* ed, Widget parent) {

    Widget widget = XtVaCreateManagedWidget(
            "default_name",           /* widget name */
            GlyphWidgetWidgetClass,   /* widget class */
            parent,                   /* parent widget*/
            NULL);                    /* terminate varargs list */

    initWidget(widget, ed);

    _GlyphWidgetRec* w = (_GlyphWidgetRec*)widget;
    ed->SetWindow(w->iv.window);

    // Xt handles mapping 
    // w->map();

    _editors->Append(new UList(ed));
    Resource::ref(ed);
    ed->Open();
}


void XtUnidraw::Close(Editor* ed) {
    // we probably don't want to call unmap here
    OverlayUnidraw::Close(ed);
}


void XtUnidraw::workProc() {

//    updated(false);

    // ### missing "updated" code

    Iterator it;
    for (_cmdq->First(it); !_cmdq->Done(it); _cmdq->First(it)) {
       CommandDoer doer(_cmdq->GetCommand(it));
       doer.Do();
       _cmdq->Remove(_cmdq->GetCommand(it));
    }

    Process();
    Sweep();

    if (updated()) {
        Update(true);
    }
}



// --------------------------------------------------------------------------


XtUdSession::XtUdSession(
    const char* name, int& argc, char** argv, XtAppContext app, Catalog* c, 
    const OptionDesc* options, const PropertyData* prop, Display* dis
)
    :XtSession(name, argc, argv, options, prop, app, dis)
{
    World* w = new XtWorld(this, default_display());
    Unidraw* ud = new XtUnidraw(c, w);
}

#if 0
XtUdSession::~XtUdSession() {
    // delete the global unidraw; this will delete the World and Catalog
    delete unidraw;
    unidraw = nil;
}
#endif


XtUdSession::initialize(
    const char* name, int& argc, char** argv, XDisplay* xdpy,
    const OptionDesc* options, const PropertyData* prop
) {
    OverlayCreator* creator = new OverlayCreator;

    DisplayRep* drep = new DisplayRep;
    drep->init(xdpy);
    Display* dpy = new Display(drep);

    XtUdSession* s = new XtUdSession(
        name, argc, argv, XtDisplayToApplicationContext(xdpy), 
        new OverlayCatalog("drawtool", creator), options, prop, dpy
    );

    // ### creator never get deleted in this case
}


XtUdSession::initialize(
    const char* name, int& argc, char** argv, XDisplay* xdpy, Catalog* c, 
    const OptionDesc* options, const PropertyData* prop
) {
    XtUdSession* s = new XtUdSession(
        name, argc, argv, XtDisplayToApplicationContext(xdpy), c, options, prop
    );
}


void XtUdSession::workProc() {
    XtSession::workProc();
    XtUnidraw::instance().workProc();
}

