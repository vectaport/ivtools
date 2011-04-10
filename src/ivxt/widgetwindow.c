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

/*
 * Copyright (c) 1993 2001 S.A.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * 2001 S.A. may not be used in any advertising or publicity relating to the
 * software without the specific, prior written permission of 2001 S.A.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL 2001 S.A. BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// find out what we really need

#include <IV-X11/Xlib.h>
#include <IV-X11/Xutil.h>
#include <IV-X11/Xext.h>
#include <IV-X11/xcanvas.h>
#include <IV-X11/xcursor.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xevent.h>
#include <IV-X11/xselection.h>
#include <IV-X11/xwindow.h>

#include <InterViews/glyph.h>
#include <InterViews/style.h>
#include <InterViews/session.h>
#include <InterViews/event.h>

#include "widgetwindow.h"
#include "glyphwidgetP.h"
#include "xtintrinsic.h"

#include <X11/StringDefs.h>

#include <OS/table.h>
#include <stdio.h>
#include <stdlib.h>

// ### this is gross, but we can't access the header
declareTable(WindowTable,XWindow,Window*)


WidgetWindow::WidgetWindow(
    _GlyphWidgetRec* w, Glyph* g
) 
      : ManagedWindow(g)
{
    widget_ = w;
}


WidgetWindow::~WidgetWindow() {
    // Multiple destroys don't hurt under Xt since they are deferred
    if (widget_ != nil) {
	XtDestroyWidget((Widget)widget_);
    }
    widget_ = nil;
}


void WidgetWindow::map() {
    if (!is_mapped()) {
	XtRealizeWidget((Widget)widget_);
	XtMapWidget((Widget)widget_);
    }
}


void WidgetWindow::compute_geometry() {
    GlyphWidgetRec& w = *widget_;
    WindowRep& wr = *((Window*)this)->rep();
    CanvasRep& c = *wr.canvas_->rep();
    Display& d = *wr.display_;

    // ### what does the Placement corres to in IV 3.1 ?
    // wr.placed_ = true;

    if (w.core.width == 0) {
	w.core.width = XCoord(c.pwidth_);
    } 
    else {
	c.pwidth_ = w.core.width;
        c.width_ = d.to_coord(c.pwidth_);

        // placement_.width = screen_->to_coord(pwidth_);
    }


    if (w.core.height == 0) {
	w.core.height = XCoord(c.pheight_);
    } 
    else {
	c.pheight_ = w.core.height;
        c.height_ = d.to_coord(c.pheight_);

        // placement_.height = screen_->to_coord(pheight_);
    }
}


void WidgetWindow::bind() {

/*
    WindowRep& wr = *rep();
    Display& d = *wr.display_;

    if (xbound(xwindow_)) {
	display_->unbind(xwindow_);
    }
    set_attributes();

    (*GlyphwidgetClassRec.core_class.superclass->core_class.realize)(
	(Widget)widget_, &xattrmask_, &xattrs_
    );

    xwindow_ = widget_->core.window;
    display_->bind(xwindow_, this);
*/

    WindowRep& wr = *((Window*)this)->rep();
    CanvasRep& c = *wr.canvas_->rep();
    Display& d = *wr.display_;
    WindowTable& t = *d.rep()->wtable_;

    if (wr.xwindow_ != WindowRep::unbound) {
        t.remove(wr.xwindow_);
    }
    set_attributes();

    (*GlyphwidgetClassRec.core_class.superclass->core_class.realize)(
	(Widget)widget_, &wr.xattrmask_, &wr.xattrs_
    );
    wr.xwindow_ = widget_->core.window;

    c.xdrawable_ = wr.xwindow_;
    t.insert(wr.xwindow_, this);
    wr.xtoplevel_ = wr.toplevel_->rep()->xwindow_;
}


void WidgetWindow::do_map() {
    WindowRep& wr = *((Window*)this)->rep();
    wr.map_pending_ = true;
}    


void WidgetWindow::xt_initialize(Widget, ArgList, Cardinal*) {
    default_geometry();
    compute_geometry();
}


void WidgetWindow::xt_realize(XtValueMask* mask, XSetWindowAttributes* attr) {
    WindowRep& wr = *((Window*)this)->rep();

    wr.xattrs_ = *attr;
    wr.xattrmask_ = *mask;

    bind();
    set_props();
    do_map();

    // ### why keep this?
    XtAddEventHandler(
	(Widget)widget_, StructureNotifyMask, FALSE,
	&WidgetWindow::xt_structure_event, this
    );
}


void WidgetWindow::xt_destroy() { }


XtGeometryResult WidgetWindow::xt_query_geometry(
    XtWidgetGeometry* intended, XtWidgetGeometry* preferred
) {
    WindowRep& wr = *((Window*)this)->rep();
    Display& d = *wr.display_;
    Glyph* g = glyph();

    if (!g) {
	*preferred = *intended;
	return XtGeometryYes;
    }

    Requisition req;
//    GlyphImpl::default_requisition(req);

    g->request(req);

    Coord w = req.requirement(Dimension_X).natural();
    Coord h = req.requirement(Dimension_Y).natural();

    // ### look into how the following works 
    XtGeometryMask mode = CWWidth | CWHeight;
    preferred->request_mode = mode;
    preferred->width = XCoord(d.to_pixels(w));
    preferred->height = XCoord(d.to_pixels(h));

    if ((intended->request_mode & mode) == mode &&
	intended->width == preferred->width &&
	intended->height == preferred->height
    ) {
	return XtGeometryYes;
    } 
    else if (preferred->width == widget_->core.width &&
	preferred->height == widget_->core.height
    ) {
	return XtGeometryNo;
    }

    return XtGeometryAlmost;
}


XtfBoolean WidgetWindow::xt_set_values(
    Widget /* req */, Widget /* new_w */, ArgList, Cardinal* /* num_args */
) {
    return False;
}


void WidgetWindow::xt_expose(XEvent* event, Region) {
/*
    XExposeEvent& xe = event->xexpose;
    ScreenRef s = screen_;
    Coord left = s->to_coord(xe.x);
    Coord top = s->to_coord(pheight_ - xe.y);
    Coord h = s->to_coord(xe.height);

    redraw(left, top - h, s->to_coord(xe.width), h);
*/

    WindowRep& wr = *((Window*)this)->rep();
    wr.expose(this, event->xexpose);
}


void WidgetWindow::set_attributes() {
/*
    if (visual_ == nil) {
	// This is wrong. We should use the visual contained in the widget.
	// For now, assume we don't mess with visuals.
	visual_ = screen_->find_visual((StyleImpl*)style_);
    }
*/

    WindowRep& wr = *((Window*)this)->rep();
    if (wr.visual_ == nil) {
        wr.visual_ = WindowVisual::find_visual(wr.display_, wr.style_);
    }

    wr.xattrmask_ |= CWEventMask;
    long events = (
	KeyPressMask | KeyReleaseMask |
	ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	EnterWindowMask | LeaveWindowMask |
	FocusChangeMask |
	OwnerGrabButtonMask
    );
    wr.xattrs_.event_mask = events | ExposureMask | StructureNotifyMask;

    // ### move this
    XtAddEventHandler(
	(Widget)widget_, events, FALSE, &WidgetWindow::xt_events, this
    );
}


void WidgetWindow::xt_events(
    Widget, XtPointer p, XEvent* xe, XtfBoolean*
) {
    WidgetWindow* w = (WidgetWindow*)p;
    WindowRep& wr = *((Window*)w)->rep();

    Event event;
    EventRep& e = *(event.rep());
    e.display_ = wr.display_;
    e.window_ = w;
    event.handle();
}


void WidgetWindow::xt_structure_event(
    Widget, XtPointer p, XEvent* xe, XtfBoolean*
) {
    WidgetWindow* w = (WidgetWindow*)p;
    GlyphWidgetRec* ww = w->widget_;

    WindowRep& wr = *((Window*)w)->rep();

    switch (xe->type) {
    case MapNotify:
	wr.map_notify(w, xe->xmap);
	break;
    case UnmapNotify:
	wr.unmap_notify(w, xe->xunmap);
	break;
    case ConfigureNotify:
	wr.configure_notify(w, xe->xconfigure);
	break;
    }
}

// --------------------------------------------------------------------------

void initWidget(Widget widget, Glyph* g) {
  _GlyphWidgetRec* w = (_GlyphWidgetRec*)widget;

  // init_window code
  WidgetWindow* xtw = new WidgetWindow(w, g);

  // ### should these next 2 lines be here?
  xtw->display(Session::instance()->default_display());
  xtw->style(new Style(xtw->display()->style()));

  xtw->xt_initialize(widget, nil, nil);
  w->iv.window = xtw;
}

// --------------------------------------------------------------------------

/* Initialization of defaults */
/* Private Data */

#define offset(field) XtOffsetOf(GlyphWidgetRec, field)

static XtResource resources[] = {
/*
    {XtNfresco, XtCFresco, XtRFresco, sizeof(Fresco*),
	offset(iv.fresco), XtRImmediate, NULL},
*/
    {XtNglyph, XtCGlyph, XtRGlyph, sizeof(Glyph*),
	offset(iv.glyph), XtRImmediate, NULL},
    {XtNfrescoWindow, XtCFrescoWindow, XtRFrescoWindow, sizeof(WidgetWindow*),
	offset(iv.window), XtRImmediate, NULL},
};

static void Realize(Widget, XtValueMask*, XSetWindowAttributes*);
static void Resize(Widget);
static void Destroy(Widget);
static XtGeometryResult QueryGeometry(
    Widget,XtWidgetGeometry*,XtWidgetGeometry*
);
static void ExposeProc(Widget, XEvent*, Region);
static XtfBoolean SetValues(Widget, Widget, Widget, ArgList, Cardinal*);

GlyphWidgetClassRec GlyphwidgetClassRec = {
  {

/* core_class fields */	

#ifndef SVR3SHLIB
#define superclass		(&widgetClassRec)
#else
#define superclass		NULL
#endif
    /* superclass	  	*/	(WidgetClass) superclass,
    /* class_name	  	*/	"Label",
    /* widget_size	  	*/	sizeof(GlyphWidgetRec),
    /* class_initialize   	*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited       	*/	FALSE,
    /* initialize	  	*/	NULL,
    /* initialize_hook		*/	NULL,
    /* realize		  	*/	Realize,
    /* actions		  	*/	NULL,
    /* num_actions	  	*/	0,
    /* resources	  	*/	resources,
    /* num_resources	  	*/	XtNumber(resources),
    /* xrm_class	  	*/	NULLQUARK,
    /* compress_motion	  	*/	TRUE,
    /* compress_exposure  	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest	  	*/	FALSE,
    /* destroy		  	*/	Destroy,
    /* resize		  	*/	NULL,
    /* expose		  	*/	ExposeProc,
    /* set_values	  	*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus	 	*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private   	*/	NULL,
    /* tm_table		   	*/	NULL,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  {
  /* window			*/	0
  }
};

WidgetClass GlyphWidgetWidgetClass = (WidgetClass)&GlyphwidgetClassRec;


static void Realize(Widget w, XtValueMask* mask, XSetWindowAttributes* attr) {
    ((GlyphWidgetRec*)w)->iv.window->xt_realize(mask, attr);
}


static void Destroy(Widget w) {
    ((GlyphWidgetRec*)w)->iv.window->xt_destroy();
}


static XtGeometryResult QueryGeometry(
    Widget w, XtWidgetGeometry* req,XtWidgetGeometry* rep
) {
    return ((GlyphWidgetRec*)w)->iv.window->xt_query_geometry(req, rep);
}


static void ExposeProc(Widget w, XEvent* ev, Region reg) {
    ((GlyphWidgetRec*)w)->iv.window->xt_expose(ev, reg);
}


static XtfBoolean SetValues(
    Widget w, Widget req, Widget new_w,
    ArgList args, Cardinal* num_args
) {
    return ((GlyphWidgetRec*)w)->iv.window->xt_set_values(
	req, new_w, args, num_args
    );
}

