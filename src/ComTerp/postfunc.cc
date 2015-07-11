/*
 * Copyright (c) 2001 Scott E. Johnston
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

#include <ComTerp/comhandler.h>

#include <ComTerp/postfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <OS/math.h>

#include <iostream.h>
#if __GNUC__>=3
#include <fstream.h>
#endif

#define TITLE "PostFunc"

extern int _detail_matched_delims;

boolean SeqFunc::_continueflag = 0;
boolean SeqFunc::_breakflag = 0;

/*****************************************************************************/

PostFixFunc::PostFixFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PostFixFunc::execute() {
  // print everything on the stack for this function
  FILEBUF(fbuf, comterp()->handler() && comterp()->handler()->wrfptr()
	  ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
  ostream out(&fbuf);
 
  boolean oldbrief = comterp()->brief();
  comterp()->brief(true);
  int numargs = nargspost();

  ComValue argoff(comterp()->stack_top());
  int topptr = argoff.int_val()-(comterp()->pfnum()-1);
  for (int i=topptr-numargs; i<=topptr-1; i++) {
    ComValue& val = comterp()->expr_top(i);
    val.comterp(comterp());
    out << val;
    if (val.is_type(AttributeValue::CommandType) ||
       (_detail_matched_delims && val.is_type(AttributeValue::SymbolType) && 
	val.nids() >= TOK_RPAREN )) {
      if (!_detail_matched_delims) {
	out << "[" << val.narg() << "|" << val.nkey() << "]";
	ComFunc* func = (ComFunc*)val.obj_val();
	if (func->post_eval()) out << "*";
      } else {
	char ldelim, rdelim;
	boolean dbldelim = 0;
	if (val.nids()==TOK_RPAREN) {ldelim = '('; rdelim = ')'; }
	else if (val.nids()==TOK_RBRACKET) {ldelim = '['; rdelim = ']'; }
	else if (val.nids()==TOK_RBRACE) {ldelim = '{'; rdelim = '}'; }
	else if (val.nids()==TOK_RANGBRACK) {ldelim = '<'; rdelim = '>'; }
	else if (val.nids()==TOK_RANGBRACK2) {ldelim = '<'; rdelim = '>'; dbldelim=1;}
	else {ldelim = ':'; rdelim = 0x0;};
	out << ldelim;
	if(dbldelim) out << ldelim;
	out << val.narg();
	if (rdelim) {
	  out << rdelim;
	  if(dbldelim) out << rdelim;
	}
      }
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
  if (nargsfixed()>4) fprintf(stderr, "Unexpected for loop with more than one body\n");
  while (!SeqFunc::breakflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);
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
  SeqFunc::breakflag(0);
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
  static int nilchk_symid = symbol_add("nilchk");
  ComValue untilflag(stack_key_post_eval(until_symid));
  ComValue nilchkflag(stack_key_post_eval(nilchk_symid));
  ComValue* bodyexpr = nil;
  if (nargsfixed()>2) fprintf(stderr, "Unexpected while loop with more than one body\n");
  while (!SeqFunc::breakflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);
    if (untilflag.is_false()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (nilchkflag.is_false() ? doneexpr.is_false() : doneexpr.is_unknown()) break;
    }
    delete bodyexpr;
    ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval(), true));
    if (keybody.is_unknown() && nargsfixed()>= 2)
      bodyexpr = new ComValue(stack_arg_post_eval(1));
    else
      bodyexpr = new ComValue(keybody);
    if (untilflag.is_true()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (nilchkflag.is_false() ? doneexpr.is_true() : doneexpr.is_unknown()) break;
    }
  }
  SeqFunc::breakflag(0);
  reset_stack();
  if (bodyexpr) {
    push_stack(*bodyexpr);
    delete bodyexpr;
  } else 
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

SeqFunc::SeqFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SeqFunc::execute() {
    ComValue arg1(stack_arg_post_eval(0, true));
    if (SeqFunc::continueflag() || SeqFunc::breakflag() || comterp()->quitflag()) {
      reset_stack();
      push_stack(arg1);       
    }
    else {
      ComValue arg2(stack_arg_post_eval(1, true));
      reset_stack();
      push_stack(arg2.is_blank() ? arg1 : arg2);
    }
}


/*****************************************************************************/

ContinueFunc::ContinueFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ContinueFunc::execute() {
  reset_stack();

  SeqFunc::continueflag(1);

  ComValue retval(ComValue::trueval());
  push_stack(retval);
  return;
}

/*****************************************************************************/

BreakFunc::BreakFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void BreakFunc::execute() {
  ComValue retval(stack_arg(0,true,ComValue::trueval()));
  reset_stack();

  SeqFunc::breakflag(1);

  push_stack(retval);
  return;
}

/*****************************************************************************/

SwitchFunc::SwitchFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SwitchFunc::execute() {
  ComValue valv(stack_arg_post_eval(0));
  int case_symid;
  if(valv.is_int()) {
    char buffer[BUFSIZ];
    snprintf(buffer, BUFSIZ, "case%s%d", 
             valv.int_val()>=0 ? "" : "_", 
             valv.int_val()>=0 ? valv.int_val() : -valv.int_val());
    case_symid = symbol_add(buffer);
  } else if (valv.is_symbol() || valv.is_string()) {
    case_symid = valv.symbol_val();
  } else if (valv.is_char()) {
    char cbuf[2];
    cbuf[0] = valv.char_val();
    cbuf[1] = '\0';
    case_symid = symbol_add(cbuf);
  }
  ComValue retval(stack_key_post_eval(case_symid));
  if (retval.is_unknown()) {
    static int default_symid = symbol_add("default");
    retval = stack_key_post_eval(default_symid);
  }
  reset_stack();
  push_stack(retval);
}

/*****************************************************************************/
int FuncObj::_symid = -1;

FuncObj::FuncObj(postfix_token* toks, int ntoks) {
  _toks = toks;
  _ntoks = ntoks;
}

FuncObj::~FuncObj() { 
  delete [] _toks;
}

/*****************************************************************************/

FuncObjFunc::FuncObjFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void FuncObjFunc::execute() {
  int toklen;
  postfix_token* tokbuf = copy_stack_arg_post_eval(0, toklen);
  static int echo_symid = symbol_add("echo");
  ComValue echov(stack_key_post_eval(echo_symid));
  reset_stack();
  if (!tokbuf)
    push_stack(ComValue::nullval());
  else {
    if (echov.is_true())
      comterp()->postfix_echo(tokbuf, toklen);
    FuncObj* tokbufobj = new FuncObj(tokbuf, toklen);
    ComValue retval(FuncObj::class_symid(), (void*)tokbufobj);
    push_stack(retval);
  }
}

