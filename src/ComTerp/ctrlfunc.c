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

#include <unistd.h>
#include <cstdio>
#include <fstream.h>
#include <iostream>
#include <ComTerp/comhandler.h>

#include <ComTerp/ctrlfunc.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/postfunc.h>
#include <ComTerp/socket.h>
#include <Attribute/attrlist.h>
#include <ComUtil/util.h>
#include <patch.h>

#include <wordexp.h>

#ifdef HAVE_ACE
#include <ace/SOCK_Connector.h>
#endif


#define TITLE "CtrlFunc"

static char newline;
#ifndef __APPLE__
#include <ext/stdio_filebuf.h>
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
    ComValue sec_val(ComValue::oneval());
    ComValue sec_keyv(stack_key(sec_symval));
    if (sec_keyv.is_known()) sec_val = sec_keyv;
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

static int _run_depth = 0;
static char* _run_curr_basepath = NULL;

void RunFunc::set_basepath(const char* path) {
    char abspath[BUFSIZ];
    if (!realpath(path, abspath)) {
        strncpy(abspath, path, BUFSIZ-1);
        abspath[BUFSIZ-1] = '\0';
    }
    char* ptr = abspath + strlen(abspath) - 1;
    while (ptr > abspath && *ptr != '/') *ptr-- = '\0';
    delete[] _run_curr_basepath;
    _run_curr_basepath = new char[strlen(abspath)+1];
    strcpy(_run_curr_basepath, abspath);
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
        int& depth = _run_depth;
	char*& curr_basepath = _run_curr_basepath;
        char* prev_basepath = NULL;
        char runpath[BUFSIZ];
        int bufleft = BUFSIZ-1;
        *runpath = '\0'; 
        if(curr_basepath && *path=='.') 
          strncpy(runpath, curr_basepath, BUFSIZ-1);
        _comterp->set_args(path);
        const char* oldptr = path;
        char* newptr = runpath+strlen(runpath);
        bufleft -= strlen(runpath);
        while(!isspace(*oldptr) && *oldptr && --bufleft)
            *newptr++ = *oldptr++;
        *newptr = '\0';
        prev_basepath = curr_basepath;
        curr_basepath = new char[BUFSIZ];
        if (realpath(runpath, curr_basepath) == 0) {   // unresolved (missing
            strncpy(curr_basepath, runpath, BUFSIZ-1);  // file, perms): fall
            curr_basepath[BUFSIZ-1] = '\0';             // back to the raw path
        }
        char* ptr = curr_basepath+strlen(curr_basepath)-1;
        while(ptr > curr_basepath && *ptr != '/') *ptr--='\0';
        
#if 0
        fprintf(stderr, "READY(%d) prev_basepath %s\n", depth, prev_basepath);
        fprintf(stderr, "READY(%d) curr_basepath %s\n", depth, curr_basepath);
        fprintf(stderr, "READY(%d) runpath %s\n", depth, runpath);
#endif
        depth++;
	if( access( runpath, F_OK ) != -1 ) {
	  _comterp->runfile(runpath, popenv.is_true());
	} else {
	  _comterp->runfile(runfilename.string_ptr(), popenv.is_true());
	}
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
  static int str_sym = symbol_add("str");
  ComValue strv(stack_key(str_sym));
  reset_stack();

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
  
  int cmdlen = strlen(cmdstr);
  int newline_flag = cmdstr[cmdlen-1]=='\n';
  if (!newline_flag) cmdstr[cmdlen]='\n';
  int nbytes = write(socket->get_handle(), cmdstr, cmdlen+(newline_flag?0:1));
  if (nbytes != cmdlen+(newline_flag?0:1))
      fprintf(stderr, "write to socket failed\n");
  if (!newline_flag) cmdstr[cmdlen]='\0';
  if (nowaitv.is_false()) {
    char buf[BUFSIZ];
    int i=0;
    do {
      if (read(socket->get_handle(), buf+i, 1) <= 0) break;  // peer closed/error:
      i++;                                                    // stop, don't spin
    } while (i<BUFSIZ-1 && buf[i-1]!='\n');
    buf[i]=0;   // NUL-terminate whatever arrived (a partial reply on early EOF)
    // fprintf(stderr, "buf read back from remote %s", buf);
    if (strv.is_false()) {
      ComValue retval(comterpserv()->run(buf, true));
      push_stack(retval);
    } else {
      ComValue retval(buf);
      push_stack(retval);
    }
  }
  
  if(!socketobj) {
    if (socket->close () == -1)
      ACE_ERROR ((LM_ERROR, "%p\n", "close"));
    delete socket;
    delete conn;
  }

  return;

}

/*****************************************************************************/

SocketFunc::SocketFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void SocketFunc::execute() {
  ComValue hostv(stack_arg(0));
  ComValue portv(stack_arg(1));
  reset_stack();

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
      Resource::ref(old_alist);
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
      if (!val || val->is_nil() && symretv.is_true()) {
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
  
  comterp()->set_attributes(old_alist);
  Unref(old_alist);

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

PatchKeyFunc::PatchKeyFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PatchKeyFunc::execute() {
    static int commitid_sym = symbol_add("commitid");
    ComValue commitidv(stack_key(commitid_sym));
    reset_stack();

    if (commitidv.is_true()) {
	/* PATCH_KEY is a plain committed literal (patch.h), not derived from
	   git, so resolving it to a commit means asking git to look up the
	   matching tag pushed at merge time (.github/workflows/patch-key.yml).
	   `rev-list -n 1` dereferences either tag kind to the commit it names.

	   shell_string() (ComUtil/util.c) called directly, not through
	   ShellFunc's stack/exec machinery -- same guard-the-empty-result
	   discipline local_hostname() uses for a failed/empty scutil call:
	   a nonexistent tag leaves stdout empty, so an empty result means
	   "unresolved" (not-yet-tagged, or a stale/mistyped key), not an
	   error to surface as a nonsense value.  Stderr routed to /dev/null
	   so a failed lookup doesn't leak git's own diagnostic text. */
	char cmdbuf[BUFSIZ];
	snprintf(cmdbuf, sizeof(cmdbuf), "git rev-list -n 1 %s 2>/dev/null", PATCH_KEY);
	const char* commitid = shell_string(cmdbuf);
	if (commitid && commitid[0] != '\0') {
	    ComValue keyv(commitid);
	    push_stack(keyv);
	} else {
	    push_stack(ComValue::nullval());
	}
	return;
    }

    ComValue keyv(PATCH_KEY);
    push_stack(keyv);
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

UpdateFunc::UpdateFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void UpdateFunc::execute() {
    ComValue longzero(0L);
    ComValue usecv(stack_arg(0, false, longzero));
    long usec = usecv.long_val();
    reset_stack();
#ifdef HAVE_ACE
    if (usec > 0) {
        ACE_Time_Value timeout(usec/1000000, usec%1000000);
        ComterpHandler::reactor_singleton()->handle_events(timeout);
    } else {
        ACE_Time_Value timeout(ACE_Time_Value::zero);
        ComterpHandler::reactor_singleton()->handle_events(timeout);
    }
#endif
    push_stack(ComValue::zeroval());
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

BlankFunc::BlankFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void BlankFunc::execute() {
    reset_stack();
    push_stack(ComValue::blankval());
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

