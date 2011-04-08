/*
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

#include <ComTerp/comhandler.h>
#include <ComTerp/comterpserv.h>

#include <iostream.h>
#if __GNUC__==2 && __GNUC_MINOR__<=7
#else
#include <vector.h>
#endif

#if BUFSIZ>1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

int ComterpHandler::_logger_mode = 0;

/*****************************************************************************/

// Default constructor.

ComterpHandler::ComterpHandler (void)
{
    comterp_ = new ComTerpServ(BUFSIZ*BUFSIZ);
    comterp_->handler(this);
    comterp_->add_defaults();
    _timeoutscriptid = -1;
    _wrfptr = _rdfptr = nil;
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
    if (ComterpHandler::logger_mode()==0)
        ACE_DEBUG ((LM_DEBUG, 
		    "(%P|%t) disconnected from %s\n", this->peer_name_));
#if 0
    COMTERP_REACTOR::instance ()->cancel_timer (this);
#endif
    this->peer ().close ();
    if (_timeoutscriptid<0)
      delete comterp_;
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
#if __GNUG__<3
	      filebuf obuf(1);
#else
	      filebuf obuf(stdout, ios_base::out);
#endif
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
#if __GNUC__==2 && __GNUC_MINOR__<=7
    char inv[BUFSIZ];
    int inv_cnt = 0;
#else
    vector<char> inv;
#endif
    char ch;

#if __GNUG__<3
    filebuf ibuf(fd);
    istream istr(&ibuf);

    // problem handling new-lines embedded in character strings
#if __GNUC__==2 && __GNUC_MINOR__<=7
    while(istr.good() && istr.get(ch),ch!='\n'&&ch!='\0' && inv_cnt<BUFSIZ-1) 
      inv[inv_cnt++] = ch;
    inv[inv_cnt++] = '\0';
#else
    while(istr.good() && istr.get(ch),ch!='\n'&&ch!='\0') 
      inv.push_back(ch);
    inv.push_back('\0');
#endif

    boolean input_good = istr.good();
#else

    if (!_wrfptr) _wrfptr = fdopen(fd, "w");
    if (!_rdfptr) _rdfptr = fdopen(fd, "r");

    ch = '\0';
    int status;
    while (ch != '\n' && status>0) {
      status = read(fd, &ch, 1);
      if (status == 1 && ch != '\n') inv.push_back(ch);
    }
    inv.push_back('\0');
      
    boolean input_good = status != -1;

#endif

    char* inbuf = &inv[0];
    if (!comterp_ || !input_good)
      return -1;
    else if (!inbuf || !*inbuf) {
#if __GNUG__<3
      filebuf obuf(fd ? fd : 1);
      ostream ostr(&obuf);
      ostr << "\n";
      ostr.flush();
      return 0;
#else
      filebuf obuf(fd ? wrfptr() : stdout, ios_base::out);
      ostream ostr(&obuf);
      ostr << "\n";
      ostr.flush();
      return 0;
#endif
    }
    if (!ComterpHandler::logger_mode()) {
      comterp_->load_string(inbuf);
      if (fd>0) 
	cerr << "command via ACE -- " << inbuf << "\n";
      comterp_->_fd = fd;
      comterp_->_outfunc = (outfuncptr)&ComTerpServ::fd_fputs;

      int  status = comterp_->ComTerp::run(false /* !once */, false /* !nested */);
      return (input_good ? 0 : -1) && status;
    } else {
      if (inbuf[0]!='\004')
	cout << inbuf << "\n";
#if __GNUG__<3
      filebuf obuf(fd ? fd : 1);
      ostream ostr(&obuf);
      ostr << "\n";
      ostr.flush();
#else
      filebuf obuf(fd ? wrfptr() : stdout, ios_base::out);
      ostream ostr(&obuf);
      ostr << "\n";
      ostr.flush();
#endif
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

      if (COMTERP_REACTOR::instance ()->register_handler 
	  (this, ACE_Event_Handler::READ_MASK) == -1)
	ACE_ERROR_RETURN ((LM_ERROR, 
			   "(%P|%t) can't register with reactor\n"), -1);
#if defined(__NetBSD__) /* this seems to be required for NetBSD */
      else if (COMTERP_REACTOR::instance ()->schedule_timer
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

#endif /* HAVE_ACE */
