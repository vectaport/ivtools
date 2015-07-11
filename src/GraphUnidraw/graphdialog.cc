/*
 * Copyright (c) 1994 Vectaport Inc.
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

#include <GraphUnidraw/graphdialog.h>
#include <IV-look/dialogs.h>
#include <IV-look/field.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <OS/string.h>
#include <stdio.h>

class NodeDialogImpl {
private:
  friend class NodeDialog;

  WidgetKit* kit_;
  Style* style_;
  NodeDialog* dialog_;
  FieldEditor* editor_;

  void init(NodeDialog*, Style*);
  void free();
  void build();
  void accept();
  void cancel();
  void accept2(FieldEditor*);
  void cancel2(FieldEditor*);
  const char* value();
    void clear();
};

declareActionCallback(NodeDialogImpl)
implementActionCallback(NodeDialogImpl)

declareFieldEditorCallback(NodeDialogImpl)
implementFieldEditorCallback(NodeDialogImpl)

NodeDialog::NodeDialog(WidgetKit* kit, Style* s) 
: Dialog(nil, s)
{
  impl_ = new NodeDialogImpl;
  NodeDialogImpl& pdi = *impl_;
  pdi.kit_ = kit;
  pdi.init(this, s);
}

NodeDialog::~NodeDialog() {
  impl_->free();
  delete impl_;
}

const char* NodeDialog::value() {
  return impl_->value();
}

void NodeDialog::clear() {
    impl_->clear();
}

/** class NodeDialogImpl **/

void NodeDialogImpl::init(NodeDialog* p, Style* s) {
  dialog_ = p;
  style_ = s;
  build();
}

void NodeDialogImpl::free() {
}

void NodeDialogImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    const DialogKit& dialog = *DialogKit::instance();
    Style* s = style_;
    String caption("Enter label for node:");
    String open("OK");
    String close("Cancel");

    Action* accept = new ActionCallback(NodeDialogImpl)(
	this, &NodeDialogImpl::accept
    );
    Action* cancel = new ActionCallback(NodeDialogImpl)(
	this, &NodeDialogImpl::cancel
    );
    FieldEditorAction* feaction = new FieldEditorCallback(NodeDialogImpl)
      (this, &NodeDialogImpl::accept2, &NodeDialogImpl::cancel2);

    TelltaleGroup* group = new TelltaleGroup;

    Glyph* g = layout.vbox(
			   layout.rmargin(kit.fancy_label(caption), 5.0, fil, 0.0),
			   layout.vglue(5.0, 0.0, 2.0),
			   layout.vcenter(editor_ = dialog.field_editor("", s, feaction)),
			   layout.vspace(15.0),
	layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.default_button(open, accept)),
	    layout.hglue(10.0)
	)
    );

    dialog_->body(
		  kit.outset_frame(layout.margin(g, 5.0))
    );
    dialog_->append_input_handler(editor_);
    dialog_->focus(editor_);
}

void NodeDialogImpl::accept() {
  dialog_->dismiss(true);
}

void NodeDialogImpl::cancel() {
  dialog_->dismiss(false);
}

void NodeDialogImpl::accept2(FieldEditor* fe) {
  dialog_->dismiss(true);
}

void NodeDialogImpl::cancel2(FieldEditor* fe) {
  dialog_->dismiss(false);
}

const char* NodeDialogImpl::value() {
  const String* s = editor_->text();
  const char* val = s->string();
  return val;
}

void NodeDialogImpl::clear() {
    editor_->field("");
}
