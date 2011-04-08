/*
 * Copyright 1996 Vectaport Inc.
 * Copyright 1995 Cartoactive Systems, Cider Press
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

#include <IVGlyph/gdialogs.h>

#include <IV-look/kit.h>

#include <InterViews/action.h>
#include <InterViews/cursor.h>
#include <InterViews/event.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/window.h>

#include <OS/string.h>

#include <stdio.h>


/*****************************************************************************/

class GAcknowledgeDialogImpl {
private:
    friend class GAcknowledgeDialog;

    WidgetKit* kit_;
    Style* style_;
    GAcknowledgeDialog* dialog_;

    void init(GAcknowledgeDialog*, Style*, const char*, const char*);
    void build(const char*, const char*);

    void ok();
};

declareActionCallback(GAcknowledgeDialogImpl)
implementActionCallback(GAcknowledgeDialogImpl)

GAcknowledgeDialog::GAcknowledgeDialog(const char* c1, const char* c2) 
: Dialog(nil, WidgetKit::instance()->style())
{
    impl_ = new GAcknowledgeDialogImpl;
    GAcknowledgeDialogImpl& impl = *impl_;
    impl.kit_ = WidgetKit::instance();
    impl.init(this, WidgetKit::instance()->style(), c1, c2);
}

GAcknowledgeDialog::~GAcknowledgeDialog() {
    delete impl_;
}

void GAcknowledgeDialog::keystroke(const Event& e) {
    GAcknowledgeDialogImpl& i = *impl_;
    char c;
    if (e.mapkey(&c, 1) != 0) {
	i.ok();
    }
}

void GAcknowledgeDialog::post(Window* window, const char* message, 
			      const char* submsg, const char* title) {
  WidgetKit& kit = *WidgetKit::instance();
  if (title) {
    Style* ts = new Style(kit.style());
    ts->attribute("name", title);
    kit.push_style(ts);
  }
  
  GAcknowledgeDialog* dialog = new GAcknowledgeDialog(message, submsg);
  Resource::ref(dialog);
  dialog->post_for(window);
  Resource::unref(dialog);
  window->cursor(defaultCursor);
  if (title)
    kit.pop_style();
}


/** class GAcknowledgeDialogImpl **/

void GAcknowledgeDialogImpl::init(GAcknowledgeDialog* d, Style* s, const char* c1, const char* c2) {
    dialog_ = d;
    style_ = s;
    build(c1, c2);
}

void GAcknowledgeDialogImpl::build(const char* c1, const char* c2) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    String caption1(c1);

    Action* ok = new ActionCallback(GAcknowledgeDialogImpl)(
	this, &GAcknowledgeDialogImpl::ok);

    Glyph *g;
    if (c2 != nil) {
        String caption2(c2);
        g = layout.vbox(
	        kit.fancy_label(caption1),
	        layout.vglue(5.0, 0.0, 2.0),
	        kit.fancy_label(caption2),
	        layout.vspace(15.0),
	        layout.hbox(
	            layout.hglue(),
	            layout.hcenter(kit.push_button(kit.label("OK"), ok)),
	            layout.hglue()
	    ));
    }
    else {
        g = layout.vbox(
	        kit.fancy_label(caption1),
	        layout.vspace(15.0),
	        layout.hbox(
		    layout.hspace(10.0),
	            layout.hcenter(kit.push_button(kit.label("OK"), ok))
	    ));
    }

    dialog_->body(
		  kit.outset_frame(layout.margin(g, 10.0))
    );
}

void GAcknowledgeDialogImpl::ok() {
    dialog_->dismiss(false);
}

/*****************************************************************************/

class GConfirmDialogImpl {
private:
    friend class GConfirmDialog;

    WidgetKit* kit_;
    Style* style_;
    GConfirmDialog* dialog_;
    boolean cancel_;

    void init(GConfirmDialog*, Style*, const char*, const char*);
    void build(const char*, const char*);

    void yes();
    void no();
    void cancel();
};

declareActionCallback(GConfirmDialogImpl)
implementActionCallback(GConfirmDialogImpl)

GConfirmDialog::GConfirmDialog(const char* c1, const char* c2) 
: Dialog(nil, WidgetKit::instance()->style())
{
    impl_ = new GConfirmDialogImpl;
    GConfirmDialogImpl& impl = *impl_;
    impl.kit_ = WidgetKit::instance();
    impl.init(this, WidgetKit::instance()->style(), c1, c2);
}

GConfirmDialog::~GConfirmDialog() {
    delete impl_;
}

boolean GConfirmDialog::cancel() {
    return impl_->cancel_;
}

void GConfirmDialog::keystroke(const Event& e) {
    GConfirmDialogImpl& i = *impl_;
    char c;
    if (e.mapkey(&c, 1) != 0) {
    	if (c == 'y') {
	    i.yes();
        } else if (c == 'n') {
	    i.no();
        }
    }
}

/** class GConfirmDialogImpl **/

void GConfirmDialogImpl::init(GConfirmDialog* d, Style* s, const char* c1, const char* c2) {
    cancel_ = false;
    dialog_ = d;
    style_ = s;
    build(c1, c2);
}

void GConfirmDialogImpl::build(const char* c1, const char* c2) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    String caption1(c1);

    Action* yes = new ActionCallback(GConfirmDialogImpl)(
	this, &GConfirmDialogImpl::yes);
    Action* no = new ActionCallback(GConfirmDialogImpl)(
	this, &GConfirmDialogImpl::no);
    Action* cancel = new ActionCallback(GConfirmDialogImpl)(
	this, &GConfirmDialogImpl::cancel);

    Glyph *g;
    if (c2 != nil) {
        String caption2(c2);
        g = layout.vbox(
	        kit.fancy_label(caption1),
	        layout.vglue(5.0, 0.0, 2.0),
	        kit.fancy_label(caption2),
	        layout.vspace(15.0),
	        layout.hbox(
	            layout.vcenter(kit.push_button(kit.label("Yes"), yes)),
	            layout.hspace(10.0),
	            layout.vcenter(kit.push_button(kit.label("No"), no)),
	            layout.hspace(10.0),
	            layout.vcenter(kit.push_button(kit.label("Cancel"), cancel))
	    ));
    }
    else {
        g = layout.vbox(
	        kit.fancy_label(caption1),
	        layout.vspace(15.0),
	        layout.hbox(
	            layout.vcenter(kit.push_button(kit.label("Yes"), yes)),
	            layout.hspace(10.0),
	            layout.vcenter(kit.push_button(kit.label("No"), no)),
	            layout.hspace(10.0),
	            layout.vcenter(kit.push_button(kit.label("Cancel"), cancel))
	    ));
    }

    dialog_->body(
		  kit.outset_frame(layout.margin(g, 10.0))
    );
}

void GConfirmDialogImpl::yes() {
  dialog_->dismiss(true);
}

void GConfirmDialogImpl::no() {
  dialog_->dismiss(false);
}

void GConfirmDialogImpl::cancel() {
    cancel_ = true;
    dialog_->dismiss(false);
}
