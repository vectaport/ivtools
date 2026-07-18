/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
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

#include <cstdio>
#include <vector>
#include <fstream.h>

#include <ComTerp/comhandler.h>

#include <ComTerp/debugfunc.h>
#include <ComTerp/comterpserv.h>
#include <strstream>
#include <iostream>
#include <fstream>

using std::cerr;
using std::vector;

#define TITLE "DebugFunc"

extern void err_str(char*, int, const char*);
extern void err_clear();
extern int err_cnt();
extern int comerr_get();

/*****************************************************************************/

ComterpTraceFunc::ComterpTraceFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ComterpTraceFunc::execute() {
  static int get_symid = symbol_add("get");
  boolean get_flag = stack_key(get_symid).is_true();
  if (get_flag) {
    reset_stack();
    int mode = comterp()->trace_mode();
    ComValue retval(mode, ComValue::IntType);
    push_stack(retval);
  } else {
    if (nargs()==0) {
      reset_stack();
      int mode = !comterp()->trace_mode();
      comterp()->trace_mode(mode);
      ComValue retval(mode, ComValue::IntType);
      push_stack(retval);
    } else {
      ComValue retval(stack_arg(0));
      reset_stack();
      comterp()->trace_mode(retval.int_val());
      push_stack(retval);
    }
  }
}

/*****************************************************************************/

ComterpPauseFunc::ComterpPauseFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ComterpPauseFunc::execute_body(ComValue& msgstrv) {

  comterp()->npause()++;

  comterp()->push_servstate();
  FILEBUF(fbufin, comterp() && comterp()->handler() && comterp()->handler()->rdfptr() 
		 ? comterp()->handler()->rdfptr() : stdin, ios_base::in);
  istream in(&fbufin);
  FILEBUF(fbufout, comterp()->handler() && comterp()->handler()->wrfptr()
		  ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
  ostream out(&fbufout);

 if (msgstrv.is_string()) {
    std::ostrstream sbuf1_s;
    sbuf1_s << (stepfunc() ? "step(" : "pause(") << comterp()->npause() << "): " 
	    << msgstrv.string_ptr() << "\n";
    sbuf1_s.put('\0');
    out << sbuf1_s.str();
    out.flush();
 }
  std::ostrstream sbuf2_s;
  sbuf2_s << (stepfunc() ? "step(" : "pause(") << comterp()->npause() << "): enter command or press C/R to continue\n";
  sbuf2_s.put('\0');
  out << sbuf2_s.str();
  out.flush();

  vector<char> cvect;
  ComValue retval;
  do {
    char ch;
    cvect.erase(cvect.begin(), cvect.end());


    /* need to handle embedded newlines differently */
    do {
      ch = in.get();
      cvect.push_back(ch);
    } while (in.good() && ch != '\n');
    if (!in.good()) {
      /* stdin exhausted (EOF) or a read error -- e.g. a script that
	 triggers step()/pause() under -runfile or piped, non-interactive
	 stdin.  A failed istream never recovers on its own, so without this
	 check the code below would misread the EOF sentinel byte as a
	 1-byte "command", and this whole do-while would spin forever
	 re-reading the same permanently-failed stream.  Treat EOF the same
	 as an explicit C/R: stop pausing instead of hanging. */
      cerr << (stepfunc() ? "step(" : "pause(") << comterp()->npause()
	   << "): stdin closed/exhausted -- continuing without further input\n";
      if (stepfunc() && comterp()->stepflag()) {
	/* step() (unlike one-shot pause()) leaves stepflag() on, so
	   eval_expr_internals() re-enters this same pause machinery before
	   EVERY subsequent statement -- each re-entry re-triggers the
	   fdopen/dup/FILEBUF setup around the ">>> funcname(...)" trace
	   line, which is fragile under rapid/repeated re-entry with no
	   interactive stdin left to serve it (a separate, deeper bug of its
	   own).  Since we can no longer read interactive step commands
	   anyway once stdin is gone, fall out of step mode entirely rather
	   than walking back into that machinery for every remaining
	   statement. */
	cerr << "step(" << comterp()->npause()
	     << "): turning off step mode (no interactive input left to serve it)\n";
	comterp()->stepflag() = false;
      }
      break;
    }
    if (cvect[0] != '\n' && (cvect[0] != '\r' || cvect[1] != '\n')) {
      if (comterpserv()) {
	retval.assignval(comterpserv()->run(&cvect[0]));
	ComValue::comterp(comterpserv());
	out << retval << "\n";
	out.flush();
      } else {
	cerr << "execution of commands during step requires comterp in server or remote mode\n";
      }
    }
  } while (cvect[0] != '\n' && (cvect[0] != '\r' || cvect[1] != '\n'));
  comterp()->pop_servstate();
  std::ostrstream sbuf_e;
  sbuf_e << (stepfunc() ? "end of step(" : "end of pause(") << comterp()->npause()-- << ")\n";
  sbuf_e.put('\0');
  out << sbuf_e.str();
  out.flush();
  push_stack(retval);
}

void ComterpPauseFunc::execute() {
  ComValue msgstrv(stack_arg(0));
  reset_stack();
  execute_body(msgstrv);
}


/*****************************************************************************/

ComterpStepFunc::ComterpStepFunc(ComTerp* comterp) : ComterpPauseFunc(comterp) {
}

void ComterpStepFunc::execute() {
  ComValue msgstrv(stack_arg(0));
  static int pause_symid = symbol_add("pause");
  ComValue pausekey(stack_key(pause_symid));
  reset_stack();
  if (pausekey.is_true()) {
    execute_body(msgstrv);
  } else {
    comterp()->stepflag() = !comterp()->stepflag();
    ComValue retval(comterp()->stepflag());
    push_stack(retval);
  }
}


/*****************************************************************************/

ComterpStackHeightFunc::ComterpStackHeightFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ComterpStackHeightFunc::execute() {
  reset_stack();
  ComValue retval(comterp()->stack_height());
  push_stack(retval);
}


/*****************************************************************************/

// #include "malloc/malloc.h"

ComterpMallInfoFunc::ComterpMallInfoFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ComterpMallInfoFunc::execute() {
#if 1
  printf("mallinfo disabled\n");
#else
  reset_stack();
  struct mallinfo mi;
  mi = mallinfo();
  printf("Total non-mmapped bytes (arena):       %d\n", mi.arena);
  printf("# of free chunks (ordblks):            %d\n", mi.ordblks);
  printf("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
  printf("# of mapped regions (hblks):           %d\n", mi.hblks);
  printf("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
  printf("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
  printf("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
  printf("Total allocated space (uordblks):      %d\n", mi.uordblks);
  printf("Total free space (fordblks):           %d\n", mi.fordblks);
  printf("Topmost releasable block (keepcost):   %d\n", mi.keepcost);
  push_stack(ComValue::zeroval());
#endif
}

/*****************************************************************************/

ErrMsgFunc::ErrMsgFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ErrMsgFunc::execute() {
    static int keep_symid = symbol_add("keep");
    ComValue keepv(stack_key(keep_symid));
    boolean keepflag = keepv.is_true();
    static int last_symid = symbol_add("last");
    ComValue lastv(stack_key(last_symid));
    boolean lastflag = lastv.is_true();
    static int num_symid = symbol_add("num");
    ComValue numv(stack_key(num_symid));
    boolean numflag = numv.is_true();
    static int cnt_symid = symbol_add("cnt");
    ComValue cntv(stack_key(cnt_symid));
    boolean cntflag = cntv.is_true();
    static int clear_symid = symbol_add("clear");
    ComValue clearv(stack_key(clear_symid));
    boolean clearflag = clearv.is_true();
    reset_stack();

    if (clearflag) {
        err_clear();
        comterp()->clear_last_errmsg();
        push_stack(ComValue::nullval());
        return;
    }

    if (cntflag) {
        ComValue retval(err_cnt(), ComValue::IntType);
        push_stack(retval);
        return;
    }

    if (numflag) {
        ComValue retval((int)comerr_get(), ComValue::IntType);
        push_stack(retval);
        return;
    }

    char errbuf[BUFSIZ];
    errbuf[0] = '\0';

    if (lastflag) {
        // :last -- read _errbuf2 directly, bypass current error state
        strncpy(errbuf, comterp()->last_errmsg(), BUFSIZ-1);
        if (!keepflag)
            comterp()->clear_last_errmsg();
    } else {
        // check current error first, fall back to last reported error
        err_str(errbuf, BUFSIZ, "comterp");
        if (strlen(errbuf) == 0)
            strncpy(errbuf, comterp()->last_errmsg(), BUFSIZ-1);
        if (!keepflag) {
            err_clear();
            comterp()->clear_last_errmsg();
        }
    }

    if (strlen(errbuf) > 0) {
        int symid = symbol_add(errbuf);
        ComValue retval(symid, ComValue::StringType);
        push_stack(retval);
    } else
        push_stack(ComValue::nullval());
}
