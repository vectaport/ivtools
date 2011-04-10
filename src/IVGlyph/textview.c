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

#include <IVGlyph/ofilechooser.h>
#include <IVGlyph/textbuff.h>
#include <IVGlyph/texteditor.h>
#include <IVGlyph/textview.h>

#include <IV-look/dialogs.h>
#include <IV-look/fchooser.h>
#include <IV-look/kit.h>
#include <IV-look/menu.h>

#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/window.h>

#include <OS/math.h>
#include <OS/string.h>

#include <IV-2_6/InterViews/perspective.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/shape.h>
#include <IV-2_6/InterViews/textdisplay.h>

#include <ctype.h>
#include <cstdio>
#include <string.h>

#define XK_MISCELLANY           /* to get the keysym's we need */
#include <X11/keysymdef.h>

// key maps
static TE_ViewKeyInfo default_key_map[] = {
   { 1,  &TE_View::beginning_of_line },	// c-a
   { 2,  &TE_View::left },		// c-b
   { 4,  &TE_View::delete_forward },	// c-d
   { 5,  &TE_View::end_of_line },	// c-e
   { 6,  &TE_View::right },		// c-f
   { 11, &TE_View::cut_eol },		// c-k
   { 14, &TE_View::down },		// c-n
   { 16, &TE_View::up },		// c-p
   { 22, &TE_View::page_down },		// c-v
   { 25, &TE_View::paste_buffer },	// c-y
   { 0, nil }
};

// Note: Sun-specific keyboard symbols
static TE_ViewKeySymInfo default_key_sym_map[] = {
   { XK_Down,  &TE_View::down },
   { XK_Up,    &TE_View::up },
   { XK_Left,  &TE_View::left },
   { XK_Right, &TE_View::right },
   { XK_BackSpace, &TE_View::delete_backward },
   { XK_Delete, &TE_View::delete_backward },
   { XK_Return, &TE_View::newline },
   { XK_Linefeed, &TE_View::newline },
   { XK_KP_Enter, &TE_View::newline },
   { XK_R9, &TE_View::page_up },		// PgUp
   { XK_R15, &TE_View::page_down },		// PgDn
   { XK_L6, &TE_View::copy },			// Copy (L6)
   { XK_L8, &TE_View::paste_buffer },		// Paste (L8)
   { XK_L9, &TE_View::find_selection_forward },	// Find (L9)
   { XK_L10, &TE_View::cut },			// Cut (L10)
   { XK_R7, &TE_View::beginning_of_text },      // Home
   { XK_R13, &TE_View::end_of_text },		// End
   { 0, nil }
};

implementSelectionCallback(TE_View);
implementActionCallback(TE_View);

CommandInfo filemenu[] = {
   { "Load...",   &TE_View::load_popup, nil },
   { "Save",      &TE_View::save, nil },
   { "Save As..", &TE_View::save_popup, nil },
   { "Quit", &TE_View::quit, nil },
   { nil }
};

CommandInfo viewmenu[] = {
   { "Find Selection Forward",  &TE_View::find_selection_forward, nil },
   { "Find Selection Backward", &TE_View::find_selection_backward, nil },
// Find popup not impl.
//   { "Find...", &TE_View::find_selection_backward, nil },
   { nil }
};

CommandInfo editmenu[] = {
   { "Copy",   &TE_View::copy, nil },
   { "Cut",    &TE_View::cut,  nil },
   { "Paste",  &TE_View::paste_buffer, nil },
   { nil }
};

CommandInfo popupmenu[] = {
   { "File", nil, filemenu },
   { "View", nil, viewmenu },
   { "Edit", nil, editmenu },
   { nil }
};

// TE View

TE_View::TE_View(Style* s, EivTextBuffer* te_buffer, int rows, int cols,
	     boolean active) 
: InputHandler(nil, s), rows_(rows)
{
   WidgetKit& kit = *WidgetKit::instance();
   te_buffer_ = te_buffer;
   style_ = s;
   Resource::ref(style_);

   // create text editor
   text_editor_ = new TE_Editor(s, this, rows, cols, 4, 4);
   text_editor_->Edit(te_buffer_, 0);

   body(new Target(text_editor_, TargetPrimitiveHit));

   start_row_= 0;
   end_row_ = rows - 1;
   lines_ = rows;

   selection_buffer_ = nil;
   chooser_ = nil;

   // init key map
   for (int i=0; i<keymap_size; i++) key_[i] = nil;
   for (TE_ViewKeyInfo* k = &default_key_map[0]; k->key != 0; k++) {
      key_[k->key] = k->func;
   }

   // init menu
   menu_ = make_menu(kit.pulldown(), popupmenu);
   menu_window_ = new PopupWindow(menu_);

   //
   click_time_ = 0;
   long t = 250;
   s->find_attribute("clickDelay", t);
   threshold_ = t;
   active_ = active;
}

TE_View::~TE_View()
{
  if (selection_buffer_ != nil)
    delete selection_buffer_;
  delete menu_window_;
  delete te_buffer_;
  delete text_editor_;
  Resource::unref(style_);
}

void TE_View::load_popup()
{
    if (chooser_ == nil) {
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style_);
	Resource::ref(chooser_);
    }
    char buf[256];
    boolean error = true;
    sprintf(buf, "Load File:");
    style_->attribute("open", "Load");
    style_->attribute("caption", "");
    style_->attribute("subcaption", buf);
    while (error) {
	if (!chooser_->post_for(current_window_)) {
	    chooser_->unmap();
	    return;
	}
	const String* s = chooser_->selected();
	switch(load(s->string())) {
	case EivTextBuffer::OpenError:
	    style_->attribute("caption", "Can't open file, Retry!");
	    error = true;
	    break;
	case EivTextBuffer::MemoryError:
	    style_->attribute("caption", "File not read, Out Of Memory!");
	 error = true;
	    break;
	case EivTextBuffer::ReadError:
	    style_->attribute("caption", "Can't read file.");
	    error = true;
	    break;
	default:
	    error = false;
	 break;
	}
    }
}
      
void TE_View::save_popup()
{
    if (chooser_ == nil) {
	chooser_ = new OpenFileChooser(".", WidgetKit::instance(), style_);
	Resource::ref(chooser_);
    }
    char buf[256];
    boolean error = true;
    sprintf(buf, "Save To File:");
    style_->attribute("open", "Save");
    style_->attribute("caption", "");
    style_->attribute("subcaption", buf);
    while (error) {
	if (!chooser_->post_for(current_window_))
	    return;
	const String* s = chooser_->selected();
	switch(save_as(s->string())) {
	case EivTextBuffer::OpenError:
	    style_->attribute("caption", "Can't open file, Retry!");
	    error = true;
	    break;
	case EivTextBuffer::WriteError:
	    style_->attribute("caption", "Can't write to file, Retry!");
	    error = true;
	    break;
	default:
	    error = false;
	    break;
	}
    }
}
      
int TE_View::load(const char* path)
{
   // delete text in editor
   text_editor_->Select(0);
   text_editor_->DeleteText(te_buffer_->characters());
   // load new file
   int rc = te_buffer_->load(path);
   text_editor_->Edit(te_buffer_, 0);
   text_editor_->BeginningOfText();
   // update line information
   start_row_= 0;
   line_update();
   return rc;
}

const char* TE_View::text() {
    return te_buffer_->Text();
}

void TE_View::text(const char* txt, boolean update) {
   // delete text in editor
   text_editor_->Select(0);
   text_editor_->DeleteText(te_buffer_->characters());

   // load new text
   te_buffer_->Insert(0, txt, strlen(txt));
   text_editor_->Edit(te_buffer_, 0);
   text_editor_->BeginningOfText();
   // update line information
   start_row_= 0;
   if (update)
       line_update();
}

int TE_View::save_as(const char* path)
{
   return te_buffer_->save(path);
}

int TE_View::save_current()
{
   return te_buffer_->save();
}

void TE_View::save()
{
   // save without error check; Bug Alert!
   (void) te_buffer_->save();
}

void TE_View::quit()
{
   // save without error check; Bug Alert!
   Session::instance()->quit();
}

void TE_View::make_visible(const boolean scroll_page)
{
   int index = text_editor_->Dot();
   int line = te_buffer_->line(index);
   if (line < start_row_)
      scroll_to_line(line);
   else if (line > end_row_) {
      if (scroll_page)
	 scroll_to_line(line);
      else
	 scroll_to_line(first_visible_line() + 1); 
   }
}

int TE_View::first_visible_line()
{
   int line_width = text_editor_->GetShape()->vunits;
   int pix_start_pos = int(start_row_ * line_width);
   return (pix_start_pos/line_width);
}

int TE_View::displayed_lines()
{
   return text_editor_->displayed_lines();
}

void TE_View::up()
{
   backward_line(1);
}

void TE_View::down()
{
   forward_line(1);
}

void TE_View::left()
{
   backward_char(1);
}

void TE_View::right()
{
   forward_char(1);
}

void TE_View::page_up()
{
   backward_page(1);
}

void TE_View::page_down()
{
   forward_page(1);
}

void TE_View::backward_char(const int count)
{
   text_editor_->BackwardCharacter(count);
   make_visible();
}

void TE_View::forward_char(const int count)
{
   text_editor_->ForwardCharacter(count);
   make_visible(/*false*/);
}

void TE_View::backward_word(const int count)
{
   text_editor_->BackwardWord(count);
   make_visible();
}

void TE_View::forward_word(const int count)
{
   text_editor_->ForwardWord(count);
   make_visible(/*false*/);
}

void TE_View::backward_line(const int count)
{
   text_editor_->BackwardLine(count);
   make_visible();
}

void TE_View::forward_line(const int count)
{
   text_editor_->ForwardLine(count);
   make_visible(/*false*/);
}

void TE_View::forward_page(const int count)
{
   text_editor_->ForwardPage(count);
   make_visible();
}

void TE_View::backward_page(const int count)
{
   text_editor_->BackwardPage(count);
   make_visible();
}

void TE_View::copy()
{
   Session* session = Session::instance();
   Display* display = session->default_display();
   SelectionManager* s = display->clipboard_selection();

   // copy selection into selection buffer
   copy_selection(s);

   // mark selection as owned
   own_selection(s);
}

void TE_View::cut()
{
   copy();
   text_editor_->DeleteSelection();
}

void TE_View::paste_buffer()
{
//   printf("selection_buffer:%s\n", selection_buffer_);
   if (selection_buffer_ != nil)
      insert_string(selection_buffer_, strlen(selection_buffer_));
   make_visible();
}

void TE_View::cut_eol()
{
   int dot = text_editor_->Dot();
   text_editor_->EndOfLine();
   int mark = text_editor_->Dot();
   text_editor_->Select(dot, mark);
   cut();
}

void TE_View::beginning_of_word()
{
   text_editor_->BeginningOfWord();
}

void TE_View::end_of_word()
{
   text_editor_->EndOfWord();
}

void TE_View::beginning_of_line()
{
   text_editor_->BeginningOfLine();
}

void TE_View::end_of_line()
{
   text_editor_->EndOfLine();
}

void TE_View::beginning_of_text()
{
   text_editor_->BeginningOfText();
   make_visible(); 
}

void TE_View::end_of_text()
{
   text_editor_->EndOfText();
   make_visible();
}

void TE_View::delete_backward()
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot != mark)
      text_editor_->DeleteSelection();
   else
      text_editor_->DeleteText(-1);
   make_visible();
}

void TE_View::delete_forward()
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot != mark)
      text_editor_->DeleteSelection();
   else
      text_editor_->DeleteText(1);
   make_visible();
}

void TE_View::find_forward(const char* pattern)
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   int index = Math::max(dot, mark);
   int end = te_buffer_->search_forward(pattern, index);
   if (end >= 0) {
      int start = te_buffer_->search_beginning();
      text_editor_->Select(start, end);
      make_visible();
   }
}

void TE_View::find_backward(const char* pattern)
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   int index = Math::min(dot, mark);
   int start = te_buffer_->search_backward(pattern, index);
   if (start >= 0) {
      int end = te_buffer_->search_end();
      text_editor_->Select(start, end);
      make_visible();
   }
}

void TE_View::find_selection_forward()
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot == mark)
      return;			// no selection!
   find_forward(selection_buffer_);
}

void TE_View::find_selection_backward()
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot == mark)
      return;			// no selection!
   find_backward(selection_buffer_);
}

void TE_View::newline()
{
   insert_char('\n');
}

void TE_View::own_selection(SelectionManager* s)
{
   s->own( 
      new SelectionCallback(TE_View)
	  (this, &TE_View::convert_selection),
	  new SelectionCallback(TE_View)
	  (this, &TE_View::free_selection), // never called!
	  new SelectionCallback(TE_View)
	  (this, &TE_View::free_selection)  // never called!
	  );
}

void TE_View::copy_selection(SelectionManager* s)
{
   // copy selection into selection buffer
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot == mark)
      return;                   // nothing to copy!
   int len = Math::abs(mark-dot);
   free_selection(s);
   selection_buffer_ = new char[len+1];
   te_buffer_->Copy(dot, selection_buffer_, mark-dot);
   selection_buffer_[len] = '\0';
//   printf("selection_buffer:%s\n", selection_buffer_);
}

void TE_View::convert_selection(SelectionManager* s)
{
   if (selection_buffer_ != nil)
      s->put_value(
	 (void*) selection_buffer_,
	 strlen(selection_buffer_));
}

void TE_View::free_selection(SelectionManager*)
{
//printf("free_selection\n");
   delete selection_buffer_;
   selection_buffer_ = nil;
}

void TE_View::insert_string(char* str, int count)
{
   int dot = text_editor_->Dot();
   int mark = text_editor_->Mark();
   if (dot != mark)
     text_editor_->DeleteSelection(); // overwrite selection
     
   text_editor_->InsertText(str, count);
   // scroll to follow new insertion
   make_visible();
}

void TE_View::insert_char(char c)
{
   insert_string(&c, 1);
}

void TE_View::press(const Event& e)
{
    if (active_) {
   // translate event coordinates for window into coordinates for
   // the editor.
   int index = event_to_index(e);

   switch(e.pointer_button()) {
   case Event::left:
      drag_mode_ = DragSelect;
      text_editor_->Select(index);
      break;
   case Event::middle:
      drag_mode_ = DragSelect;
      break;
   case Event::right:
      popup_menu(e);
      drag_mode_ = DragMenu;
      break;
   }
}
}

void TE_View::drag(const Event& e) 
{
    if (active_) {
   int index = event_to_index(e);
   switch(drag_mode_) {
   case DragSelect:
      text_editor_->SelectMore(index);
      break;
   case DragMenu:
      // give control to Menu::drag()
      menu_->drag(e);
      break;
   default:
      break;
   }
}
}

void TE_View::release(const Event& e) 
{
    if (active_) {
   current_window_ = e.window();

   SelectionManager* s = e.display()->primary_selection();
   switch(drag_mode_) {
   case DragSelect:
      // copy and own current selection
      copy_selection(s);
      own_selection(s);
      break;
   case DragMenu:
      // unselect menu and unmap window
      menu_->release(e);
      menu_->unselect();
      menu_window_->unmap();
      menu_window_->unbind();      // required ??
      break;
   default:
      break; 
   }
}
}

void TE_View::keystroke(const Event& e)
{
    if (active_) {
   current_window_ = e.window();
   
   // check if known key symbol
   unsigned long keysym = e.keysym();
   for (TE_ViewKeySymInfo* k = &default_key_sym_map[0];
	k->keysym != 0; k++) {
      if (keysym == k->keysym) {
	 TE_ViewKeyFunc f = k->func;
	 (this->*f)();
	 return;
      }
   }

   // map event to key
   signed char c;
   if (e.mapkey((char *)&c, 1) == 0)
      return;

   // check if known key map
#ifndef __sgi /* avoid SGI gcc warning */
   if (c >= 0) { // fix alt-V on linux/pc
#else
   if (1) { 
#endif
       TE_ViewKeyFunc f =  key_[c];
       if (f != nil) {
	   (this->*f)();
	   return;
       }
   }

#ifndef __sgi /* avoid SGI gcc warning */
   if (c >= 0 && (isspace(c) || !iscntrl(c))) 
#else
   if (isspace(c) || !iscntrl(c)) 
#endif
      insert_char(c);
   else 
      printf("Unknown character - ignored!\n");
}
}

void TE_View::double_click(const Event& e)
{
    if (active_) {
   boolean tripple_click = false;
   int dot, mark;
   unsigned long t = e.time();
   if (t - click_time_ < threshold_) {
      tripple_click = true;
   }
   click_time_ = t;

   int index = event_to_index(e);
   text_editor_->Select(index);

   // select work or line
   if (tripple_click) {
      // line
      text_editor_->BeginningOfLine();
      dot = text_editor_->Dot();
      text_editor_->EndOfLine();
      mark = text_editor_->Dot();
   }
   else {
      // word
      text_editor_->BeginningOfWord();
      dot = text_editor_->Dot();
      text_editor_->EndOfWord();
      mark = text_editor_->Dot();
   }
   text_editor_->Select(dot, mark);

   // copy and own current selection
   SelectionManager* s = e.display()->primary_selection();
   copy_selection(s);
   own_selection(s);
}
}

void TE_View::popup_menu(const Event&e)
{
  // display popup-menu
  const Window& rel = *e.window();
  Coord x = rel.left() + e.pointer_x();
  Coord y = rel.bottom() + e.pointer_y() - menu_window_->height() + 6;
  menu_window_->place(x, y);
  menu_window_->map();
}

void TE_View::line_update()
{
   // update line information
   lines_ = displayed_lines();
   end_row_ = start_row_ + lines_ - 1;
   notify();			// notify adjustable
}

int TE_View::event_to_index(const Event& e)
{
   // pixels relative root window
   int absleft, absbottom;
   absleft = e.display()->to_pixels(e.pointer_root_x());
   absbottom = e.display()->to_pixels(e.pointer_root_y());
   // interactor's pixels relative
   int left, bottom;
   text_editor_->GetPosition(left, bottom);
   // x and y are pixels
   int x= absleft - left;
   int y= absbottom - bottom;
   int index = text_editor_->Locate(x, y);
   return(index);
}

void TE_View::do_scroll(GlyphIndex new_start)
{
   scroll_to_line((int)new_start);
    //      notify();
}

void TE_View::scroll_to_line(int line)
{
//   printf("scroll_to_line(%d)\n", line);
   GlyphIndex new_start, new_end;
   GlyphIndex max_end = te_buffer_->lines();
   if (line < 0) 
      new_start = 0;
   else if (max_end - line < lines_) 
      new_start = max_end - lines_;
   else 
      new_start = line;
   new_end = new_start + lines_ - 1;

   if (new_start == start_row_)
      return;			// nothing to scroll

   // scroll
   int line_width = text_editor_->GetShape()->vunits;
   int from_pix = int(start_row_ * line_width);
   int to_pix = int(new_start * line_width);
   int diff_pix = from_pix - to_pix;

   text_editor_->ScrollBy(0, diff_pix);
   start_row_ = new_start;
   end_row_ = new_end;
   notify();			// notify adjustable
}

Menu* TE_View::make_menu(Menu* m, CommandInfo* info)
{
   WidgetKit& kit = *WidgetKit::instance();
   TelltaleGroup* group = nil;

   // loop for command array info
   for (CommandInfo* i = info; i->str != nil; i++) {
      if (i->str[0] == '\0') {
	 // string is empty (""), add separator
	 m->append_item(kit.menu_item_separator());
      }
      else {
	 // create menu item
	 MenuItem* item;
	 if (i->submenu == nil)  {
	    // not a sub-menu, just add callback
	    item = kit.menu_item(i->str);
	    item->action(
	       new ActionCallback(TE_View)(this, i->func)
	       );
	 }
	 else {
	    // sub-menu, create recursively
	    item = kit.menu_item(i->str);
	    item->menu(make_menu(kit.pullright(), i->submenu));
	 }
	 // appen created item
	 m->append_item(item);
      }
   }
   return m;
}

int TE_View::lines() { return te_buffer_->lines(); }

void TE_View::disable_caret() { text_editor_->DisableCaret(); }

void TE_View::enable_caret() { text_editor_->EnableCaret(); }
