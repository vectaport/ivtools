/*
 * Copyright 1998 Vectaport Inc.
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

#include <IVGlyph/fieldedit.h>
#include <IVGlyph/stredit.h>
#include <IVGlyph/textbuff.h>

#include <IV-look/kit.h>

#include <InterViews/action.h>
#include <InterViews/cursor.h>
#include <InterViews/event.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/window.h>

#include <OS/string.h>
#include <string.h>
#include <stdio.h>


/*****************************************************************************/

class StrEditDialogImpl {
protected:
    friend class StrEditDialog;

    WidgetKit* kit_;
    Style* style_;
    StrEditDialog* dialog_;
    boolean cancel_;

    void init(StrEditDialog*, Style*, const char*, const char*);
    void build(const char*, const char*);

    void accept();
    void cancel();

    GFieldEditor* editor_;
};

declareActionCallback(StrEditDialogImpl)
implementActionCallback(StrEditDialogImpl)

StrEditDialog::StrEditDialog(const char* c1, const char* c2) 
: Dialog(nil, WidgetKit::instance()->style())
{
    impl_ = new StrEditDialogImpl;
    StrEditDialogImpl& impl = *impl_;
    impl.kit_ = WidgetKit::instance();
    impl.init(this, WidgetKit::instance()->style(), c1, c2);
}

StrEditDialog::~StrEditDialog() {
    delete impl_;
}

boolean StrEditDialog::cancel() {
    return impl_->cancel_;
}

const char* StrEditDialog::text() {
    return impl_->editor_->text();
}

void StrEditDialog::keystroke(const Event& e) {
    StrEditDialogImpl& i = *impl_;
    char c;
    if (e.mapkey(&c, 1) != 0) {
    	if (c == '\r') {
	    i.accept();
        } else if (c == '') {
	    i.cancel();
        } else
	  i.editor_->keystroke(e);
    }
}

char* StrEditDialog::post(Window* window, const char* message, 
			 const char* string, const char* title) {
  WidgetKit& kit = *WidgetKit::instance();
  if (title) {
    Style* ts = new Style(kit.style());
    ts->attribute("name", title);
    kit.push_style(ts);
  }
  
  StrEditDialog* dialog = new StrEditDialog(message, string);
  Resource::ref(dialog);
  boolean accepted = dialog->post_for(window);
  char* retstr = strdup(dialog->text());
  Resource::unref(dialog);
  window->cursor(defaultCursor);
  if (title)
    kit.pop_style();
  return accepted ? retstr : nil;
}

/** class StrEditDialogImpl **/

void StrEditDialogImpl::init(StrEditDialog* d, Style* s, const char* c1, const char* c2) {
    cancel_ = false;
    dialog_ = d;
    style_ = s;
    editor_ = nil;
    build(c1, c2);
}

void StrEditDialogImpl::build(const char* msg, const char* txt) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    String message(msg);

    Action* accept = new ActionCallback(StrEditDialogImpl)(
	this, &StrEditDialogImpl::accept);
    Action* cancel = new ActionCallback(StrEditDialogImpl)(
	this, &StrEditDialogImpl::cancel);

#if 1
    editor_ = new GFieldEditor((char *) txt, 
			       (GFieldEditorAction*)nil, 10);
#else
    editor_ = new GFieldEditor((char *) txt);
#endif
    editor_->field()->righttrim();
    Glyph *g;
    g = layout.vbox(
		    kit.fancy_label(message),
		    layout.vglue(5.0, 0.0, 2.0),
		    editor_,
		    layout.vspace(15.0),
		    layout.hbox(
				layout.vcenter(kit.push_button(kit.label("Accept"), accept)),
				layout.hspace(10.0),
				layout.vcenter(kit.push_button(kit.label("Cancel"), cancel))
				));


    
    InputHandler* topih = 
      new InputHandler(kit.outset_frame(layout.margin(g, 10.0)), style_);
    topih->append_input_handler(editor_);

    dialog_->body(topih);
}

void StrEditDialogImpl::accept() {
  dialog_->dismiss(true);
}

void StrEditDialogImpl::cancel() {
    cancel_ = true;
    dialog_->dismiss(false);
}
