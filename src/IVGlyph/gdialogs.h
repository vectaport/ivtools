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

#ifndef confdialog_h
#define confdialog_h

#include <IV-3_1/InterViews/dialog.h>

#include <InterViews/_enter.h>

class GAcknowledgeDialogImpl;
class GConfirmDialogImpl;
class WidgetKit;

class GAcknowledgeDialog : public Dialog {
public:
    GAcknowledgeDialog(const char* title, const char* subtitle = "");
    virtual ~GAcknowledgeDialog();

    virtual void keystroke(const Event&);

    static void post(Window*, const char* message, 
		     const char* submsg=nil, const char* title=nil);

private:
    GAcknowledgeDialogImpl* impl_;
};

class GConfirmDialog : public Dialog {
public:
    GConfirmDialog(const char* caption1, const char* caption2 = nil);
    virtual ~GConfirmDialog();

    virtual void keystroke(const Event&);

    boolean cancel();

private:
    GConfirmDialogImpl* impl_;
};

#include <InterViews/_leave.h>

#endif
