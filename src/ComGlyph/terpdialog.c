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

#define FUNC_CHOOSER

#include <ComGlyph/terpdialog.h>

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

implementActionCallback(TerpDialogImpl)

TerpDialog* TerpDialog::instance_ = nil;

/*****************************************************************************/

TerpDialog::TerpDialog(boolean session, int argc, char** argv, boolean init) 
: Dialog(nil, WidgetKit::instance()->style())
{
    if (init) {
        impl_ = new TerpDialogImpl();
        TerpDialogImpl& cdi = *impl_;
        cdi.kit_ = WidgetKit::instance();
        cdi.initterp();
        focus(cdi.init(this, WidgetKit::instance()->style(), session, argc, argv));
    }
}


TerpDialog::~TerpDialog() {
    delete impl_;
}

void TerpDialog::comterpserv(ComTerpServ* terp) {
    delete impl_->terpserv_;
    impl_->terpserv_ = terp;
}

ComTerpServ* TerpDialog::comterpserv() {
    return impl_->terpserv_;
}

boolean TerpDialog::cancel() {
    return impl_->cancel_;
}

TerpDialog& TerpDialog::instance() {
    if (!instance_) {
        Style* style;
	style = new Style(Session::instance()->style());
	style->attribute("subcaption", "Import graphic from file:");
	style->attribute("open", "Import");
	instance_ = new TerpDialog();
	Resource::ref(instance_);
    }
    return *instance_;
}

void TerpDialog::instance(TerpDialog* instance) {
    if (instance_)
        Unref(instance_);
    Resource::ref(instance);
    instance_ = instance;
}

/*****************************************************************************/

TerpDialogImpl::TerpDialogImpl() { 
  terpserv_ = nil;
  return;
}

InputHandler* TerpDialogImpl::init(TerpDialog* d, Style* s, boolean session, int argc, char** argv) {
    cancel_ = false;
    sign_ = 1;
    dialog_ = d;
    style_ = s;
    cancelsession_ = session;
    return build(argc, argv);
}

void TerpDialogImpl::initterp(ComTerpServ* comterp) {
  if (!comterp && !terpserv_) {
    terpserv_ = new ComTerpServ(BUFSIZ);
    terpserv_->add_defaults();
  } else if (comterp)
    terpserv_ = comterp;
}

InputHandler* TerpDialogImpl::build(int argc, char** argv) {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;

    /* expression editor */
    expredit_ = new EivTextEditor(kit.style());
    if (argc > 1)
        expredit_->load(argv[argc-1]);

    /* result display */
    result_ = new ObservableText("");
    resultview_ = new TextObserver(result_, "Result: ");

    /* err display */
    err_ = new ObservableText("");
    errview_ = new TextObserver(err_, "");

    /* function chooser */
#ifdef FUNC_CHOOSER
    func_choices_ = new StringList();
    int nfunc_symids;
    int* func_symids = terpserv_->get_commands(nfunc_symids, true);
    for (int i=0; i<nfunc_symids; i++) {
      String* func_str = new String(symbol_pntr(func_symids[i]));
      func_choices_->append(*func_str);
    }
    delete func_symids;

    func_chooser_ = new StrChooser(func_choices_, new String("Functions:"), 
        MFKit::instance(), Session::instance()->style(), nil, true, 
	(strchooser_callback)&TerpDialogImpl::insert_func, this);
    Resource::ref(func_chooser_);
#endif

    /* variable chooser */
#ifdef VAR_CHOOSER
    var_choices_ = new StringList();

    var_chooser_ = new StrChooser(var_choices_, new String("Variables"), 
        MFKit::instance(), Session::instance()->style(), nil, true, 
	(strchooser_callback)&TerpDialogImpl::insert_var, this);
    Resource::ref(var_chooser_);
#endif

    /* number pad */
    Action* and_    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::and_op);
    Action* or_     = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::or_op);
    Action* negate = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::negate);
    Action* div    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::div);
    Action* mpy    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::mpy);
    Action* sub    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::sub);
    Action* add    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::add);
    Action* assign = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::assign);

    Action* point  = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::point);
    Action* sign   = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::sign);
    Action* parens = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::parens);
    Action* braces = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::braces);
 
    Action* zero   = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::zero);
    Action* one    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::one);
    Action* two    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::two);
    Action* three  = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::three);
    Action* four   = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::four);
    Action* five   = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::five);
    Action* six    = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::six);
    Action* seven  = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::seven);
    Action* eight  = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::eight);
    Action* nine   = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::nine);

    Action* clear  = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::clear);
    Action* nothing = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::nothing);

    Style* button_style = new Style(Session::instance()->style());
    button_style->attribute("minimumWidth", "48.0");
    kit.push_style();
    kit.style(button_style);

    Glyph* numpad = layout.vbox(
	layout.hbox(
	    layout.vcenter(kit.push_button(kit.label("AC"), clear)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("()"), parens)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("{}"), braces)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("/"), div)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("&&"), and_))
        ),
	layout.vspace(5.0),
	layout.hbox(
	    layout.vcenter(kit.push_button(kit.label("7"), seven)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("8"), eight)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("9"), nine)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("*"), mpy)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("||"), or_))
        ),
	layout.vspace(5.0),
	layout.hbox(
	    layout.vcenter(kit.push_button(kit.label("4"), four)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("5"), five)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("6"), six)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("-"), sub)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("!"), negate))
        ),
	layout.vspace(5.0),
	layout.hbox(
	    layout.vcenter(kit.push_button(kit.label("1"), one)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("2"), two)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("3"), three)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("+"), add))
        ),
	layout.vspace(5.0),
	layout.hbox(
	    layout.vcenter(kit.push_button(kit.label("0"), zero)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("."), point)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("+/-"), sign)),
	    layout.hspace(5.0),
	    layout.vcenter(kit.push_button(kit.label("="), assign))
        )
    );
    kit.pop_style();    

    /* general buttons */
    Action* eval = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::eval);
    Action* cancel = new ActionCallback(TerpDialogImpl)(this, &TerpDialogImpl::cancel);
    _eval_button = kit.push_button(kit.label("Eval"), eval);
    _cancel_button = kit.push_button(kit.label("Cancel"), cancel);

    /* general layout */
    Glyph* interior = layout.vbox(
	layout.hcenter(expredit_),
	layout.vspace(15.0),
        layout.hcenter(resultview_),
        layout.hcenter(errview_),
	layout.vspace(15.0),
        layout.hcenter(layout.hbox(
#ifdef FUNC_CHOOSER
	    func_chooser_,
	    layout.hspace(30.0),
#endif
#ifdef VAR_CHOOSER
	    var_chooser_,
	    layout.hspace(15.0),
#endif
	    numpad)),
	layout.vspace(30.0),
	layout.hcenter(layout.hbox(
	    layout.vcenter(_eval_button),
	    layout.hspace(10.0),
	    layout.vcenter(_cancel_button)
	))
    );

    dialog_->body(kit.outset_frame(layout.margin(interior, 10.0)));
    dialog_->append_input_handler(expredit_->focusable());
    return expredit_->focusable();
}

void TerpDialogImpl::and_op() {
    expredit_->insert_string("&&", 2);
}

void TerpDialogImpl::or_op() {
    expredit_->insert_string("||", 2);
}

void TerpDialogImpl::negate() {
    expredit_->insert_string("!", 1);
}

void TerpDialogImpl::div() {
    expredit_->insert_string("/", 1);
}

void TerpDialogImpl::mpy() {
    expredit_->insert_string("*", 1);
}

void TerpDialogImpl::sub() {
    expredit_->insert_string("-", 1);
}

void TerpDialogImpl::add() {
    expredit_->insert_string("+", 1);
}

void TerpDialogImpl::assign() {
    eval();
}

void TerpDialogImpl::point() {
    expredit_->insert_string(".", 1);
}

void TerpDialogImpl::sign() {
    int i;
    char exprbuf[BUFSIZ];
    const char* expr = expredit_->text();
    if (sign_ > 0) {
        sprintf(exprbuf, "-%s", expr);
        exprbuf[strlen(exprbuf)] = '\0';
	sign_ = -1;
    } else {
        for (i=1; i<strlen(expr); i++)
            exprbuf[i-1] = expr[i];
        exprbuf[i-1] = '\0';
       	sign_ = 1;
    }

    expredit_->text("");
    expredit_->insert_string(exprbuf, strlen(exprbuf));
}

void TerpDialogImpl::parens() {
    expredit_->insert_string("()", 2);
    expredit_->textview()->backward_char();
}

void TerpDialogImpl::braces() {
    expredit_->insert_string("{}", 2);
    expredit_->textview()->backward_char();
}

void TerpDialogImpl::zero() {
    expredit_->insert_string("0", 1);
}

void TerpDialogImpl::one() {
    expredit_->insert_string("1", 1);
}

void TerpDialogImpl::two() {
    expredit_->insert_string("2", 1);
}

void TerpDialogImpl::three() {
    expredit_->insert_string("3", 1);
}

void TerpDialogImpl::four() {
    expredit_->insert_string("4", 1);
}

void TerpDialogImpl::five() {
    expredit_->insert_string("5", 1);
}

void TerpDialogImpl::six() {
    expredit_->insert_string("6", 1);
}

void TerpDialogImpl::seven() {
    expredit_->insert_string("7", 1);
}

void TerpDialogImpl::eight() {
    expredit_->insert_string("8", 1);
}

void TerpDialogImpl::nine() {
    expredit_->insert_string("9", 1);
}

void TerpDialogImpl::clear() {
    expredit_->text("");
    clear_results();
}

void TerpDialogImpl::clear_results() {
    result_->textvalue("");
    resultview_->update(result_);
    err_->textvalue("");
    errview_->update(err_);
}

void TerpDialogImpl::nothing() {
}

void TerpDialogImpl::eval() {
    char exprbuf[BUFSIZ];
    const char* expr = expredit_->text();
    if (expr[strlen(expr)-1] != '\n') 
        sprintf(exprbuf, "%s\n", expr);
    else
        sprintf(exprbuf, "%s", expr);

    ComValue retval(terpserv_->run(exprbuf));

    const char* errmsg = terpserv_->errmsg();
    if (*errmsg) {
        result_->textvalue("");
        err_->textvalue(errmsg);
    } else if (retval.type() == ComValue::UnknownType) {
        result_->textvalue("nil");
        err_->textvalue("");
    } else {
        char buf[BUFSIZ];
        buf[0] = '\0';
        std::ostrstream ostr(buf, BUFSIZ);
	
        ostr << retval;
	ostr << '\0';

        result_->textvalue(buf);
        err_->textvalue("");
    }

    resultview_->update(result_);
    errview_->update(err_);
    
}

void TerpDialogImpl::insert_func(void *base) {
    TerpDialogImpl* i = (TerpDialogImpl*)base;
    i->expredit_->text("");
    if (i->func_chooser_->selected() > -1) {
        String func = i->func_choices_->item_ref(i->func_chooser_->selected());
        char* funcstr = (char*)func.string();
        i->expredit_->insert_string(funcstr, strlen(funcstr));
        const int curspos = strlen(funcstr)-1;
        i->expredit_->select(curspos, curspos);
    }
}

void TerpDialogImpl::insert_var(void *base) {
    TerpDialogImpl* i = (TerpDialogImpl*)base;
    i->expredit_->text("");
    if (i->var_chooser_->selected() > -1) {
        String var = i->var_choices_->item_ref(i->var_chooser_->selected());
        char* varstr = (char*)var.string();
        i->expredit_->insert_string(varstr, strlen(varstr));
    }
}

void TerpDialogImpl::cancel() {
    cancel_ = true;
    if (cancelsession_) {
        Session& s = *Session::instance();
        s.quit();
    } else {
        dialog_->dismiss(false);
    }
}
