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
 * SessionId - object to encapsulate session member info
 */
#ifndef sid_h
#define sid_h

#include <Unidraw/globals.h>

//: object to encapsulate session member info
class SessionId {
public:
  SessionId(unsigned int sid, unsigned int osid, int pid, 
	    const char* username, const char* hostname, 
	    int hostid, DrawLink* link = nil);
  virtual ~SessionId();
  
  unsigned int sid() { return _sid; }
  // return session id
  unsigned int osid() { return _osid; }
  // return original session id
  unsigned int pid() { return _pid; }
  // return associated process id
  const char* username() { return _username; }
  // return associated user name
  const char* hostname() { return _hostname; }
  // return associated host name
  int hostid() { return _hostid; }
  // return associated host id.
  DrawLink* drawlink() { return _drawlink; }
  // return associated DrawLink
  void drawlink(DrawLink* link) {_drawlink =  link; }
  // set associated DrawLink
protected:
  unsigned int _sid;
  unsigned int _osid;
  int _pid;
  char* _username;
  char* _hostname;
  int _hostid;
  DrawLink* _drawlink;
};

#endif
