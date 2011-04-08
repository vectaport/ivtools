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

#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>

int DrawServHandler::_sigpipe_handler_initialized = 0;

/*****************************************************************************/

// Default constructor.

DrawServHandler::DrawServHandler (ComTerpServ* serv) : UnidrawComterpHandler(serv)
{
  _drawlink = nil;
  if (!_sigpipe_handler_initialized) {
    if (ComterpHandler::reactor_singleton()->register_handler 
	(SIGPIPE, this) == -1)
      ACE_DEBUG ((LM_ERROR, 
		  "(%P|%t) can't register signal handler with reactor\n"));
    _sigpipe_handler_initialized = 1;
    _sigpipe_handler = 1;
  } else
    _sigpipe_handler = 0;
}

int DrawServHandler::open (void * ptr)
{
  ComterpHandler::open(ptr);
  return 0;
}

void DrawServHandler::destroy (void) {
  if (_sigpipe_handler) {
#if 0
    if (ComterpHandler::reactor_singleton()->remove_handler 
	(SIGPIPE, (ACE_Sig_Action*)nil) == -1)
      ACE_DEBUG ((LM_ERROR, 
		  "(%P|%t) can't remove signal handler from reactor\n"));
#endif
    _sigpipe_handler = _sigpipe_handler_initialized = 0;
  }
  ComterpHandler::destroy();
  if (drawlink()) {
    ((DrawServ*)unidraw)->linkdown(drawlink());
    drawlink(nil);
  }
}

int DrawServHandler::handle_signal(int signum, siginfo_t* s, ucontext_t* u) {
  if (signum==SIGPIPE) {
    fprintf(stderr, "ignoring SIGPIPE because we don't know what handle it is on\n");
  } else
    fprintf(stderr, "unknown signal handled %d\n", signum);
  return 0;
}
#endif /* HAVE_ACE */
