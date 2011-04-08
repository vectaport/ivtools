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

/*
 * Modifications by Vectaport Inc.:
 *
 * scroll to follow cursor in (hopefully) all cases
 * fix scrollbar (actually its adjustable) so it positions perfectly
 * fix alt-V on linux/pc keyboard (and any other <0 characters)
 * improved layout/color
 * added text get/set methods for easier integration
 */

#include <IV-look/kit.h>
#include <InterViews/adjust.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <InterViews/hit.h>
#include <InterViews/layout.h>
#include <InterViews/monoglyph.h>
#include <InterViews/style.h>
#include <InterViews/session.h>

#include <OS/file.h>
#include <OS/math.h>
#include <OS/memory.h>
#include <OS/string.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "textbuff.h"
#include "textedit.h"
#include "texteditor.h"
#include "textview.h"

// TE Adjustable

TE_Adjustable::TE_Adjustable(TE_View* te_view)
{
   te_view_ = te_view;
}

void TE_Adjustable::update(Observable*)
{
   notify(Dimension_Y);		// notify scrollbar
}

Coord TE_Adjustable::lower(DimensionName) const
{
   return Coord(0);
}

Coord TE_Adjustable::upper(DimensionName) const
{
    return Coord(te_view_->lines());
}

Coord TE_Adjustable::length(DimensionName) const
{
    return Coord(te_view_->lines() /*- 1*/);
}

Coord TE_Adjustable::cur_lower(DimensionName) const
{
    return Coord(te_view_->lines() - te_view_->end_row() -1);
}

Coord TE_Adjustable::cur_upper(DimensionName) const
{
    return Coord(te_view_->lines() /*- 1*/ - te_view_->start_row());
}

Coord TE_Adjustable::cur_length(DimensionName) const
{
   return Coord(te_view_->end_row() - te_view_->start_row() +1);
}

void TE_Adjustable::scroll_forward(DimensionName d)
{
   scroll_by(d, -long(small_scroll(d)));
}

void TE_Adjustable::scroll_backward(DimensionName d)
{
   scroll_by(d, long(small_scroll(d)));
}

void TE_Adjustable::page_forward(DimensionName d)
{
   scroll_by(d, -long(large_scroll(d)));
}

void TE_Adjustable::page_backward(DimensionName d)
{
   scroll_by(d, long(large_scroll(d)));
}

void TE_Adjustable::scroll_to(DimensionName, Coord lower)
{
   GlyphIndex max_end = te_view_->lines();
   GlyphIndex new_end = max_end - Math::round(lower);
   GlyphIndex new_start = new_end -
      te_view_->end_row() + te_view_->start_row();
   te_view_->do_scroll(new_start);
   notify(Dimension_Y);		// notify scrollbar
}

void TE_Adjustable::scroll_by(DimensionName, long offset) 
{
   te_view_->do_scroll(te_view_->start_row() + offset);
   notify(Dimension_Y);		// notify scrollbar
}


EivTextEditor::EivTextEditor() { }

EivTextEditor::EivTextEditor(Style* s, boolean active)
{
  const LayoutKit& layout = *LayoutKit::instance();
  WidgetKit& kit = *WidgetKit::instance();

   style_ = new Style("TextEditor", s);
   Resource::ref(style_);

   double rows = 24;
   (void) style_->find_attribute("rows", rows);
   double cols = 80;
   (void) style_->find_attribute("columns", cols);

   // create buffer and view
   EivTextBuffer* te_buffer = new EivTextBuffer();
   te_view_ = new TE_View(style_, te_buffer, int(rows), int(cols), active);

   // attach adjustable and scrollbar
   te_adjustable_ = new TE_Adjustable(te_view_);
   te_view_->attach(te_adjustable_);
   sb_ = kit.vscroll_bar(te_adjustable_);

   Display* d = Session::instance()->default_display();
   const Color* bg = Color::lookup(d, "#aaaaaa");
   if (bg == nil)
       bg = new Color(0.7,0.7,0.7,1.0);

  body(
      new Background(
	  layout.margin(
	      layout.hbox(
		  kit.inset_frame(
		      layout.vcenter(layout.margin(te_view_, 2.0), 1.0)
		  ),
		  layout.hspace(4.0),
		  sb_
	      ),
	      5.0),
	  bg
      )
  );
}

EivTextEditor::~EivTextEditor()
{
   te_view_->detach(te_adjustable_);
   delete te_adjustable_;
   delete te_view_;
   delete sb_;
   Resource::unref(style_);
}

int EivTextEditor::load(const char* path)
{
  if (path == nil || *path == '\0')
    return 1;
  return te_view_->load(path);
}

int EivTextEditor::save(const char* path)
{
   if (path == nil)
      return te_view_->save_current();
   else
      return te_view_->save_as(path);
}

void EivTextEditor::save_popup()
{
   te_view_->save_popup();
}

void EivTextEditor::load_popup()
{
   te_view_->load_popup();
}

void EivTextEditor::quit()
{
   // quit without error check; Bug Alert!
   te_view_->quit();
}

int EivTextEditor::dot()
{
  return te_view_->text_editor()->Dot();
}

int EivTextEditor::mark()
{
  return te_view_->text_editor()->Mark();
}

void EivTextEditor::select(const int dot, const int mark)
{
  te_view_->text_editor()->Select(dot, mark);
}

void EivTextEditor::select_all()
{
  te_view_->text_editor()->SelectAll();
}

void EivTextEditor::select_beginning(const EivTextUnit unit)
{
   switch (unit) {
   case EivTextEditor::Character:
      // not implemented
      break;		
   case EivTextEditor::Word:
      te_view_->beginning_of_word();
      break;
   case EivTextEditor::Line:
      te_view_->beginning_of_line();
      break;
   case EivTextEditor::Text:
      te_view_->beginning_of_text();
      break;
   }
}

void EivTextEditor::select_end(const EivTextUnit unit)
{
   switch (unit) {
   case EivTextEditor::Character:
      // not implemented
      break;		
   case EivTextEditor::Word:
      te_view_->end_of_word();
      break;
   case EivTextEditor::Line:
      te_view_->end_of_line();
      break;
   case EivTextEditor::Text:
      te_view_->end_of_text();
      break;
   }
}

void EivTextEditor::select_backward(const EivTextUnit unit, const int count)
{
   switch (unit) {
   case EivTextEditor::Character:
      te_view_->backward_char(count);
      break;		
   case EivTextEditor::Word:
      te_view_->backward_word(count);
      break;
   case EivTextEditor::Line:
      te_view_->backward_line(count);
      break;
   case EivTextEditor::Text:
      // not implemented
      break;
   }
}

void EivTextEditor::select_forward(const EivTextUnit unit, const int count)
{
   switch (unit) {
   case EivTextEditor::Character:
      te_view_->forward_char(count);
      break;		
   case EivTextEditor::Word:
      te_view_->forward_word(count);
      break;
   case EivTextEditor::Line:
      te_view_->forward_line(count);
      break;
   case EivTextEditor::Text:
      // not implemented
      break;
   }
}

void EivTextEditor::find_forward(const char* pattern)
{
   te_view_->find_forward(pattern);
}

void EivTextEditor::find_backward(const char* pattern)
{
   te_view_->find_backward(pattern);
}

const char* EivTextEditor::text() {
    return te_view_->text();
}

void EivTextEditor::text(const char* txt, boolean update) {
    te_view_->text(txt, update);
}

void EivTextEditor::insert_string(char* str, int count) {
    te_view_->insert_string(str, count);
}

InputHandler* EivTextEditor::focusable() {
    return te_view_;
}

TE_View* EivTextEditor::textview() { return te_view_; }
