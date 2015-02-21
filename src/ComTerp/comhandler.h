/*
 * Copyright (c) 1996,1999 Vectaport Inc.
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

#ifndef _comterp_handler_
#define _comterp_handler_

/*
 * ComterpHandler is an ACE handler that can be invoked by an ACE acceptor
 */

#ifdef HAVE_ACE

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <stdio.h>
#ifdef __alpha__
#define __USE_GNU
#endif
#include <signal.h>
#include <ace/Acceptor.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/Test_and_Set.h>

// GNU HURD has no fixed limit
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 4096
#endif

class ComTerpServ;

//: An ACE_Test_and_Set Singleton.
typedef ACE_Singleton<ACE_Test_and_Set <ACE_Null_Mutex, sig_atomic_t>, ACE_Null_Mutex> 
	COMTERP_QUIT_HANDLER;

//: handler to invoke ComTerp on socket (version with ACE)
class ComterpHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{

public:
  // = Initialization and termination methods.
  ComterpHandler (ComTerpServ* serv=NULL);
  virtual ~ComterpHandler();

  virtual void destroy (void);
  // Ensure dynamic allocation.

  // = Hooks for opening and closing handlers.
  virtual int open (void *);
  // open handler hook.
  virtual int close (u_long);
  // close handler hook.

  const char* timeoutscript();
  // return text of commands to execute every timeout.
  void timeoutscript(const char*);
  // install new text of commands to execute every timeout.
  int timeoutscriptid();
  // return symbol id of text of commands to execute every timeout.
  void timeoutscriptid(int);
  // set symbol id of text of commands to execute every timeout.

  void timeoutseconds(int seconds) { _seconds = seconds; }
  // set timeout period in seconds.
  int timeoutseconds() { return _seconds; }
  // return timeout period in seconds.

  ComTerpServ* comterp() { return comterp_; }
  // return associated ComTerpServ pointer.

  static void logger_mode(int flag) { _logger_mode = flag; }
  // set flag to put comterp in logging mode, where commands are echoed
  // to stdout without executing
  
  static int logger_mode() { return _logger_mode; }
  // return flag that indicates comterp is in logging-only mode

  FILE* wrfptr() { return _wrfptr; }
  // file pointer for writing to handle

  FILE* rdfptr() { return _rdfptr; }
  // file pointer for reading from handle

  static ACE_Reactor* reactor_singleton();
  // alternate way of getting at reactor singleton

  int log_only() { return _log_only; }
  // return flag that indicates whether just this handler is in logging mode 

  void log_only(int flag ) { _log_only = flag; }
  // set flag that indicates whether just this handler is in logging mode 

protected:
  // = Demultiplexing hooks.
  virtual int handle_input (ACE_HANDLE);
  // called when input ready on ACE_HANDLE.
  virtual int handle_timeout (const ACE_Time_Value &tv, 
			      const void *arg); 
  // called when timer goes off.

  char peer_name_[MAXHOSTNAMELEN + 1];
  // Host we are connected to.

  ComTerpServ* comterp_;
  // private local comterp server

  int  _timeoutscriptid;
  // command for timeoutscript execution

  int _seconds;
  // timeout in seconds

  FILE* _wrfptr;
  // file pointer for writing to handle

  FILE* _rdfptr;
  // file pointer for reading from handle

  static int _logger_mode;
  // mode for logging commands: 0 = no log, 1 = log only

  int _log_only;
  // put just this handler into log mode
};

//: Specialize a ComterpAcceptor.
typedef ACE_Acceptor <ComterpHandler, ACE_SOCK_ACCEPTOR> 
	ComterpAcceptor;


#else 

#include <ComTerp/comterpserv.h>

class ComTerpServ;

//: version without ACE
class ComterpHandler {
public:
    ComterpHandler(ComTerpServ* serv=nil) {comterp_ = serv ? serv : new ComTerpServ(); _handle = 0; comterp_->add_defaults();}
    ComterpHandler(int id, ComTerpServ* serv = nil) { comterp_ = serv ? serv : new ComTerpServ(); _handle = id; comterp_->add_defaults();}
    int get_handle() { return _handle;}

    FILE* wrfptr() { return nil; }
    // file pointer for writing to handle
    
    FILE* rdfptr() { return nil; }
    // file pointer for reading from handle

    ComTerp* comterp() { return comterp_; }

protected:
    int _handle;
    ComTerpServ* comterp_;

};
#endif

#endif /* _comterp_handler_ */
