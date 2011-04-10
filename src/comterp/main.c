/*
 * Copyright (c) 1994-1999 Vectaport, Inc.
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

#include <sys/stat.h>
#include <unistd.h>

#include <version.h>

#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

#define GEOMOBJS_LINKING_TEST
#if defined (GEOMOBJS_LINKING_TEST)
#include <Unidraw/Graphic/geomobjs.h>
#include <TopoFace/fgeomobjs.h>
PointObj ip(0,0);
FPointObj fp(0.,0.);
#endif

int main(int argc, char *argv[]) {

    boolean server_flag = argc>1 && strcmp(argv[1], "server") == 0;
    boolean remote_flag = argc>1 && strcmp(argv[1], "remote") == 0;
    boolean client_flag = argc>1 && strcmp(argv[1], "client") == 0;
    boolean telcat_flag = argc>1 && strcmp(argv[1], "telcat") == 0;
    boolean run_flag = argc>1 && strcmp(argv[1], "run") == 0;

#ifdef HAVE_ACE
    if (server_flag) {
        ComterpAcceptor* peer_acceptor = new ComterpAcceptor();

        int portnum = argc > 2 ? atoi(argv[2]) : atoi(ACE_DEFAULT_SERVER_PORT_STR);
        if (peer_acceptor->open (ACE_INET_Addr (portnum)) == -1)
            cerr << "comterp: unable to open port " << portnum << " with ACE\n";

        else if (COMTERP_REACTOR::instance ()->register_handler
                  (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
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
	ComterpHandler* stdin_handler = new ComterpHandler();
#if 0
	if (ACE::register_stdin_handler(stdin_handler, COMTERP_REACTOR::instance(), nil) == -1)
#else
	if (COMTERP_REACTOR::instance()->register_handler(0, stdin_handler, 
							  ACE_Event_Handler::READ_MASK)==-1)
#endif
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
    char buffer[BUFSIZ*BUFSIZ];

    FILE* inptr = argc>=5 ? fopen(argv[4], "r") : stdin;

    if (!telcat_flag) {
      
      filebuf obuf;
      obuf.attach(server.get_handle());
      ostream out(&obuf);
      
      filebuf ibuf;
      ibuf.attach(server.get_handle());
      istream in(&ibuf);
      
      for (;;) {
	fgets(buffer, BUFSIZ*BUFSIZ, inptr);
	if (feof(inptr)) break;
	out << buffer;
	out.flush();
	char* inbuf;
	char ch;
	ch = in.get();
	if (ch == '>')
	  ch = in.get(); // ' '
	else {
	  in.unget();
	  in.gets(&inbuf);
	  if (client_flag) 
	    cout << inbuf << "\n";
	}
      }
      
    } else {

      filebuf inbuf;
      inbuf.attach(fileno(inptr));
      istream in(&inbuf);
      
      filebuf obuf;
      obuf.attach(server.get_handle());
      ostream out(&obuf);

#if 0      
      for (;;) {
	char ch;
	in.get(ch);
	if (!in.good()) break;
	out << ch;
      }
#else
      char buffer[BUFSIZ*BUFSIZ];
      while(!in.eof() && in.good()) {
	in.read(buffer, BUFSIZ*BUFSIZ);
	if (!in.eof() || in.gcount())
	  out.write(buffer, in.gcount());
      }
#endif
      out.flush();
      
#if 0
      for (;;) {
	fgets(buffer, BUFSIZ*BUFSIZ, inptr);
	if (feof(inptr)) break;
	fputs(buffer, fptr);
	fflush(fptr);
#if 0
	fgets(buffer, BUFSIZ*BUFSIZ, fptr);
	fputs(buffer, stdout);
#else
	char ch;
	ch = getc(fptr);
	if (ch == '>') {
	  ch = getc(fptr);
	  if (ch != ' ') {
	    ungetc(ch, fptr);
	    ungetc('>', fptr);
	    fgets(buffer, BUFSIZ*BUFSIZ, fptr);
	    fputs(buffer, stdout);
	  } else {
	    printf( "> " );
	  }
	} else {
	  ungetc(ch, fptr);
	  fgets(buffer, BUFSIZ*BUFSIZ, fptr);
	  fputs(buffer, stdout);
	}
#endif
      }
#endif

    }

    if (argc<=4 && inptr)
      fclose(inptr);
    
    if (server.close () == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "close"), -1);

    return 0;
    }
#endif

    if (server_flag || remote_flag) {
      ComTerpServ* terp = new ComTerpServ(BUFSIZ*BUFSIZ);
      terp->add_defaults();
      struct stat buf;
      int status = fstat(fileno(stdin), &buf);
      if (S_ISREG(buf.st_mode) || S_ISFIFO(buf.st_mode))
	terp->disable_prompt();
      else
	fprintf(stderr, "ivtools-%s comterp: type help for more info\n", VersionString);
      return terp->run();
    } else {
      ComTerp* terp = new ComTerp();
      terp->add_defaults();
      if (run_flag && argc > 2 ) {
        const char *rfile = argv[2];
	terp->runfile(rfile);
	return 0;
      } else {
	
	struct stat buf;
	int status = fstat(fileno(stdin), &buf);
#if 0
	fprintf(stderr,"S_ISLNK %d\n", S_ISLNK(buf.st_mode));
	fprintf(stderr,"S_ISREG %d\n", S_ISREG(buf.st_mode));
	fprintf(stderr,"S_ISDIR %d\n", S_ISDIR(buf.st_mode));
	fprintf(stderr,"S_ISCHR %d\n", S_ISCHR(buf.st_mode));
	fprintf(stderr,"S_ISBLK %d\n", S_ISBLK(buf.st_mode));
	fprintf(stderr,"S_ISFIFO %d\n", S_ISFIFO(buf.st_mode));
	fprintf(stderr,"S_ISSOCK %d\n", S_ISSOCK(buf.st_mode));
#endif
        if (S_ISREG(buf.st_mode) || S_ISFIFO(buf.st_mode))
	  terp->disable_prompt();
	else
	  fprintf(stderr, "ivtools-%s comterp:  type help for more info\n", VersionString);
	return terp->run();
      }
    }

}

