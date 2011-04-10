/*
 * Copyright (c) 2000 IET Inc.
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

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch.h>

static u_short SERVER_PORT = 20000;
static const char *const SERVER_HOST = ACE_DEFAULT_SERVER_HOST;
#endif

#include <ComTerp/comterpserv.h>


#include <fstream.h>

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

#if __GNUC__>=3
static char newline;
#endif

int main(int argc, char *argv[]) {

    boolean server_flag = argc>1 && strcmp(argv[1], "server") == 0;
    boolean logger_flag = argc>1 && strcmp(argv[1], "logger") == 0;
    boolean remote_flag = argc>1 && strcmp(argv[1], "remote") == 0;
    boolean client_flag = argc>1 && strcmp(argv[1], "client") == 0;
    boolean telcat_flag = argc>1 && strcmp(argv[1], "telcat") == 0;
    boolean run_flag = argc>1 && strcmp(argv[1], "run") == 0;

#ifdef HAVE_ACE
    if (server_flag || logger_flag) {
        ComterpAcceptor* peer_acceptor = 
	    new ComterpAcceptor(ComterpHandler::reactor_singleton());
	ComterpHandler::logger_mode(logger_flag);

        int portnum = argc > 2 ? atoi(argv[2]) : atoi(ACE_DEFAULT_SERVER_PORT_STR);
        if (peer_acceptor->open (ACE_INET_Addr (portnum),
				 ComterpHandler::reactor_singleton()) == -1)
            cerr << "comterp: unable to open port " << portnum << " with ACE\n";

#if !defined(__NetBSD__)  /* this is not the way to do it for NetBSD */
        else if (ComterpHandler::reactor_singleton()->register_handler
                  (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
          cerr << "comterp: error registering acceptor with ACE reactor\n";
#endif

	else if (ComterpHandler::logger_mode()==0)
	  cerr << "accepting comterp port (" << portnum << ") connections\n";
    
        // Register COMTERP_QUIT_HANDLER to receive SIGINT commands.  When received,
        // COMTERP_QUIT_HANDLER becomes "set" and thus, the event loop below will
        // exit.
        if (ComterpHandler::reactor_singleton()->register_handler 
    	     (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
          ACE_ERROR_RETURN ((LM_ERROR, 
    			 "registering service with ACE_Reactor\n"), -1);
    
	// Start up one on stdin
	if (!logger_flag) {
	  ComterpHandler* stdin_handler = new ComterpHandler();
	  if (ComterpHandler::reactor_singleton()->register_handler(0, stdin_handler, 
							    ACE_Event_Handler::READ_MASK)==-1)
	    cerr << "comterp: unable to open stdin with ACE\n";
	}

        // Perform logging service until COMTERP_QUIT_HANDLER receives SIGINT.
        while (COMTERP_QUIT_HANDLER::instance ()->is_set () == 0)
            ComterpHandler::reactor_singleton()->handle_events ();
    
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
      
#if __GNUC__<3
      filebuf obuf;
      obuf.attach(server.get_handle());
#else
      FILE* ofptr = nil;
      fileptr_filebuf obuf(ofptr = fdopen(server.get_handle(), "w"), ios_base::out);
#endif
      ostream out(&obuf);
      
#if __GNUC__<3
      filebuf ibuf;
      ibuf.attach(server.get_handle());
#else
      FILE* ifptr = nil;
      fileptr_filebuf ibuf(ifptr = fdopen(server.get_handle(), "r"), ios_base::in);
#endif
      
      istream in(&ibuf);
      
      for (;;) {
	fgets(buffer, BUFSIZ*BUFSIZ, inptr);
	if (feof(inptr)) break;
	out << buffer;
	out.flush();
#if __GNUC__<3
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
#else
	char inbuf[BUFSIZ];
	char ch;
	ch = in.get();
	if (ch == '>')
	  ch = in.get(); // ' '
	else {
	  in.unget();
	  in.get(inbuf, BUFSIZ);
	  in.get(newline);
	  if (client_flag) 
	    cout << inbuf << "\n";
	}
#endif
      }

#if __GNUC__>=3
      if (ofptr) fclose(ofptr);
      if (ifptr) fclose(ifptr);
#endif
      
    } else if (inptr) {


#if __GNUC__<3
      filebuf inbuf;
      inbuf.attach(fileno(inptr));
#else
      fileptr_filebuf inbuf(inptr, ios_base::in);
#endif
      istream in(&inbuf);
      

#if __GNUC__<3
      filebuf obuf;
      obuf.attach(server.get_handle());
#else
      FILE* ofptr = nil;
      fileptr_filebuf obuf(fdopen(server.get_handle(), "w"), ios_base::out);
#endif
      ostream out(&obuf);

      char buffer[BUFSIZ*BUFSIZ];
      while(!in.eof() && in.good()) {
	in.read(buffer, BUFSIZ*BUFSIZ);
	if (!in.eof() || in.gcount())
	  out.write(buffer, in.gcount());
      }
      out.flush();
#if __GNUC__>=3
      if (ofptr) fclose(ofptr);
#endif
    } else 
      cerr << "comterp: unable to open file:  " << argv[4] << "\n";

    if (argc<=4 && inptr)
      fclose(inptr);
    
    if (server.close () == -1)
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "close"), -1);

    return 0;
    }
#endif /* defined(HAVE_ACE) */

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

