/*
 * Copyright (c) 2004 Scott E. Johnston
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

/*
 * Implementation of Drawlink class.
 */

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <DrawServ/ackback-handler.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/sid.h>
#include <Unidraw/globals.h>
#include <fstream.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

using std::cout;
using std::cerr;

int DrawLink::_linkcnt = 0;

const char* DrawLink::_state_strings[] =  { "new_link", "one_way", "two_way", "redundant" };

/*****************************************************************************/

DrawLink::DrawLink (const char* hostname, int portnum, int state)
{
  _host = strnew(hostname);
  _althost = nil;
  _port = portnum;
  _ok = false;
  uuid_clear(_linkid);
  _state = state;

#ifdef HAVE_ACE
  _addr = nil;
  _socket = nil;
  _conn = nil;
#endif

  _comhandler = nil;
  _ackhandler = nil;
}

DrawLink::~DrawLink () 
{
#ifdef HAVE_ACE
    if (_socket->close () == -1)
        ACE_ERROR ((LM_ERROR, "%p\n", "close"));
    delete _conn;
    delete _socket;
    delete _addr;
    delete _host;
    delete _althost;
#endif
}

int DrawLink::open(uuid_t linkid) {

#if defined(HAVE_ACE) && (__GNUC__>3 || __GNUC__==3 && __GNUC_MINOR__>0)
  _addr = new ACE_INET_Addr(_port, _host);
  _socket = new ACE_SOCK_Stream;
  _conn = new ACE_SOCK_Connector;
  if (_conn->connect (*_socket, *_addr) == -1) {
    ACE_ERROR ((LM_ERROR, "%p\n", "open"));
    return -1;
  } else {
    _socket->enable(ACE_NONBLOCK);

    /* set up handler to monitor backtalk */
    ackhandler(new AckBackHandler);
    ackhandler()->drawlink(this);
    ackhandler()->set_handle(_socket->get_handle());
    if (ComterpHandler::reactor_singleton()->register_handler(ackhandler(), ACE_Event_Handler::READ_MASK|ACE_Event_Handler::TIMER_MASK)==-1)
      fprintf(stderr, "drawserv: error registering ackback handler (handle==%d)\n", _socket->get_handle());

    FILEBUF(obuf, fdopen(dup(_socket->get_handle()), "w"), ios_base::out);
    ostream out(&obuf);
    std::ostringstream sbuf;
    sbuf << "drawlink(\"";
    char buffer[HOST_NAME_MAX];
    gethostname(buffer, HOST_NAME_MAX);
    
    uuid_t& sid = ((DrawServ*)unidraw)->sessionid();
    uuid_string_t sid_str;
    uuid_unparse(sid, sid_str);
    
    uuid_string_t linkid_str;
    uuid_unparse(linkid, linkid_str);
    
    void* ptr = nil;
    ((DrawServ*)unidraw)->sessionidtable()->find(ptr, uuid_key(sid));
    SessionId* sessionid = (SessionId*)ptr;
    sbuf << buffer << "\"";
    sbuf << " :port " << ((DrawServ*)unidraw)->comdraw_port();
    sbuf << " :state " << _state+1;
    sbuf << " :sid " << "\"" << sid_str << "\"";
    sbuf << " :linkid " << "\"" << linkid_str << "\"";
    if (sessionid) {
      sbuf << " :pid " << sessionid->pid();
      sbuf << " :user \"" << sessionid->username() << "\"";
    }
    sbuf << ")";
    log_outgoing_command(sbuf.str().c_str());
    sbuf << "\n";
    out << sbuf.str().c_str();
    out.flush();
    _ok = true;

    /* schedule timer on ackback link, to detect timeout */
    ackhandler()->start_timer();

    return 0;
  }
#else
  fprintf(stderr, "drawserv requires ACE and >= gcc-3.1 for full functionality\n");
  return -1;
#endif
}

int DrawLink::close() {
#ifdef HAVE_ACE
  fprintf(stderr, "Closing link to %s (%s) port # %d (lid=%.8s)\n", 
	  hostname(), althostname(), portnum(), linkid_str());
  if (comhandler()) comhandler()->drawlink(nil);
  if (_socket) {
    if (ackhandler()) {
      if (ackhandler()->get_handle() !=-1)
	if (ComterpHandler::reactor_singleton()->remove_handler(ackhandler(), ACE_Event_Handler::READ_MASK|ACE_Event_Handler::TIMER_MASK)==-1)
	  cerr << "drawserv: error removing ackback handler\n";
      delete ackhandler();
      ackhandler(nil);
    }
    if (_socket->close () == -1)
      ACE_ERROR ((LM_ERROR, "%p\n", "close"));
  }
#endif
  return 1;
}

void DrawLink::hostname(const char* host) {
  delete _host;
  _host = nil;
  if (host) _host = strnew(host);
  notify();
}

void DrawLink::althostname(const char* althost) {
  delete _althost;
  _althost = nil;
  if (althost) _althost = strnew(althost);
  notify();
}

int DrawLink::handle() {
#ifdef HAVE_ACE
  if (_socket) 
    return _socket->get_handle();
  else 
#endif
    return -1;
}

void DrawLink::dump(FILE* fptr) {
  fprintf(fptr, "Host                            Alt.                            Port    LID       State\n");
  fprintf(fptr, "------------------------------  ------------------------------  ------  --------  -----\n");
  fprintf(fptr, "%-30.30s  %-30.30s  %-6d  %.8s  %-3d\n", 
	  hostname(), althostname(), portnum(),
	  linkid_str(), state());
}

const char* DrawLink::linkid_str() {
  if (!uuid_is_null(_linkid)) {
    if (_linkid_str[0] == '\0') {
      uuid_unparse(_linkid, _linkid_str);
    }
    return _linkid_str;
  } else
    return "";
}

void DrawLink::log_outgoing_command(const char* cmdstring) {
  log_command(cmdstring, ">");
}

void DrawLink::log_incoming_command(const char* cmdstring) {
  log_command(cmdstring, "<");
}

void DrawLink::log_command(const char* cmdstring, const char* port_prefix) {
  if (_comhandler != NULL) {
    _comhandler->log_command(cmdstring, port_prefix, portnum());
  }
}

