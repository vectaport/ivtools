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
#include <ComTerp/funcobjscan.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Attribute/attribute.h>

#include <OS/math.h>

#include <iostream.h>
#if __GNUC__>=3
#include <fstream.h>
#endif
#include <strstream>

#define TITLE "PostFunc"

extern int _detail_matched_delims;

boolean SeqFunc::_continueflag = 0;
boolean SeqFunc::_breakflag = 0;

/*****************************************************************************/

PostFixFunc::PostFixFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PostFixFunc::execute() {
  // print everything on the stack for this function
  // use strstreambuf + fputs to avoid FILEBUF destructor closing stdout fd
  std::strstreambuf sbuf;
  ostream out(&sbuf);
 
  boolean oldbrief = comterp()->brief();
  comterp()->brief(true);
  int numargs = nargspost();

  ComValue argoff(comterp()->stack_top());
  int topptr = argoff.int_val()-(comterp()->pfnum()-1);
  for (int i=topptr-numargs; i<topptr; i++) {
    ComValue& val = comterp()->expr_top(i);
    val.comterp(comterp());
    out << val;
    if (val.is_type(AttributeValue::CommandType) ||
       (_detail_matched_delims && val.is_type(AttributeValue::SymbolType) && 
	val.nids() >= TOK_RPAREN )) {
      if (!_detail_matched_delims) {
	out << "[" << val.narg() << "|" << val.nkey() << "|" << val.nids() << "]";
	if (val.is_type(AttributeValue::CommandType)) {
  	  ComFunc* func = (ComFunc*)val.obj_val();
	  if (func->post_eval()) out << "*";
	}
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
    if (i+1<topptr) out << " ";
  }
  out << '\0';
  comterp()->brief(oldbrief);
  reset_stack();
  /* trim trailing space if present */
  char* str = sbuf.str();
  int len = strlen(str);
  while (len > 0 && str[len-1] == ' ') { str[--len] = '\0'; }
  ComValue retval(str);
  push_stack(retval);
  
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
  if (nargsfixed()>4) {
    fprintf(stderr, "Error: for loop with more than one body -- missing semicolon between statements (line %d)\n", funcstate()->linenum());
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }
  while (!SeqFunc::breakflag() && !comterp()->returnflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);
    
    ComValue whileexpr(stack_arg_post_eval(1));
    if (whileexpr.is_false()) break;
    delete bodyexpr;
    ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval()));
    if (keybody.is_unknown() && nargsfixed()>= 4) {
      bodyexpr = new ComValue(stack_arg_post_eval(3));
    } 
    else {
      bodyexpr = new ComValue(keybody);
    }
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
  if (nargsfixed()>2) {
    fprintf(stderr, "Error: while loop with more than one body -- missing semicolon between statements (line %d)\n", funcstate()->linenum());
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }
  while (!SeqFunc::breakflag() && !comterp()->returnflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);
    if (untilflag.is_false()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (nilchkflag.is_false() ? doneexpr.is_false() : doneexpr.is_unknown()) break;
    }
    delete bodyexpr;
    ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval()));
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
    if (SeqFunc::continueflag() || SeqFunc::breakflag() || comterp()->returnflag() || comterp()->quitflag()) {
      reset_stack();
      push_stack(arg1);
    }
    else {
      /* Drain arg1 (the statement before this ";") BEFORE evaluating
	 arg2, not after -- if it's an orphaned stream (refcount_==1: no
	 variable binding or anything else still holds it), draining it
	 can have visible side effects of its own (e.g. a print()
	 overdrive stream defers each repetition's actual print() call
	 until that element is pulled -- draining fires all of them at
	 once), and evaluating arg2 first would run arg2's own output
	 before arg1's deferred output, out of script order.  Mirrors
	 ComTerp::orphan_stream_count()'s use at the top level for the
	 very last statement, and ComTerpServ::runfile()'s equivalent
	 discard point for separate top-level lines (comterpserv.c) --
	 together they cover every freestanding stream in a script, not
	 just the final one.
	 Assumes arg2 won't turn out blank -- the one case where arg1
	 would have been kept as the ";" expression's own result rather
	 than discarded, and draining it first would return it already
	 exhausted.  Accepted: arg2 is blank only when there's no real
	 second operand at all (a bare trailing ";"), vanishingly rare in
	 combination with arg1 additionally being an undrained stream --
	 getting the common case's output order right matters more. */
      if (arg1.is_stream() && arg1.stream_list() && arg1.stream_list()->refcount_==1)
	comterp()->orphan_stream_count(arg1);
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

ReturnFunc::ReturnFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ReturnFunc::execute() {
  ComValue retval(stack_arg(0, true, ComValue::blankval()));
  reset_stack();

  comterp()->returnflag(true);

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
int FuncObjPendingArg::_symid = -1;

FuncObj::FuncObj(postfix_token* toks, int ntoks) {
  _toks = toks;
  _ntoks = ntoks;
  _posteval = false;
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
  static int posteval_symid = symbol_add("posteval");
  ComValue postevalv(stack_key_post_eval(posteval_symid));
  reset_stack();
  if (!tokbuf)
    push_stack(ComValue::nullval());
  else {
    if (echov.is_true())
      comterp()->postfix_echo(tokbuf, toklen);
    FuncObj* tokbufobj = new FuncObj(tokbuf, toklen);
    tokbufobj->posteval(postevalv.is_true());

    /* capture this body's free variables (read-only or
       read-before-write -- see funcobjscan.h) at
       declaration time, so a later fire sees the value that was live now,
       not whatever's live at call time.  is_plain_var[i] tells the
       classifier which tokens are ordinary variable references rather
       than registered commands -- built by the same shared helper the help path's
       :help fire-time analysis uses (FuncObjVarScan::build_is_plain_var),
       not reimplemented here. */
    boolean* is_plain_var = FuncObjVarScan::build_is_plain_var(comterp(), tokbuf, toklen);
    AttributeList* classification = FuncObjVarScan::classify(tokbuf, toklen, is_plain_var);
    /* RAII guard, not dead code: the AttributeValue ctor/dtor pair
       ref/unrefs classification automatically (HACKING.md's "Resource
       ref/unref and AttributeValue Constructors") so it's freed at scope
       exit -- classification itself is only ever read through the raw
       pointer below, never through this wrapper. */
    ComValue classification_owner(AttributeList::class_symid(), (void*)classification);
    delete [] is_plain_var;

    AttributeList* captures = nil;
    ALIterator cit;
    for (classification->First(cit); !classification->Done(cit); classification->Next(cit)) {
      Attribute* attr = classification->GetAttr(cit);
      int kind = attr->Value()->int_val();
      if (kind == FuncObjVarScan::ReadOnly || kind == FuncObjVarScan::ReadBeforeWrite) {
	if (!captures) captures = new AttributeList();
	/* lookup_symval(int) checks localtable() only; the full fallthrough an
	   ordinary read uses -- _alist, then localtable unless global_flag,
	   then globaltable -- is the ComValue& overload.  Anything narrower
	   under-captures a name set only through global()=. */
	ComValue symval(attr->SymbolId(), ComValue::SymbolType);
	ComValue curval(comterp()->lookup_symval(symval));
	captures->add_attr(attr->SymbolId(), curval);
      }
    }
    if (captures)
      tokbufobj->captures() = ComValue(AttributeList::class_symid(), (void*)captures);

    ComValue retval(FuncObj::class_symid(), (void*)tokbufobj);
    retval.comterp(comterp());
    push_stack(retval);
  }
}

