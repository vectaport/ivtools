/*
 * Copyright (c) 1994,1995,1998,1999 Vectaport Inc.
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

#if !defined(_ctrlfunc_h)
#define _ctrlfunc_h

#include <ComTerp/comfunc.h>

class ComTerp;

//: quit command for ComTerp.
// quit() -- quit this interpreter.
class QuitFunc : public ComFunc {
public:
    QuitFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "quit() -- quit this interpreter"; }

};

//: exit command for ComTerp.
// exit() -- exit entire application.
class ExitFunc : public ComFunc {
public:
    ExitFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "exit() -- exit entire application"; }

};

//: timer expression command for ComTerp.
// timeexpr(comstr :sec n) -- command string to execute at intervals.
class TimeExprFunc : public ComFunc {
public:
    TimeExprFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(comstr :sec n) -- command string to execute at intervals"; }

};

//: run command for ComTerp.
// run(filename) -- run commands from a file.
class RunFunc : public ComFunc {
public:
    RunFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(filename) -- run commands from a file"; }

};

//: remote execution command for ComTerp.
// remote(hoststr portnum cmdstr :nowait) -- remotely evaluate command string then locally 
// evaluate result string.
class RemoteFunc : public ComFunc {
public:
    RemoteFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(hoststr|sockobj [portnum] cmdstr :nowait) -- remotely evaluate command string then locally evaluate result string"; }

};

#ifdef HAVE_ACE
class ACE_SOCK_STREAM;
class ACE_SOCK_Connector;

class SocketObj {
 public:
  SocketObj(const char* host, unsigned short port); 
  virtual ~SocketObj();
  ACE_SOCK_STREAM* socket() { return _socket; }
  int connect();
  int close();
  const char* host() { return _host; }
  unsigned short port() { return _port; }
  int get_handle();

  ACE_SOCK_STREAM* _socket;
  ACE_SOCK_Connector* _conn;
  char* _host;
  unsigned short _port;

  CLASS_SYMID("SocketObj");
};
#endif
  
//: create socket object
// sockobj=socket(hoststr portnum) -- create and open socket object
// create and open socket object
class SocketFunc : public ComFunc {
public:
    SocketFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(hoststr portnum ) -- create and open socket object"; }
};

//: eval string command for ComTerp.
// str|lst=eval(cmdstr [cmdstr ...] :symret) -- evaluate string as commands, optionally returning symbol instead of nil.
class EvalFunc : public ComFunc {
public:
    EvalFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "str|lst=%s(cmdstr :symret) -- evaluate string as commands, optionally return symbol instead of nil"; }

};

//: shell escape command for ComTerp.
// shell(cmdstr) -- evaluate command in shell.
class ShellFunc : public ComFunc {
public:
    ShellFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(cmdstr) -- evaluate command in shell"; }

};

//: usleep sleep microseconds
// usleep(msec) -- sleep microseconds
class USleepFunc : public ComFunc {
public:
    USleepFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(msec) -- sleep microseconds"; }

};

//: nil command for ComTerp.
// nil([...]) -- accept any arguments and return nil.
class NilFunc : public ComFunc {
public:
    NilFunc(ComTerp*);

    virtual boolean post_eval() { return true; }
    virtual void execute();
    virtual const char* docstring() { 
      return "%s([...]) -- accept any arguments and return nil"; }

};

//: mute command for ComTerp
// mute([flag]) -- set or toggle mute flag
class MuteFunc : public ComFunc {
public:
    MuteFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s([flag]) -- set or toggle mute flag"; }

};

#endif /* !defined(_ctrlfunc_h) */
