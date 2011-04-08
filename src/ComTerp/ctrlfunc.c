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

#include <ComTerp/ctrlfunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#endif

#define TITLE "CtrlFunc"

/*****************************************************************************/

QuitFunc::QuitFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void QuitFunc::execute() {
    reset_stack();
    _comterp->quit();
}

ExitFunc::ExitFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ExitFunc::execute() {
    reset_stack();
    _comterp->exit();
}

SeqFunc::SeqFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SeqFunc::execute() {
    ComValue arg1(stack_arg(0));
    ComValue arg2(stack_arg(1));
    reset_stack();
    push_stack(arg2);
}

DotFunc::DotFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotFunc::execute() {
    ComValue thisval(stack_arg(0));
    ComValue methval(stack_arg(1));
    reset_stack();
    push_stack(methval);
}

TimeExprFunc::TimeExprFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TimeExprFunc::execute() {
#ifdef HAVE_ACE
    ComValue timeoutstr(stack_arg(0));
    reset_stack();

    ComterpHandler* handler = ((ComTerpServ*)_comterp)->handler();
    if (handler) {
        if (nargs()) {
	  if (timeoutstr.type() == ComValue::StringType) {
	      handler->timeoutscriptid(timeoutstr.string_val());
	      push_stack(timeoutstr);
	  } else 
	    push_stack(ComValue::nullval());
	} else {
	    ComValue retval(handler->timeoutscriptid(), ComValue::StringType);
	    push_stack(retval);
	}
	    
	    
    }
#endif
}

RunFunc::RunFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void RunFunc::execute() {
    ComValue runfilename(stack_arg(0));
    reset_stack();

    if (runfilename.type() == ComValue::StringType)
        _comterp->runfile(runfilename.string_ptr());
    return;
}

RemoteFunc::RemoteFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void RemoteFunc::execute() {
  ComValue hostv(stack_arg(0, true));
  ComValue portv(stack_arg(1));
  ComValue cmdstrv(stack_arg(2));
  reset_stack();

#ifdef HAVE_ACE
  if (hostv.is_string() && portv.is_known() && cmdstrv.is_string()) {

    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    ACE_INET_Addr addr (portnum, hoststr);
    ACE_SOCK_Stream socket;
    ACE_SOCK_Connector conn;
    if (conn.connect (socket, addr) == -1) {
      ACE_ERROR ((LM_ERROR, "%p\n", "open"));
      push_stack(ComValue::nullval());
      return;
    }

    filebuf ofbuf;
    ofbuf.attach(socket.get_handle());
    ostream out(&ofbuf);
    const char* cmdstr = cmdstrv.string_ptr();
    out << cmdstr;
    if (cmdstr[strlen(cmdstr)-1] != '\n') out << "\n";
    out.flush();
    filebuf ifbuf;
    ifbuf.attach(socket.get_handle());
    istream in(&ifbuf);
    char* buf;
    in.gets(&buf);
    ComValue& retval = comterpserv()->run(buf, true);
    push_stack(retval);

    if (socket.close () == -1)
        ACE_ERROR ((LM_ERROR, "%p\n", "close"));
  } 
    
  return;

#else

  reset_stack();
  cerr << "for the remote command to work rebuild comterp with ACE\n";
  return;

#endif

}

ShellFunc::ShellFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ShellFunc::execute() {
    ComValue shellcmdstr(stack_arg(0));
    reset_stack();

    ComValue retval;
    if (shellcmdstr.type() == ComValue::StringType) {
        retval.int_ref() = system(shellcmdstr.string_ptr());
	retval.type(ComValue::IntType);
    }
    push_stack(retval);
    return;
}




