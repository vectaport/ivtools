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
      return "exit(status) -- exit entire application"; }

};

//: timer expression command for ComTerp.
// timeexpr(comstr :sec n) -- command string to execute at intervals.
class TimeExprFunc : public ComFunc {
public:
    TimeExprFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(comstr :sec n) -- command string to execute at intervals"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":sec n     seconds between execution (default is 1)",
	nil
      };
      return keys;
    }

};

//: run command for ComTerp.
// run(filename :str) -- run commands from a file (or string).
class RunFunc : public ComFunc {
public:
    RunFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(filename|cmdstr :str :popen) -- run commands from a file (or string)"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":str       run commands from string",
	":popen     run commands from pipe command",
	nil
      };
      return keys;
    }

    static const char* expand_tilde(const char* path);
    static void set_basepath(const char* path);
    // set base path for resolving relative paths in run() calls;
    // call before runfile() when launching a script from the command line.

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
    virtual const char** dockeys() {
      static const char* keys[] = {
	":nowait    do not wait for response from other end",
	":str       return response string without evaluation",
	nil
      };
      return keys;
    }

};

//: create socket object
// sockobj=socket(hoststr portnum) -- create and open socket object
// create and open socket object
class SocketFunc : public ComFunc {
public:
    SocketFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "sockobj=%s(hoststr portnum) -- create and open socket object"; }
};

//: eval string command for ComTerp.
// str|lst=eval(cmdstr|tokbuf [cmdstr|tokbuf ...] :symret) -- evaluate string (or tokbuf) as commands, optionally returning symbol instead of nil.
class EvalFunc : public ComFunc {
public:
    EvalFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "str|lst=%s(cmdstr|funcobj [cmdstr|funcobj ...] :symret :alist attrlist) -- evaluate string (or funcobj) as commands, optionally return symbol instead of nil"; }
    virtual const char** dockeys() {
      static const char* keys[] = {
	":symret         return symbol without evalation",
	":alist attrlist use attribute list for local variables",
	nil
      };
      return keys;
    }

};

//: shell escape command for ComTerp.
// shell(cmdstr) -- evaluate command in shell.
class ShellFunc : public ComFunc {
public:
    ShellFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "status=%s(cmdstr) -- evaluate command in shell"; }

};

//: usleep sleep microseconds
// usleep(usec) -- sleep microseconds
class USleepFunc : public ComFunc {
public:
    USleepFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(usec) -- sleep microseconds"; }

};

//: update event loop for usec microseconds
// update([usec]) -- yield to ACE reactor event loop for usec microseconds
class UpdateFunc : public ComFunc {
public:
    UpdateFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "%s([usec]) -- yield to ACE reactor event loop for usec microseconds"; }

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

//: blank command for ComTerp.
// blank([...]) -- accept any arguments and return blank.
class BlankFunc : public ComFunc {
public:
    BlankFunc(ComTerp*);

    virtual boolean post_eval() { return true; }
    virtual void execute();
    virtual const char* docstring() { 
      return "%s([...]) -- accept any arguments and return blank"; }

};

//: mute command for ComTerp
// mute([flag]) -- set or toggle mute flag
class MuteFunc : public ComFunc {
public:
    MuteFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s([flag]) -- set or toggle mute flag (2 disables command echo)"; }

};

//: empty command for ComTerp.
// empty() -- empty statement
class EmptyFunc : public ComFunc {
public:
    EmptyFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "empty() -- empty statement"; }

};

#endif /* !defined(_ctrlfunc_h) */
