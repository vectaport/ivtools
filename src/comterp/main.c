/*
 * Copyright (c) 1994-1995 Vectaport, Inc.
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

#include <ComTerp/comterpserv.h>

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch.h>

static u_short SERVER_PORT = 20000;
static const char *const SERVER_HOST = ACE_DEFAULT_SERVER_HOST;
#endif

#include <iostream.h>
#include <string.h>

int main(int argc, char *argv[]) {

    boolean server_flag = argc>1 && strcmp(argv[1], "server") == 0;
    boolean client_flag = argc>1 && strcmp(argv[1], "client") == 0;
    boolean telcat_flag = argc>1 && strcmp(argv[1], "telcat") == 0;

#ifdef HAVE_ACE
    if (server_flag) {
        ComterpAcceptor peer_acceptor;

        int portnum = argc > 2 ? atoi(argv[2]) : atoi(ACE_DEFAULT_SERVER_PORT_STR);
        if (peer_acceptor.open (ACE_INET_Addr (portnum)) == -1)
            cerr << "comterp: unable to open port " << portnum << " with ACE\n";

        else if (COMTERP_REACTOR::instance ()->register_handler
                  (&peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
          cerr << "comterp: error registering acceptor with ACE reactor\n";

	else
	  cerr << "accepting comterp port (" << portnum << ") connections\n";
    
        // Register COMTERP_QUIT_HANDLER to receive SIGINT commands.  When received,
        // COMTERP_QUIT_HANDLER becomes "set" and thus, the event loop below will
        // exit.
        if (COMTERP_REACTOR::instance ()->register_handler 
    	     (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
          ACE_ERROR_RETURN ((LM_ERROR, 
    			 "registering service with ACE_Reactor\n"), -1);
    
	// Start up one on stdin
	ComterpHandler stdin_handler;
	if (ACE::register_stdin_handler(&stdin_handler, COMTERP_REACTOR::instance(), nil) == -1)
	    cerr << "comterp: unable to open stdin with ACE\n";

        // Perform logging service until COMTERP_QUIT_HANDLER receives SIGINT.
        while (COMTERP_QUIT_HANDLER::instance ()->is_set () == 0)
            COMTERP_REACTOR::instance ()->handle_events ();
    
        return 0;
    }
    if (client_flag || telcat_flag) {
      
        const char *server_host = argc > 2 ? argv[2] : SERVER_HOST;
	u_short server_port  = argc > 3 ? 
	    ACE_OS::atoi (argv[3]) : SERVER_PORT;

	ACE_SOCK_Stream server;
	ACE_SOCK_Connector connector;
	ACE_INET_Addr addr (server_port, server_host);

    if (connector.connect (server, addr) == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "open"), -1);
  
    FILE* fptr = fdopen(server.get_handle(), "r+");
    char buffer[BUFSIZ];

    FILE* inptr = argc>=5 ? fopen(argv[4], "r") : stdin;

    if (!telcat_flag) {
      for (;;) {
	fgets(buffer, BUFSIZ, inptr);
	if (feof(inptr)) break;
	fputs(buffer, fptr);
	fflush(fptr);
	fgets(buffer, BUFSIZ, fptr);
	fputs(buffer, stdout);
      }

    } else {

      for (;;) {
	unsigned char ch = fgetc(inptr);
	if (feof(inptr)) break;
	fputc(ch, fptr);
      }
      fflush(fptr);

    }

    if (argc<=4 && inptr)
      fclose(inptr);
    
    if (server.close () == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "close"), -1);

    return 0;
    }
#endif


    ComTerp* terp;
    if (server_flag) 
      terp = new ComTerpServ(BUFSIZ);
    else
      terp = new ComTerp();

    terp->add_defaults();

    return terp->run();

}

