/*
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

#include <ComTerp/comhandler.h>
#include <ComTerp/comterpserv.h>

#include <iostream.h>

#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

/*****************************************************************************/

// Default constructor.

ComterpHandler::ComterpHandler (void)
{
    comterp_ = new ComTerpServ(BUFSIZ*BUFSIZ);
    comterp_->handler(this);
    comterp_->add_defaults();
    _timeoutscriptid = -1;
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
      COMTERP_REACTOR::instance ()->cancel_timer (this);

    _timeoutscriptid = timeoutscriptid;
    if (_timeoutscriptid != -1) {
      if (COMTERP_REACTOR::instance ()->schedule_timer
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
    ACE_DEBUG ((LM_DEBUG, 
	      "(%P|%t) disconnected from %s\n", this->peer_name_));
#if 0
    COMTERP_REACTOR::instance ()->cancel_timer (this);
#endif
    this->peer ().close ();
    delete comterp_;
}

int
ComterpHandler::handle_timeout (const ACE_Time_Value &,
				 const void *arg)
{
    if (_timeoutscriptid<0) return 0;
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
	    comterp_ = nil;
	    return -1;
	} else {
	    if (!comterp_->stack_empty()) {
	      filebuf obuf(1);
	      ostream ostr(&obuf);
	      comterp_->print_stack_top(ostr);
	      ostr << "\n";
	      ostr.flush();
	    }
	}
    }
    return 0;
}

// Perform the logging record receive. 
static const unit = 15;

// #define INPUT_BY_READ

int
ComterpHandler::handle_input (ACE_HANDLE fd)
{
#if 1
    const int bufsiz = BUFSIZ*BUFSIZ;
    char inbuf[bufsiz];
    char outbuf[bufsiz];
    inbuf[0] = '\0';
#if !defined(INPUT_BY_READ)
    filebuf ibuf(fd);
    istream istr(&ibuf);
    istr.getline(inbuf, bufsiz);
    if (!comterp_ || !istr.good())
      return -1; 
    else if (!*inbuf) {
      filebuf obuf(fd ? fd : 1);
      ostream ostr(&obuf);
      ostr << "\n";
      ostr.flush();
      return 0;
    }
#else
    int len = 0;
    while (len<bufsiz-2) {
      int nread = read(fd, inbuf+len, 1);
      if (nread!=1) {
	cerr << "zero bytes read, connection closed\n";
	return -1;
      }
      if (inbuf[len] == '\n') break;
      len++;
    }
    inbuf[len] = '\0';
#endif
    comterp_->load_string(inbuf);
    if (fd) 
      cerr << "command via ACE -- " << inbuf << "\n";
#else
    comterp_->_infunc = (infuncptr)&ComTerpServ::fd_fgets;
#endif
    comterp_->_fd = fd;
    comterp_->_outfunc = (outfuncptr)&ComTerpServ::fd_fputs;

#if 1
    int  status = comterp_->ComTerp::run(false /* !once */);
#else
    if (comterp_->read_expr()) {
        if (comterp_->eval_expr()) {
	    err_print( stderr, "comterp" );
	    filebuf obuf(fd ? fd : 1);
	    ostream ostr(&obuf);
	    ostr << "err\n";
	    ostr.flush();
        } else if (comterp_->quitflag()) {
	    return -1;
	} else {
	    filebuf obuf(fd ? fd : 1);
	    ostream ostr(&obuf);
	    comterp_->print_stack_top(ostr);
	    ostr << "\n";
	    ostr.flush();
	}
    }
#endif
#if 1
#if !defined(INPUT_BY_READ)
    return (istr.good() ? 0 : -1) && status;
#else
    return status;
#endif
#else
    return comterp_->_instat ? 0 : -1;
#endif
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

      if (COMTERP_REACTOR::instance ()->register_handler 
	  (this, ACE_Event_Handler::READ_MASK) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "(%P|%t) can't register with reactor\n"), -1);
#if 0
      else if (COMTERP_REACTOR::instance ()->schedule_timer
	  (this, 
	  (const void *) this, 
	   ACE_Time_Value (10), 
	   ACE_Time_Value (10)) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "can'(%P|%t) t register with reactor\n"), -1);
#endif
      else
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

#endif /* HAVE_ACE */
