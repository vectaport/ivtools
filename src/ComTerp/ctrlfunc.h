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

//: ; (sequence) operator.
class SeqFunc : public ComFunc {
public:
    SeqFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "; is the sequencing operator"; }

};

//: . (dot) operator.
class DotFunc : public ComFunc {
public:
    DotFunc(ComTerp*);

    virtual void execute();

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
      return "%s(hoststr portnum cmdstr :nowait) -- remotely evaluate command string then locally evaluate result string"; }

};

//: eval string command for ComTerp.
// eval(cmdstr [cmdstr ...] :symret) -- evaluate string as commands, optionally returning symbol instead of nil.
class EvalFunc : public ComFunc {
public:
    EvalFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(cmdstr) -- evaluate string as commands, optionally return symbol instead of nil"; }

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

//: nil command for ComTerp.
// nil([...]) -- accept any arguments and return nil.
class NilFunc : public ComFunc {
public:
    NilFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s([...]) -- accept any arguments and return nil"; }

};

#endif /* !defined(_ctrlfunc_h) */
