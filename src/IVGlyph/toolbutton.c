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

#include <InterViews/enter-scope.h>

#include <IV-look/kit.h>

#include <InterViews/action.h>
#include <InterViews/deck.h>
#include <InterViews/event.h>
#include <InterViews/hit.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/style.h>
#include <InterViews/target.h>

#include <OS/string.h>

#include <IVGlyph/observables.h>
#include <IVGlyph/toolbutton.h>

/*****************************************************************************/

ToolButton::ToolButton(Glyph* g, const char* k, Style* s, TelltaleGroup* tg, Action* a,
    ObservableText* mousedoc, const char* doc) : Button(nil, s, 
	new TelltaleState(TelltaleState::is_enabled | TelltaleState::is_choosable), a) {

    _mousedoc = mousedoc;
    _doc = doc;

    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    if (k[0]) {
	infr = kit.bright_inset_frame(
	    layout.overlay(
		layout.center(layout.shape_of_xy(layout.hspace(60), layout.vspace(25))),
		layout.vcenter(layout.hbox(layout.hglue(),kit.label(k)),1.0),
		layout.center(g)
	    )
	);
    }
    else
	infr = kit.bright_inset_frame(
	    layout.overlay(
		layout.center(layout.shape_of_xy(layout.hspace(60), layout.vspace(25))),
		layout.center(g)
	    )
	);
    Resource::ref(infr);
    if (k[0]) {
	outfr = kit.outset_frame(
	    layout.overlay(
		layout.center(layout.shape_of_xy(layout.hspace(60), layout.vspace(25))),
		layout.vcenter(layout.hbox(layout.hglue(),kit.label(k)),1.0),
		layout.center(g)
	    )
	);
    }
    else
	outfr = kit.outset_frame(
	    layout.overlay(
		layout.center(layout.shape_of_xy(layout.hspace(60), layout.vspace(25))),
		layout.center(g)
	    )
	);
    Resource::ref(outfr);
    deck = layout.deck(infr, outfr);
    deck->flip_to(1);
    bod = new Patch(deck);
    body(new Target(bod, TargetPrimitiveHit));
    state()->join(tg);
}

ToolButton::~ToolButton() {
    Resource::unref(infr);
    Resource::unref(outfr);
}

void ToolButton::enter() {}
void ToolButton::leave() {}

void ToolButton::update(Observable* obs) {
    if (((TelltaleState*)obs)->test(TelltaleState::is_chosen)) {
	deck->flip_to(0);
	if (_mousedoc)
	    _mousedoc->textvalue(_doc);
    }
    else
	deck->flip_to(1);
    bod->redraw();
}
