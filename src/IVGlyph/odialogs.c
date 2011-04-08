/*
 * Copyright 2000 IET Inc.
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

#include <IVGlyph/odialogs.h>
#include <IVGlyph/textform.h>

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

class ObsTextDialogImpl {
private:
    friend class ObsTextDialog;

    WidgetKit* kit_;
    Style* style_;
    ObsTextDialog* dialog_;

    void init(ObsTextDialog*, Style*, ObservableText*, const char*);
    void build(ObservableText*, const char*);

    void close();
};

declareActionCallback(ObsTextDialogImpl)
implementActionCallback(ObsTextDialogImpl)

ObsTextDialog::ObsTextDialog(ObservableText* otext, const char* title) 
: Dialog(nil, WidgetKit::instance()->style())
{
    impl_ = new ObsTextDialogImpl;
    ObsTextDialogImpl& impl = *impl_;
    impl.kit_ = WidgetKit::instance();
    impl.init(this, WidgetKit::instance()->style(), otext, title);
}

ObsTextDialog::~ObsTextDialog() {
    delete impl_;
}

void ObsTextDialog::keystroke(const Event& e) {
    ObsTextDialogImpl& i = *impl_;
    char c;
    if (e.mapkey(&c, 1) != 0) {
	i.close();
    }
}

#if 0
void ObsTextDialog::post(Window* window, const char* message, 
			      const char* submsg, const char* title) {
  WidgetKit& kit = *WidgetKit::instance();
  if (title) {
    Style* ts = new Style(kit.style());
    ts->attribute("name", title);
    kit.push_style(ts);
  }
  
  ObsTextDialog* dialog = new ObsTextDialog(message, submsg);
  Resource::ref(dialog);
  dialog->post_for(window);
  Resource::unref(dialog);
  window->cursor(defaultCursor);
  if (title)
    kit.pop_style();
}
#endif


/** class ObsTextDialogImpl **/

void ObsTextDialogImpl::init(ObsTextDialog* d, Style* s, 
			     ObservableText* otext, const char* title) {
    dialog_ = d;
    style_ = s;
    build(otext, title);
}

void ObsTextDialogImpl::build(ObservableText* otext, const char* title) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    String titlestr(title);

    Action* close = new ActionCallback(ObsTextDialogImpl)(
	this, &ObsTextDialogImpl::close);

    Glyph *g;
    TextObserver* otextview = new TextObserver(otext, "");
    g = layout.vbox(
		    kit.fancy_label(titlestr),
		    layout.vglue(5.0, 0.0, 2.0),
		    layout.hbox(
				layout.hglue(),
				layout.hcenter(otextview),
				layout.hglue()),
		    layout.vspace(15.0),
		    layout.hbox(
				layout.hglue(),
				layout.hcenter(kit.push_button(kit.label("Close"), close)),
				layout.hglue()
				));

    dialog_->body(
		  kit.outset_frame(layout.margin(g, 10.0))
    );
}

void ObsTextDialogImpl::close() {
    dialog_->unmap();
}

