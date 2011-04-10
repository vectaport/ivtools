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

#include <DrawServ/ackback-handler.h>
#include <DrawServ/drawlink.h>
#include <DrawServ/drawserv.h>
#include <DrawServ/drawserv-handler.h>
#include <DrawServ/sid.h>
#include <Unidraw/globals.h>
#include <fstream.h>
#include <unistd.h>

int DrawLink::_linkcnt = 0;

implementTable(IncomingSidTable,unsigned int,unsigned int)

char* DrawLink::_state_strings[] =  { "new_link", "one_way", "two_way", "redundant" };

/*****************************************************************************/

DrawLink::DrawLink (const char* hostname, int portnum, int state)
{
  _host = strnew(hostname);
  _althost = nil;
  _port = portnum;
  _ok = false;
  _local_linkid = _linkcnt++;
  _remote_linkid = -1;
  _state = state;

  _addr = nil;
  _socket = nil;
  _conn = nil;

  _comhandler = nil;
  _ackhandler = nil;
  _incomingsidtable = new IncomingSidTable(32);
  _incomingsidtable_size = 0;
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
    delete _incomingsidtable;
}

int DrawLink::open() {

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

#if __GNUC__<4
    fileptr_filebuf obuf(_socket->get_handle(), ios_base::out, false, static_cast<size_t>(BUFSIZ));
#else
    fileptr_filebuf obuf(_socket->get_handle(), ios_base::out, static_cast<size_t>(BUFSIZ));
#endif
    ostream out(&obuf);
    out << "drawlink(\"";
    char buffer[HOST_NAME_MAX];
    gethostname(buffer, HOST_NAME_MAX);
    unsigned int sid = ((DrawServ*)unidraw)->sessionid();
    void* ptr = nil;
    ((DrawServ*)unidraw)->sessionidtable()->find(ptr, sid);
    SessionId* sessionid = (SessionId*)ptr;
    out << buffer << "\"";
    out << " :port " << ((DrawServ*)unidraw)->comdraw_port();
    out << " :state " << _state+1;
    out << " :rid " << _local_linkid;
    out << " :lid " << _remote_linkid;
    out << " :sid 0x" << std::hex << sid << std::dec;
    if (sessionid) {
      out << " :pid " << sessionid->pid();
      out << " :user \"" << sessionid->username() << "\"";
    }
    out << ")\n";
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
  fprintf(stderr, "Closing link to %s (%s) port # %d (lid=%d, rid=%d)\n", 
	  hostname(), althostname(), portnum(), local_linkid(), remote_linkid());
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
  if (_socket) return _socket->get_handle();
  else return -1;
}

unsigned DrawLink::sid_lookup(unsigned int sid) {
  unsigned int local_sid = 0;
  if (_incomingsidtable_size==0) return sid;
  incomingsidtable()->find(local_sid, sid);
  return local_sid ? local_sid : sid;
}

void DrawLink::sid_change(unsigned int& id) {
  if (_incomingsidtable_size==0) return;
  id = sid_lookup(id & DrawServ::SessionIdMask) | (id & DrawServ::GraphicIdMask);
  return;
}

void DrawLink::sid_insert(unsigned int sid, unsigned int alt_sid) {
  if (sid!=alt_sid) {
    _incomingsidtable_size++;
    _incomingsidtable->insert(sid, alt_sid);
  }
}

void DrawLink::dump(FILE* fptr) {
  fprintf(fptr, "Host                            Alt.                            Port    LID  RID  State\n");
  fprintf(fptr, "------------------------------  ------------------------------  ------  ---  ---  -----\n");
  fprintf(fptr, "%-30.30s  %-30.30s  %-6d  %-3d  %-3d  %-3d\n", 
	  hostname(), althostname(), portnum(),
	  local_linkid(), remote_linkid(), state());
  dump_incomingsidtable(fptr);
}

void DrawLink::dump_incomingsidtable(FILE* fptr) {
  IncomingSidTable* table = incomingsidtable();
  IncomingSidTable_Iterator it(*table);
  printf("sid         osid\n");
  printf("----------  ----------\n");
  while(it.more()) {
    unsigned int osid = (unsigned int)it.cur_value();
    unsigned int sid = (unsigned int)it.cur_key();
    fprintf(fptr, "0x%08x  0x%08x\n", sid, osid);
    it.next();
  }
}
