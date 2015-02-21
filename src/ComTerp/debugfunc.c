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





