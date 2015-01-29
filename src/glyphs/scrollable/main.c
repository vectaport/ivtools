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

/* original code released into the public domain by Chen Wang (chen@ie.toronto.edu) */

#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <InterViews/page.h>
#include <InterViews/patch.h>
#include <stdio.h>
#include <InterViews/debug.h>

#include <IVGlyph/scrollable.h>

class App {
public:
    void msg();
};

declareActionCallback(App)
implementActionCallback(App)
declareActionCallback(Session)

void App::msg() {
    printf("hi mom!\n");
}

int main(int argc, char** argv) {
    Session* session = new Session("Himom", argc, argv);
    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    App* a = new App;
    TelltaleGroup* group = new TelltaleGroup;

    Page* pg = new Page(nil);
    pg->insert(0, 
            kit.push_button("Hi", new ActionCallback(App)(a, &App::msg))
	   );
    pg->move(0, 50, 50);
    pg->show(0, true);
    
    Scrollable* s = new Scrollable(pg, pg, 300, 300);
 
    Glyph* g2 = layout.vcenter(layout.variable_span(
                 layout.natural_span(s, 200, 200)), 1.0);

    Menu * top_menu = kit.menubar();
    MenuItem *quit = kit.menubar_item("Quit");
    ActionCallback(Session) *acb = new ActionCallback(Session)(Session::instance(),
					     &Session::quit);
    quit->action(acb);
    top_menu->append_item(quit);

    session->run_window(
	new ApplicationWindow(
	    new Background(
		layout.vbox(
		    top_menu,
		    layout.hbox(
			g2,
			kit.vscroll_bar(s)
		    ),
		    kit.hscroll_bar(s),
		    layout.hglue(5.0)
		),
		kit.background()
	    )
	)
    );
    return 0;
}
