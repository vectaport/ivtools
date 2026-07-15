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

#include <cstdio>

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch.h>

static u_short SERVER_PORT = 20000;
static const char *const SERVER_HOST = ACE_DEFAULT_SERVER_HOST;
#endif

#include <fstream.h>

#include <iostream.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sysexits.h>

#include <version.h>
#include <patch.h>

#include <ComUtil/util.h>

#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/ctrlfunc.h>

#include <execinfo.h>

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

using std::cout;
using std::cerr;

void stack_trace_handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

#ifdef LEAKCHECK
/* the per-class LeakCheckers are lazily new'ed statics (never deleted), so
   their report-on-destruct never fires -- print alive counts at exit instead */
#include <Attribute/attrlist.h>
static void leakcheck_report() {
  if (AttributeValue::_leakchecker)
    fprintf(stderr, "LEAKCHECK: AttributeValue alive = %d\n",
	    AttributeValue::_leakchecker->alive());
  if (AttributeValueList::_leakchecker)
    fprintf(stderr, "LEAKCHECK: AttributeValueList alive = %d\n",
	    AttributeValueList::_leakchecker->alive());
  if (AttributeList::_leakchecker)
    fprintf(stderr, "LEAKCHECK: AttributeList alive = %d\n",
	    AttributeList::_leakchecker->alive());
}
#endif

int main(int argc, char *argv[]) {

#ifdef LEAKCHECK
    atexit(leakcheck_report);
#endif
    signal(SIGSEGV, stack_trace_handler);
    /* ignore SIGPIPE: a peer that closed its socket should make a write return
       EPIPE (which remote()/the socket path handles) instead of terminating the
       interpreter.  matters for `comterp listen` driving multiple drawservs --
       e.g. drawmo's gsbrushB, where a far node's connection can drop mid-run. */
    signal(SIGPIPE, SIG_IGN);
    /* Ctrl-C (SIGINT) is the common way an interactive session ends --
       restore tty echo first if tty_echo_off() ever ran, issue #76. */
    tty_echo_install_signal_handlers();

    boolean server_flag = argc>1 && strcmp(argv[1], "server") == 0;
    boolean logger_flag = argc>1 && strcmp(argv[1], "logger") == 0;
    boolean remote_flag = argc>1 && strcmp(argv[1], "remote") == 0;
    boolean client_flag = argc>1 && strcmp(argv[1], "client") == 0;
    boolean telcat_flag = argc>1 && strcmp(argv[1], "telcat") == 0;
    boolean run_flag = argc>1 && strcmp(argv[1], "run") == 0;
    boolean listen_flag = argc>1 && strcmp(argv[1], "listen") == 0;
    boolean help_flag = argc>1 && (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "--help") == 0 ||
                        strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "--?") == 0 || strcmp(argv[1], "?") == 0);
    boolean expr_flag = argc>1 && !server_flag && !logger_flag &&
                        !remote_flag && !client_flag && !telcat_flag && !run_flag && !listen_flag && !help_flag;

    if (help_flag) {
        fprintf(stdout,
"ivtools-%s comterp %s\n"
"Usage:\n"
"  comterp                            interactive REPL, reading from stdin\n"
"  comterp '<expr>'                   evaluate a single expression and print the result\n"
"  comterp run <file> [args...]       run a ComTerp script file\n"
"  comterp listen [port [file]]       accept ComTerp connections on port (default %s), optionally seeded by running a script\n"
"  comterp server [port]              like listen, but also serves stdin as an interactive session\n"
"  comterp logger [port]              like server, but suppresses the \"accepting connections\" banner\n"
"  comterp remote [port]              large-buffer interactive/piped mode, for embedding via IPC\n"
"  comterp client host [port [file]]  connect to a comterp/comterp_listen server and relay stdin/replies\n"
"  comterp telcat host [port [file]]  like client, but with raw pass-through (no reply echoing)\n"
"  comterp -help | --help | -? | --? | ?\n"
"                                      print this message and exit\n",
                VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY), ACE_DEFAULT_SERVER_PORT_STR);
        return 0;
    }

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
        while (COMTERP_QUIT_HANDLER::instance ()->is_set () == 0) {
            ComterpHandler::reactor_singleton()->handle_events ();
	}
        return 0;
    }
    if (listen_flag) {
        int portnum = argc > 2 ? atoi(argv[2]) : atoi(ACE_DEFAULT_SERVER_PORT_STR);
        const char* rfile = argc > 3 ? argv[3] : nil;

        ComterpAcceptor* peer_acceptor =
	    new ComterpAcceptor(ComterpHandler::reactor_singleton());

        if (peer_acceptor->open(ACE_INET_Addr(portnum),
				ComterpHandler::reactor_singleton()) == -1) {
            cerr << "comterp: unable to open port " << portnum << " with ACE\n";
            return EX_TEMPFAIL;  // signal comterp_listen.bash to retry on next port
        }

#if !defined(__NetBSD__)
        else if (ComterpHandler::reactor_singleton()->register_handler
                  (peer_acceptor, ACE_Event_Handler::READ_MASK) == -1)
          cerr << "comterp: error registering acceptor with ACE reactor\n";
#endif

	else
	  cerr << "accepting comterp port (" << portnum << ") connections\n";

        if (ComterpHandler::reactor_singleton()->register_handler
	     (SIGINT, COMTERP_QUIT_HANDLER::instance ()) == -1)
          ACE_ERROR_RETURN ((LM_ERROR,
			 "registering service with ACE_Reactor\n"), -1);

	ComterpHandler* stdin_handler = new ComterpHandler();
	if (ComterpHandler::reactor_singleton()->register_handler(0, stdin_handler,
							    ACE_Event_Handler::READ_MASK)==-1)
	  cerr << "comterp: unable to open stdin with ACE\n";

	/* run script after all reactor registrations are in place */
	if (rfile) {
	    ComTerpServ* terp = stdin_handler->comterp();
	    int endcnt = 0;
	    for (int i=argc-1; i>3; i--) {
	        if (*argv[i]=='\0') endcnt++;
	        else break;
	    }
	    terp->set_args(argc-3-endcnt, argv+3);
	    /* anchor relative run() paths in the script to the script's own
	       directory (realpath of rfile), as the `run' launch path does --
	       otherwise a shebang script found via $PATH and launched from an
	       unrelated CWD resolves its run("./...") loaders against that CWD
	       and fails.  See RunFunc::set_basepath / execute(). */
	    RunFunc::set_basepath(rfile);
	    fprintf(stdout, "ivtools-%s comterp: type help for more info %s\n%s", VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY), get_command_prompt());
	    if (terp->runfile(rfile) < 0)
	        cerr << "comterp: error running script file: " << rfile << "\n";
	}

        // Run event loop until COMTERP_QUIT_HANDLER receives SIGINT.
        while (COMTERP_QUIT_HANDLER::instance ()->is_set () == 0) {
            ComterpHandler::reactor_singleton()->handle_events ();
	}
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

    FILE* ofptr = nil;

    if (!telcat_flag) {
      
      FILEBUF(obuf, ofptr = fdopen(server.get_handle(), "w"), ios_base::out);
      ostream out(&obuf);
      
      FILE* ifptr = nil;
      FILEBUF(ibuf, ifptr = fdopen(server.get_handle(), "r"), ios_base::in);
      
      istream in(&ibuf);
      
      for (;;) {
	if (!fgets(buffer, BUFSIZ*BUFSIZ, inptr)) break;  // EOF or read error
	out << buffer;
	out.flush();
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
      }

#ifndef __APPLE__
      if (ofptr) fclose(ofptr);
      if (ifptr) fclose(ifptr);
#endif
      
    } else if (inptr) {


      FILEBUF(inbuf, inptr, ios_base::in);
      istream in(&inbuf);
      

      FILEBUF(obuf, fdopen(server.get_handle(), "w"), ios_base::out);
      ostream out(&obuf);

      char buffer[BUFSIZ*BUFSIZ];
      while(!in.eof() && in.good()) {
	in.read(buffer, BUFSIZ*BUFSIZ);
	if (!in.eof() || in.gcount())
	  out.write(buffer, in.gcount());
      }
      out.flush();
#ifndef __APPLE__
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

    if (server_flag || remote_flag) {
      ComTerpServ* terp = new ComTerpServ(BUFSIZ*BUFSIZ);
      terp->add_defaults();
      struct stat buf;
      int status = fstat(fileno(stdin), &buf);
      if (S_ISREG(buf.st_mode) || S_ISFIFO(buf.st_mode))
	terp->disable_prompt();
      else {
	tty_echo_off();  // issue #76 -- see ComUtil/ttyecho.c
	fprintf(stdout, "ivtools-%s comterp: type help for more info %s\n%s", VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY), get_command_prompt());
      }
      return terp->run();
    } else {

      ComTerpServ* terp = new ComTerpServ();
      terp->add_defaults();
      if (run_flag && argc > 2 ) {
	fprintf(stderr, "ivtools-%s comterp %s\n", VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY));
	int endcnt=0;
	for(int i=argc-1; i>2; i--) {
	  if(*argv[i]=='\0') {
	    endcnt++;
          } else {
	    break;
	  }
	    
	}
        const char *rfile = argv[2];
	terp->set_args(argc-2-endcnt, argv+2);
	RunFunc::set_basepath(rfile);
	terp->runfile(rfile);
	// echo the file's last expression, same as `expr` does for a single
	// one -- runfile() already pushes it onto the stack (see
	// ComTerp::runfile()), it just never prints it on its own.
	terp->brief(1);
	ComValue::comterp(terp);
	cout << terp->stack_top() << '\n';
	cout.flush();
	return 0;
      }

      if (expr_flag) {
	terp->brief(1);
	ComValue::comterp(terp);
        ComValue comval(terp->run(argv[1]));
        cout << comval << '\n';
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
	else {
	  tty_echo_off();  // issue #76 -- see ComUtil/ttyecho.c
	  fprintf(stdout, "ivtools-%s comterp:  type help for more info %s\n%s", VersionString, build_stamp(__DATE__, __TIME__, PATCH_KEY), get_command_prompt());
	}
	return terp->run();
      }
    }

}

