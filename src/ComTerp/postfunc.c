/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include <ComTerp/postfunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <iostream.h>

#define TITLE "PostFunc"

/*****************************************************************************/

PostFixFunc::PostFixFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PostFixFunc::execute() {
  // print everything on the stack for this function
  filebuf fbuf;
  if (comterp()->handler()) {
    int fd = max(1, comterp()->handler()->get_handle());
    fbuf.attach(fd);
  } else
    fbuf.attach(fileno(stdout));
  ostream out(&fbuf);
 
  boolean oldbrief = comterp()->brief();
  comterp()->brief(true);
  int numargs = nargspost();

  ComValue argoff(comterp()->stack_top());
  int topptr = argoff.int_val()-(comterp()->pfnum()-1);
  for (int i=topptr-numargs; i<=topptr; i++) {
    ComValue& val = comterp()->expr_top(i);
    val.comterp(comterp());
    out << val;
    if (val.is_type(AttributeValue::CommandType)) {
      out << "[" << val.narg() << "|" << val.nkey() << "]";
      ComFunc* func = (ComFunc*)val.obj_val();
      if (func->post_eval()) out << "*";
    }
    else if (val.is_type(AttributeValue::SymbolType) && 
	     (val.narg() || val.nkey()))
      out << "{" << val.narg() << "|" << val.nkey() << "}";
    else if (val.is_type(AttributeValue::KeywordType))
      out << "(" << val.keynarg_val() << ")";
    out << ((i+1>topptr) ? "\n" : " ");
  }
  comterp()->brief(oldbrief);
  reset_stack();
}

/*****************************************************************************/

PostEvalFunc::PostEvalFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PostEvalFunc::execute() {
  // evaluate every fixed argument on the stack and return in array
  int numargs = nargstotal();
  if (numargs) {
    AttributeValueList* avl = nil;
    for (int i=0; i<numargs; i++) {
      ComValue* val = new ComValue(stack_arg_post_eval(i));
      if (val->is_nil()) {
	delete val;
	break;
      }
      if (!avl) avl = new AttributeValueList();
      avl->Append(val);
    }
    reset_stack();
    if (avl) {
      ComValue retval(avl);
      push_stack(retval);
    }
  } else
    reset_stack();
}

/*****************************************************************************/

IfThenElseFunc::IfThenElseFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void IfThenElseFunc::execute() {
  ComValue booltest(stack_arg_post_eval(0));
  static int then_symid = symbol_add("then");
  static int else_symid = symbol_add("else");
  ComValue retval(booltest.is_true() 
		  ? stack_key_post_eval(then_symid)
		  : stack_key_post_eval(else_symid));
  reset_stack();
  push_stack(retval);
}

/*****************************************************************************/

ForFunc::ForFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ForFunc::execute() {
  static int body_symid = symbol_add("body");
  ComValue initexpr(stack_arg_post_eval(0));
  ComValue* bodyexpr = nil;
  while (1) {
    ComValue whileexpr(stack_arg_post_eval(1));
    if (whileexpr.is_false()) break;
    delete bodyexpr;
    ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval(), true));
    if (keybody.is_unknown() && nargsfixed()>= 4)
      bodyexpr = new ComValue(stack_arg_post_eval(3));
    else
      bodyexpr = new ComValue(keybody);
    ComValue nextexpr(stack_arg_post_eval(2));
  }
  reset_stack();
  if (bodyexpr) {
    push_stack(*bodyexpr);
    delete bodyexpr;
  } else 
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

WhileFunc::WhileFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void WhileFunc::execute() {
  static int body_symid = symbol_add("body");
  static int until_symid = symbol_add("until");
  ComValue untilflag(stack_key_post_eval(until_symid));
  ComValue* bodyexpr = nil;
  while (1) {
    if (untilflag.is_false()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (doneexpr.is_false()) break;
    }
    delete bodyexpr;
    ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval(), true));
    if (keybody.is_unknown() && nargsfixed()>= 2)
      bodyexpr = new ComValue(stack_arg_post_eval(1));
    else
      bodyexpr = new ComValue(keybody);
    if (untilflag.is_true()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (doneexpr.is_true()) break;
    }
  }
  reset_stack();
  if (bodyexpr) {
    push_stack(*bodyexpr);
    delete bodyexpr;
  } else 
    push_stack(ComValue::nullval());
}

