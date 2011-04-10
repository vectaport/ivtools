/*
 * Copyright (c) 1994,1995,1998 Vectaport Inc.
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

class QuitFunc : public ComFunc {
public:
    QuitFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "quit() -- quit this interpreter"; }

};

class ExitFunc : public ComFunc {
public:
    ExitFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "exit() -- exit entire application"; }

};

class SeqFunc : public ComFunc {
public:
    SeqFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "; is the sequencing operator"; }

};

class DotFunc : public ComFunc {
public:
    DotFunc(ComTerp*);

    virtual void execute();

};

class TimeExprFunc : public ComFunc {
public:
    TimeExprFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(comstr) -- command string to execute at intervals"; }

};

class RunFunc : public ComFunc {
public:
    RunFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(filename) -- run commands from a file"; }

};

class RemoteFunc : public ComFunc {
public:
    RemoteFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(hoststr portnum cmdstr) -- remotely evaluate command string then locally evaluate result string"; }

};

class ShellFunc : public ComFunc {
public:
    ShellFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() { 
      return "%s(cmdstr) -- evaluate command in shell"; }

};

#endif /* !defined(_ctrlfunc_h) */
