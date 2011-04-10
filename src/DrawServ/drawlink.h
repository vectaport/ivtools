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
 * DrawLink - object to encapsulate 2-way link with remote drawserv
 */
#ifndef drawlink_h
#define drawlink_h

#include <InterViews/observe.h>

class AckBackHandler;
class DrawServ;
class DrawServHandler;


#include <OS/table.h>
declareTable(IncomingSidTable,unsigned int,unsigned int)

#ifdef HAVE_ACE
#include <ComTerp/comhandler.h>
#include <ace/SOCK_Connector.h>

class ACE_INET_Addr;
class ACE_SOCK_Stream;
class ACE_SOCK_Stream;
#endif
#include <stdio.h>

//: object to encapsulate 2-way link with remote drawserv
class DrawLink : public Observable {
public:
    DrawLink(const char* hostname, int portnum, int state);
    virtual ~DrawLink();

    enum { new_link=0, one_way, two_way, redundant };
    
    static char* state_string(int state)
      { return state>=0 && state<=redundant ?  _state_strings[state] : nil; }

    const char* hostname() { return _host; }
    // return name of remote host

    void hostname(const char* host);
    // set name of remote host

    const char* althostname() { return _althost; }
    // return alternate name of remote host

    void althostname(const char* althost);
    // set alternate name of remote host

    int portnum() { return _port; }
    // return port on remote host

    int open();
    // open link to remote DrawServ

    int close();
    // close link to remote DrawServ

    int up() { state() == two_way; }
    // return 1 if link up, 0 if down

    int ok() { return _ok; }
    // return 1 if link ok (but not necessarily two-way yet), 0 if not

    int handle();
    // return file descriptor associated with link

    int local_linkid() { return _local_linkid; }
    // get local DrawLink id
    int remote_linkid() { return _remote_linkid; }
    // get remote DrawLink id

    void local_linkid(int id) { _local_linkid = id; }
    // get local DrawLink id
    void remote_linkid(int id) { _remote_linkid = id; }
    // get remote DrawLink id

    void comhandler(DrawServHandler* handler) { _comhandler = handler; }
    // set DrawServHandler associated with incoming connection

    DrawServHandler* comhandler() { return _comhandler; }
    // get DrawServHandler associated with incoming connection

    void ackhandler(AckBackHandler* handler) { _ackhandler = handler; }
    // set AckBackHandler associated with incoming connection

    AckBackHandler* ackhandler() { return _ackhandler; }
    // get AckBackHandler associated with incoming connection

    IncomingSidTable* incomingsidtable() { return _incomingsidtable; }
    // return pointer to table mapping incoming sessionid's to locally unique sessionid's

    void state(int val) { _state = val; notify(); }
    // set state of DrawLink

    int state() { return _state; }
    // get state of DrawLink

#ifdef HAVE_ACE
    ACE_SOCK_Stream* socket() { return _socket; }
    // return pointer to connected socket.
#endif

    unsigned int sid_lookup(unsigned int sid);
    // map incoming sid to local sid.

    void sid_change(unsigned int& id);
    // change sid portion of an id to use local sid.

    void sid_insert(unsigned int sid, unsigned int alt_sid);
    // insert new sid pair into table

    void dump(FILE*);
    // dump complete information on this DrawLink

    void dump_incomingsidtable(FILE*);
    // dump complete information on this DrawLink's IncomingSidTable

protected:
    const char* _host;
    const char* _althost;
    int _port;
    int _ok;
    static int _linkcnt;
    int _local_linkid;
    int _remote_linkid;
    int _state;
    IncomingSidTable* _incomingsidtable;
    int _incomingsidtable_size;

#ifdef HAVE_ACE
    ACE_INET_Addr* _addr;
    ACE_SOCK_Connector* _conn;
    ACE_SOCK_Stream* _socket;
#endif

    DrawServHandler* _comhandler;
    AckBackHandler* _ackhandler;

    static char* _state_strings[];
};

#endif
