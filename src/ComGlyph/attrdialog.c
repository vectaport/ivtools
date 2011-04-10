/*
 * Copyright 1995-1997 Vectaport Inc.
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

#include <ComGlyph/attrdialog.h>
#include <Attribute/attrlist.h>

#include <ComTerp/comvalue.h>
#include <ComTerp/ctrlfunc.h>
#include <ComTerp/numfunc.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/strmfunc.h>

#include <IVGlyph/strchooser.h>
#include <IVGlyph/textedit.h>
#include <IVGlyph/textform.h>
#include <IVGlyph/textwindow.h>
#include <IVGlyph/textview.h>

#include <IV-look/kit.h>
#include <IV-look/mf_kit.h>

#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <InterViews/session.h>
#include <InterViews/target.h>

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <strstream>

/*****************************************************************************/

implementActionCallback(AttrDialogImpl)

AttrDialog* AttrDialog::instance_ = nil;

AttrDialog::AttrDialog(boolean session, int argc, char** argv, boolean init) 
: TerpDialog(session, argc, argv, false)
{
    if (init) _init(nil, session, argc, argv);
}

AttrDialog::AttrDialog(ComTerpServ* comterp, boolean session, int argc, char** argv, boolean init) 
: TerpDialog(session, argc, argv, false)
{
    if (init) _init(comterp, session, argc, argv);
}

void AttrDialog::_init(ComTerpServ* comterp, boolean session, int argc, char** argv) 
{
    impl_ = new AttrDialogImpl();
    AttrDialogImpl& cdi = *(AttrDialogImpl*)impl_;
    cdi.kit_ = WidgetKit::instance();
    cdi.initterp(comterp);
    focus(cdi.init(this, WidgetKit::instance()->style(), session, argc, argv));
}


AttrDialog::~AttrDialog() {
}

AttrDialog& AttrDialog::instance() {
    if (!instance_) {
        Style* style;
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Evaluate Attribute Expressions");
	style->attribute("open", "AttrExpr");
	instance_ = new AttrDialog();
	Resource::ref(instance_);
    }
    return *instance_;
}

void AttrDialog::instance(AttrDialog* instance) {
    if (instance_)
        Unref(instance_);
    Resource::ref(instance);
    instance_ = instance;
}

void AttrDialog::next_expr(const char* expr) {
    delete (((AttrDialogImpl*)impl_)->next_expr_);
    ((AttrDialogImpl*)impl_)->next_expr_ = strdup(expr);
    delete (((AttrDialogImpl*)impl_)->next_code_);
    ((AttrDialogImpl*)impl_)->next_code_ = 
        comterpserv()->gen_code(expr, ((AttrDialogImpl*)impl_)->next_len_);
}

void AttrDialog::true_expr(const char* expr) {
    delete (((AttrDialogImpl*)impl_)->true_expr_);
    ((AttrDialogImpl*)impl_)->true_expr_ = strdup(expr);
    delete (((AttrDialogImpl*)impl_)->true_code_);
    ((AttrDialogImpl*)impl_)->true_code_ = 
        comterpserv()->gen_code(expr, ((AttrDialogImpl*)impl_)->true_len_);
}

void AttrDialog::false_expr(const char* expr) {
    delete (((AttrDialogImpl*)impl_)->false_expr_);
    ((AttrDialogImpl*)impl_)->false_expr_ = strdup(expr);
    delete (((AttrDialogImpl*)impl_)->false_code_);
    ((AttrDialogImpl*)impl_)->false_code_ = 
        comterpserv()->gen_code(expr, ((AttrDialogImpl*)impl_)->false_len_);
}

void AttrDialog::done_expr(const char* expr) {
    delete (((AttrDialogImpl*)impl_)->done_expr_);
    ((AttrDialogImpl*)impl_)->done_expr_ = strdup(expr);
    delete (((AttrDialogImpl*)impl_)->done_code_);
    ((AttrDialogImpl*)impl_)->done_code_ = 
        comterpserv()->gen_code(expr, ((AttrDialogImpl*)impl_)->done_len_);
}



/*****************************************************************************/

AttrDialogImpl::AttrDialogImpl() { 
    next_code_ = nil;
    true_code_ = nil;
    false_code_ = nil;
    next_expr_ = nil;
    true_expr_ = nil;
    false_expr_ = nil;
    next_len_ = 0;
    true_len_ = 0;
    false_len_ = 0;
    return; 
}

InputHandler* AttrDialogImpl::init(TerpDialog* d, Style* s, boolean session, int argc, char** argv) {
  InputHandler* ih = TerpDialogImpl::init(d, s, session, argc, argv);
  _eval_button->action(new ActionCallback(AttrDialogImpl)(this, &AttrDialogImpl::eval));
  return ih;
}

void AttrDialogImpl::eval() {
    char exprbuf[BUFSIZ];
    const char* expr = expredit_->text();
    if (expr[strlen(expr)-1] != '\n') 
        sprintf(exprbuf, "%s\n", expr);
    else
        sprintf(exprbuf, "%s", expr);

    
    int exprlen;
    postfix_token* exprcode = terpserv_->gen_code(exprbuf, exprlen);

    ComValue retval = ComValue::nullval();

    do {

        retval = terpserv_->run(exprcode, exprlen);
    
        const char* errmsg = terpserv_->errmsg();

        if (*errmsg) {

            result_->textvalue("");
            err_->textvalue(errmsg);
    	    return;

        } else {

	    const int bufsiz = BUFSIZ;
	    char buf[bufsiz];
	    std::strstream outstr(buf, bufsiz);
	    outstr << retval;
	    outstr.put('\0');
            result_->textvalue(buf);
            err_->textvalue("");

	    if (true_code_ && false_code_)
	      retval = retval.boolean_val() ? 
  	        terpserv_->run(true_code_, true_len_) :
	      terpserv_->run(false_code_, false_len_);
	    
        }
	
        resultview_->update(result_);
        errview_->update(err_);

	
        if (next_code_) 
	  retval = terpserv_->run(next_code_, next_len_);

    } while (retval.boolean_val());
    
    if (done_code_) 
      retval = terpserv_->run(done_code_, done_len_);
}
