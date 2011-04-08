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

#include <IVGlyph/enumform.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/bitmap.h>
#include <InterViews/color.h>
#include <InterViews/deck.h>
#include <InterViews/display.h>
#include <InterViews/font.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <InterViews/stencil.h>
#include <InterViews/style.h>
#include <InterViews/telltale.h>
#include <IV-look/kit.h>

#define cycle_width 17
#define cycle_height 17
static unsigned char cycle_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0f, 0x00, 0x30, 0x1f, 0x00,
   0x38, 0x30, 0x00, 0x3c, 0x60, 0x00, 0x3c, 0xc0, 0x00, 0x36, 0xc0, 0x00,
   0x26, 0xc8, 0x00, 0x06, 0xd8, 0x00, 0x06, 0x78, 0x00, 0x0c, 0x78, 0x00,
   0x18, 0x38, 0x00, 0xf0, 0x19, 0x00, 0xe0, 0x09, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};
#define bkcycle_width 17
#define bkcycle_height 17
static unsigned char bkcycle_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x09, 0x00, 0xf0, 0x19, 0x00,
   0x18, 0x38, 0x00, 0x0c, 0x78, 0x00, 0x06, 0x78, 0x00, 0x06, 0xd8, 0x00,
   0x26, 0xc8, 0x00, 0x36, 0xc0, 0x00, 0x3c, 0xc0, 0x00, 0x3c, 0x60, 0x00,
   0x38, 0x30, 0x00, 0x30, 0x1f, 0x00, 0x20, 0x0f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};
#define dnarrow_width 17
#define dnarrow_height 17
static unsigned char dnarrow_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
   0x00, 0x3c, 0x00, 0x00, 0x3f, 0x00, 0xc0, 0x3f, 0x00, 0xf0, 0x3f, 0x00,
   0xf8, 0x3f, 0x00, 0xf0, 0x3f, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0x3f, 0x00,
   0x00, 0x3c, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};
#define uparrow_width 17
#define uparrow_height 17
static unsigned char uparrow_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
   0x78, 0x00, 0x00, 0xf8, 0x01, 0x00, 0xf8, 0x07, 0x00, 0xf8, 0x1f, 0x00,
   0xf8, 0x3f, 0x00, 0xf8, 0x1f, 0x00, 0xf8, 0x07, 0x00, 0xf8, 0x01, 0x00,
   0x78, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};

/*****************************************************************************/

declareEnumActionCallback(MenuEnumEditor)
implementEnumActionCallback(MenuEnumEditor)

MenuEnumEditor::MenuEnumEditor(ObservableEnum* obs, Macro* macro)
: Patch(nil)
{
    _obs = obs;
    _macro = macro;
    build();
}

MenuEnumEditor::MenuEnumEditor() : Patch(nil) {}

void MenuEnumEditor::build() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    body(buildmenu());
}

Glyph* MenuEnumEditor::buildmenu() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    int i;
    Coord maxwidth = 0;
    Coord tempwid;
    for (i = 0; i < _obs->maxvalue(); i++) {
	if ((tempwid = wk.font()->width(_obs->labelvalue(i).string(),
					_obs->labelvalue(i).length()))
	    > maxwidth)
	    maxwidth = tempwid;
    }
    _enumobs = new EnumObserver(_obs, "", maxwidth);
    _menu = wk.menubar();
    MenuItem* mbi = wk.menubar_item(_enumobs);
    mbi->menu(wk.pullright());
    Action* action1;
    Action* action2;
    for (i = 0; i < _obs->maxvalue(); i++) {
	action1 = new EnumActionCallback(MenuEnumEditor)(
	    this, &MenuEnumEditor::edit, _obs->labelvalue(i)
	);
	_macro ? action2 = _macro->action(i) : action2 = nil;
	MenuItem* mi = wk.menu_item(lk.overlay(
	    lk.vcenter(lk.hspace(maxwidth)),
	    lk.vcenter(wk.label(_obs->labelvalue(i)))));
	mbi->menu()->append_item(mi);
	mi->action(new Macro(action1, action2));
    }
    _menu->append_item(mbi);
    return lk.hfixed(_menu, maxwidth+20);
}

MenuEnumEditor::~MenuEnumEditor() {
}

void MenuEnumEditor::edit(String i) {
    _obs->setvalue(_obs->value(i));
}

/*****************************************************************************/

declareEnumActionCallback(RadioEnumEditor)
implementEnumActionCallback(RadioEnumEditor)

RadioEnumEditor::RadioEnumEditor(ObservableEnum* obs, char* labl,
				 boolean horiz, boolean noframe)
: Patch(nil), Observer()
{
    lab = labl;
    _group = new TelltaleGroup;
    _group->ref();
    _obs = obs;
    _obs->attach(this);
    _horiz = horiz;
    _noframe = noframe;
    build();
    update(_obs);
}

RadioEnumEditor::RadioEnumEditor() : Patch(nil), Observer() {}

void RadioEnumEditor::build() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    mainglyph = lk.vbox();
    mainglyph->append(lk.hcenter(wk.label(lab)));
    buildbox();
    mainglyph->append(lk.hcenter(_buttonbox));
    if (_noframe) 
      body(mainglyph);
    else
      body(wk.inset_frame(lk.margin(mainglyph, 10)));
}

void RadioEnumEditor::buildbox() {
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();
    Glyph* glu = _horiz ? lk.hspace(5) : lk.vspace(5);
    _buttonbox = _horiz ? lk.hbox() : lk.vbox();
    Style *style_ = new Style(Session::instance()->style());
    style_->attribute("frameThickness", "2.5");
    style_->attribute("radioScale", "1.0");
    wk.push_style();
    wk.style(style_);
    for (int i = 0; i < _obs->maxvalue(); i++) {
	Action* action = new EnumActionCallback(RadioEnumEditor)(
	    this, &RadioEnumEditor::edit, _obs->labelvalue(i)
	);
	Button* button = wk.radio_button(_group, _obs->labelvalue(i), action);
	_buttonbox->append(_horiz ? lk.hbox(glu, button) : lk.vbox(glu, button));
    }
    wk.pop_style();
}

RadioEnumEditor::~RadioEnumEditor() {
    _obs->detach(this);
    _group->unref();
}

void RadioEnumEditor::edit(String i) {
    _obs->setvalue(_obs->value(i));
}

void RadioEnumEditor::update(Observable* obs) {
    if (_obs->listchanged()) {
	for (int i = _buttonbox->count()-1; i >= 0; i--) {
	    _buttonbox->remove(i);
	}
	build();
	redraw();
    }
    ((Button*)((PolyGlyph*)_buttonbox->component(_obs->intvalue()))->component(1))
	->state()->set(TelltaleState::is_chosen, true);
}

/*****************************************************************************/

declareActionCallback(CycleEnumEditor)
implementActionCallback(CycleEnumEditor)

CycleEnumEditor::CycleEnumEditor(ObservableEnum* obs, char* labl)
: MonoGlyph(), Observer()
{
    WidgetKit& wk = *WidgetKit::instance();
    const LayoutKit& lk = *LayoutKit::instance();

    _obs = obs;
    _obs->attach(this);

    Stencil* sten = new Stencil(new Bitmap(cycle_bits, cycle_width, cycle_height),
			    wk.foreground());
    Action* action = new ActionCallback(CycleEnumEditor)(
	this, &CycleEnumEditor::cycle
    );
    Button* _button = wk.push_button(lk.center(sten), action);

    Stencil* bksten = new Stencil(new Bitmap(bkcycle_bits, bkcycle_width, bkcycle_height),
			    wk.foreground());
    Action* bkaction = new ActionCallback(CycleEnumEditor)(
	this, &CycleEnumEditor::bkcycle
    );
    Button* bkbutton = wk.push_button(lk.center(bksten), bkaction);

    Action* upaction = new ActionCallback(CycleEnumEditor)(
	this, &CycleEnumEditor::up
    );
    Button* upbutton = wk.push_button(
	lk.center(
	    new Stencil(
		new Bitmap(uparrow_bits, uparrow_width, uparrow_height),
		wk.foreground()
	    )
	),
	upaction);

    Action* dnaction = new ActionCallback(CycleEnumEditor)(
	this, &CycleEnumEditor::down
    );
    Button* dnbutton = wk.push_button(
	lk.center(
	    new Stencil(
		new Bitmap(dnarrow_bits, dnarrow_width, dnarrow_height),
		wk.foreground()
	    )
	),
	dnaction);

    _values = lk.deck(_obs->maxvalue());
    for (int i = 0; i < _obs->maxvalue(); i++)
	_values->append(wk.label(_obs->labelvalue(i)));
    _view = new Patch(_values);
    update(_obs);

    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil) {
	c = new Color(0.7, 0.7, 0.7, 1.0);
    }
    body(new Background(
	lk.vbox(
	    lk.hcenter(
		lk.hbox(wk.label(labl), lk.hspace(10), _view)),
	    lk.vspace(5),
	    lk.hcenter(
		lk.hbox(
		    lk.vcenter(
			lk.vbox(
			    _button,
			    lk.vspace(5),
			    bkbutton
			)
		    ),
		    lk.hspace(5),
		    lk.vcenter(
			lk.vbox(
			    dnbutton,
			    lk.vspace(5),
			    upbutton
			)
		    )
		)
	    )
	),
	c));
}

CycleEnumEditor::~CycleEnumEditor() {
    _obs->detach(this);
}

void CycleEnumEditor::cycle() {
    if (_obs->intvalue()+1 == _obs->maxvalue())
	_obs->setvalue(0);
    else
	_obs->setvalue(_obs->intvalue()+1);
}

void CycleEnumEditor::bkcycle() {
    if (_obs->intvalue()-1 < 0)
	_obs->setvalue(_obs->maxvalue()-1);
    else
	_obs->setvalue(_obs->intvalue()-1);
}

void CycleEnumEditor::up() {
    if (_obs->intvalue()+1 < _obs->maxvalue())
	_obs->setvalue(_obs->intvalue()+1);
}

void CycleEnumEditor::down() {
    if (_obs->intvalue()-1 >= 0)
	_obs->setvalue(_obs->intvalue()-1);
}

void CycleEnumEditor::update(Observable* obs) {
    _values->flip_to(((ObservableEnum*)obs)->intvalue());
    _view->redraw();
}

/*****************************************************************************/

EnumObserver::EnumObserver(ObservableEnum* obs, char* labl, Coord minwid)
: MonoGlyph(), Observer()
{
    WidgetKit& kit_ = *WidgetKit::instance();
    const LayoutKit& layout_ = *LayoutKit::instance();

    minw = minwid;
    _view = new Patch(layout_.hfixed(kit_.label(""), minw));
    body(layout_.hbox(kit_.label(labl), _view));
    _obs = obs;
    _obs->attach(this);
    update(_obs);
}

EnumObserver::~EnumObserver() {
    _obs->detach(this);
}

void EnumObserver::update(Observable* obs) {
    WidgetKit& kit_ = *WidgetKit::instance();
    const LayoutKit& layout_ = *LayoutKit::instance();

    _view->redraw();
    _view->body(layout_.hfixed(
	kit_.label(((ObservableEnum*)obs)->labelvalue()), 
	minw));
    _view->redraw();
}
