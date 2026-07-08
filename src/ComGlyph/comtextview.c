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


#include <ComGlyph/comtextview.h>
#include <IVGlyph/textbuff.h>
#include <IVGlyph/texteditor.h>
#include <InterViews/event.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <Dispatch/dispatcher.h>
#include <ctype.h>
#include <iostream.h>
#include <strstream>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

using std::cerr;
using std::cout;

#define XK_MISCELLANY           /* to get the keysym's we need */
#include <X11/keysymdef.h>

// Note: Sun-specific keyboard symbols
static TE_ViewKeySymInfo default_key_sym_map[] = {
   { XK_Down,  &TE_View::down },
   { XK_Up,    &TE_View::up },
   { XK_Left,  &TE_View::left },
   { XK_Right, &TE_View::right },
   { XK_BackSpace, &TE_View::delete_backward },
   { XK_Delete, &TE_View::delete_backward },
   { XK_Return, (TE_ViewKeyFunc)&ComTE_View::newline },
   { XK_Linefeed, (TE_ViewKeyFunc)&ComTE_View::newline },
   { XK_KP_Enter, (TE_ViewKeyFunc)&ComTE_View::newline },
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

implementSelectionCallback(ComTE_View);
implementActionCallback(ComTE_View);

// TE View

ComTE_View::ComTE_View(Style* s, EivTextBuffer* te_buffer, int rows, int cols,
	     boolean active) 
: TE_View(s, te_buffer, rows, cols, active)
{
  _continuation = false;
  _parendepth = 0;
}

ComTE_View::~ComTE_View()
{
}

/*****************************************************************************/

ComTE_PaneCapture::ComTE_PaneCapture(ComTE_View* view, int readfd, int termfd)
: _view(view), _readfd(readfd), _termfd(termfd)
{
    Dispatcher::instance().link(_readfd, Dispatcher::ReadMask, this);
}

ComTE_PaneCapture::~ComTE_PaneCapture()
{
    Dispatcher::instance().unlink(_readfd);
}

int ComTE_PaneCapture::inputReady(int)
{
    drain();
    return 0;
}

void ComTE_PaneCapture::drain()
{
    char buf[4096];
    int n;
    /* the pipe's read end is O_NONBLOCK (see newline()), so this loop
       stops as soon as nothing more is immediately available instead
       of blocking for it -- fine both when the Dispatcher calls
       inputReady() (there IS data, but maybe less than a full buf) and
       when newline() calls this directly for the final post-run()
       flush (there may be nothing left at all). */
    while ((n = read(_readfd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        write(_termfd, buf, n);
        _view->insert_string(buf, n);
    }
}

/*****************************************************************************/

void ComTE_View::keystroke(const Event& e)
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
     cerr << "Unknown character - ignored!\n"; 
}
}

void ComTE_View::newline()
{
  /* extract current line from text buffer */
  beginning_of_line();
  int mark =  text_editor_->Dot();
  end_of_line();
  int dot =  text_editor_->Dot();
  int len = dot-mark;

  /* zero-length input happens when only a C/R was entered, which means unpause */
  /* if pause is active */
  if (!len && comterp()->npause()) {
    comterp()->npause()--;
    return;
  }
  char* buffer = new char [len+1];
  te_buffer_->Copy(mark, buffer, len);
  buffer[len] = '\0';

  /* if at the end of the buffer, just add a newline, otherwise, copy the whole line */
  end_of_text();
  if (dot != text_editor_->Dot())
    insert_string(buffer, len);
  insert_char('\n');

  /* run this line through comterp */
  boolean old_brief = comterp()->brief();
  comterp()->brief(1);
  cout << comterp()->linenum()+1 << ": " << buffer << "\n";

  /* strip # comments */
  /* and keep track of paren depth at the same time */
  boolean inquote = false;
  char* bufptr = buffer;
  while(*bufptr) { 
    if (!inquote && (*bufptr== '(' || *bufptr=='[' || *bufptr =='{')) 
      _parendepth++;
    else if (!inquote && (*bufptr== ')' || *bufptr==']' || *bufptr =='}')) 
      _parendepth--;

    if (!inquote && *bufptr=='#')
      *bufptr = '\0';
    else if (*bufptr=='"') {
      if (inquote) { 
	if (*(bufptr-1)!= '\\') inquote = false;
      } else
	inquote = true;
    }
    ++bufptr;
  }

  /* check for trailing semi-colon that isn't in parens, and remove it */
  bufptr = buffer + strlen(buffer) - 1;
  if (!_parendepth) {
    while(bufptr>=buffer) {
      char ch = *bufptr;
      if (ch==';') {
	*bufptr=' ';
	break;
      } else if (!isspace(ch))
	break;
      bufptr--;
    }
  }

  /* remove the "> " prompt if there was one */
  bufptr = buffer;
  if (_continuation) {
    if (buffer[0]=='>') {
      beginning_of_line();
      backward_line();
      delete_forward();
      insert_char(' ');
      bufptr++;
      if (buffer[1]==' ') {
	delete_forward();
	insert_char(' ');
	bufptr++;
      }
    }
    forward_line();
    end_of_line();
  }

  /* load and interpret if expression closed */
  comterp()->load_string(bufptr);

  /* print()/say() write to the real stdout, and ComTerp::run() itself
     also echoes the top-of-stack result -- or an error -- to stdout
     when the line completes (print_stack_top(), a non-popping peek;
     see comterp.c).  comterp has no notion of "the requesting window",
     so none of that reaches the pane on its own.

     Route stdout through a pipe rather than a plain fd swap to a file,
     and hand the read end to a ComTE_PaneCapture IOHandler (see above).
     A script that calls update() in a loop -- keydrive(), an
     interactive test loop -- hands control back to the Dispatcher on
     every pass, which is exactly when the pipe gets drained: output
     shows up in the pane WHILE the script runs, not only once after
     run() finally returns (a plain one-shot command like h() still
     works the same as before -- everything it wrote surfaces in the
     final drain() below since it never yields to the Dispatcher at
     all).  Each drained chunk goes to both the real terminal (same as
     it always saw) and the pane, so nothing below re-adds that text a
     second time.

     The write end is left blocking: if a script wrote more than one
     pipe buffer's worth (64K on most systems) between two Dispatcher
     pumps, print() itself would stall until the next pump drains it --
     not a concern for anything in this codebase today, but worth
     knowing if that ever changes. */
  int pfd[2];
  boolean piped = (pipe(pfd) == 0);
  ComTE_PaneCapture* capture = nil;
  int savedfd = -1;
  if (piped) {
    fcntl(pfd[0], F_SETFL, O_NONBLOCK);
    fflush(stdout);
    savedfd = dup(1);
    capture = new ComTE_PaneCapture(this, pfd[0], savedfd);
    dup2(pfd[1], 1);
    close(pfd[1]);
  }

  int  status = comterp()->ComTerp::run(false /* !once */, true /* nested */);

  if (piped) {
    fflush(stdout);
    dup2(savedfd, 1);
    capture->drain();      /* whatever was written since the last Dispatcher pump */
    delete capture;         /* unlinks from the Dispatcher */
    close(savedfd);
    close(pfd[0]);
  }
  // comterp()->linenum()--;

  /* run() already echoed (and the above already replayed/inserted) the
     result or error text -- this pop only discards the leftover value
     print_stack_top() peeked but never popped; it is not displayed. */
  comterp()->pop_stack(false);
  if (!*comterp()->errmsg()) {
    if (status==0) {
      _continuation = false;
      _parendepth=0;
    } else if (status==1) {
      insert_string("> ", 2);
      _continuation = true;
    }
  }
  comterp()->brief(old_brief);
  delete[] buffer;
}

