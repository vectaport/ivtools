//
// Simple Text Editor Implementation
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
//

#ifndef textview_h
#define textview_h

#include <InterViews/action.h>
#include <InterViews/input.h>
#include <InterViews/observe.h>
#include <InterViews/selection.h>

class EivTextBuffer;
class Event;
class OpenFileChooser;
class Font;
class Menu;
class PopupWindow;
class Style;
class TE_Editor;
class TE_View;
class Window;

// key mapping information

typedef void (TE_View::*TE_ViewKeyFunc)();

struct TE_ViewKeyInfo {
   char key;
   TE_ViewKeyFunc func;
};
static const int keymap_size = 256;
struct TE_ViewKeySymInfo {
   unsigned long keysym;
   TE_ViewKeyFunc func;
};

struct CommandInfo;

class TE_View :public InputHandler, public Observable {
public:
   TE_View(Style*, EivTextBuffer*, int rows, int cols, boolean active);
   ~TE_View();

   void load_popup();
   void save_popup();

   int load(const char* path);
   int save_as(const char* path);
   int save_current();
   void save();

   // InputHandler
   virtual void press(const Event&);
   virtual void drag(const Event&);
   virtual void release(const Event&);
   virtual void keystroke(const Event&);
   virtual void double_click(const Event&);

   // various edit functions
   void up();
   void down();
   void left();
   void right();
   void page_up();
   void page_down();
   void forward_char(const int cont = 1);
   void backward_char(const int cont = 1);
   void forward_word(const int cont = 1);
   void backward_word(const int cont = 1);
   void forward_line(const int cont = 1);
   void backward_line(const int cont = 1);
   void forward_page(const int cont = 1);
   void backward_page(const int cont = 1);
   void copy();
   void cut();
   void paste_buffer();
   void cut_eol();
   void beginning_of_word();
   void end_of_word();
   void beginning_of_line();
   void end_of_line();
   void beginning_of_text();
   void end_of_text();
   void delete_backward();
   void delete_forward();
   void find_forward(const char* pattern);
   void find_backward(const char* pattern);
   void find_selection_forward();
   void find_selection_backward();
   void newline();

   // selections
   void copy_selection(SelectionManager*);
   void own_selection(SelectionManager*);
   void convert_selection(SelectionManager*);
   void free_selection(SelectionManager*);

   // text
   void insert_string(char*, int count);
   void insert_char(char c);

   // various information about displayed lines
   int start_row() { return start_row_; }
   int end_row() { return end_row_; }
   int first_visible_line();
   int displayed_lines();
   void line_update();
   // number of lines in buffer
   int lines();

   // scroll to index
   void do_scroll(GlyphIndex);

   // access text editor
   TE_Editor* text_editor() { return text_editor_; }

    const char* text();
    void text(const char*, boolean update =true);

protected:
   void scroll_to_line(int line);
   void make_visible(const boolean scroll_page = true);
   int event_to_index(const Event&);

   // popup menu stuff
   Menu* menu_;
   PopupWindow* menu_window_;
   Menu* make_menu(Menu*, CommandInfo*);
   void popup_menu(const Event&);

   // variables to track "tripple-click"
   unsigned long click_time_;
   unsigned long threshold_;

   // modes during a drag operation
   enum { DragSelect, DragMenu, DragNone } drag_mode_;

   EivTextBuffer* te_buffer_;
   TE_Editor* text_editor_;
   OpenFileChooser* chooser_;
   Window* current_window_;
   Style* style_;
   char* selection_buffer_;
   TE_ViewKeyFunc key_[keymap_size];

   int rows_;			// rows in view
   GlyphIndex start_row_;	// 1'st row in view
   GlyphIndex end_row_;		// last row in view
   GlyphIndex lines_;		// displayed lines
   boolean active_;
};

declareSelectionCallback(TE_View);
declareActionCallback(TE_View);

// popup menu
struct CommandInfo {
   const char* str;
   ActionMemberFunction(TE_View) func;
   CommandInfo* submenu;
};

#endif
