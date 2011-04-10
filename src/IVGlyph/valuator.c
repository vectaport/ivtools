/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

/*
 * Copyright (c) 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
/* derived from idemo/main.c */
/* class Valuator */

#include <IVGlyph/bdvalue.h>
#include <IVGlyph/fieldedit.h>
#include <IVGlyph/valuator.h>
#include <IVGlyph/dragedit.h>
#include <IVGlyph/textbuff.h>

#include <IV-look/dialogs.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/patch.h>
#include <InterViews/style.h>
#include <OS/string.h>
#include <cstdio>
#include <string.h>

DragValuator::DragValuator(BoundedValue* bv, Style* style, Action* up, Action* down)
: MonoGlyph(nil) {
    Style* s = new Style(style);
    s->alias("DragValuator");
    bvalue_ = bv;
    bv->attach(Dimension_X, this);
    editor_ = new DragEditor(bv, WidgetKit::instance(), s, up, down);
    body(editor_);
    update(bv->observable(Dimension_X));
}

DragValuator::~DragValuator() {
    if (bvalue_ != nil) {
	bvalue_->detach(Dimension_X, this);
    }
}

InputHandler* DragValuator::focusable() const {
    return editor_;
}

void DragValuator::update(Observable*) {
    char buf[40];
    sprintf(buf, "%s", bvalue_->valuestring());
    editor_->field(buf);
}

void DragValuator::disconnect(Observable*) {
    bvalue_ = nil;
}

/* class Valuator */

declareGFieldEditorCallback(Valuator)
implementGFieldEditorCallback(Valuator)

Valuator::Valuator(BoundedValue* bv, Style* style, char* sample) : MonoGlyph(nil) {
    Style* s = new Style(style);
    s->alias("Valuator");
    bvalue_ = bv;
    bv->attach(Dimension_X, this);
    editor_ = new GFieldEditor(
	sample, 
	new GFieldEditorCallback(Valuator)(
	    this, &Valuator::accept_editor, &Valuator::cancel_editor
	)
    );
    body(editor_);
    update(bv->observable(Dimension_X));
}

Valuator::~Valuator() {
    if (bvalue_ != nil) {
	bvalue_->detach(Dimension_X, this);
    }
}

InputHandler* Valuator::focusable() const {
    return editor_;
}

void Valuator::update(Observable*) {
    const char* str = bvalue_->valuestring();
    editor_->field()->Delete(0, editor_->field()->Width());
    editor_->field()->Insert(0, str, strlen(str));
    editor_->update();
}

void Valuator::disconnect(Observable*) {
    bvalue_ = nil;
}

void Valuator::accept_editor(GFieldEditor*) {
    Coord v;
    const String value(editor_->text());
    if (value.convert(v)) {
	bvalue_->detach(Dimension_X, this);
	bvalue_->current_value(v);
	bvalue_->attach(Dimension_X, this);
    }
}

void Valuator::cancel_editor(GFieldEditor*) {
    update(bvalue_->observable(Dimension_X));
}
