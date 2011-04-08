/*
 * Copyright (c) 1995-1996 Vectaport Inc.
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

#include <IVGlyph/fieldedit.h>
#include <IVGlyph/figure.h>
#include <IVGlyph/textbuff.h>

#include <InterViews/brush.h>
#include <InterViews/character.h>
#include <InterViews/color.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <IV-look/kit.h>
#include <OS/math.h>

#include <iostream.h>
#include <string.h>

/*****************************************************************************/

GFieldEditor::GFieldEditor(char* init, GFieldEditorAction* act, float minwidth)
: InputHandler(nil, Session::instance()->style())
{
    field_ = new EivTextBuffer();
    if (init && strlen(init) > 0) {
	field_->Insert(0, init, strlen(init));
    }
    point_pos_ = 0;
    mark_pos_ = 0;
    action_ = act;
    minwidth_ = minwidth;
    cursor_is_on_= false;
    int thickness;
    Style *s = Session::instance()->style();
    s->find_attribute("frameThickness", _frame_thickness);
    make_body();
}

GFieldEditor::~GFieldEditor() {
    delete field_;
}

void GFieldEditor::keystroke(const Event& e) {
    char buf[3];
    int count = e.mapkey(buf, 3);
    if (count == 1)
	switch(buf[0]) {
	case '\001':  // Ctrl-A
	    {
		beginning_of_line();
		break;
	    }
	case '\002':  // Ctrl-B
	    {
		backward_char();
		break;
	    }
	case '\004':  // Ctrl-D
	    {
		if (mark_pos_ == point_pos_) 
		    delete_char_forward();
		else 
		    delete_region();
		break;
	    }
	case '\005':  // Ctrl-E
	    {
		end_of_line();
		break;
	    }
	case '\006':  // Ctrl-F
	    
	    {
		forward_char();
		break;
	    }
	case '\007':  // Ctrl-G
	case '\033':  // Escape
	    {
		if (action_ != nil)
		    action_->cancel(this);
		clear_buffer();
		break;
	    }
	case '\010':  // Ctrl-H
	case '\177':  // DEL
	    {
		if (mark_pos_ == point_pos_) 
		    delete_char_backward();
		else 
		    delete_region();
		break;
	    }
	case '\013':  // Ctrl-K
	    {
		delete_to_eol();
		break;
	    }
	case '\015':  //  Ctrl-M
	    {
		if (action_ != nil)
		    action_->accept(this);
		break;
	    }
          default:  // the "normal" case, insert a char, too bad it's buried here..
	    {
		if (mark_pos_ != point_pos_)
		    delete_region();
		insert_char(buf[0]);
		if (action_ != nil)
		    action_->accept(this);
		break;
	    }
	}
}

InputHandler* GFieldEditor::focus_in() {
    cursor_on();
    update();
    return InputHandler::focus_in();
}

void GFieldEditor::focus_out() {
    cursor_off();
    update();
    if (action_ != nil)
	action_->accept(this);
    InputHandler::focus_out();
}

void GFieldEditor::cursor_on() {
    cursor_is_on_= true;
}

void GFieldEditor::cursor_off() {
    cursor_is_on_= false;
}

const char* GFieldEditor::text() {
    return field_->Text();
}

void GFieldEditor::make_body() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& wk = *WidgetKit::instance();

    PolyGlyph* label_;
    PolyGlyph* cursor_;
    PolyGlyph* body_;
    body_ = lk.overlay();
    body_->append(lk.vcenter(label_ = lk.hbox()));
    
    label_->append(lk.hspace(2));
    int bar1 = Math::min(point_pos_, mark_pos_);
    int bar2 = Math::max(point_pos_, mark_pos_);
#if 0
    if (!cursor_is_on_) bar1 = bar2;
#endif
    int i = 0;
    Display* d = Session::instance()->default_display();
    for (; i < bar1; i++) {
	label_->append(new Character(*field_->Text(i), wk.font(), Color::lookup(d, "black")));
    }
    for (; i < bar2; i++) {
	label_->append(new Character(*field_->Text(i), wk.font(), Color::lookup(d, "white")));
    }
    for (; i <field_->Length(); i++) {
	label_->append(new Character(*field_->Text(i), wk.font(), Color::lookup(d, "black")));
    }
#if 0
    for (; i <minwidth_; i++) {
	label_->append(new Character(' ', wk.font(), Color::lookup(d, "black")));
    }
#endif
    label_->append(lk.hglue(0, fil, fil));
    if (cursor_is_on_) {
	body_->append(lk.vcenter(cursor_ = lk.hbox()));
	float curoff = wk.font()->width(field_->Text(), point_pos_);
	cursor_->append(lk.hspace(curoff));
	cursor_->append(new Fig31Line(new Brush(0), wk.foreground(), nil,
				      0, 0, 0, wk.font()->Height()));
	cursor_->append(lk.hglue(0, fil, fil));
    }
    if (minwidth_>0.0) 
      body(wk.inset_frame(lk.hnatural(lk.vfixed(new Target(body_, TargetPrimitiveHit),wk.font()->Height()), minwidth_)));
    else
      body(wk.inset_frame(lk.vfixed(new Target(body_, TargetPrimitiveHit), wk.font()->Height())));
}

void GFieldEditor::update() {
    make_body();
    redraw();
}

void GFieldEditor::beginning_of_line() {
    mark_pos_ = point_pos_ = 0;
    update();
}

void GFieldEditor::end_of_line() {
    mark_pos_ = point_pos_ = field_->Length();
    update();
}

void GFieldEditor::forward_char() {
    if (point_pos_ < field_->Length()) {
	mark_pos_ = ++point_pos_;
	update();
    }
}

void GFieldEditor::backward_char() {
    if (point_pos_ > 0) {
	mark_pos_ = --point_pos_;
	update();
    }
}

void GFieldEditor::delete_char_forward() {
    if (point_pos_ < field_->Length()) {
	field_->Delete(point_pos_, 1);
	update();
    }
}

void GFieldEditor::delete_char_backward() {
    if (point_pos_ > 0) {
	mark_pos_ = --point_pos_;
	field_->Delete(point_pos_, 1);
	update();
    }
}

void GFieldEditor::delete_region() {
    if (mark_pos_ > point_pos_) {
	for (int i= point_pos_; i<mark_pos_; i++) 
	    field_->Delete(point_pos_, 1);
	mark_pos_ = point_pos_;
    } else {
	for (int i= mark_pos_; i<point_pos_; i++) 
	    field_->Delete(mark_pos_, 1);
	point_pos_ = mark_pos_;
    }
    update();
}

void GFieldEditor::delete_to_eol() {
    while (field_->Length() > point_pos_)
	field_->Delete(point_pos_, 1);
    update();
}

void GFieldEditor::insert_char(char ch) {	    

    /* check to see if this character would exceed the allocation */
    const Allocation& a = allocation();
    WidgetKit& wk = *WidgetKit::instance();
    float swidth = wk.font()->width(field_->Text(), field_->Length());
    char cbuf[] = { ch, '\0' };
    float cwidth = wk.font()->width((char*) cbuf, 1);
    float fwidth = a.right() - a.left() - _frame_thickness*2;
    if (swidth + cwidth > fwidth ) {
	cerr << "\007";
	return;
	}

    mark_pos_ = ++point_pos_;
    char s[2];
    s[0] = ch;
    s[1] = '\0';
    field_->Insert(point_pos_ - 1, s, 1);
    update();
}

void GFieldEditor::clear_buffer() {
    field_->Delete(0,field_->characters());
    mark_pos_ = point_pos_ = 0;
    update();
}

void GFieldEditor::select_all() { 
    mark_pos_ = 0;
    point_pos_ = field_->characters();
    update();
}

GFieldEditorAction::GFieldEditorAction() { }
GFieldEditorAction::~GFieldEditorAction() { }
void GFieldEditorAction::accept(GFieldEditor*) { }
void GFieldEditorAction::cancel(GFieldEditor*) { }

void GFieldEditor::press (const Event& event) {
    point_pos_ = locate(event);
    if (!event.shift_is_down())
	mark_pos_ = point_pos_;
}

void GFieldEditor::drag (const Event& event) {
    point_pos_ = locate(event);
    update();
}

int GFieldEditor::locate (const Event& event) {
    WidgetKit& wk = *WidgetKit::instance();

    const Allocation& a = allocation();
    float x = event.pointer_x();
    int loc = 0;
    float xoff = a.left() + _frame_thickness;
    while (loc<field_->Length()) {
	xoff += wk.font()->width(field_->Text()+loc, 1);
	if (xoff < x ) 
	    loc++;
	else
	    break;
    }
    return loc;
}

