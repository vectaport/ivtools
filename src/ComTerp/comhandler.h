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

#include <signal.h>
#include <ace/Acceptor.h>
#include <ace/Reactor.h>
#include <ace/Singleton.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Acceptor.h>

class ComTerpServ;

//: handler to invoke ComTerp on socket (version with ACE)
class ComterpHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{

public:
  // = Initialization and termination methods.
  ComterpHandler (void);
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
};

//: A Reactor Singleton.
typedef ACE_Singleton<ACE_Reactor, ACE_Null_Mutex> 
	COMTERP_REACTOR;

//: An ACE_Test_and_Set Singleton.
typedef ACE_Singleton<ACE_Test_and_Set <ACE_Null_Mutex, sig_atomic_t>, ACE_Null_Mutex> 
	COMTERP_QUIT_HANDLER;

//: Specialize a ComterpAcceptor.
typedef ACE_Acceptor <ComterpHandler, ACE_SOCK_ACCEPTOR> 
	ComterpAcceptor;


#else 

#include <ComTerp/comterpserv.h>

class ComTerpServ;

//: version without ACE
class ComterpHandler {
public:
    ComterpHandler(void) {comterp_ = new ComTerpServ(); _handle = 0;}
    ComterpHandler(int id) { comterp_ = new ComTerpServ(); _handle = id;}
    int get_handle() { return _handle;}

protected:
    int _handle;
    ComTerpServ* comterp_;
};
#endif

#endif /* _comterp_handler_ */
