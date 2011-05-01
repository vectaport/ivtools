//
// Simple Text Editor Buffer Implementation
//
// Copyright (C) 1993 Ellemtel Telecommunication Systems Labratories
//
// Permission is granted to any individual or institution to use, copy,
// modify, and distribute this software, provided that this complete
// copyright and permission notice is maintained, intact, in all copies
// and supporting documentation.
//
// Ellemtel Telecommunication Systems Labratories make no representation
// of the suitability of this software for any purpose. It is provided
// "as is" without any expressed or implied warranty.
//
// Jan Andersson, Torpa Konsult AB
// janne@torpa.se - 1993-08-29

#include <stdio.h>

#include <IV-look/kit.h>
#include <InterViews/display.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>

#include <IVGlyph/textedit.h>
#include <IVGlyph/textwindow.h>

class App {
public:
   App(int, char**);
   int run();
private:
   Session* session_;
   EivTextEditor* text_edit_;
   TextEditAppWindow* base_;
};

static PropertyData props[] = {
   { "TextEdit*iconName", "Text Editor" },
   { "TextEdit*title", "InterViews Text Editor" },
   { "TextEdit*TextEditor*rows", "30" },
   { "TextEdit*TextEditor*columns", "80" },
//   { "TextEdit*TextEditor*textFont", "lucidasanstypewriter-14" },
   { "TextEdit*TextEditor*FileChooser*rows", "10" },
   { nil }
};

static OptionDesc options[] = {
   { "-rows", "TextEdit*TextEditor*rows", OptionValueNext },
   { nil }
};
   

int main(int argc, char** argv) {
   App* a = new App(argc, argv);
   return a->run();
}

App::App(int argc, char** argv)
{
   session_ = new Session("TextEdit", argc, argv, options, props);
   Display* display = session_->default_display();
   WidgetKit& kit = *WidgetKit::instance();
   const LayoutKit& layout = *LayoutKit::instance();

   text_edit_ = new EivTextEditor(kit.style());

   if (argc > 1)
     text_edit_->load(argv[argc-1]);
   
   base_ = new TextEditAppWindow(text_edit_);
}

int App::run()
{
   return session_->run_window(base_);
}

