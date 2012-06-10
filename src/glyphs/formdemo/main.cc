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
#include <IVGlyph/boolform.h>
#include <IVGlyph/enumform.h>
#include <IVGlyph/textform.h>
#include <IVGlyph/bdvalue.h>
#include <IVGlyph/valuator.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/dialog.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>

class App {
public:
    App(Window *, ObservableBoolean* obs, ObservableEnum* obse, 
	BoundedValue* bdv, ObservableText* obst);
    void post_it();
    void accept();
    void cancel();
protected:
    Window     * win;
    Dialog* dlog;
    BoundedValueEditor* val;
};

declareActionCallback(Session)
declareActionCallback(App)
implementActionCallback(App)

App::App(Window * w, ObservableBoolean* obs, ObservableEnum* obse,
     BoundedValue* bdv, ObservableText* obst) {
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    win = w;
    CheckBooleanEditor* chk = new CheckBooleanEditor(obs, "Boolean");
    PaletteBooleanEditor* pal = new PaletteBooleanEditor(obs, "Boolean");
    RadioEnumEditor* rad = new RadioEnumEditor(obse, "Enum");
    CycleEnumEditor* cyc = new CycleEnumEditor(obse, "Enum");
    val = new BoundedValueEditor(bdv, "Bounded Float");
    ObsTextEditor* txt = new ObsTextEditor(obst, "Text");

    Action* cancelaction = new ActionCallback(App)(
	this, &App::cancel
    );
    Button* cancel = wk.push_button("Cancel", cancelaction);

    Action* okaction = new ActionCallback(App)(
	this, &App::accept
    );
    Button* ok = wk.push_button("Done", okaction);

    MonoGlyph* g = wk.outset_frame(
	lk.margin(
	    lk.vbox(
		lk.hcenter(wk.label("Value Editing Dialog")),
		lk.vglue(15),
		lk.hcenter(
		    lk.hbox(
			lk.vcenter(
			    lk.hbox(
				lk.vcenter(pal),
				lk.hglue(40,0,fil),
				lk.vcenter(chk)
			    )
			)
		    )
		),
		lk.vglue(15),
		lk.hcenter(
		lk.hbox(
		    lk.vcenter(rad),
		    lk.hspace(20),
		    lk.vcenter(cyc)
		)),
		lk.vglue(15),
		lk.vbox(
		    lk.hcenter(val),
		    lk.vglue(15),
		    lk.hcenter(txt),
		    lk.vglue(15),
		    lk.hcenter(ok)
		)
	    ),
	    15
	)
    );

    dlog = new Dialog(g, wk.style());
    Resource::ref(dlog);
    dlog->append_input_handler(val->focusable());
    dlog->append_input_handler(txt->focusable());
}

void App::post_it() {
    dlog->post_at_aligned(win->left()+win->width()+10, win->bottom()+win->height(), 0.0, 1.0);
}

void App::cancel() {
    dlog->dismiss(false);
}

void App::accept() {
    dlog->dismiss(true);
}

static OptionDesc options[] = {
    { nil }
};

static PropertyData properties[] = {
    { "*background", "#7d9ec0" },
    { "*radioScale", "2.0" },
    { nil }
};

int main(int argc, char** argv) {
    Session* session = new Session("Forms demo", argc, argv, options, properties);
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    PolyGlyph * mainglyph = lk.vbox();
    ApplicationWindow * mainwin = new ApplicationWindow(mainglyph);

    ObservableBoolean* obsb = new ObservableBoolean();
    StringList* list = new StringList;
    String* str_i;
    str_i = new String("Athos   ");
    list->append(*str_i);
    str_i = new String("Porthos ");
    list->append(*str_i);
    str_i = new String("Aramis  ");
    list->append(*str_i);
    ObservableEnum* obse = new ObservableEnum(list);
    BoundedValue* bdv = new BoundedValue(0.0, 100.0, 5.0, 10.0, 50.0);
    ObservableText* obst = new ObservableText("any text");

    App * tryme = new App(mainwin, obsb, obse, bdv, obst);

    BooleanObserver* bview = new BooleanObserver(obsb,"Boolean Value: ");
    EnumObserver* eview = new EnumObserver(obse, "Enum Value: ");
    BoundedValueObserver* bdview = new BoundedValueObserver(bdv,
							    "Bounded Float Value: ");
    TextObserver* txtview = new TextObserver(obst, "Text Value: ");

    Menu * top_menu = wk.menubar();
    MenuItem *quit = wk.menubar_item("Quit");
    quit->action(new ActionCallback(Session)(Session::instance(),
					     &Session::quit));
    top_menu->append_item(quit);

    MenuItem *post = wk.menubar_item("Edit");
    post->action(new ActionCallback(App)(tryme, &App::post_it));
    top_menu->append_item(post);

    mainglyph->append(
	new Background(
	    lk.vbox(
		top_menu,
		lk.natural(
		    lk.tmargin(
			lk.lmargin(
			    lk.vbox(
				bview,
				lk.vspace(10),
				eview,
				lk.vspace(10),
				bdview,
				lk.vspace(10),
				txtview
			    ),
			    10
			),
			10
		    ),
		    200, 200
		)
	    ),
	    wk.background()
	)
    );

    return session->run_window(mainwin);
}
