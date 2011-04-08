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
#include <InterViews/composition.h>
#include <InterViews/dialog.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <InterViews/simplecomp.h>
#include <InterViews/window.h>
#include <IV-look/dialogs.h>
#include <IV-look/kit.h>

class App {
public:
    App(Window *, ObservableEnum* obse);
    void post_it();
    void accept();
    void cancel();
    void add();
    void remove();
protected:
    Window     * win;
    Dialog* dlog;
    FieldEditor* fe;
    ObservableEnum* obs;
};

declareActionCallback(Session)
declareActionCallback(App)
implementActionCallback(App)

App::App(Window * w, ObservableEnum* obse) {
    DialogKit& dk = *DialogKit::instance();
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();
    Style* s = wk.style();

    obs = obse;
    win = w;

    Action* addaction = new ActionCallback(App)(
	this, &App::add
    );
    Button* addbutton = wk.push_button("Add", addaction);
    Action* remaction = new ActionCallback(App)(
	this, &App::remove
    );
    Button* rembutton = wk.push_button("Remove", remaction);
    fe = dk.field_editor("", s);

    Action* okaction = new ActionCallback(App)(
	this, &App::accept
    );
    Button* ok = wk.push_button("Done", okaction);

    MonoGlyph* g = wk.outset_frame(
	lk.margin(
	    lk.vbox(
		lk.hcenter(wk.label("List Editing Dialog")),
		lk.vglue(15),
		lk.hcenter(lk.hfixed(fe, 125)),
		lk.vglue(15),
		lk.hcenter(
		lk.hbox(
		    lk.vcenter(addbutton),
		    lk.hspace(20),
		    lk.vcenter(rembutton)
		)),
		lk.vglue(15),
		lk.hcenter(ok)
	    ),
	    15
	)
    );

    dlog = new Dialog(g, wk.style());
    Resource::ref(dlog);
    dlog->append_input_handler(fe);
}

void App::post_it() {
    dlog->focus(fe);
    dlog->post_at_aligned(win->left()+win->width()+10, win->bottom()+win->height(), 0.0, 1.0);
}

void App::cancel() {
    dlog->dismiss(false);
}

void App::accept() {
    dlog->dismiss(true);
}

EnumObserver* eview;
RadioEnumEditor* rad;
Menu * top_menu;
TBComposition* comp;

Glyph* buildtop() {
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    comp = new TBComposition(lk.vbox(), new SimpleCompositor(), lk.vspace(10),
			     750);
    comp->append(lk.hcenter(eview));
    comp->append(lk.hcenter(lk.margin(rad, 10)));
    PolyGlyph* mainbox = lk.vbox();
    mainbox->append(
	lk.natural(
	    new Background(
		lk.vbox(
		    lk.hcenter(top_menu),
		    lk.hcenter(
			lk.margin(
			    lk.flexible(
				comp
			    ),
			    10
			)
		    )
		),
		wk.background()
	    )
	    ,300, 300)
    );
    return lk.flexible(mainbox);
}

void App::add() {
    LayoutKit& lk = *LayoutKit::instance();
    const String* txt = fe->text();
    if (txt->length() > 0) {
	if (obs->value(*txt) == -1) {
	    obs->append(*(new CopyString(*txt)));
	    comp->change(comp->count()-1);
	    comp->repair();
	    ((Patch*)win->glyph())->redraw();
	}
    }
}

void App::remove() {
    LayoutKit& lk = *LayoutKit::instance();
    const String* txt = fe->text();
    if (txt->length() > 0) {
	int pos;
	if ((pos = obs->value(*txt)) > -1) {
	    obs->remove(pos);
	    comp->change(comp->count()-1);
	    comp->repair();
	    ((Patch*)win->glyph())->redraw();
	}
    }
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
    Session* session = new Session("Radio Button Group demo", argc, argv, options, properties);
    WidgetKit& wk = *WidgetKit::instance();

    StringList* list = new StringList;
    String* str_i;
    str_i = new String("Athos");
    list->append(*str_i);
    str_i = new String("Porthos");
    list->append(*str_i);
    str_i = new String("Aramis");
    list->append(*str_i);
    ObservableEnum* obse = new ObservableEnum(list);
    eview = new EnumObserver(obse, "Enum Value: ");
    rad = new RadioEnumEditor(obse, "Enum");
    Patch* mainglyph = new Patch(nil);
    ApplicationWindow * mainwin = new ApplicationWindow(mainglyph);

    App * tryme = new App(mainwin, obse);

    top_menu = wk.menubar();
    MenuItem *quit = wk.menubar_item("Quit");
    quit->action(new ActionCallback(Session)(Session::instance(),
					     &Session::quit));
    top_menu->append_item(quit);

    MenuItem *post = wk.menubar_item("Edit");
    post->action(new ActionCallback(App)(tryme, &App::post_it));
    top_menu->append_item(post);
    mainglyph->body(buildtop());
    return session->run_window(mainwin);
}
