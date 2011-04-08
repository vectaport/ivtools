/*
 * Copyright 1995 Vectaport Inc.
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

#include <IVGlyph/textbuff.h>
#include <IVGlyph/texteditor.h>
#include <IVGlyph/textview.h>

#include <IV-look/kit.h>
#include <InterViews/color.h>
#include <InterViews/font.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <OS/string.h>
#include <IV-2_6/InterViews/painter.h>
#include <IV-2_6/InterViews/perspective.h>
#include <IV-2_6/InterViews/textdisplay.h>

TE_Editor::TE_Editor(Style* s, TE_View* te_view, int r, int c, int t, int h)
: TextEditor(r, c, t, h)
{
   te_view_ = te_view;
   style_  = s;
   Resource::ref(style_);
   // init font to be used inside editor
   String font_name("fixed");
   style_->find_attribute("textFont", font_name);
   font_ = Font::lookup(font_name);
   Resource::ref(font_);
}

TE_Editor::~TE_Editor()
{
   Resource::unref(style_);
   Resource::unref(font_);
}

void TE_Editor::Reconfig()
{
   // set text edit attributes (colors and font)
   WidgetKit& kit = *WidgetKit::instance();
   kit.push_style();
   kit.style(style_);
   Painter* p = new Painter(output);
//   p->SetColors(kit.foreground(), kit.background());
   Display* d = Session::instance()->default_display();
   const Color* bg = Color::lookup(d, "#aaaaaa");
   if (bg == nil)
       bg = new Color(0.7,0.7,0.7,1.0);
   p->SetColors(kit.foreground(), bg);
   if (font_ != nil)
      p->SetFont(font_);
   Resource::unref(output);
   output = p;
   TextEditor::Reconfig();
   kit.pop_style();
}

void TE_Editor::Resize()
{
   TextEditor::Resize();
   te_view_->line_update();	// update line info
}

int TE_Editor::displayed_lines()
{
   return perspective->curheight / perspective->sy;
}

void TE_Editor::reinit()
{
   int lines = text->Height();
   for (int i = 0; i < lines; ++i) {
      int bol = text->LineIndex(i);
      int eol = text->EndOfLine(bol);
      display->ReplaceText(i, text->Text(bol, eol), eol - bol);
   }
   //display->Redraw(0, 0, xmax, ymax);
}

