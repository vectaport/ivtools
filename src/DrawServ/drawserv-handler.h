/*
 * Copyright (c) 2004 Scott E. Johnston
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

#ifdef HAVE_ACE

#ifndef _drawserv_handler_
#define _drawserv_handler_

#include <ComUnidraw/comterp-acehandler.h>

class AckBackHandler;
class DrawLink;

//: specialized UnidrawComterpHandler for integration into DrawServ
class DrawServHandler : public UnidrawComterpHandler
{

public:
  // = Initialization and termination methods.
  DrawServHandler ();

  DrawLink* drawlink() { return _drawlink; }
  // get DrawLink associated with this handler
  void drawlink(DrawLink* link) { _drawlink = link; }
  // set DrawLink associated with this handler

  virtual void destroy (void);
  // traps disconnects

  virtual int open (void *);
  // open handler hook.

  virtual int handle_signal(int, siginfo_t*, ucontext_t*);
  // handle signals

protected:
  DrawLink* _drawlink;
  static int _sigpipe_handler_initialized;
  int _sigpipe_handler;

};

//: Acceptor specialized for use with DrawServ and ComTerp.
typedef ACE_Acceptor <DrawServHandler, ACE_SOCK_ACCEPTOR> 
	DrawServAcceptor;

#endif /* _drawserv_handler_ */

#endif /* HAVE_ACE */
