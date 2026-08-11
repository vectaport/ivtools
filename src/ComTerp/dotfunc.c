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

#include <ComTerp/dotfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comterpserv.h>
#include <ComTerp/postfunc.h>
#include <ComTerp/boolfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define TITLE "DotFunc"

using std::cout;
using std::cerr;

/*****************************************************************************/

int DotFunc::_symid = -1;

/* off by default -- capturing the pre-fire source text of both args (see
   execute() below) costs a cout redirect + two print_stack_arg_post_eval
   calls on every dot-expression, even the overwhelmingly common case where
   nothing goes wrong.  Toggle at runtime with dot(:dbg true)/dot(:dbg false)
   -- see DotFunc::execute() -- only while chasing a "expression before/after
   dot" warning. */
static boolean dotfunc_debug_expr = false;

DotFunc::DotFunc(ComTerp* comterp) : ComFunc(comterp) {
}

/* true if a and b hold the same value -- reuses EqualFunc's own
   type-promoting comparison (numeric promotion, nil/blank handling, etc.)
   rather than re-deriving it, by pushing both values and invoking it
   directly the way ComFunc::exec() lets any command be called out of
   band. */
static boolean values_equal(ComTerp* comterp, AttributeValue& a, AttributeValue& b) {
  ComValue va(a), vb(b);
  comterp->push_stack(va);
  comterp->push_stack(vb);
  EqualFunc eqf(comterp);
  eqf.exec(2, 0);
  ComValue result(comterp->pop_stack());
  return result.is_true();
}

/* bookkeeping for one keyword arg to a method call -- see the comment on
   the keyword-handling block in fire_attrlist_method for what "ephemeral
   unless written" means here. */
struct KwPending {
  int symid;
  boolean existed;
  AttributeValue oldval;
  AttributeValue injectedval;
};

static void apply_kw(AttributeList* al, int symid, AttributeValue& newval, KwPending& pending) {
  pending.symid = symid;
  Attribute* existing = al->GetAttr(symid);
  pending.existed = existing!=nil;
  if (pending.existed) pending.oldval = *existing->Value();
  pending.injectedval = newval;
  al->add_attr(symid, newval);
}

static void restore_kw_if_unwritten(ComTerp* comterp, AttributeList* al, KwPending& pending) {
  Attribute* now = al->GetAttr(pending.symid);
  if (!now || !values_equal(comterp, *now->Value(), pending.injectedval))
    return;   // written during the call (or vanished outright) -- leave it
  if (pending.existed)
    *now->Value() = pending.oldval;
  else
    al->Remove(now);
}

/* obj.method(args) -- fire a FuncObj-valued attribute self-bound to obj.
   Evaluates args in the caller's own scope (before any _alist swap, so a
   variable reference in an arg resolves against the caller, not obj), then
   temporarily swaps _alist to obj -- the same mechanism eval(fo :alist obj)
   already uses (ctrlfunc.c's EvalFunc) -- and runs the method's own token
   buffer directly, so self-bound reads/writes of obj's own fields mutate
   the real object, not a per-call copy.  Positional args flow through the
   funcobj_args channel (set_funcobj_args/funcobj_argvals, comterp.h) the
   same way an ordinary FuncObj invocation's arg()/narg() are served.

   Getting at args without evaluating "method" itself: a symbol with an
   attached arglist that isn't a registered global command gets rebound to
   the nil command by ComTerp::token_to_comvalue at expression-conversion
   time, unconditionally, before any command (including this one) runs --
   that check only ever consults the global command table, never _alist, so
   there is no way to make "method(args)" resolve through obj by evaluating
   it as an ordinary sub-expression.  Instead: look "method" up in obj
   directly (the same GetAttr() the bare-access path below already uses),
   and get the args evaluated by retargeting a copy of just the arg tokens
   at the existing echo() command -- an ordinary (non-post_eval) command
   that already packages positionals/keywords into an inspectable value for
   the ~~ round-trip -- so ordinary dispatch does the evaluation, still
   never touching "method" as a symbol at all. */
static void fire_attrlist_method(ComFunc* self, ComTerp* comterp,
				  AttributeList* al, postfix_token* argtoks,
				  int nargtoks) {
  postfix_token& method_tok = argtoks[nargtoks-1];
  int method_symid = method_tok.v.symbolid;
  int method_narg = method_tok.narg;
  int method_nkey = method_tok.nkey;

  Attribute* attr = al ? al->GetAttr(method_symid) : nil;
  if (!attr || !attr->Value()->is_object(FuncObj::class_symid())) {
    cout << "WARNING: \"" << symbol_pntr(method_symid)
	 << "\" is not a func-valued attribute -- line "
	 << self->funcstate()->linenum() << "\n";
    delete [] argtoks;
    self->push_stack(ComValue::nullval());
    return;
  }
  FuncObj* fo = (FuncObj*) attr->Value()->obj_val();

  /* echoresult owns poslist's/kwlist's storage (a ComValue destructor
     unrefs its list/attrlist) -- keep it alive across both the extraction
     below and this whole block, not just the narrower "did echo run"
     scope. */
  ComValue echoresult;
  AttributeValueList* poslist = nil;
  AttributeList* kwlist = nil;
  int npos = 0;
  if (method_narg>0 || method_nkey>0) {
    static int echo_symid = symbol_add("echo");
    method_tok.v.symbolid = echo_symid;
    echoresult = self->comterpserv()->run(argtoks, nargtoks);
    if (echoresult.is_list()) {
      /* positionals present -- echo tails the list with one singleton
	 attrlist per keyword (see echo()'s own docstring); trailing
	 method_nkey entries are those singletons, not positionals. */
      poslist = echoresult.list_val();
      npos = poslist->Number() - method_nkey;
      if (npos<0) npos = 0;
    } else if (echoresult.is_attributelist()) {
      /* no positionals -- echo returns the keywords bare, as one
	 multi-attribute attrlist. */
      kwlist = (AttributeList*) echoresult.obj_val();
    }
  }
  delete [] argtoks;

  ComValue* posvals = npos>0 ? new ComValue[npos] : nil;
  if (npos>0) {
    for (int i=0; i<npos; i++)
      posvals[i] = *poslist->Get(i);
  }

  /* keyword args are ephemeral unless the method's own body writes that
     same name -- reading it (or ignoring it) leaves al exactly as it was;
     only a write (self-bound, so it lands on al) makes it stick.  Apply
     each keyword now (saving what it's replacing), fire below, then
     compare-and-revert after: unchanged from what was just injected means
     nothing wrote it, so put the old value back (or remove it entirely if
     the name didn't exist on al before this call). */
  int nkw = method_nkey;
  KwPending* kwpending = nkw>0 ? new KwPending[nkw] : nil;
  if (nkw>0) {
    if (poslist) {
      for (int i=0; i<nkw; i++) {
	AttributeList* singleton = (AttributeList*) poslist->Get(npos+i)->obj_val();
	ALIterator it;
	singleton->First(it);
	Attribute* a = singleton->GetAttr(it);
	apply_kw(al, a->SymbolId(), *a->Value(), kwpending[i]);
      }
    } else if (kwlist) {
      ALIterator it;
      int i = 0;
      for (kwlist->First(it); !kwlist->Done(it); kwlist->Next(it), i++) {
	Attribute* a = kwlist->GetAttr(it);
	apply_kw(al, a->SymbolId(), *a->Value(), kwpending[i]);
      }
    }
  }

  AttributeList* old_alist = comterp->get_attributes();
  Resource::ref(old_alist);
  comterp->set_attributes(al);

  ComValue* saved_argvals = comterp->funcobj_argvals();
  int saved_nargs = comterp->funcobj_narg();
  boolean saved_active = comterp->funcobj_active();
  comterp->set_funcobj_args(posvals, npos, true);

  ComValue result(self->comterpserv()->run(fo->toks(), fo->ntoks()));

  comterp->set_funcobj_args(saved_argvals, saved_nargs, saved_active);
  delete [] posvals;

  comterp->set_attributes(old_alist);
  Unref(old_alist);

  for (int i=0; i<nkw; i++)
    restore_kw_if_unwritten(comterp, al, kwpending[i]);
  delete [] kwpending;

  self->push_stack(result);
}

void DotFunc::peek_and_fire(ComValue& before_part, ComValue& after_raw, int& after_nids,
			     std::string& before_expr_text, std::string& after_expr_text) {
    /* Grab the raw, unfired source text of both args for the warnings
       below *before* either can be evaluated -- stack_arg_post_eval(0)
       just below fires arg 0 when it's a nested dot (node.left.val),
       which moves the stack bookmark print_stack_arg_post_eval relies
       on; capturing after that point prints garbage (confirmed: showed
       a stale offset int instead of the real argument). */
    if (dotfunc_debug_expr) {
      std::ostringstream before_expr_stream, after_expr_stream;
      std::streambuf* saved_cout = cout.rdbuf(before_expr_stream.rdbuf());
      print_stack_arg_post_eval(0);
      cout.rdbuf(after_expr_stream.rdbuf());
      print_stack_arg_post_eval(1);
      cout.rdbuf(saved_cout);
      before_expr_text = before_expr_stream.str();
      after_expr_text = after_expr_stream.str();
    }

    before_part = stack_arg(0, true);
    if (before_part.type()==ComValue::CommandType) {
      /* a real command reference (e.g. the inner dot of node.left.val) --
	 fire it to get its actual value.  A bare, unbound symbol (the
	 compound-variable-on-first-use case just below) never gets promoted
	 to CommandType by ComTerp::token_to_comvalue in the first place, so
	 this never fires on a name DotFunc's own lookup below needs raw.
	 symbol=false (the default) here, not true: this needs pop_stack's
	 full finalization (resolve a symbol, unwrap an Attribute down to
	 its own Value()), not the raw, unprocessed result. */
      before_part = stack_arg_post_eval(0);
    }
    after_raw = stack_arg(1, true);
    after_nids = after_raw.nids();
}

void DotFunc::execute_core(ComValue before_part, ComValue after_raw, int after_nids,
			    const std::string& before_expr_text, const std::string& after_expr_text) {
    if (!before_part.is_symbol() &&
	!(before_part.is_attribute() &&
	  (((Attribute*)before_part.obj_val())->Value()->is_unknown() ||
	  ((Attribute*)before_part.obj_val())->Value()->is_attributelist())) &&
	!before_part.is_attributelist()) {
      cout << "WARNING: expression before \".\" needs to evaluate to a symbol or <AttributeList> (instead of "
	   << symbol_pntr(before_part.type_symid());
      if (before_part.is_object())
        cout << " of class " << symbol_pntr(before_part.class_symid());
      cout << ") -- line " << funcstate()->linenum() << "\n";
      if (dotfunc_debug_expr)
        cout << "expression before dot:  " << before_expr_text;
      else
        cout << "(dot(:dbg true) to see the expression before the dot)\n";
      cout << "expression after dot:  " << after_raw << "\n";
      reset_stack();

      return;
    }
    /* An arglist-attached rhs (after_nids != -1, e.g. al.method(2)) is
       validated later, in fire_attrlist_method -- by the time this token
       reaches here it has already been converted to CommandType (rebound
       to nil, see the note above fire_attrlist_method), which is expected
       and not itself a malformed-expression signal.  Only the bare/string
       rhs path needs this check. */
    if (after_nids==-1 && nargsfixed()>1 && !after_raw.is_string() && !after_raw.is_symbol()) {
      cout << "WARNING: expression after \".\" needs to be a symbol or evaluate to a symbol (instead of "
	   << symbol_pntr(after_raw.type_symid());
      if (before_part.is_object())
	cout << " for class " << symbol_pntr(before_part.class_symid());
      cout << ") -- line " << funcstate()->linenum() << "\n";
      if (dotfunc_debug_expr)
        cout << "expression after dot:  " << after_expr_text;
      else
        cout << "(dot(:dbg true) to see the expression after the dot)\n";
      reset_stack();
      return;
    }

    /* lookup value of before variable */
    void* vptr = nil;
    AttributeList* al = nil;
    if (!before_part.is_attribute() && !before_part.is_attributelist()) {
      int before_symid = before_part.symbol_val();
      boolean global = before_part.global_flag();
      /* func scope (_alist) is checked before local/global, same order
	 ComTerp::lookup_symval uses -- otherwise a func-local variable
	 (e.g. a keyword-bound param whose value only lives in _alist, never
	 in localtable()) is invisible to dot access from inside the func
	 body (#292). */
      AttributeList* funcscope = !global ? comterp()->get_attributes() : nil;
      AttributeValue* fsval = funcscope ? funcscope->find(before_symid) : nil;
      if (fsval) {
	if (fsval->is_attributelist())
	  al = (AttributeList*) fsval->obj_val();
	else {
	  al = new AttributeList();
	  AttributeValue newval(AttributeList::class_symid(), (void*) al);
	  *fsval = newval;
	}
      } else {
	if (!global) {
	  comterp()->localtable()->find(vptr, before_symid);
	  if (!vptr) comterp()->globaltable()->find(vptr, before_symid);
	} else {
	  comterp()->globaltable()->find(vptr, before_symid);
	}
	if (vptr &&((ComValue*) vptr)->class_symid() == AttributeList::class_symid()) {
	  al = (AttributeList*) ((ComValue*) vptr)->obj_val();
	} else {
	  al = new AttributeList();
	  ComValue* comval = new ComValue(AttributeList::class_symid(), (void*)al);
	  if (!global)
	    comterp()->localtable()->insert(before_symid, comval);
	  else
	    comterp()->globaltable()->insert(before_symid, comval);
	}
      }
    } else if (!before_part.is_attributelist()) {
      if (((Attribute*)before_part.obj_val())->Value()->is_attributelist())
	al = (AttributeList*) ((Attribute*) before_part.obj_val())->Value()->obj_val();
      else {
	al = new AttributeList();
	AttributeValue newval(AttributeList::class_symid(), (void*) al);
	*((Attribute*)before_part.obj_val())->Value() = newval;
      }
    } else
      al = (AttributeList*) before_part.obj_val();

    if (after_nids!=-1) {
      /* al.method(args) -- fire, self-bound, not a plain attribute fetch.
	 copy_stack_arg_post_eval needs the argoff bookmark still on the
	 stack, so it must run before reset_stack(), same as every other
	 stack_arg* read here. */
      int nargtoks;
      postfix_token* argtoks = copy_stack_arg_post_eval(1, nargtoks);
      reset_stack();
      fire_attrlist_method(this, comterp(), al, argtoks, nargtoks);
    } else if (nargs()>1) {
      int after_symid = after_raw.symbol_val();
      if (after_raw.type()==ComValue::StringType) {
        symbol_reference(after_symid);
      }
      reset_stack();
      Attribute* attr = al ? al->GetAttr(after_symid) :  nil;
      if (!attr) {
	attr = new Attribute(after_symid, new AttributeValue());
	al->add_attribute(attr);
      }
      ComValue retval(Attribute::class_symid(), attr);
      push_stack(retval);
    } else {
      reset_stack();
      ComValue retval(AttributeList::class_symid(), al);
      push_stack(retval);
    }
}

void DotFunc::execute() {
    /* dot(:dbg [true|false]) -- get/set dotfunc_debug_expr at runtime, so a
       live session (a running drawserv, say) can turn on the "expression
       before/after dot" detail in the malformed-expression warning without
       an edit+rebuild.  Checked first and unconditionally: an ordinary
       a.b expression never supplies a :dbg keyword, so this never touches
       the normal dispatch path below. */
    static int dbg_symid = symbol_add("dbg");
    static int dbg_bare_symid = symbol_add("__dot_dbg_bare__");
    ComValue dbg_bare_sentinel(dbg_bare_symid, ComValue::SymbolType);
    ComValue dbgv(stack_key_post_eval(dbg_symid, false, dbg_bare_sentinel));
    if (!dbgv.is_unknown()) {
      reset_stack();
      boolean is_bare = dbgv.is_type(ComValue::SymbolType) && dbgv.symbol_val()==dbg_bare_symid;
      if (!is_bare) dotfunc_debug_expr = dbgv.is_true();
      ComValue retval(dotfunc_debug_expr);
      push_stack(retval);
      return;
    }

    ComValue before_part, after_raw;
    int after_nids;
    std::string before_expr_text, after_expr_text;
    peek_and_fire(before_part, after_raw, after_nids, before_expr_text, after_expr_text);
    execute_core(before_part, after_raw, after_nids, before_expr_text, after_expr_text);
}

/*****************************************************************************/

DotNameFunc::DotNameFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotNameFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    reset_stack();
    if (dotted_pair.class_symid() != Attribute::class_symid()) {
        fprintf(stderr, "attrname: argument is not a dotted pair attribute (line %d)\n", funcstate()->linenum());
        push_stack(ComValue::nullval());
        return;
    }
    Attribute *attr = (Attribute*)dotted_pair.obj_val();
    ComValue retval(attr->SymbolId(), ComValue::StringType);
    push_stack(retval);
}

/*****************************************************************************/

DotValFunc::DotValFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotValFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    reset_stack();
    if (dotted_pair.class_symid() != Attribute::class_symid()) {
        fprintf(stderr, "attrval: argument is not a dotted pair attribute (line %d)\n", funcstate()->linenum());
        push_stack(ComValue::nullval());
        return;
    }
    Attribute *attr = (Attribute*)dotted_pair.obj_val();
    push_stack(*attr->Value());
}
