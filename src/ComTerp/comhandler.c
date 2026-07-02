/*
 * Copyright (c) 2005 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1996 Vectaport Inc.
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
#include <iostream.h>
#include <fstream.h>
using namespace std;
#include <vector>

#include <ComTerp/comhandler.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>

#include <signal.h>

#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

int ComterpHandler::_logger_mode = 0;

/*****************************************************************************/

// Default constructor.

ComterpHandler::ComterpHandler (ComTerpServ* serv) 
#if 0
: ACE_Svc_Handler<ACE_SOCK_Stream, ACE_NULL_SYNCH>(0,0,ComterpHandler::reactor_singleton())
#endif
{
  //comterp_ = serv ? serv : new ComTerpServ(BUFSIZ*BUFSIZ);
    comterp_ = serv ? serv : new ComTerpServ(BUFSIZ*16);
    comterp_->handler(this);
    comterp_->add_defaults();
    _timeoutscriptid = -1;
    _wrfptr = _rdfptr = nil;
    _log_only = 0;
    _alt_fd = -1;
}

ComterpHandler::~ComterpHandler() {
}

const char* ComterpHandler::timeoutscript() { return symbol_pntr(_timeoutscriptid); }

void ComterpHandler::timeoutscript(const char* timeoutscript) {
    if (timeoutscript)
        timeoutscriptid(symbol_add((char *)timeoutscript));
}

int ComterpHandler::timeoutscriptid() { return _timeoutscriptid; }
void ComterpHandler::timeoutscriptid(int timeoutscriptid) {
    if (_timeoutscriptid != -1) 
      ComterpHandler::reactor_singleton()->cancel_timer (this);

    _timeoutscriptid = timeoutscriptid;
    if (_timeoutscriptid != -1) {
      if (ComterpHandler::reactor_singleton()->schedule_timer
	  (this, 
	   (const void *) this, 
	   ACE_Time_Value (timeoutseconds()), 
	   ACE_Time_Value (timeoutseconds())) == -1)
	/* ACE_ERROR_RETURN ((LM_ERROR, 
	   "can'(%P|%t) t register with reactor\n"), -1) */;
    }
}

void
ComterpHandler::destroy (void)
{
  const char* peer_name = this->peer_name_;
  if (ComterpHandler::logger_mode()==0) {
      if (*peer_name != '\0')
	ACE_DEBUG ((LM_DEBUG, 
		    "(%P|%t) disconnected from %s\n", peer_name));
  }
#if 0
    ComterpHandler::reactor_singleton()->cancel_timer (this);
#endif
    this->peer ().close ();
    if (_timeoutscriptid<0) {
      if (comterp_ && comterp_->running()) 
	comterp_->delete_later(1);
      else {
	delete comterp_;
	comterp_ = nil;
      }
    }
    else /* timer could be still running */;
    if (_wrfptr) {
      fclose(_wrfptr);
      _wrfptr = nil;
    }
    if (_rdfptr) {
      fclose(_rdfptr);
      _rdfptr = nil;
    }
}

int
ComterpHandler::handle_timeout (const ACE_Time_Value &,
				 const void *arg)
{
    if (_timeoutscriptid<0) return 0;
    comterp_->push_servstate();
    comterp_->load_string(symbol_pntr(_timeoutscriptid));
    if (comterp_->read_expr()) {
        if (comterp_->eval_expr()) {
	    err_print( stderr, "comterp" );
	    timeoutscriptid(-1);
        } else if (comterp_->quitflag()) {
#if 0 /* bug */	 
	    delete comterp_;
#endif
	    timeoutscriptid(-1);
	    comterp_->pop_servstate();
	    comterp_ = nil;
	    return -1;
	} else {
	    if (!comterp_->stack_empty()) {
	      FILEBUF(obuf, stdout, ios_base::out);
	      ostream ostr(&obuf);
	      ostr << "timeexpr result:  ";
	      comterp_->print_stack_top(ostr);
	      ostr << "\n";
	      ostr.flush();
	    }
	}
    }
    comterp_->pop_servstate();
    return 0;
}

int
ComterpHandler::handle_input (ACE_HANDLE fd)
{
    _wrfd = fd;
    if (!_wrfptr) _wrfptr = fdopen(dup(fd), "w");
    // if (!_rdfptr) _rdfptr = fdopen(fd, "r");

    vector<char> inv;
    char ch;

    ch = '\0';
    int status=1;
    int bytesavail=1;
    while (ch != '\n' && status>0 && bytesavail) {
      status = read(fd, &ch, 1);
      if (status == 1 && ch != '\n') inv.push_back(ch);
      bytesavail=0;
      ioctl(fd, FIONREAD, &bytesavail);
    }
    inv.push_back('\0');
      
    boolean input_good = status > 0;

    char* inbuf = &inv[0];
    if (!comterp_ || !input_good)
      return -1;
    else if (!inbuf ) {
	return -1;
    }
    else if ( !*inbuf) {
	return 0;
    }

    if (!ComterpHandler::logger_mode() && !log_only()) {

      /* Typed input can arrive while a script is already running on this same
         interpreter -- e.g. a -runfile for-loop whose update() pumped the ACE
         reactor and dispatched this line.  ComTerp::run(!nested) resets the
         shared operand stack (eval_expr's `_stack_top = -1`, plus the trailing
         `if (!nested) decr_stack(_stack_top+1)`), which wipes the suspended
         script's in-progress stack -> it resumes on an empty stack and crashes
         (ForFunc reads a garbage argoff via stack_top()).  When re-entrant,
         isolate the eval so the running script sees no trace of it:
           - push_servstate() protects the postfix buffer (_pfbuf/_pfnum/...)
             that load_string()/read_expr() would otherwise clobber;
           - run *nested* so the shared operand stack is not reset, then pop the
             typed line's result(s) so the cursor is exactly where it was;
           - save/restore _just_reset.  The script can be suspended right after
             a reset_stack() (comfunc.c) with _just_reset==1 -- the signal for
             eval_expr_internals to push the blankval that stands in for the
             just-reset result.  The typed eval's own push_stack() clears the
             flag to 0 (comterp.c), so without this the blankval is never pushed,
             the script's stack comes up one short, and skip_arg walks off the
             end (offlimit).  push_servstate() does NOT cover this flag (its save
             is commented out, because the synchronous re-entrant callers -- run,
             remote -- rely on _just_reset propagating across that boundary; an
             async stdin interruption must instead be fully transparent). */
      boolean reentrant = comterp_->running();
      int stack_base = 0;
      boolean old_just_reset = false;
      if (reentrant) {
	comterp_->push_servstate();
	stack_base = comterp_->stack_height();
	old_just_reset = comterp_->_just_reset;
      }

      comterp_->load_string(inbuf);

      // this hides the logging of a ready command, interesting
      if (fd>0 && !comterp_->muted() ) {
	  struct timeval tv;
	  log_command(inbuf, "<", (_alt_fd>-1 ? _alt_fd : fd));
      }

      comterp_->_fd = fd;
      comterp_->_outfunc = (fd == 0) ? (outfuncptr)&stdout_puts : (outfuncptr)&ComTerpServ::fd_fputs;
      int  status = comterp_->ComTerp::run(false /* !once */,
			   reentrant ? true : comterp_->force_nested() /* nested */);
      if (reentrant) {
	while (comterp_->stack_height() > stack_base) comterp_->pop_stack(false);
	comterp_->_just_reset = old_just_reset;
	comterp_->pop_servstate();
      } else if (comterp_->force_nested())
	ComValue retval(comterp_->pop_stack(false));
      if (comterp_->delete_later()) {
	delete comterp_;
	comterp_ = nil;
      }
      return input_good&&(status==0||status==3||status==2) ? 0 : -1;
    } else {
      if (inbuf[0]!='\004')
	cout << "from pipe(" << fd << "):  " << inbuf << "\n";
      FILE* fp = fd ? fdopen(dup(fd), "w") : stdout;
      fputs("\n", fp);
      fclose(fp);
      return (input_good && inbuf[0]!='\004') ? 0 : -1;
    }
}

int
ComterpHandler::open (void *)
{
  ACE_INET_Addr addr;
  
  if (this->peer ().get_remote_addr (addr) == -1)
    return -1;
  else
    {
      const char* hostname = addr.get_host_name();
      char buffer[MAXHOSTNAMELEN];
      addr.addr_to_string(buffer, MAXHOSTNAMELEN);
      ACE_OS::strncpy (this->peer_name_, buffer,
		       MAXHOSTNAMELEN + 1);

      if (ComterpHandler::reactor_singleton()->register_handler 
	  (this, ACE_Event_Handler::READ_MASK) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "(%P|%t) can't register with reactor\n"), -1);
#if defined(__NetBSD__) /* this seems to be required for NetBSD */
      else if (ComterpHandler::reactor_singleton()->schedule_timer
	  (this, 
	  (const void *) this, 
	   ACE_Time_Value (10), 
	   ACE_Time_Value (10)) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "can'(%P|%t) t register with reactor\n"), -1);
#endif
      else
	if (ComterpHandler::logger_mode()==0) 
	  ACE_DEBUG ((LM_DEBUG, 
		      "(%P|%t) connected with %s\n", this->peer_name_));
      return 0;
    }
}

// Perform termination activities when deregistered from the
// ACE_Reactor.

int
ComterpHandler::close (u_long)
{
  this->destroy ();
  return 0;
}

// Our Reactor Singleton.
typedef ACE_Singleton<ACE_Reactor, ACE_Null_Mutex>
REACTOR;

ACE_Reactor* ComterpHandler::reactor_singleton() {
  return REACTOR::instance();
}

void ComterpHandler::log_command(const char* cmdstring, const char* fd_or_port_prefix, int fd_or_port) {
  char buffer[BUFSIZ];
  snprintf(buffer, BUFSIZ, "%s%d: %s", fd_or_port_prefix, fd_or_port, cmdstring);
  log_with_timestamp(buffer);
}

#endif /* HAVE_ACE */
