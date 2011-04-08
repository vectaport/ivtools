/*
 * Copyright (c) 1995-1997 Vectaport Inc.
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

#ifndef terpdialog_h
#define terpdialog_h

#include <InterViews/dialog.h>

class Action;
class ComTerpServ;
class TerpDialogImpl;

class TerpDialog : public Dialog {
public:
    TerpDialog(boolean session = false, int argc = -1, char** argv = nil,
	       boolean init = true);
    virtual ~TerpDialog();

    boolean cancel();

    static TerpDialog& instance();
    static void instance(TerpDialog*);

    void comterpserv(ComTerpServ*);
    ComTerpServ* comterpserv();

protected:
    TerpDialogImpl* impl_;
    static TerpDialog* instance_;
};

class Button;
class EivTextEditor;
class ObservableText;
class StringList;
class StrChooser;
class Style;
class TextObserver;
class WidgetKit;

class TerpDialogImpl {
protected:
    TerpDialogImpl();
    friend class TerpDialog;

    WidgetKit* kit_;
    Style* style_;
    TerpDialog* dialog_;
    boolean cancel_;
    boolean cancelsession_;

    InputHandler* init(TerpDialog*, Style*, boolean session = false, 
        int argc = -1, char** argv = nil);
    void initterp(ComTerpServ* = nil);
    InputHandler* build(int argc, char** argv);

    ComTerpServ* terpserv_;

    /* expression editor */
    EivTextEditor* expredit_;

    /* results */
    ObservableText* result_;
    TextObserver* resultview_;

    /* errors */
    ObservableText* err_;
    TextObserver* errview_;

    /* function chooser */
    StrChooser* func_chooser_;
    StringList * func_choices_;

    /* buttons */
    Button* _eval_button;
    Button* _cancel_button;

    static void insert_func(void*);

    /* variable chooser */
    StrChooser* var_chooser_;
    StringList * var_choices_;

    static void insert_var(void*);

    /* number pad */
    int sign_;

    void and_op();
    void or_op();
    void negate();
    void div();
    void mpy();
    void sub();
    void add();
    void assign();

    void point();
    void sign();
    void parens();
    void braces();

    void zero();
    void one();
    void two();
    void three();
    void four();
    void five();
    void six();
    void seven();
    void eight();
    void nine();

    void clear();
    void clear_results();
    void nothing();

    /* general buttons */
    void eval();
    void cancel();
};

#include <InterViews/action.h>
declareActionCallback(TerpDialogImpl)

#endif
