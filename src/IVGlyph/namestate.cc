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

#include <IVGlyph/namestate.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <IV-look/kit.h>
#include <Dispatch/dispatcher.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

implementIOCallback(NameView)

NameView::NameView(NameState* s1) :MonoGlyph(), Observer()
{
    st1 = s1;
    _blink_state = 0;
    _blink_in = 0;
    _blink_handler = new IOCallback(NameView)(this, &NameView::blink_view);
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    int len = 1;
    if (st1 && st1->name())
	len = strlen(st1->name())+1;
    char str1[len];
    if (st1 && st1->name())
	sprintf(str1,"%s", st1->name());
    else
	sprintf(str1,"");
    _label = new Patch(
	lk.hbox(
	    kit.label(str1),
	    lk.hglue()
	)
    );
    body(_label);
    if (s1)
	s1->attach(this);
}

NameView::~NameView() {}

void NameView::update(Observable* obs) {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    int len = 1;
    if (st1 && st1->name())
	len = strlen(st1->name())+1;
    char str1[len];
    if (st1 && st1->name())
	sprintf(str1,"%s", st1->name());
    else
	sprintf(str1,"");
    Glyph* text;
    if (_blink_state) {
      if (_blink_in)
	text = kit.chiseled_label(str1);
      else
	text = kit.raised_label(str1);
    }
    else
      text = kit.label(str1);
    _label->redraw();
    _label->body(
	lk.hbox(
	    text,
	    lk.hglue()
	)
    );
    _label->reallocate();
    _label->redraw();
}

void NameView::blink_view(long, long) {
    if (_blink_in) {
	_blink_in = 0;
    } else {
	_blink_in = 1;
    }
    _blink_state = 1;
    update(nil);
    long flash_rate = long(0.5 * 1000000);
    Dispatcher::instance().startTimer(0, flash_rate, _blink_handler);
}

void NameView::stop_blinking() {
  Dispatcher::instance().stopTimer(_blink_handler);
  _blink_state = 0;
  update(nil);
}

/*****************************************************************************/

NameState::NameState(const char* n) : Observable()
{
    _name = (char*)n;
}

NameState::~NameState() {}

const char* NameState::name() { return _name; }

void NameState::name(const char* n, boolean notif) {
    _name = (char*)n;
    if (notif)
	notify();
}

