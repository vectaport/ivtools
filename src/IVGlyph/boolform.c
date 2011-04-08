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

#include <IVGlyph/boolform.h>

#include <InterViews/action.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/telltale.h>
#include <IV-look/kit.h>

/*****************************************************************************/

declareActionCallback(CheckBooleanEditor)
implementActionCallback(CheckBooleanEditor)

CheckBooleanEditor::CheckBooleanEditor(ObservableBoolean* obs, char* labl)
: MonoGlyph(), Observer()
{
    WidgetKit& kit_ = *WidgetKit::instance();

    Action* checkedit = new ActionCallback(CheckBooleanEditor)(
	this, &CheckBooleanEditor::edit
    );
    _checkbox = kit_.check_box(labl, checkedit);
    _obs = obs;
    _obs->attach(this);
    _checkbox->state()->set(TelltaleState::is_chosen, _obs->value());
    body(_checkbox);
}

CheckBooleanEditor::~CheckBooleanEditor() {
    _obs->detach(this);
}

void CheckBooleanEditor::edit() {
    _obs->setvalue(_checkbox->state()->test(TelltaleState::is_chosen));
}

void CheckBooleanEditor::update(Observable*) {
    _checkbox->state()->set(TelltaleState::is_chosen, _obs->value());
}

/*****************************************************************************/

declareActionCallback(PaletteBooleanEditor)
implementActionCallback(PaletteBooleanEditor)

PaletteBooleanEditor::PaletteBooleanEditor(ObservableBoolean* obs, char* labl)
: MonoGlyph(), Observer()
{
    WidgetKit& kit_ = *WidgetKit::instance();

    Action* checkedit = new ActionCallback(PaletteBooleanEditor)(
	this, &PaletteBooleanEditor::edit
    );
    _palette = kit_.palette_button(labl, checkedit);
    _obs = obs;
    _obs->attach(this);
    _palette->state()->set(TelltaleState::is_chosen, _obs->value());
    body(_palette);
}

PaletteBooleanEditor::~PaletteBooleanEditor() {
    _obs->detach(this);
}

void PaletteBooleanEditor::edit() {
    _obs->setvalue(_palette->state()->test(TelltaleState::is_chosen));
}

void PaletteBooleanEditor::update(Observable*) {
    _palette->state()->set(TelltaleState::is_chosen, _obs->value());
}

/*****************************************************************************/

BooleanObserver::BooleanObserver(ObservableBoolean* obs, char* labl)
: MonoGlyph(), Observer()
{
    WidgetKit& kit_ = *WidgetKit::instance();
    const LayoutKit& layout_ = *LayoutKit::instance();

    _view = new Patch(kit_.label("false"));
    body(layout_.hbox(kit_.label(labl), _view));
    _obs = obs;
    _obs->attach(this);
    update(_obs);
}

BooleanObserver::~BooleanObserver() {
    _obs->detach(this);
}

void BooleanObserver::update(Observable* obs) {
    WidgetKit& kit_ = *WidgetKit::instance();

    if (((ObservableBoolean*)obs)->value())
	_view->body(kit_.label("true "));
    else
	_view->body(kit_.label("false"));
    _view->redraw();
}
