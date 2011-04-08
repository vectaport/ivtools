/*
 * Copyright (c) 1998 Vectaport, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <IueServ/iuefunc.h>
#include <IueServ/iuehandler.h>

#include <Unidraw/Components/component.h>

#include <ComTerp/comterpserv.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#include <ace/Synch.h>

static u_short SERVER_PORT = 30001;
static const char *const SERVER_HOST = "localhost";
#endif

#include <iostream.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

#ifdef HAVE_ACE
  Component::use_unidraw(false);

  IueAcceptor* peer_acceptor = new IueAcceptor();
  
  int portnum = argc > 1 ? atoi(argv[1]) : SERVER_PORT;
  if (peer_acceptor->open (ACE_INET_Addr (portnum)) == -1)
    cerr << "iueserv: unable to open port " << portnum << " with ACE\n";
  
  else if (COMTERP_REACTOR::instance ()->register_handler
	   (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
    cerr << "iueserv: error registering acceptor with ACE reactor\n";
  
  else
    cerr << "accepting iueserv port (" << portnum << ") connections\n";
  
  // Register COMTERP_QUIT_HANDLER to receive SIGINT commands.  When received,
       // COMTERP_QUIT_HANDLER becomes "set" and thus, the event loop below will
       // exit.
       if (COMTERP_REACTOR::instance ()->register_handler 
	   (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
	 ACE_ERROR_RETURN ((LM_ERROR, 
			    "registering service with ACE_Reactor\n"), -1);
  
  // Start up one on stdin
       IueHandler* stdin_handler = new IueHandler();
#if 0
  if (ACE::register_stdin_handler(stdin_handler, COMTERP_REACTOR::instance(), nil) == -1)
#else
    if (COMTERP_REACTOR::instance()->register_handler(0, stdin_handler, 
						      ACE_Event_Handler::READ_MASK)==-1)
#endif
    cerr << "iueserv: unable to open stdin with ACE\n";
  
  // Perform logging service until COMTERP_QUIT_HANDLER receives SIGINT.
       while (COMTERP_QUIT_HANDLER::instance ()->is_set () == 0)
	 COMTERP_REACTOR::instance ()->handle_events ();
  
  return 0;
#endif
}

