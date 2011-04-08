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

#include <Attribute/attrlist.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovgdialog.h>

#include <AttrGlyph/attredit.h>
#include <IVGlyph/textedit.h>

#include <IV-look/dialogs.h>
#include <IV-look/field.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <OS/string.h>

#include <stdio.h>

class AnnotateDialogImpl {
private:
friend class AnnotateDialog;

    WidgetKit* kit_;
    Style* style_;
    AnnotateDialog* dialog_;
    EivTextEditor* editor_;

    void init(const char*, AnnotateDialog*, Style*);
    void free();
    void build(const char*);
    void accept();
    void cancel();
    const char* value();
    void value(const char*, boolean update);
    void clear();
};

declareActionCallback(AnnotateDialogImpl)
implementActionCallback(AnnotateDialogImpl)

AnnotateDialog::AnnotateDialog(const char* anno, WidgetKit* kit, Style* s) 
: Dialog(nil, s)
{
  impl_ = new AnnotateDialogImpl;
  AnnotateDialogImpl& adi = *impl_;
  adi.kit_ = kit;
  adi.init(anno, this, s);
}

AnnotateDialog::~AnnotateDialog() {
  impl_->free();
  delete impl_;
}

const char* AnnotateDialog::value() {
  return impl_->value();
}

void AnnotateDialog::value(const char* v, boolean update) {
    impl_->value(v, update);
}

void AnnotateDialog::clear() {
    impl_->clear();
}

/** class AnnotateDialogImpl **/

void AnnotateDialogImpl::init(const char* anno, AnnotateDialog* p, Style* s) {
  dialog_ = p;
  style_ = s;
  build(anno);
}

void AnnotateDialogImpl::free() {
}

void AnnotateDialogImpl::build(const char* anno) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    const DialogKit& dialog = *DialogKit::instance();
    Style* s = style_;
    String caption("Enter annotation for object:");
    String open("OK");
    String close("Cancel");

    Action* accept = new ActionCallback(AnnotateDialogImpl)(
	this, &AnnotateDialogImpl::accept
    );
    Action* cancel = new ActionCallback(AnnotateDialogImpl)(
	this, &AnnotateDialogImpl::cancel
    );

    Glyph* g = layout.vbox(
	layout.rmargin(kit.fancy_label(caption), 5.0, fil, 0.0),
	layout.vglue(5.0, 0.0, 2.0),
	layout.vcenter(editor_ = new EivTextEditor(kit.style())),
	layout.vspace(15.0),
	layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(open, accept)),
	    layout.hglue(10.0)
	)
    );
    editor_->text(anno, false);
    dialog_->body(
	kit.outset_frame(layout.margin(g, 5.0))
    );
    dialog_->append_input_handler(editor_->focusable());
    dialog_->focus(editor_->focusable());
}

void AnnotateDialogImpl::accept() {
  dialog_->dismiss(true);
}

void AnnotateDialogImpl::cancel() {
  dialog_->dismiss(false);
}

const char* AnnotateDialogImpl::value() {
    return editor_->text();
}

void AnnotateDialogImpl::value(const char* v, boolean update) {
    editor_->text(v, update);
}

void AnnotateDialogImpl::clear() {
    editor_->text("");
}



class AttributeDialogImpl {
private:
friend class AttributeDialog;

    WidgetKit* kit_;
    Style* style_;
    AttributeDialog* dialog_;
    AttributeListEditor* editor_;
    OverlayComp* comp_;
    AttributeList* copylist_;

    void init(OverlayComp*, AttributeDialog*, Style*);
    void free();
    void build(AttributeList*);
    void accept();
    void cancel();
    const char* value();
    void value(const char*, boolean update);
    void clear();
};

declareActionCallback(AttributeDialogImpl)
implementActionCallback(AttributeDialogImpl)

AttributeDialog::AttributeDialog(OverlayComp* oc, WidgetKit* kit, Style* s) 
: Dialog(nil, s)
{
  impl_ = new AttributeDialogImpl;
  AttributeDialogImpl& adi = *impl_;
  adi.kit_ = kit;
  adi.init(oc, this, s);
}

AttributeDialog::~AttributeDialog() {
  impl_->free();
  delete impl_;
}

/** class AttributeDialogImpl **/

void AttributeDialogImpl::init(OverlayComp* oc, AttributeDialog* p, Style* s) {
  dialog_ = p;
  style_ = s;
  comp_ = oc;
  copylist_ = new AttributeList(comp_->GetAttributeList());
  build(copylist_);
}

void AttributeDialogImpl::free() {
}

void AttributeDialogImpl::build(AttributeList* al) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    const DialogKit& dialog = *DialogKit::instance();
    Style* s = style_;
    String caption("Attribute List Editor");
    String open("OK");
    String close("Cancel");

    Action* accept = new ActionCallback(AttributeDialogImpl)(
	this, &AttributeDialogImpl::accept
    );
    Action* cancel = new ActionCallback(AttributeDialogImpl)(
	this, &AttributeDialogImpl::cancel
    );

    Glyph* g = layout.vbox(
	layout.hcenter(kit.fancy_label(caption)),
	layout.vspace(15.0),
	layout.hcenter(layout.hbox(
	    layout.hglue(10.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(open, accept)),
	    layout.hglue(10.0)
	)),
	layout.vspace(15.0),
	layout.hcenter(editor_ = new AttributeListEditor(al)),
	layout.vglue(1)
    );
//    editor_->text(anno, false);
    dialog_->body(
	kit.outset_frame(layout.margin(g, 5.0))
    );
    dialog_->append_input_handler(editor_->focusable1());
    dialog_->append_input_handler(editor_->focusable2());
    dialog_->focus(editor_->focusable1());
}

void AttributeDialogImpl::accept() {
    comp_->SetAttributeList(copylist_);
    dialog_->dismiss(true);
}

void AttributeDialogImpl::cancel() {
    delete copylist_;
    dialog_->dismiss(false);
}
