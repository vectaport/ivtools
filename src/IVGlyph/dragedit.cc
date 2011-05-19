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

#include <IVGlyph/bdvalue.h>
#include <IVGlyph/dragedit.h>
#include <InterViews/action.h>
#include <InterViews/canvas.h>
#include <InterViews/cursor.h>
#include <InterViews/event.h>
#include <InterViews/patch.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>
#include <stdio.h>

DragEditor::DragEditor(BoundedValue* bv, WidgetKit* kit, Style* s,
		   Action* up, Action* down)
: InputHandler(nil, s)
{
    bv_ = bv;
    kit_ = kit;
    patch_ = new Patch(kit->label(bv->valuestring()));
    body(patch_);
    up_ = up;
    down_ = down;
}

DragEditor::~DragEditor() {}

void DragEditor::drag (const Event& event) {
    if (!event.left_is_down() && !event.middle_is_down() && !event.right_is_down()) {
	canvas()->window()->cursor(arrow);
	notify();
    }
    else
    if (event.pointer_y() < ey) {
	if (down_) {
	    canvas()->window()->cursor(kit_->dfast_cursor());
	    down_->execute();
	}
    }
    else {
	if (up_) {
	    canvas()->window()->cursor(kit_->ufast_cursor());
	    up_->execute();
	}
    }
    ey = (int)event.pointer_y();
}

void DragEditor::press (const Event& event) {
    ey = (int)event.pointer_y();
}

void DragEditor::release (const Event& event) {
    ey = (int)event.pointer_y();
    canvas()->window()->cursor(arrow);
    notify();
}

void DragEditor::field(const char* str) {
    patch_->body(kit_->label(str));
    patch_->reallocate();
    redraw();
}
