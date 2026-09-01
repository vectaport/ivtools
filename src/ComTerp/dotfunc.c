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
#include <ComTerp/strmfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <iostream>
#include <fstream>
#include <sstream>

#define TITLE "DotFunc"

using std::cout;
using std::cerr;

/*****************************************************************************/


/* off by default: capturing the pre-fire source text of both args costs a
   cout redirect and two print_stack_arg_post_eval calls on every dot
   expression.  A malformed-dot warning shows the resolved value of both sides
   regardless; this only adds the raw postfix-token dump on top.  Internal --
   set it through check_dbg_keyword()'s :dbg keyword if needed. */
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
  eqf.funcid(symbol_add("eq"));
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

/* the unconditional counterpart for captures rather than keywords: a capture
   was never something the caller passed at this call site, so it reverts every
   time, written or not, and never leaves a permanent field on obj.  Otherwise
   obj.bump=func(c=c+1) on a never-before-seen c leaves a :c field behind that
   accumulates across calls instead of restarting from the capture. */
static void restore_capture(AttributeList* al, KwPending& pending) {
  Attribute* now = al->GetAttr(pending.symid);
  if (!now) return;
  if (pending.existed)
    *now->Value() = pending.oldval;
  else
    al->Remove(now);
}

/* obj.method(args) -- fire a FuncObj-valued attribute self-bound to obj.  Args
   are evaluated in the caller's own scope, before any _alist swap, so a
   variable in an arg resolves against the caller rather than obj.  _alist is
   then swapped to obj -- the mechanism eval(fo :alist obj) already uses -- and
   the method's token buffer run directly, so self-bound reads and writes
   mutate the real object.  Positionals flow through the funcobj_args channel,
   as for any FuncObj call.

   "method" itself is never evaluated as a symbol: a symbol with an arglist
   that is not a registered global command is rebound to nil at conversion
   time, and that check consults the global command table only, never _alist.
   So look "method" up in obj directly, and get the args evaluated by
   retargeting a copy of just the arg tokens at echo(). */
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

  /* this funcobj's declaration-time captures are ephemeral defaults layered
     onto al the way keyword args are below, so they reuse the same inject-
     fire-revert mechanism rather than mutating obj's real fields.  Applied
     before keywords, so an explicit :x val still overrides a capture. */
  int method_nkey_for_skip = method_nkey;
  int* kwsymids = method_nkey_for_skip>0 ? new int[method_nkey_for_skip] : nil;
  if (method_nkey_for_skip>0) {
    if (poslist) {
      for (int i=0; i<method_nkey_for_skip; i++) {
	AttributeList* singleton = (AttributeList*) poslist->Get(npos+i)->obj_val();
	ALIterator it;
	singleton->First(it);
	kwsymids[i] = singleton->GetAttr(it)->SymbolId();
      }
    } else if (kwlist) {
      ALIterator it;
      int i = 0;
      for (kwlist->First(it); !kwlist->Done(it); kwlist->Next(it), i++)
	kwsymids[i] = kwlist->GetAttr(it)->SymbolId();
    }
  }

  int ncap = 0;
  KwPending* cappending = nil;
  if (fo->captures().is_object(AttributeList::class_symid())) {
    AttributeList* caps = (AttributeList*) fo->captures().obj_val();
    cappending = caps->Number()>0 ? new KwPending[caps->Number()] : nil;
    ALIterator capit;
    for (caps->First(capit); !caps->Done(capit); caps->Next(capit)) {
      Attribute* capattr = caps->GetAttr(capit);
      int capsymid = capattr->SymbolId();
      /* a name obj already owns is a real object field, not the free variable
	 this capture was taken for -- skip it, so self-bound reads and writes
	 see obj's current value rather than a declaration-time snapshot.
	 Without the guard, obj.increment=func(count=count+1) on obj=(:count 5)
	 overwrites the real count with an Unknown capture. */
      if (al->GetAttr(capsymid)) continue;
      /* a name the caller also supplied as an explicit keyword belongs to the
	 keyword mechanism entirely -- a self-bound write persists whether it
	 came from a keyword or not.  Skip the capture, so apply_kw's own
	 existed/oldval bookkeeping reflects the true pre-call state; applied
	 first, the capture's unconditional revert would wipe out the keyword
	 write afterward. */
      boolean also_keyword = false;
      for (int k=0; k<method_nkey_for_skip; k++)
	if (kwsymids[k]==capsymid) { also_keyword = true; break; }
      if (also_keyword) continue;
      apply_kw(al, capsymid, *capattr->Value(), cappending[ncap]);
      ncap++;
    }
  }
  delete [] kwsymids;

  /* keyword args are ephemeral unless the method's body writes that name:
     reading or ignoring it leaves al as it was, and only a self-bound write
     makes it stick.  Apply each keyword now, saving what it replaces, fire,
     then compare and revert -- unchanged means nothing wrote it. */
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

  for (int i=0; i<ncap; i++)
    restore_capture(al, cappending[i]);
  delete [] cappending;

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
      /* a real command reference, such as the inner dot of node.left.val --
	 fire it for its value.  A bare unbound symbol is never promoted to
	 CommandType, so this never fires on a name the lookup below needs raw.
	 symbol=false, since this needs pop_stack's full finalization rather
	 than the raw result. */
      before_part = stack_arg_post_eval(0);
    } else if (before_part.type()==ComValue::BlankType) {
      /* a parenthesized sub-expression whose own result is a never-yet-pulled
	 stream reads back as BlankType through the stack_arg(0,true) peek
	 above: streams do not self-evaluate mid-expression, so a Blank
	 placeholder stands in.  stack_arg_post_eval(0) -- the same firing used
	 for the CommandType case above -- recovers the real StreamType value,
	 which the LHS-stream support below needs. */
      before_part = stack_arg_post_eval(0);
    }
    after_raw = stack_arg(1, true);
    after_nids = after_raw.nids();
}

void DotFunc::execute_core(ComValue before_part, ComValue after_raw, int after_nids,
			    const std::string& before_expr_text, const std::string& after_expr_text,
			    boolean force_named_field) {
    /* a named variable bound to a stream (sb=$$barnyard; sb.calls) arrives
       as a raw SymbolType, since execute_core() below does its own symbol
       lookup.  Resolve a copy far enough to test is_stream(); before_part
       stays untouched, so the ordinary path below still gets the bare
       symbol it knows how to resolve. */
    ComValue before_resolved = before_part;
    if (before_resolved.is_symbol()) {
      AttributeValue* rv = comterp()->lookup_symval(&before_resolved, false);
      if (rv) before_resolved = ComValue(*rv);
    }
    if (before_resolved.is_stream() && after_nids==-1) {
      /* (stream).field -- lazy, pulling .field from each element on
	 demand rather than erroring on a raw StreamType before_part.  Method-call-with-streaming-LHS (stream.method(args))
	 isn't handled here yet -- that falls through to the ordinary
	 validity check below, which correctly warns rather than
	 mishandling it silently. */
      reset_stack();
      int after_symid = after_raw.symbol_val();
      if (after_raw.type()==ComValue::StringType) symbol_reference(after_symid);
      static DotStreamNextFunc* dsnfunc = nil;
      if (!dsnfunc) {
	dsnfunc = new DotStreamNextFunc(comterp());
	dsnfunc->funcid(symbol_add("dotstreamnext"));
      }
      AttributeValueList* avl = new AttributeValueList();
      avl->Append(new AttributeValue(before_resolved));                      // [0] underlying before-stream (resolved)
      avl->Append(new AttributeValue(after_symid, AttributeValue::SymbolType)); // [1] fixed field symbol
      ComValue stream(dsnfunc, avl);
      stream.stream_mode(STREAM_INTERNAL); // for internal use (use by DotStreamNextFunc)
      push_stack(stream);
      return;
    }
    if (!before_part.is_symbol() &&
	!(before_part.is_attribute() &&
	  (((Attribute*)before_part.obj_val())->Value()->is_unknown() ||
	  ((Attribute*)before_part.obj_val())->Value()->is_attributelist())) &&
	!before_part.is_attributelist()) {

      /* A list before "." is the common case in practice (e.g.
	 zoo.who("Ellie").moves, dotting straight into a query result
	 instead of unwrapping it with at() first) -- worth a specific,
	 actionable message rather than the generic type-mismatch one
	 below, and empty vs non-empty call for different advice. */
      if (before_part.is_array()) {
	AttributeValueList* avl = before_part.array_val();
	int n = avl ? avl->Number() : 0;
	if (n == 0)
	  cout << "WARNING: nothing before \".\" to look up -- the list is empty";
	else
	  cout << "WARNING: expression before \".\" is a list of " << n
	       << " item" << (n == 1 ? "" : "s")
	       << " -- pick one with at(...) before dotting into it";
	cout << " -- line " << funcstate()->linenum() << "\n";
	cout << "expression before dot:  " << before_part << "\n";
	if (dotfunc_debug_expr)
	  cout << "raw expr before dot:  " << before_expr_text;
	cout << "expression after dot:  " << after_raw << "\n";
	reset_stack();
	push_stack(ComValue::nullval());

	return;
      }

      cout << "WARNING: expression before \".\" needs to evaluate to a symbol or <AttributeList> (instead of "
	   << symbol_pntr(before_part.type_symid());
      if (before_part.is_object())
        cout << " of class " << symbol_pntr(before_part.class_symid());
      cout << ") -- line " << funcstate()->linenum() << "\n";
      cout << "expression before dot:  " << before_part << "\n";
      if (dotfunc_debug_expr)
        cout << "raw expr before dot:  " << before_expr_text;
      cout << "expression after dot:  " << after_raw << "\n";
      reset_stack();
      push_stack(ComValue::nullval());

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
      cout << "expression after dot:  " << after_raw << "\n";
      if (dotfunc_debug_expr)
        cout << "raw expr after dot:  " << after_expr_text;
      reset_stack();
      push_stack(ComValue::nullval());
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
	 body. */
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
    } else if (force_named_field || nargs()>1) {
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

boolean DotFunc::check_dbg_keyword() {
    /* internal: get or set dotfunc_debug_expr at runtime through a :dbg
       keyword, so a live session can turn on the raw postfix-token dump
       without a rebuild.  Checked first and unconditionally -- an ordinary
       a.b expression never supplies :dbg, so this never touches the normal
       dispatch below. */
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
      return true;
    }
    return false;
}

void DotFunc::execute() {
    if (check_dbg_keyword()) return;

    ComValue before_part, after_raw;
    int after_nids;
    std::string before_expr_text, after_expr_text;
    peek_and_fire(before_part, after_raw, after_nids, before_expr_text, after_expr_text);
    execute_core(before_part, after_raw, after_nids, before_expr_text, after_expr_text);
}

/*****************************************************************************/

DotStreamNextFunc::DotStreamNextFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void DotStreamNextFunc::execute() {
    /* invoked by the next mechanism -- arg 0 is our own stream, carrying
       [0] the underlying before-stream and [1] the fixed after-dot field
       symbol in its stream_list() (see the STREAM_INTERNAL construction
       in DotFunc::execute_core() above). */
    /* deliberately no reset_stack() here, unlike the other *NextFunc
       siblings: this one delegates to execute_core() below, which does its
       own single reset in whichever branch it takes.  A second reset would
       decr_stack(1) again and cancel out the push_stack() below, netting the
       call to zero growth so the caller substitutes a blank. */
    ComValue selfstream(stack_arg(0));

    AttributeValueList* avl = selfstream.stream_list();
    if (!avl) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    Iterator i;
    avl->First(i);
    AttributeValue* beforeval = avl->GetAttrVal(i);   // [0] underlying before-stream
    avl->Next(i);
    AttributeValue* afterval = avl->GetAttrVal(i);    // [1] fixed field symbol

    ComValue before_next;
    if (beforeval->is_stream()) {
      /* same temporary-copy-then-drive pattern ConcatNextFunc/
	 ReplayNextFunc already use: the copy shares the same underlying
	 (ref-counted) stream_list() as the original, so advancing it via
	 NextFunc::execute_impl persists back through *beforeval on the
	 next pull, without this function needing to mutate *beforeval
	 itself. */
      ComValue beforecopy(*beforeval);
      NextFunc::execute_impl(comterp(), beforecopy);
      if (comterp()->stack_top().is_unknown()) {
	comterp()->pop_stack();
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }
      before_next = comterp()->pop_stack();
    } else {
      /* not exercised by the LHS-only case this lands in (the first
	 slice) -- kept generic so a future RHS/zip extension (a fixed,
	 non-stream "before" reused every pull while args advance) can
	 reuse this same next-func without a second implementation. */
      before_next = ComValue(*beforeval);
    }

    ComValue after_raw(afterval->symbol_val(), ComValue::SymbolType);
    execute_core(before_next, after_raw, -1, "", "", true);

    /* execute_core()'s named-field branch pushes the raw dotted-pair
       Attribute* wrapper, as an ordinary .field access does.  A normal read
       unwraps that further up the call chain, but this synthetic per-pull
       call has no such caller, so unwrap explicitly -- otherwise each pulled
       element is the wrapper rather than its value. */
    ComValue unwrapped(comterp()->pop_stack(true));
    push_stack(unwrapped);
}

/*****************************************************************************/

/* attrname()/attrval() accept either shape a single attribute takes on the
   stack: the internal dotted-pair Attribute* that "." exposes for a named
   lookup -- the language has no attribute literal, so this is how one lands
   on the stack at all -- or a single-entry AttributeList, which is what at()
   and "@" return for an attrlist position.  nil if neither shape matches, or
   the list holds other than one entry. */
static Attribute* dotted_pair_or_singleton_attr(ComValue& val) {
    if (val.class_symid() == Attribute::class_symid())
        return (Attribute*)val.obj_val();
    if (val.is_object(AttributeList::class_symid())) {
        AttributeList* al = (AttributeList*)val.obj_val();
        if (al && al->Number()==1) {
            ALIterator it;
            al->First(it);
            return al->GetAttr(it);
        }
    }
    return nil;
}

DotNameFunc::DotNameFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotNameFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    /* stack_arg's symbol=true skips ordinary resolution, so an inline at()
       or "." result is not run back through lookup_symval()'s dotted-pair
       unwrapping.  The cost is that a bound variable argument -- attrname(x)
       rather than attrname(at(al 0)) -- arrives as a raw SymbolType and fails
       the shape check below.  Resolve it here, but only while it is still a
       symbol, which cannot reintroduce the case symbol=true avoids. */
    if (dotted_pair.type() == ComValue::SymbolType)
        lookup_symval(dotted_pair);
    reset_stack();
    Attribute* attr = dotted_pair_or_singleton_attr(dotted_pair);
    if (!attr) {
        fprintf(stderr, "attrname: argument is not a dotted pair attribute or single-entry attrlist (line %d)\n", funcstate()->linenum());
        push_stack(ComValue::nullval());
        return;
    }
    ComValue retval(attr->SymbolId(), ComValue::StringType);
    push_stack(retval);
}

/*****************************************************************************/

DotValFunc::DotValFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotValFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    /* see DotNameFunc::execute()'s identical resolve-if-still-a-symbol
       comment above -- same fix, same reasoning. */
    if (dotted_pair.type() == ComValue::SymbolType)
        lookup_symval(dotted_pair);
    reset_stack();
    Attribute* attr = dotted_pair_or_singleton_attr(dotted_pair);
    if (!attr) {
        fprintf(stderr, "attrval: argument is not a dotted pair attribute or single-entry attrlist (line %d)\n", funcstate()->linenum());
        push_stack(ComValue::nullval());
        return;
    }
    push_stack(*attr->Value());
}
