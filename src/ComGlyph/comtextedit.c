/*
 * Copyright (c) 2000  IET Inc
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * related documentation and data files for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of the copyright holders not
 * be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  The copyright holders
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


// layered on top of classes distributed with this license:
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

#include <ComTerp/comterpserv.h>
#include <ComGlyph/comtextedit.h>
#include <ComGlyph/comtextview.h>
#include <IVGlyph/textbuff.h>
#include <InterViews/background.h>
#include <InterViews/color.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/style.h>
#include <IV-look/kit.h>
#include <OS/string.h>

ComTextEditor::ComTextEditor(Style* s, ComTerpServ* comterp, boolean active) : EivTextEditor() {

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
  te_view_ = new ComTE_View(style_, te_buffer, int(rows), int(cols), active);
  ((ComTE_View*)te_view_)->comterp(comterp);
  
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

ComTextEditor::~ComTextEditor()
{
}

ComTE_View* ComTextEditor::comtextview() { return (ComTE_View*)te_view_; }






