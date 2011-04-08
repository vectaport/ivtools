/*
 * Copyright (c) 1993 David B. Hollenbeck
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notice and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * David B. Hollenbeck may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of David B. Hollenbeck.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL DAVID B. HOLLENBECK BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <InterViews/background.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <InterViews/label.h>
#include <InterViews/scrbox.h>
#include <InterViews/layout.h>
#include <IV-look/mf_kit.h>
#include <IV-look/fbrowser.h>
#include <OS/string.h>
#include <IVGlyph/strchooser.h>
#include <iostream.h>

static OptionDesc options[] = {
    { nil }
};

static PropertyData properties[] = {
    { "*background", "#aaaaaa" },
    { nil }
};

class App {
public:
  App(Window *);
  void post_the_file_chooser();
private:
  StrChooser * pickme;
  Window     * winner;
  StringList * choices;
};


App::App(Window * w) {
  winner = w;
  choices = new StringList;

  String * str_i;

  str_i = new String("Item 1");
  choices->append(*str_i);
  str_i = new String("Item 2");
  choices->append(*str_i);
  str_i = new String("Item 3");
  choices->append(*str_i);
  str_i = new String("Item 4");
  choices->append(*str_i);
  str_i = new String("Item 5");
  choices->append(*str_i);
  str_i = new String("Item 6");
  choices->append(*str_i);
  str_i = new String("Item 7");
  choices->append(*str_i);
  str_i = new String("Item 8");
  choices->append(*str_i);
  str_i = new String("Item 9");
  choices->append(*str_i);
  str_i = new String("Item 10");
  choices->append(*str_i);
  str_i = new String("Item 11");
  choices->append(*str_i);
  str_i = new String("Item 12");
  choices->append(*str_i);
  str_i = new String("Item 13");
  choices->append(*str_i);
  str_i = new String("Item 14");
  choices->append(*str_i);
  str_i = new String("Item 15");
  choices->append(*str_i);
  str_i = new String("Item 16");
  choices->append(*str_i);
  str_i = new String("Item 17");
  choices->append(*str_i);
  str_i = new String("Item 18");
  choices->append(*str_i);
  str_i = new String("Item 19");
  choices->append(*str_i);
  str_i = new String("Item 20");
  choices->append(*str_i);

  
  pickme = new StrChooser(choices,
			  new String("Select an Item:"),
			  MFKit::instance(),
			  Session::instance()->style());
  Resource::ref(pickme);
}

void App::post_the_file_chooser() {
  pickme->post_for(winner);
  String selectme;
  if (pickme->selected() > -1)
    selectme = choices->item_ref(pickme->selected());
  else
    selectme = "";
  cout << "You picked \"" << selectme.string() << "\"" << endl;
}

declareActionCallback(Session)

declareActionCallback(App)
implementActionCallback(App)

main(int argc, char *argv[]) {
  Session * session =  new Session("strchooser", argc, argv, options, properties);
  LayoutKit &    lk = *LayoutKit::instance();
  WidgetKit *    wk =  MFKit::instance();

  PolyGlyph * mainglyph = lk.vbox();

  ApplicationWindow * mainwin = new ApplicationWindow(mainglyph);

  App * tryme = new App(mainwin);

  Menu * top_menu = wk->menubar();
  MenuItem *quit = wk->menubar_item("Quit");
  quit->action(new ActionCallback(Session)(Session::instance(),
					   &Session::quit));
  top_menu->append_item(quit);

  MenuItem *post = wk->menubar_item("Post");
  post->action(new ActionCallback(App)(tryme, &App::post_the_file_chooser));
  top_menu->append_item(post);

    mainglyph->append(
	new Background(
	    lk.vcenter(
		lk.vbox(
		    top_menu,
		    lk.fixed(nil, 200, 200)
		    )
	    ),
	    wk->background()
	)
    );

  return session->run_window(mainwin);

}
