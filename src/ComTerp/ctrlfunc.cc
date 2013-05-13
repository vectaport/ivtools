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

#include <fstream.h>
#include <iostream>
#include <ComTerp/comhandler.h>

#include <ComTerp/ctrlfunc.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/postfunc.h>
#include <Attribute/attrlist.h>

#include <wordexp.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#endif


#define TITLE "CtrlFunc"

#if __GNUC__>=3
static char newline;
#if __GNUC__>3||__GNUC_MINOR_>1
#include <ext/stdio_filebuf.h>
#endif
#endif

using std::cerr;

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
    ComValue statusv(stack_arg(0));
    reset_stack();
    if(statusv.is_int())
        _comterp->exit(statusv.int_val());
    else
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
    static int str_sym = symbol_add("str");
    ComValue strv(stack_key(str_sym));
    static int popen_sym = symbol_add("popen");
    ComValue popenv(stack_key(popen_sym));
    reset_stack();

    if (runfilename.type() == ComValue::StringType) {
      const char* path = expand_tilde(runfilename.string_ptr());
      if (strv.is_true()) {
	ComValue retval (((ComTerpServ*)_comterp)->run(path, true /* nested */));
	push_stack(retval);
      }
      else {
        static int depth = 0;
	static char*  curr_basepath = NULL;
        char* prev_basepath = NULL;
        char runpath[BUFSIZ];
        int bufleft = BUFSIZ-1;
        *runpath = '\0'; 
        if(curr_basepath && *path=='.') 
          strncpy(runpath, curr_basepath, BUFSIZ-1);
        _comterp->set_args(path);
        const char* oldptr = path;
        char* newptr = runpath+strlen(runpath);
        bufleft -+ strlen(runpath);
        while(!isspace(*oldptr) && *oldptr && --bufleft)
            *newptr++ = *oldptr++;
        *newptr = '\0';
        prev_basepath = curr_basepath;
        curr_basepath = new char[BUFSIZ];
        realpath(runpath, curr_basepath);
        char* ptr = curr_basepath+strlen(curr_basepath)-1;
        while(ptr > curr_basepath && *ptr != '/') *ptr--='\0';
        
#if 0
        fprintf(stderr, "READY(%d) prev_basepath %s\n", depth, prev_basepath);
        fprintf(stderr, "READY(%d) curr_basepath %s\n", depth, curr_basepath);
        fprintf(stderr, "READY(%d) runpath %s\n", depth, runpath);
#endif
        depth++;
	_comterp->runfile(runpath, popenv.is_true());
        if(_comterp->quitflag()) _comterp->quitflag(0);
        depth--;

        delete curr_basepath;
        curr_basepath = prev_basepath;
#if 0
        fprintf(stderr, "DONE(%d) curr_basepath %s\n", depth, curr_basepath);
#endif
      }
    }
    return;
}

const char* RunFunc::expand_tilde(const char* path) {
  static wordexp_t p;
  wordfree(&p);
  if(wordexp(path, &p, 0)!=0)
    return path;
  return p.we_wordv[0];
}

/*****************************************************************************/

RemoteFunc::RemoteFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void RemoteFunc::execute() {
  ComValue arg1v(stack_arg(0));
  ComValue arg2v(stack_arg(1));
  ComValue arg3v(stack_arg(2));
  static int nowait_sym = symbol_add("nowait");
  ComValue nowaitv(stack_key(nowait_sym));
  reset_stack();


#ifdef HAVE_ACE

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  ACE_SOCK_STREAM *socket = nil;
  ACE_SOCK_Connector *conn = nil;
  SocketObj* socketobj = nil;
  char* cmdstr = nil;
  if (arg1v.is_string() && arg2v.is_num() && arg3v.is_string()) {

    cmdstr = (char*)arg3v.string_ptr();
    const char* hoststr = arg1v.string_ptr();
    const char* portstr = arg2v.is_string() ? arg2v.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : arg2v.ushort_val();
    ACE_INET_Addr addr (portnum, hoststr);
    socket = new ACE_SOCK_STREAM;
    conn = new ACE_SOCK_Connector;
    if (conn->connect (*socket, addr) == -1) {
      ACE_ERROR ((LM_ERROR, "%p\n", "open"));
      push_stack(ComValue::nullval());
      return;
    }

  } else if (arg1v.is_object() && arg2v.is_string()) {
    
    cmdstr = (char*)arg2v.string_ptr();
    socketobj = (SocketObj*)arg1v.geta(SocketObj::class_symid());
    if (socketobj) 
      socket = socketobj->socket();
    
  } else
    return;
  
#if 0
#if __GNUC__<3
  filebuf ofbuf;
  ofbuf.attach(socket->get_handle());
#elif __GNUC__<4 && !defined(__CYGWIN__)
  fileptr_filebuf ofbuf((int)socket->get_handle(), ios_base::out,
			false, static_cast<size_t>(BUFSIZ));
#else
  fileptr_filebuf ofbuf((int)socket->get_handle(), ios_base::out,
			static_cast<size_t>(BUFSIZ));
#endif
  ostream out(&ofbuf);
  out << cmdstr;
  if (cmdstr[strlen(cmdstr)-1] != '\n') out << "\n";
  out.flush();
#else
#if 0
  int i=0;
  do {
    if(write(socket->get_handle(), cmdstr+i++, 1)!=1)
      fprintf(stderr, "Unexpected error writing byte to socket\n");
  } while ( cmdstr[i]!='\0');
  if (cmdstr[i-1] != '\n')  
    write(socket->get_handle(), "\n", 1);
#else
  int cmdlen = strlen(cmdstr);
  int newline_flag = cmdstr[cmdlen-1]=='\n';
  if (!newline_flag) cmdstr[cmdlen]='\n';
  int nbytes = write(socket->get_handle(), cmdstr, cmdlen+(newline_flag?0:1));
  if (nbytes != cmdlen+(newline_flag?0:1))
      fprintf(stderr, "write to socket failed\n");
  if (!newline_flag) cmdstr[cmdlen]='\0';
#endif
#endif
  if (nowaitv.is_false()) {
#if __GNUC__<3
    filebuf ifbuf;
    ifbuf.attach(socket->get_handle());
    istream in(&ifbuf);
    char* buf;
    in.gets(&buf);
#else
    char buf[BUFSIZ];
    int i=0;
    do {
      read(socket->get_handle(), buf+i++, 1);
    } while (i<BUFSIZ-1 && buf[i-1]!='\n');
    if (buf[i-1]=='\n') buf[i]=0;
    // fprintf(stderr, "buf read back from remote %s", buf);
#endif
    ComValue retval(comterpserv()->run(buf, true));
    push_stack(retval);
  }
  
  if(!socketobj) {
    if (socket->close () == -1)
      ACE_ERROR ((LM_ERROR, "%p\n", "close"));
    delete socket;
    delete conn;
  }

  return;

#else

  cerr << "for the remote command to work rebuild comterp with ACE\n";
  return;

#endif

}

/*****************************************************************************/
#ifdef HAVE_ACE
int SocketObj::_symid = -1;

SocketObj::SocketObj(const char* host, unsigned short port) {
  _socket = nil; 
  _conn = nil; 
  _host = strnew(host); 
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
  _socket = new ACE_SOCK_STREAM;
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

/*****************************************************************************/

SocketFunc::SocketFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void SocketFunc::execute() {
  ComValue hostv(stack_arg(0));
  ComValue portv(stack_arg(1));
  reset_stack();

#ifdef HAVE_ACE

#if __GNUC__==3&&__GNUC_MINOR__<1
  fprintf(stderr, "Please upgrade to gcc-3.1 or greater\n");
  push_stack(ComValue::nullval());
  return;
#endif

  if (hostv.is_string() && portv.is_known()) {

    const char* hoststr = hostv.string_ptr();
    const char* portstr = portv.is_string() ? portv.string_ptr() : nil;
    u_short portnum = portstr ? atoi(portstr) : portv.ushort_val();
    SocketObj* socket = new SocketObj(hoststr, portnum);
    if (socket->connect() == -1 ) {
      ACE_ERROR ((LM_ERROR, "%p\n", "open"));
      push_stack(ComValue::nullval());
      return;
    }

    ComValue retval(SocketObj::class_symid(), (void*)socket);
    push_stack(retval);
  }

  return;

#else
  cerr << "for the socket command to work rebuild comterp with ACE\n";
  return;

#endif

}

/*****************************************************************************/

EvalFunc::EvalFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EvalFunc::execute() {
  static int symret_sym = symbol_add("symret");
  ComValue symretv(stack_key(symret_sym));
  static int alist_sym = symbol_add("alist");
  ComValue alistv(stack_key(alist_sym));

  if (!comterp()->is_serv()) {
    cerr << "need server mode comterp (or remote mode) for eval command\n";
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  AttributeList* alist = 
      (AttributeList*) alistv.geta(AttributeList::class_symid());
  AttributeList* old_alist = NULL;
  if (alist) {
      old_alist = comterp()->get_attributes();
      comterp()->set_attributes(alist);
  }

  // evaluate every string fixed argument on the stack and return in array
  int numargs = nargsfixed();
  if (numargs>1) {
    AttributeValueList* avl = nil;
    for (int i=0; i<numargs; i++) {
      ComValue argv (stack_arg(i));
      if (argv.is_nil()) break;
      ComValue* val = NULL;
      if (argv.is_string()) {
	val = new ComValue(comterpserv()->run(argv.symbol_ptr(), true /* nested */));
      } else if (argv.is_object(FuncObj::class_symid())) {
        FuncObj* tokbuf = (FuncObj*)argv.obj_val();
        val = new ComValue(comterpserv()->run(tokbuf->toks(), tokbuf->ntoks()));
      }
      if (val->is_nil() && symretv.is_true()) {
	delete val;
	val = new ComValue(argv.symbol_val(), AttributeValue::SymbolType);
      }
      if (!avl) avl = new AttributeValueList();
      avl->Append(val);
    }
    reset_stack();
    if (avl) {
      ComValue retval(avl);
      push_stack(retval);
    }

  }
  /* unless only single argument */
  else if (numargs==1) {

    ComValue argv (stack_arg(0));
    reset_stack();
    if (argv.is_nil()) {
      push_stack(ComValue::nullval());
    } else if (argv.is_string()) {
      ComValue val(comterpserv()->run(argv.symbol_ptr(), true /* nested */));
      if (val.is_nil() && symretv.is_true()) {
	val.assignval(ComValue(argv.symbol_val(), AttributeValue::SymbolType));
      }
      comterp()->push_stack(val);
      
    } else if (argv.is_object(FuncObj::class_symid())) {
      FuncObj* tokbuf = (FuncObj*)argv.obj_val();
      ComValue val(comterpserv()->run(tokbuf->toks(), tokbuf->ntoks()));
      if (val.is_nil() && symretv.is_true()) {
	val.assignval(ComValue(argv.symbol_val(), AttributeValue::SymbolType));
      }
      comterp()->push_stack(val);
    }
  } else
    reset_stack();
  
  if (old_alist)
    comterp()->set_attributes(old_alist);
  return;
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

USleepFunc::USleepFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void USleepFunc::execute() {
    ComValue msecv(stack_arg(0));
    reset_stack();

    if (msecv.int_val()>0) 
    usleep(msecv.int_val());
    push_stack(msecv);
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

/*****************************************************************************/

MuteFunc::MuteFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void MuteFunc::execute() {
    ComValue mutev(stack_arg(0));
    reset_stack();

    if (mutev.is_unknown())
      comterp()->muted(!comterp()->muted());
    else
      comterp()->muted(mutev.int_val());
    ComValue retval(comterp()->muted());
    push_stack(retval);
    return;
}

/*****************************************************************************/

EmptyFunc::EmptyFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void EmptyFunc::execute() {
    reset_stack();
    // fprintf(stderr, "*** empty statement ***\n");
}

