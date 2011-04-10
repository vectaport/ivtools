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

#ifndef comtextview_h
#define comtextview_h

#include <IVGlyph/textview.h>

class ComTerpServ;

class ComTE_View :public TE_View {
public:
   ComTE_View(Style*, EivTextBuffer*, int rows, int cols, boolean active);
   ~ComTE_View();

   virtual void keystroke(const Event&);
   void newline();
   ComTerpServ* comterp() { return _comterp; }
   void comterp(ComTerpServ* cterp) { _comterp = cterp; }
private:

   ComTerpServ* _comterp;
   boolean _continuation;
   int _parendepth;
};

declareSelectionCallback(ComTE_View);
declareActionCallback(ComTE_View);

#endif
