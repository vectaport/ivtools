/*
 * Copyright (c) 1995 Vectaport Inc.
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
#include <IVGlyph/fieldedit.h>
#include <IVGlyph/scrollable.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <InterViews/page.h>
#include <InterViews/patch.h>
#include <InterViews/target.h>
#include <IV-look/kit.h>
#include <IV-look/slider.h>

#include <stdio.h>

declareActionCallback(Session)

#if 0
class MyScrollable : public Scrollable {
public:
    MyScrollable(Glyph* g, Page* pg, Coord w, Coord h);
    virtual Coord lower(DimensionName) const;
    virtual Coord upper(DimensionName) const;
    virtual Coord length(DimensionName) const;
    Coord ylower_, yupper_;
};

MyScrollable::MyScrollable(Glyph* g, Page* pg, Coord w, Coord h)
: Scrollable(g, pg, w, h)
{
}

Coord MyScrollable::lower(DimensionName d) const {
    if (d == Dimension_X) return 0;
    else return ylower_;
}
Coord MyScrollable::upper(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return yupper_;
}
Coord MyScrollable::length(DimensionName d) const {
    if (d == Dimension_X) return width_;
    else return yupper_ - ylower_;
}

#endif

class App {
public:
    App();
    void build();
    void add();
    void del();

    int window_width;
    int window_height;
    int feditor_width;
    int feditor_height;
    int ncols;
    int nrows;
    int scrollable_width;
    int scrollable_height;
    GlyphIndex index;
    Page* pg;
    InputHandler* topih;
#if 0
    MyScrollable* s;
#else
    Scrollable* s;
#endif
    Background* top;
};

declareActionCallback(App)
implementActionCallback(App)

App::App() {
    build();
}

void App::build() {
    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    window_width = 200;
    window_height = 200;
    feditor_width = 50;
    feditor_height = 16;
    ncols = 8;
    nrows = 16;
    scrollable_width = ncols*feditor_width;
    scrollable_height = nrows*feditor_height;
    pg = new Page(nil);
    Style* style = kit.style();
    topih = new InputHandler(pg, style);
    index = 0;
    for (int i=0; i<nrows; i++) {
	for (int j=0; j<ncols; j++) {
	    char str[10];
	    str[3] = '\0';
	    sprintf(str, "%d:%x", j,i);
	    GFieldEditor* gfield =  new GFieldEditor(str, nil, feditor_width);
	    pg->insert(index, layout.fixed_span(gfield, feditor_width, feditor_height));
	    pg->move(index, j*feditor_width, scrollable_height - feditor_height/2 - feditor_height*i);
	    pg->show(index, true);
	    topih->append_input_handler(gfield);
	    index++;
	}
    }

#if 0    
    s = new MyScrollable(topih, pg, scrollable_width, scrollable_height);
#else
    s = new Scrollable(topih, pg, scrollable_width, scrollable_height);
#endif
#if 0
    s->yupper_ = s->ylower_ = scrollable_height;
#endif
    s->scroll_to(Dimension_Y, scrollable_height-window_height);
 
    Glyph* g2 = layout.vcenter(layout.variable_span(
                 layout.natural_span(s, window_width, window_height)), 1.0);

    Glyph* scbv = kit.vscroll_bar(s);
    Glyph* scbh = kit.hscroll_bar(s);

    Menu * top_menu = kit.menubar();
    MenuItem *quit = kit.menubar_item("Quit");
    ActionCallback(Session) *acb = new ActionCallback(Session)(Session::instance(),
					     &Session::quit);
    quit->action(acb);
    top_menu->append_item(quit);

#if 0
    MenuItem* addm = kit.menubar_item("Add");
    Action* addcb = new ActionCallback(App)(
	this, &App::add
    );
    addm->action(addcb);
    top_menu->append_item(addm);

    MenuItem* delm = kit.menubar_item("Del");
    Action* delcb = new ActionCallback(App)(
	this, &App::del
    );
    delm->action(delcb);
    top_menu->append_item(delm);
#endif

    top = new Background(
	layout.vbox(
	    top_menu,
	    layout.hbox(
		g2,
		scbv
	    ),
	    scbh,
	    layout.hglue(5.0)
	),
	kit.background()
    );
}

void App::add() {
    const LayoutKit& layout = *LayoutKit::instance();
    int i = nrows++;
    for (int j=0; j<ncols; j++) {
	char str[10];
	str[3] = '\0';
	sprintf(str, "%d:%x", j,i);
	GFieldEditor* gfield =  new GFieldEditor(str, nil, feditor_width);
	pg->insert(index,
		  layout.fixed_span(gfield, feditor_width, feditor_height));
	pg->move(index, j*feditor_width,
		scrollable_height - feditor_height/2 - feditor_height*i);
	pg->change(index);
	topih->append_input_handler(gfield);
	index++;
    }
    s->height_ += feditor_height;
#if 0
    s->ylower_ = scrollable_height - feditor_height/2.0 - feditor_height*i;
#endif
    s->reallocate();
    s->redraw();
    s->notify(Dimension_Y);
}

void App::del() {
    --nrows;
    for (int j=0; j<ncols; j++) {
	index--;
	pg->change(index);
	pg->remove(index);
    }
    s->redraw();
    s->height_ -= feditor_height;
#if 0
    s->ylower_ += feditor_height;
#endif
    s->reallocate();
    s->redraw();
    s->notify(Dimension_Y);

}

int main(int argc, char** argv) {
    Session* session = new Session("scrollfield", argc, argv);
    App* app = new App();
    session->run_window(
	new ApplicationWindow(
	    app->top
      )
    );
    return 0;
}
