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
  /* :body predates today's positional multi-body support; see
     WhileFunc::execute() for the fuller rationale -- mixed with positional
     bodies it's flatly ignored (with a warning), but used alone it's the
     legacy sole-body idiom and must keep firing every iteration, or a
     for() whose only side effect lived in :body would silently stop having
     any (Greptile's "legacy for bodies are skipped").  Dropped from
     docstring()/dockeys() either way, so it no longer shows up in help()
     or the man page.  stack_key_present() only decides which warning (if
     any) to print without itself ever firing the keyword's expression. */
  boolean body_present = stack_key_present(body_symid);
  if (body_present) {
    if (nargsfixed()>= 4)
      fprintf(stderr, "Warning: for()'s :body keyword has no effect when a positional body is also given -- use positional bodies instead (line %d)\n", funcstate()->linenum());
    else
      fprintf(stderr, "Warning: for()'s :body keyword is deprecated -- use a positional body instead (line %d)\n", funcstate()->linenum());
  }
  ComValue initexpr(stack_arg_post_eval(0));
  ComValue* bodyexpr = nil;
  while (!SeqFunc::breakflag() && !comterp()->returnflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);

    ComValue whileexpr(stack_arg_post_eval(1));
    if (whileexpr.is_false()) break;
    delete bodyexpr;
    if (nargsfixed()>= 4) {
      /* positions 3..N-1 are one or more space-separated bodies.  All but
	 the last run for side effects only; an orphaned stream among them
	 gets drained instead of silently dropped.  The last body's value
	 is kept.  A control transfer (break/continue/return/quit) raised by
	 an earlier body stops the remaining ones from running, same as
	 SeqFunc::execute does for ';'. */
      for (int i=3; i<nargsfixed(); i++) {
	ComValue v(stack_arg_post_eval(i));
	boolean control = SeqFunc::continueflag() || SeqFunc::breakflag() ||
	  comterp()->returnflag() || comterp()->quitflag();
	if (i==nargsfixed()-1 || control) {
	  bodyexpr = new ComValue(v);
	  if (control) break;
	} else if (v.is_stream() && v.stream_list() && v.stream_list()->refcount_==1) {
	  comterp()->orphan_stream_count(v);
	}
      }
    }
    else {
      /* no positional body -- :body (if present) is the legacy sole body
	 and must keep running every iteration for backward compatibility;
	 absent, this is just a bodyless for(). */
      ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval()));
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
  /* :body predates today's positional multi-body support.  Mixed with
     positional bodies it never composed with them (a :body value alongside
     positional bodies used to silently discard the positionals) -- that
     combination is now flatly ignored (with a warning) rather than
     evaluated, since nothing could have been relying on behavior that was
     already broken.  Used ALONE, though, :body is the pre-multibody idiom
     for the entire loop body (e.g. "while(i :body i=i-1)") and has to keep
     firing every iteration exactly as before -- Greptile correctly flagged
     that a from-now-on-inert :body hangs a legacy loop whose condition only
     changes inside it.  Either way it's dropped from docstring()/dockeys()
     (so it no longer shows up in help() or the man page) in favor of
     positional bodies.  stack_key_present() only walks the keyword's token
     span to decide which warning (if any) to print -- unlike
     stack_key_post_eval, it never calls post_eval_expr, so this presence
     check alone never fires the keyword's expression. */
  boolean body_present = stack_key_present(body_symid);
  if (body_present) {
    if (nargsfixed()>= 2)
      fprintf(stderr, "Warning: while()'s :body keyword has no effect when a positional body is also given -- use positional bodies instead (line %d)\n", funcstate()->linenum());
    else
      fprintf(stderr, "Warning: while()'s :body keyword is deprecated -- use a positional body instead (line %d)\n", funcstate()->linenum());
  }
  ComValue* bodyexpr = nil;
  while (!SeqFunc::breakflag() && !comterp()->returnflag() && !comterp()->quitflag()) {
    SeqFunc::continueflag(0);
    if (untilflag.is_false()) {
      ComValue doneexpr(stack_arg_post_eval(0));
      if (nilchkflag.is_false() ? doneexpr.is_false() : doneexpr.is_unknown()) break;
    }
    delete bodyexpr;
    if (nargsfixed()>= 2) {
      /* positions 1..N-1 are one or more space-separated bodies.  All but
	 the last run for side effects only; an orphaned stream among them
	 gets drained instead of silently dropped.  The last body's value
	 is kept.  A control transfer (break/continue/return/quit) raised by
	 an earlier body stops the remaining ones from running, same as
	 SeqFunc::execute does for ';'. */
      for (int i=1; i<nargsfixed(); i++) {
	ComValue v(stack_arg_post_eval(i));
	boolean control = SeqFunc::continueflag() || SeqFunc::breakflag() ||
	  comterp()->returnflag() || comterp()->quitflag();
	if (i==nargsfixed()-1 || control) {
	  bodyexpr = new ComValue(v);
	  if (control) break;
	} else if (v.is_stream() && v.stream_list() && v.stream_list()->refcount_==1) {
	  comterp()->orphan_stream_count(v);
	}
      }
    }
    else {
      /* no positional body -- :body (if present) is the legacy sole body
	 and must keep running every iteration for backward compatibility;
	 absent, this is just a bodyless while() (e.g. "while(v=next(s))"). */
      ComValue keybody(stack_key_post_eval(body_symid, false, ComValue::unkval()));
      bodyexpr = new ComValue(keybody);
    }
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
      /* arg1 (the statement before this ";") is simply discarded, same as
	 any other discarded value -- no forced draining of an orphaned
	 stream here.  for()/while()/func() bodies drain explicitly instead
	 (each one's own body-sequencing loop calls orphan_stream_count()
	 on a non-final result); ';' stays a plain discard on purpose. */
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

FuncObj::FuncObj(postfix_token* toks, int ntoks, int* spanlens, int nspans) {
  _toks = toks;
  _ntoks = ntoks;
  if (spanlens) {
    _spanlens = spanlens;
    _nspans = nspans;
  } else {
    _spanlens = new int[1];
    _spanlens[0] = ntoks;
    _nspans = 1;
  }
  _posteval = false;
}

FuncObj::~FuncObj() {
  delete [] _toks;
  delete [] _spanlens;
}

/*****************************************************************************/

FuncObjFunc::FuncObjFunc(ComTerp* comterp) : ComFunc(comterp) {
}


void FuncObjFunc::execute() {
  /* one or more space-separated bodies, same shape as for()/while()'s
     positional bodies -- each copied span is a complete,
     independently-parsed expression, so concatenating them back to back
     (no separator token needed) gives ComTerpServ::run_funcobj_body() a
     buffer it can walk one span at a time at fire time, autostreaming all
     but the last exactly as for()/while() already do for their own bodies. */
  int nspans = nargsfixed();
  postfix_token** spanbufs = nspans>0 ? new postfix_token*[nspans] : nil;
  int* spanlens = nspans>0 ? new int[nspans] : nil;
  int total = 0;
  for (int i=0; i<nspans; i++) {
    spanbufs[i] = copy_stack_arg_post_eval(i, spanlens[i]);
    if (spanbufs[i]) total += spanlens[i];
  }
  static int echo_symid = symbol_add("echo");
  ComValue echov(stack_key_post_eval(echo_symid));
  static int posteval_symid = symbol_add("posteval");
  ComValue postevalv(stack_key_post_eval(posteval_symid));
  reset_stack();
  if (nspans==0 || !spanbufs[0]) {
    push_stack(ComValue::nullval());
    for (int i=0; i<nspans; i++) delete [] spanbufs[i];
    delete [] spanbufs;
    delete [] spanlens;
  }
  else {
    postfix_token* tokbuf = new postfix_token[total];
    int toklen = total;
    int offset = 0;
    for (int i=0; i<nspans; i++) {
      for (int j=0; j<spanlens[i]; j++) tokbuf[offset+j] = spanbufs[i][j];
      offset += spanlens[i];
      delete [] spanbufs[i];
    }
    delete [] spanbufs;

    if (echov.is_true())
      comterp()->postfix_echo(tokbuf, toklen);
    FuncObj* tokbufobj = new FuncObj(tokbuf, toklen, spanlens, nspans);
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

