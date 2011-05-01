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

#include <IVGlyph/textform.h>
#include <IVGlyph/charfield.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <IV-look/dialogs.h>
#include <IV-look/field.h>
#include <IV-look/kit.h>
#include <OS/string.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

declareFieldEditorCallback(ObsTextEditor)
implementFieldEditorCallback(ObsTextEditor)

ObsTextEditor::ObsTextEditor(ObservableText* ot, char* labl, int width)
: MonoGlyph(), Observer()
{
    DialogKit& dk = *DialogKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    _obs = ot;
    _obs->attach(this);
    FieldEditorAction* feaction = new FieldEditorCallback(ObsTextEditor)(
	this, &ObsTextEditor::accept_editor, &ObsTextEditor::cancel_editor
    );
    _editor = new CharFieldEditor("", &wk, wk.style(), feaction);
    update(_obs);
    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil) {
	c = new Color(0.7, 0.7, 0.7, 1.0);
    }
    Glyph* mainglyph;
    if (labl)
	mainglyph = lk.vbox(
	    lk.hcenter(wk.label(labl)),
	    lk.vspace(5),
	    lk.hcenter(lk.hfixed(_editor, width))
	);
    else
	mainglyph = lk.hfixed(_editor, width);
    body(new Background(mainglyph, c));
}

ObsTextEditor::~ObsTextEditor() {
}

InputHandler* ObsTextEditor::focusable() const {
    return _editor;
}

void ObsTextEditor::update(Observable* ob) {
    _editor->field(((ObservableText*)ob)->textvalue());
}

void ObsTextEditor::accept_editor(FieldEditor*) {
    _obs->detach(this);
    _obs->textvalue(_editor->text()->string());
    _obs->attach(this);
}

void ObsTextEditor::cancel_editor(FieldEditor*) {
    _obs->textvalue("");
}

/*****************************************************************************/

TextObserver::TextObserver(ObservableText* obs, char* labl, int max)
:MonoGlyph(), Observer()
{
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    char sample[max+1];
    for (int i = 0; i < max; i++)
	sample[i] = ' ';
    sample[max] = '\0';
    _view = new Patch(lk.hbox(wk.label(sample), lk.hglue()));
    body(lk.hbox(wk.label(labl), _view));
    _obs = obs;
    _obs->attach(this);
    update(_obs);
}

TextObserver::~TextObserver() {
    _obs->detach(this);
}

void TextObserver::update(Observable* obs) {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    _view->body(lk.hbox(wk.label(((ObservableText*)obs)->textvalue()), lk.hglue()));
    _view->reallocate();
    _view->redraw();
}
