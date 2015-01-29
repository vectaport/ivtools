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

#ifndef dragedit_h
#define dragedit_h

#include <InterViews/input.h>
#include <InterViews/observe.h>

class Action;
class BoundedValue;
class Event;
class Patch;
class Style;
class WidgetKit;

class DragEditor : public InputHandler, public Observable {
public:
    DragEditor(BoundedValue* bv, WidgetKit*, Style*, Action* =nil, Action* =nil);
    ~DragEditor();

    virtual void drag(const Event&);
    virtual void press(const Event&);
    virtual void release(const Event&);
    void field(const char*);

    BoundedValue* value() { return bv_; }
protected:
    Patch* patch_;
    BoundedValue* bv_;
    WidgetKit* kit_;
    int ey;
    Action* up_;
    Action* down_;
};

#endif
