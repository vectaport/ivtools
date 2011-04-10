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

#include <IVGlyph/bdfltform.h>
#include <IVGlyph/bdvalue.h>
#include <IVGlyph/valuator.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <IV-look/kit.h>
#include <cstdio>
#include <string.h>

/*****************************************************************************/

BoundedValueEditor::BoundedValueEditor(BoundedValue* bdv, char* labl, boolean scr)
: MonoGlyph()
{
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    val = new Valuator(bdv, wk.style());
    Glyph* scb;
    if (scr)
	scb = wk.hscroll_bar(bdv);
    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil) {
	c = new Color(0.7, 0.7, 0.7, 1.0);
    }
    PolyGlyph* ebox = lk.vbox();
    if (labl) {
	ebox->append(lk.hcenter(wk.label(labl)));
	ebox->append(lk.vspace(5));
    }
    char bufmax[40];
    char bufmin[40];
    sprintf(bufmin, bdv->format(), bdv->lower(Dimension_X));
    sprintf(bufmax, bdv->format(), bdv->upper(Dimension_X));
    ebox->append(
	lk.hcenter(
	    lk.overlay(
		lk.center(lk.hmargin(lk.shape_of(wk.label(bufmin)), 4.0)),
		lk.center(lk.hmargin(lk.shape_of(wk.label(bufmax)), 4.0)),
		lk.center(val)
	    )
	)
    );
    if (scr) {
	ebox->append(lk.vspace(5));
	ebox->append(lk.hcenter(lk.hfixed(scb, 150)));
    }
    Background* bg = new Background(ebox, c);
    body(bg);
}

BoundedValueEditor::~BoundedValueEditor() {}

InputHandler* BoundedValueEditor::focusable() {
    return val->focusable();
}

void BoundedValueEditor::accept() {
    val->accept_editor(nil);
}

/*****************************************************************************/

BoundedValueObserver::BoundedValueObserver(BoundedValue* bdv, char* labl)
: MonoGlyph(), Observer()
{
    WidgetKit& kit_ = *WidgetKit::instance();
    const LayoutKit& layout_ = *LayoutKit::instance();

    _view = new Patch(kit_.label("                    "));
    _value = bdv;
    _value->attach(Dimension_X, this);
    update(nil);
    
    body(layout_.hbox(kit_.label(labl), _view));
}

BoundedValueObserver::~BoundedValueObserver() {
    _value->detach(Dimension_X, this);
}

void BoundedValueObserver::update(Observable* obs) {
    WidgetKit& kit_ = *WidgetKit::instance();

    _view->body(kit_.label(_value->valuestring()));
    _view->redraw();
}

/*****************************************************************************/

MeterObserver::MeterObserver(BoundedValue* bdv, char* label, boolean int_display) 
: MonoGlyph(), Observer() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    _int_display = int_display;
    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil)
        c = new Color(0.7, 0.7, 0.7, 1.0);

    _view = new Patch(wk.label("         "));
    _value = bdv;
    _value->attach(Dimension_Y, this);

    Glyph* scb = wk.vslider(bdv);
    PolyGlyph* interior = lk.vbox();

    interior->append(lk.hcenter(lk.vfixed(scb, 150)));
    interior->append(lk.vspace(5));
    interior->append(lk.hcenter(wk.label(label)));
    interior->append(lk.vspace(5));
    interior->append(lk.hcenter(_view));

    Background* bg = new Background(lk.margin(interior, 5), c);
    body(bg);

    Coord v = _value->cur_lower(Dimension_Y);
    char buf[40];
    _int_display ? sprintf(buf, "%i", (int)v) : sprintf(buf, "%.2f", v);
    _view->body(wk.label(buf));
    _view->redraw();
}

MeterObserver::~MeterObserver() {
    _value->detach(Dimension_Y, this);
}

void MeterObserver::update(Observable* o) {
    WidgetKit& wk = *WidgetKit::instance();

    Coord v = _value->cur_lower(Dimension_Y);
    char buf[40];
    _int_display ? sprintf(buf, "%i", (int)v) : sprintf(buf, "%.2f", v);
    _view->body(wk.label(buf));
    _view->reallocate();
    _view->redraw();
    _view->draw(_view->canvas(), _view->allocation());
}

