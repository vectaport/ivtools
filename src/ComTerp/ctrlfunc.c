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

#include <ComTerp/ctrlfunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>
#include <Attribute/attrlist.h>
#include <fstream.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#endif

#define TITLE "CtrlFunc"

#if __GNUG__>=3
static char newline;
#endif

/*****************************************************************************/

QuitFunc::QuitFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void QuitFunc::execute() {
    reset_stack();
    _comterp->quit();
}

/*****************************************************************************/

ExitFunc::ExitFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ExitFunc::execute() {
    reset_stack();
    _comterp->exit();
}

/*****************************************************************************/

TimeExprFunc::TimeExprFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TimeExprFunc::execute() {
#ifdef HAVE_ACE
    ComValue timeoutstr(stack_arg(0));
    static int sec_symval = symbol_add("sec");
    ComValue sec_val(stack_key(sec_symval, false, ComValue::oneval(), true));
    reset_stack();

    ComterpHandler* handler = ((ComTerpServ*)_comterp)->handler();
    if (handler) {
        if (nargs()) {
	  if (timeoutstr.type() == ComValue::StringType) {
	      handler->timeoutseconds(sec_val.int_val());
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

/*****************************************************************************/

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
  static int nowait_sym = symbol_add("nowait");
  ComValue nowaitv(stack_key(nowait_sym));
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

#if __GNUG__<3
    filebuf ofbuf;
    ofbuf.attach(socket.get_handle());
#else
    fileptr_filebuf ofbuf(comterp()->handler() && comterp()->handler()->wrfptr() 
		  ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
#endif
    ostream out(&ofbuf);
    const char* cmdstr = cmdstrv.string_ptr();
    out << cmdstr;
    if (cmdstr[strlen(cmdstr)-1] != '\n') out << "\n";
    out.flush();
    if (nowaitv.is_false()) {
#if __GNUG__<3
      filebuf ifbuf;
      ifbuf.attach(socket.get_handle());
      istream in(&ifbuf);
      char* buf;
      in.gets(&buf);
#else
      fileptr_filebuf ifbuf(comterp()->handler()->rdfptr(), ios_base::in);
      istream in(&ifbuf);
      char buf[BUFSIZ];
      in.get(buf, BUFSIZ);
      in.get(newline);
#endif
      ComValue& retval = comterpserv()->run(buf, true);
      push_stack(retval);
    }

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

/*****************************************************************************/

EvalFunc::EvalFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EvalFunc::execute() {
  static int symret_sym = symbol_add("symret");
  ComValue symretv(stack_key(symret_sym));

  if (!comterp()->is_serv()) {
    cerr << "need server mode comterp (or remote mode) for eval command\n";
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  // evaluate every string fixed argument on the stack and return in array
  int numargs = nargsfixed();
  if (numargs>1) {
    AttributeValueList* avl = nil;
    for (int i=0; i<numargs; i++) {
      ComValue argstrv (stack_arg(i));
      if (argstrv.is_nil()) break;
      if (argstrv.is_string()) {
	ComValue* val = new ComValue(comterpserv()->run(argstrv.symbol_ptr(), true /* nested */));
	if (val->is_nil() && symretv.is_true()) {
	  delete val;
	  val = new ComValue(argstrv.symbol_val(), AttributeValue::SymbolType);
	}
	if (!avl) avl = new AttributeValueList();
	avl->Append(val);
      }
    }
    reset_stack();
    if (avl) {
      ComValue retval(avl);
      push_stack(retval);
    }

  }
  /* unless only single argument */
  else if (numargs==1) {

    ComValue argstrv (stack_arg(0));
    reset_stack();
    if (argstrv.is_nil()) {
      push_stack(ComValue::nullval());
    } else if (argstrv.is_string()) {
	ComValue val(comterpserv()->run(argstrv.symbol_ptr(), true /* nested */));
	if (val.is_nil() && symretv.is_true()) {
	  val.assignval(ComValue(argstrv.symbol_val(), AttributeValue::SymbolType));
	}
	push_stack(val);
	
    }
  } else
    reset_stack();
}

/*****************************************************************************/

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

/*****************************************************************************/

NilFunc::NilFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NilFunc::execute() {
    reset_stack();
    static int nil_symid = symbol_add("nil");
    int comm_symid = funcstate()->command_symid();
    if (comm_symid && comm_symid!= nil_symid)
      cerr << "unknown command \"" << symbol_pntr(comm_symid)
	<< "\" returned nil\n";
    push_stack(ComValue::nullval());
}




