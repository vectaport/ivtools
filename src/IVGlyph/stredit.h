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

#ifndef streditdialog_h
#define streditdialog_h

#include <IV-3_1/InterViews/dialog.h>

#include <InterViews/_enter.h>

class Action;
class StrEditDialogImpl;
class WidgetKit;

class StrEditDialog : public Dialog {
public:
    StrEditDialog(const char* caption1, const char* caption2 = nil, 
		  Glyph* extra=nil, boolean custom=false);
    virtual ~StrEditDialog();

    boolean cancel();
    const char* text();

    virtual void keystroke(const Event&);


    static char* post(Window*, const char* message, 
		      const char* string=nil, const char* title=nil,
		      Glyph* extra = nil, boolean custom=false);

    static StrEditDialog* map(Window*, const char* message, 
			      const char* string=nil, const char* title=nil,
			      Glyph* extra = nil, boolean custom=false);

    static const char* accept_custom() { return _accept_custom; }
    static void accept_custom(const char* caption);
    static const char* cancel_custom() { return _cancel_custom; }
    static void cancel_custom(const char* caption);
    static void action_custom(Action* accept, Action* cancel);
    static Action* accept_action_custom() { return _accept_action_custom; }
    static Action* cancel_action_custom() { return _cancel_action_custom; }
private:
    static const char* _accept_custom;
    static const char* _cancel_custom;
    static Action* _accept_action_custom;
    static Action* _cancel_action_custom;
    StrEditDialogImpl* impl_;
};

#include <InterViews/_leave.h>

#endif
