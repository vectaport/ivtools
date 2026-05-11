/*
 * Copyright (c) 1994,1995,1998,1999 Vectaport Inc.
 * Copyright (c) 2011 Wave Semiconductor Inc.
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

#include <string.h>

#include <ComTerp/socket.h>

#ifdef HAVE_ACE
#endif


#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

int SocketObj::_symid= -1;

SocketObj::SocketObj() {
  _socket = nil; 
  _conn = nil; 
  _host = nil;
  _port = 0;
}

SocketObj::SocketObj(const char* host, unsigned short port) {
  _socket = nil; 
  _conn = nil; 
  _host = strdup(host); 
  _port = port; 
}

SocketObj::~SocketObj() { 
  if( _socket ) {
    _socket->close();
    delete _socket;
    delete _conn; 
    delete _host; }
}

int SocketObj::connect() { 
  ACE_INET_Addr addr(_port, _host); 
  _socket = new ACE_SOCK_Stream;
  _conn = new ACE_SOCK_Connector; 
  return _conn->connect(*_socket, addr); 
}

int SocketObj::close() { 
  return _socket->close(); 
}

int SocketObj::get_handle() { 
  return _socket->get_handle(); 
}
#endif

