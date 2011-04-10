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

#ifndef attrdialog_h
#define attrdialog_h

#include <ComGlyph/terpdialog.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comfunc.h>


class Action;
class AttrDialogImpl;
class NextAttrDialogFunc;
class TrueAttrDialogFunc;
class FalseAttrDialogFunc;
class DoneAttrDialogFunc;

class AttrDialog : public TerpDialog {
public:
    AttrDialog(boolean session = false, int argc = -1, char** argv = nil,
	       boolean init = true);
    AttrDialog(ComTerpServ* comterp, boolean session = false, int argc = -1, char** argv = nil,
	       boolean init = true);
    virtual ~AttrDialog();

    boolean cancel();

    static AttrDialog& instance();
    static void instance(AttrDialog*);

    void next_expr(const char*);
    void true_expr(const char*);
    void false_expr(const char*);
    void done_expr(const char*);

protected:
    void _init(ComTerpServ* comterp, boolean session, int argc, char** argv);
    static AttrDialog* instance_;
};

class AttrDialogImpl : public TerpDialogImpl {
protected:
    friend class AttrDialog;
    AttrDialogImpl();

    InputHandler* init(TerpDialog*, Style*, boolean session = false, 
        int argc = -1, char** argv = nil);

    /* general buttons */
    void eval();

    /* script for iter, eval-true, and eval-false */
    char* next_expr_;
    postfix_token* next_code_;
    int next_len_;

    char* true_expr_;
    postfix_token* true_code_;
    int true_len_;

    char* false_expr_;
    postfix_token* false_code_;
    int false_len_;

    char* done_expr_;
    postfix_token* done_code_;
    int done_len_;

};

declareActionCallback(AttrDialogImpl)

#endif




