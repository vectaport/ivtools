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

#include <IVGlyph/bdtable.h>
#include <IVGlyph/bdfltform.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <IV-look/kit.h>

implementPtrList(BoundedValueList,BoundedValue)

BoundedValueTable::BoundedValueTable(StringList* list, BoundedValueList* vlist)
: ObservableEnum(list), Observer()
{
    _values = vlist;
    for (int i = 0; i < _values->count(); i++)
	_values->item(i)->observable(Dimension_X)->attach(this);
}

BoundedValueTable::~BoundedValueTable() {}

void BoundedValueTable::update(Observable*) {
    // one bdvalue changed
    notify();
}

BoundedValue* BoundedValueTable::bdvalue(int i) {
    return _values->item(i);
}

void BoundedValueTable::prepend(const String& s, BoundedValue* bv) {
    _values->prepend(bv);
    ObservableEnum::prepend(s);
}

void BoundedValueTable::append(const String& s, BoundedValue* bv) {
    _values->append(bv);
    ObservableEnum::append(s);
}

void BoundedValueTable::insert(long index, const String& s, BoundedValue* bv) {
    _values->insert(index, bv);
    ObservableEnum::insert(index, s);
}

void BoundedValueTable::remove(long index) {
    _values->remove(index);
    ObservableEnum::remove(index);
}

/*****************************************************************************/

BoundedValueTableEditor::BoundedValueTableEditor(BoundedValueTable* bvt, char* labl)
: Patch(nil), Observer()
{
    _lab = labl;
    _obs = bvt;
    _obs->attach(this);
    build();
    update(_obs);
}

BoundedValueTableEditor::~BoundedValueTableEditor() {}

void BoundedValueTableEditor::edit(String s) {
}

void BoundedValueTableEditor::update(Observable*) {
    if (_obs->listchanged()) {
	for (int i = _labelbox->count()-1; i >= 0; i--) {
	    _labelbox->remove(i);
	    _editbox->remove(i);
	}
	build();
	redraw();
    }
}

void BoundedValueTableEditor::build() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    _mainglyph = lk.vbox();
    _mainglyph->append(lk.hcenter(wk.label(_lab)));
    Glyph* glu = lk.vspace(5);
    _labelbox = lk.vbox();
    _editbox = lk.vbox();
    InputHandler* ih = new InputHandler(nil, wk.style());
    for (int i = 0; i < _obs->maxvalue(); i++) {
	BoundedValueEditor* edit = new BoundedValueEditor(_obs->bdvalue(i), nil,
							  false);
	Resource::ref(edit);
	_editbox->append(glu);
	_editbox->append(edit);
	ih->append_input_handler(edit->focusable());
	Glyph* label = wk.label(_obs->labelvalue(i));
	_labelbox->append(glu);
	_labelbox->append(lk.overlay(lk.center(lk.shape_of_xy(label,edit)),
				     lk.center(label)));
    }
    _mainglyph->append(lk.hcenter(lk.hbox(_labelbox, lk.hspace(10), _editbox)));
    ih->body(wk.inset_frame(lk.margin(_mainglyph, 10)));
    body(ih);
}
