/*
 * Copyright 1995 Lawrence Berkeley Laboratory
 * Copyright 1994 Vectaport Inc.
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
#include <InterViews/color.h>
#include <IVGlyph/enumform.h>
#include <IVGlyph/textform.h>
#include <IVGlyph/bdvalue.h>
#include <IVGlyph/valuator.h>

#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/dialog.h>
#include <InterViews/display.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <IV-look/kit.h>

class App {
public:
    App(Window*, BoundedValue*, BoundedValue*, BoundedValue*);
    void post_it();

    void buildmeter();
    void meter_it();
    void cancel_meter();

    void buildmultimeter();
    void multimeter_it();
    void cancel_multimeter();

    void accept();
    void cancel();

protected:
    Window* win_;
    Dialog* dlog;
    BoundedValue* bdv1_;
    BoundedValue* bdv2_;
    BoundedValue* bdv3_;
    Background* meter_;
    Background* multimeter_;
    TransientWindow* transient_;
    TransientWindow* multitransient_;
};

declareActionCallback(Session)
declareActionCallback(App)
implementActionCallback(App)

App::App(Window *w, BoundedValue* bdv1, BoundedValue* bdv2, BoundedValue* bdv3) {
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    bdv1_ = bdv1;
    bdv2_ = bdv2;
    bdv3_ = bdv3;
    win_ = w;

    Action* cancel = new ActionCallback(App)(this, &App::cancel);
    String cancelstr("Cancel");
    
    Action* accept = new ActionCallback(App)(this, &App::accept);
    String acceptstr("OK");

    BoundedValueEditor* bdve1 = new BoundedValueEditor(bdv1, nil, false);
    BoundedValueEditor* bdve2 = new BoundedValueEditor(bdv2, nil, false);
    BoundedValueEditor* bdve3 = new BoundedValueEditor(bdv3, nil, false);

    MonoGlyph* g = wk.outset_frame(
	lk.margin(
	    lk.vbox(
		lk.hcenter(wk.label("Meter Example Program")),
		lk.vglue(15),
            lk.hbox(lk.vcenter(wk.label("Start:")),
		    lk.hspace(15),
		    lk.vcenter(bdve1),
		    lk.hspace(7),
		    lk.vcenter(wk.label(""))),
	    lk.vglue(15),
            lk.hbox(lk.vcenter(wk.label("Stop:")),
		    lk.hspace(15),
		    lk.vcenter(bdve2),
		    lk.hspace(7),
		    lk.vcenter(wk.label(""))),
	lk.vspace(15.0),
	lk.hbox(
	    lk.hglue(10.0),
	    lk.vcenter(wk.push_button(cancelstr, cancel)),
	    lk.hglue(10.0, 0.0, 5.0),
	    lk.vcenter(wk.push_button(acceptstr, accept)),
	    lk.hglue(10.0))),
	15));

    dlog = new Dialog(g, wk.style());
    Resource::ref(dlog);
    dlog->append_input_handler(bdve1->focusable());
    dlog->append_input_handler(bdve2->focusable());
}

void App::buildmeter() {
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    Action* cancelmeteraction = new ActionCallback(App)(this, &App::cancel_meter);
    Button* cancelmeter = wk.push_button("Cancel", cancelmeteraction);
    MeterObserver* meterobs = new MeterObserver(bdv1_, "int value");
    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil)
        c = new Color(0.7, 0.7, 0.7, 1.0);
    PolyGlyph* meterpoly = lk.vbox(
	lk.vglue(5),
        lk.hcenter(lk.margin(wk.label("Meter Observer"), 5)),
	lk.hcenter(meterobs),
	lk.vglue(7),
	lk.hcenter(cancelmeter),
	lk.vglue(5)
    );
    meter_ = new Background(meterpoly, c);
}

void App::buildmultimeter() {
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    Action* cancelmultimeteraction = new ActionCallback(App)(this, &App::cancel_multimeter);
    Button* cancelmultimeter = wk.push_button("Cancel", cancelmultimeteraction);
    MeterObserver* meterobs1 = new MeterObserver(bdv1_, "int value");
    MeterObserver* meterobs2 = new MeterObserver(bdv2_, "float value", false);
    MeterObserver* meterobs3 = new MeterObserver(bdv3_, "float value", false);
    Display* d = Session::instance()->default_display();
    const Color* c = Color::lookup(d, "#aaaaaa");
    if (c == nil)
        c = new Color(0.7, 0.7, 0.7, 1.0);

    PolyGlyph* multimeterpoly = lk.vbox(
	lk.vglue(5),
        lk.hcenter(lk.margin(wk.label("Multi-Meter Observer"), 5)),
        lk.hcenter( 
            lk.hbox(
	        lk.hcenter(meterobs1),
	    	lk.hcenter(meterobs2),
	    	lk.hcenter(meterobs3)
            )
        ),
	lk.vglue(7),
	lk.hcenter(cancelmultimeter),
	lk.vglue(5)
    );
    multimeter_ = new Background(multimeterpoly, c);
}

void App::post_it() {
    dlog->post_at_aligned(win_->left()+win_->width()+10, win_->bottom()+win_->height(), 0.0, 1.0);
}

void App::meter_it() {
    buildmeter();
    transient_ = new TransientWindow(meter_);
    transient_->map();
}

void App::multimeter_it() {
    buildmultimeter();
    multitransient_ = new TransientWindow(multimeter_);
    multitransient_->map();
}

void App::cancel_meter() {
    transient_->unmap();
    transient_->display()->sync();
    delete transient_;
}

void App::cancel_multimeter() {
    multitransient_->unmap();
    multitransient_->display()->sync();
    delete multitransient_;
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
    Session* session = new Session("meter demo", argc, argv, options, properties);
    WidgetKit& wk = *WidgetKit::instance();
    LayoutKit& lk = *LayoutKit::instance();

    PolyGlyph * mainglyph = lk.vbox();
    ApplicationWindow * mainwin = new ApplicationWindow(mainglyph);

    BoundedValue* bdv1 = new BoundedValue(0.0, 1000.0, 5.0, 10.0, 747, "%6.0f");
    BoundedValue* bdv2 = new BoundedValue(0.0, 1000.0, 5.0, 10.0, 512, "%6.0f");
    BoundedValue* bdv3 = new BoundedValue(0.0, 1000.0, 5.0, 10.0, 66, "%6.0f");

    App * tryme = new App(mainwin, bdv1, bdv2, bdv3);

    BoundedValueObserver* bdview1 = new BoundedValueObserver(bdv1,
        "Bounded Float Value: ");
    BoundedValueObserver* bdview2 = new BoundedValueObserver(bdv2,
        "Bounded Float Value: ");
    BoundedValueObserver* bdview3 = new BoundedValueObserver(bdv3,
        "Bounded Float Value: ");

    Menu * top_menu = wk.menubar();

    MenuItem *quit = wk.menubar_item("Quit");
    quit->action(new ActionCallback(Session)(Session::instance(), &Session::quit));
    top_menu->append_item(quit);

    MenuItem *meter = wk.menubar_item("Meter");
    meter->action(new ActionCallback(App)(tryme, &App::meter_it));
    top_menu->append_item(meter);

    MenuItem *multimeter = wk.menubar_item("MultiMeter");
    multimeter->action(new ActionCallback(App)(tryme, &App::multimeter_it));
    top_menu->append_item(multimeter);

    mainglyph->append(
	new Background(
	    lk.vbox(
		top_menu,
		lk.natural(
		    lk.tmargin(
			lk.lmargin(
			    lk.vbox(
				bdview1,
				lk.vspace(10),
				bdview2,
				lk.vspace(10),
				bdview3
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
