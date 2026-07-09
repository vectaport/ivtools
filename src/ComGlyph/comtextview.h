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
#include <Dispatch/iohandler.h>

class ComTerpServ;

class ComTE_View :public TE_View {
public:
   ComTE_View(Style*, EivTextBuffer*, int rows, int cols, boolean active);
   ~ComTE_View();

   virtual void keystroke(const Event&);
   void newline();
   ComTerpServ* comterp() { return _comterp; }
   void comterp(ComTerpServ* cterp) { _comterp = cterp; }

   boolean driving() { return _driving; }
   void driving(boolean flag) { _driving = flag; }
   // true while THIS pane is the one feeding the currently executing
   // line to comterp -- set around newline()'s own run() call, and
   // around ComTextEditor::runfile()'s -comt bootstrap load.  Read via
   // ComTextEditor::driving() (ComGlyph) and comdraw's textpane()
   // command (src/ComUnidraw/unifunc.h) -- comterp() shares one
   // interpreter with the terminal REPL/-runfile/remote() sessions, so
   // this can't be inferred from the ComTerp object itself; it has to be
   // a fact the pane records about its own current call.

private:

   ComTerpServ* _comterp;
   boolean _continuation;
   int _parendepth;
   boolean _driving;
};

declareSelectionCallback(ComTE_View);
declareActionCallback(ComTE_View);

// ComTE_PaneCapture -- drains a pipe into a ComTE_View's buffer AND a
// saved real-stdout fd as soon as the Dispatcher notices data on it, so
// print() output from a long-running script (something a func like
// keydrive() or an interactive test loop, calling update() every pass,
// is FULL of) shows up in the pane WHILE the script runs, not just once
// after the whole synchronous line finally returns.  See
// ComTE_View::newline() for how this gets wired up around one line's
// evaluation.
class ComTE_PaneCapture : public IOHandler {
public:
    ComTE_PaneCapture(ComTE_View* view, int readfd, int termfd);
    virtual ~ComTE_PaneCapture();
    virtual int inputReady(int fd);
    void drain();     // one-shot, non-Dispatcher-triggered drain (final flush)
protected:
    ComTE_View* _view;
    int _readfd;
    int _termfd;
};

#endif
