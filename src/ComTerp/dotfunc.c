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
#include <ComTerp/listfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>

#define TITLE "DotFunc"

using std::cout;
using std::cerr;

/*****************************************************************************/

int DotFunc::_symid = -1;

/* off by default -- capturing the pre-fire source text of both args (see
   execute() below) costs a cout redirect + two print_stack_arg_post_eval
   calls on every dot-expression, even the overwhelmingly common case where
   nothing goes wrong.  A malformed-dot warning always shows the RESOLVED
   value of both sides regardless of this flag (before_part/after_raw,
   already on hand -- no capture needed) -- this only adds the raw postfix-
   token dump on top of that, for tracking down something the resolved
   value alone doesn't explain.  Intentionally undocumented/internal: set
   it via check_dbg_keyword()'s :dbg keyword if ever needed again, but it's
   not advertised in DotFunc's public docstring. */
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

/* Unconditional counterpart for captures (#310), not keywords -- a capture
   was never something the caller passed at this call site (unlike a
   keyword, where a resulting object mutation is visible and deliberate to
   whoever wrote :x val), so it must always behave like the bare-call
   case's fresh, disposable per-call seed: revert every time, whether the
   body wrote it or not, never leave a new permanent field on obj.
   Confirmed live: without this, obj.bump=func(c=c+1) on a never-before-
   seen capture c leaked a permanent :c field onto obj and accumulated
   across calls (11, 12, 13) instead of restarting from the frozen capture
   every time (11, 11, 11). */
static void restore_capture(AttributeList* al, KwPending& pending) {
  Attribute* now = al->GetAttr(pending.symid);
  if (!now) return;
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

  /* #310: this funcobj's own declaration-time captures (read-only/
     read-before-write free variables, funcobjscan.h) are ephemeral
     defaults layered onto al the same way keyword args are below --
     apply_kw/restore_capture solve the same "inject, fire, revert"
     problem for captures that apply_kw/restore_kw_if_unwritten already
     solve for keywords, so captures reuse the same mechanism rather than
     mutating obj's real fields. Applied *before* keywords (this block) so
     an explicit :x val keyword still overrides a capture's *value* via
     the same apply_kw call landing on top. */
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
      /* A name obj already owns as its own attribute is a real object
	 field, not the free variable this capture was taken for -- skip
	 it so the field stays live (self-bound reads/writes of obj's own
	 fields must see obj's current value, never a declaration-time
	 snapshot; only a name obj does NOT already have is genuinely
	 free here). Confirmed live: without this guard,
	 obj.increment=func(count=count+1) on obj=(:count 5) overwrote
	 obj's real count with an unrelated (Unknown) capture before the
	 body ran, "Unknown add operand: UnknownType+IntType". */
      if (al->GetAttr(capsymid)) continue;
      /* A name the caller also supplied as an explicit keyword this call
	 is entirely the keyword mechanism's -- established, deliberate,
	 tested behavior (62f557fb, LANGUAGE.md's "Keyword arguments to a
	 method call are ephemeral unless the method writes them"): a
	 self-bound write always persists, keyword-sourced or not. Skip
	 applying the capture at all so apply_kw's own existed/oldval
	 bookkeeping for the keyword below reflects the true pre-call
	 state, not a value this capture injected first -- confirmed live:
	 without this, a name that's both captured and keyword-supplied
	 (nope=nope+1 declared free, then called as .setit(:nope 5)) had
	 the capture's own unconditional revert wipe out the keyword
	 write's result afterward, breaking attrlist.comt test 43. */
      boolean also_keyword = false;
      for (int k=0; k<method_nkey_for_skip; k++)
	if (kwsymids[k]==capsymid) { also_keyword = true; break; }
      if (also_keyword) continue;
      apply_kw(al, capsymid, *capattr->Value(), cappending[ncap]);
      ncap++;
    }
  }
  delete [] kwsymids;

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

  for (int i=0; i<ncap; i++)
    restore_capture(al, cappending[i]);
  delete [] cappending;

  self->push_stack(result);
}

/* #318: resolve before_part (a symbol, Attribute, or already-resolved
   value) to the array/attrlist it names, read-only -- the same lookup
   the integer-index path and the split-decimal-index path below both
   need, factored out so neither has to duplicate it.  Never vivifies:
   a bare-symbol lookup here uses AttributeList::find()/table find(),
   not the insert-a-fresh-attrlist path plain dot-assignment falls back
   to for a non-list value (dotfunc.c's execute_core, further down). */
static boolean resolve_dotted_list(ComTerp* comterp, ComValue& before_part, ComValue& listv) {
  if (before_part.is_array() || before_part.is_attributelist()) {
    listv = before_part;
    return true;
  } else if (before_part.is_attribute()) {
    AttributeValue* av = ((Attribute*)before_part.obj_val())->Value();
    if (av->is_array() || av->is_attributelist()) {
      listv = *av;
      return true;
    }
  } else if (before_part.is_symbol()) {
    int before_symid = before_part.symbol_val();
    boolean global = before_part.global_flag();
    AttributeList* funcscope = !global ? comterp->get_attributes() : nil;
    AttributeValue* curval = funcscope ? funcscope->find(before_symid) : nil;
    if (!curval) {
      void* vptr = nil;
      if (!global) {
	comterp->localtable()->find(vptr, before_symid);
	if (!vptr) comterp->globaltable()->find(vptr, before_symid);
      } else {
	comterp->globaltable()->find(vptr, before_symid);
      }
      if (vptr) curval = (ComValue*) vptr;
    }
    if (curval && (curval->is_array() || curval->is_attributelist())) {
      listv = *curval;
      return true;
    }
  }
  return false;
}

/* #318: one level of read-only lst.N / al.N indexing -- for a plain
   list, the same at() call the ordinary integer-rhs path below makes;
   for an attrlist, the same detached-singleton construction (never the
   live internal Attribute*, see the ordinary path's own comment on why:
   a numeric position is reflection, not a stable write address).
   Shared so the split-decimal-index path below can apply it twice
   without duplicating either case. */
static ComValue read_list_index_once(ComTerp* comterp, ComValue& listv, int idx) {
  if (listv.is_array()) {
    ComValue listcopy(listv);
    ComValue idxv(idx, ComValue::IntType);
    comterp->push_stack(listcopy);
    comterp->push_stack(idxv);
    ListAtFunc atf(comterp);
    atf.exec(2, 0);
    return comterp->pop_stack();
  } else if (listv.is_attributelist()) {
    AttributeList* al = (AttributeList*) listv.obj_val();
    if (al && idx < al->Number()) {
      int count = 0;
      Iterator it;
      for (al->First(it); !al->Done(it); al->Next(it)) {
	if (count==idx) {
	  Attribute* found = al->GetAttr(it);
	  AttributeList* singleton = new AttributeList();
	  singleton->add_attr(found->SymbolId(), *found->Value());
	  return ComValue(AttributeList::class_symid(), (void*) singleton);
	}
	count++;
      }
    }
  }
  return ComValue::blankval();
}

/* #318: lst.0.1 and lst.0.1.2 tokenize with "0.1" merged into a single
   DOUBLE token (ComUtil/_lexscan.c: a decimal point encountered while
   already scanning digits always continues the token into a float --
   there is no way to tell "0.1" apart from "0" DOT "1" at the lexer,
   and postfix_token only ever stores the parsed value, never the
   source text, so there's nothing to recover downstream either).

   Formats with %.9f (always fixed notation, never scientific, always
   has a decimal point) rather than %g: %g's shortest-round-trip
   behavior collapses a whole-valued double like 1.0 to the bare text
   "1" -- no decimal point at all -- which would wrongly reject
   lst.1.0.2 (a real 3-deep chain: index 1, then index 0, then index
   2) as not looking like int.frac in the first place. Trailing zeros
   are stripped back off (keeping at least one digit) after formatting
   to recover "1" from "1.000000000", "34" from "0.340000000", etc.

   Requires a nonzero fractional part. This is a real, deliberate
   restriction, not just a simplification: 1e0 and 1.0 are the exact
   same double once parsed (scientific notation is a source-text
   distinction lost before this ever runs, same loss as the
   digit-count one below), so there is no way to accept "lst.1.0" as
   a legitimate 0-index chain step while also rejecting "lst.1e0" as
   requested -- they are literally the same bits. Requiring frac!=0
   resolves that the only way available: any whole-valued double,
   however it was written, is rejected outright rather than guessed
   at. lst.1.0.2 as a chain needs a different spelling (e.g. explicit
   at() calls) as a result -- a known, accepted trade-off, not an
   oversight.

   Beyond that: a fractional part that isn't clearly two short digit
   groups (scientific notation; negative; anything that doesn't
   reconstruct to the original value) is rejected outright rather than
   guessed at -- this recovers the common "chained single-digit
   indices" case, not a general float-as-index feature, and it can't
   be more than that: how many digits followed the decimal point is
   information already lost by the time this runs (0.1 and 0.10 are
   the identical double), so index 10 can never be reliably
   distinguished from index 1 this way -- out of scope, same as at()'s
   own integer-only contract. */
static boolean split_decimal_index(double v, int& intpart, int& fracpart) {
  if (v < 0) return false;
  if (v >= 1e9) return false;
  char buf[64];
  snprintf(buf, sizeof(buf), "%.9f", v);
  char* dot = strchr(buf, '.');
  if (!dot) return false;
  *dot = '\0';
  const char* intstr = buf;
  char* fracstr = dot + 1;
  int fraclen = (int)strlen(fracstr);
  while (fraclen > 1 && fracstr[fraclen-1] == '0') fracstr[--fraclen] = '\0';
  if (!*intstr || !*fracstr) return false;
  for (const char* p = intstr; *p; p++) if (!isdigit((unsigned char)*p)) return false;
  for (const char* p = fracstr; *p; p++) if (!isdigit((unsigned char)*p)) return false;
  intpart = atoi(intstr);
  fracpart = atoi(fracstr);
  if (fracpart == 0) return false;
  double recon = intpart + fracpart / pow(10.0, (double)strlen(fracstr));
  return fabs(recon - v) < 1e-6;
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
    /* #318: lst.N / al.N -- a numeric rhs on a list or attrlist is sugar
       for at(lst N), read-only.  Checked first, ahead of both the ordinary
       validation below (which would otherwise reject a numeric rhs
       outright -- see the "needs to be a symbol" warning further down)
       and the symbol/attribute lookup that follows it, which -- for a
       bare symbol or attribute currently holding a plain list rather than
       an AttributeList -- destructively replaces that value with a fresh
       empty AttributeList (the existing auto-vivify behavior dot-
       assignment relies on, e.g. `x.foo=1` on an unset `x`; wrong here,
       since `lst.3` must read `lst`'s existing list, not blow it away).
       Peeking at a symbol's or attribute's current value this way is
       read-only (AttributeList::find, not insert), so nothing is created
       or mutated when this doesn't match and falls through to the
       existing logic below.

       Deliberately not gated on after_nids==-1 the way the malformed-rhs
       check further down is: nids defaults to 0 (zero_vals()) for a bare
       numeric literal rather than the -1 a plain symbol token carries, so
       that guard would silently exclude every numeric rhs and fall
       through into the arglist-attached path below with a numeric token
       standing in for a method name -- confirmed live, produced
       nonsense "\"<garbage>\" is not a func-valued attribute" warnings.
       A numeric literal can never have an arglist attached in valid
       syntax regardless of what nids happens to hold, so is_num() alone
       is the correct and sufficient guard here.

       Positive/zero only, matching at()'s own limit (ListAtFunc::execute,
       listfunc.c, requires nv.int_val()>=0 for the ArrayType case) --
       negative indexing isn't a thing this sugar adds on top of at(),
       just what at() itself already supports.  In practice a literal
       negative rhs (lst.-1) never reaches here as a single numeric
       token anyway (the lexer has no negative-literal token; "-1" is
       always unary minus applied to 1, so after_raw arrives as an
       unevaluated CommandType reference to "minus", not IntType) --
       this check is belt-and-suspenders against that changing, not
       covering a case observed in practice.

       is_integer(), not is_num(): a floating-point rhs is never a
       valid single index (only positive integers are) -- handled
       separately below via split_decimal_index() instead, since
       lst.0.1 tokenizes with "0.1" merged into one DOUBLE token (see
       that function's own comment) and silently truncating it here
       (int_val() on 0.1 would give 0) would be wrong, not just
       imprecise. */
    if (after_raw.is_integer() && after_raw.int_val()>=0) {
      ComValue listv;
      boolean have_list = resolve_dotted_list(comterp(), before_part, listv);
      if (have_list && listv.is_array()) {
	/* lhs_assign() is set by AssignFunc (assignfunc.c) on this dot
	   command's own token whenever it starts a "something.something"
	   assignment lhs -- checked the same way GlobalFunc/LocalFunc
	   already do (symbolfunc.c) -- before this evaluation's own
	   reset_stack().  For lst.N=val on a plain list, don't read at
	   all: hand back the list plus the target index so AssignFunc can
	   build and fire at(list idx :set val) itself -- a plain list has
	   no per-element Attribute the way a named attrlist key does, so
	   there's nothing here to return that the existing
	   Attribute-write-through path could use directly (#318).

	   Offset is nkeys()+1, not nargs()+nkeys() the way symbolfunc.c's
	   GlobalFunc/LocalFunc check it -- confirmed live (dumping
	   stack_top(0..5) for lst.N=val): that formula only coincidentally
	   matches nkeys()+1 for a 1-positional-arg command; dot has 2
	   (before, after), and its own flagged token sits at stack_top(1)
	   regardless, not stack_top(nargs()+nkeys())=stack_top(2).

	   The list+index pair travels as a tiny 2-element ArrayValueList
	   [list, idx], not stashed in a spare ComValue field (nids() was
	   tried first: confirmed live that it does NOT survive the round
	   trip -- something along the push/copy path recomputes it for an
	   ArrayType value, so AssignFunc saw a stale/unrelated number, not
	   the index this pushed). Reusing AttributeValueList's own,
	   already-safe element storage avoids that -- no custom field
	   semantics to fight, and no ownership hazard either: aliasing the
	   array's own internal AttributeValue* (e.g. via avl->Get(idx),
	   wrapped in a throwaway Attribute for the existing write-through
	   branch to use) was the other option considered and rejected --
	   Attribute owns and deletes its Value() on destruction, and so
	   does the array itself, a double-free waiting to happen. */
	boolean assign_lhs = comterp()->stack_top(nkeys()+1).lhs_assign();
	if (assign_lhs) {
	  reset_stack();
	  AttributeValueList* pair = new AttributeValueList();
	  pair->Append(new AttributeValue(listv));
	  pair->Append(new AttributeValue(after_raw.int_val(), AttributeValue::IntType));
	  ComValue retval(pair);
	  retval.lhs_assign(1);
	  push_stack(retval);
	  return;
	}
	reset_stack();
	push_stack(listv);
	push_stack(after_raw);
	ListAtFunc atf(comterp());
	atf.exec(2, 0);
	return;
      } else if (have_list) {
	/* AttributeList: a numeric position is reflection/inspection only
	   -- "the indexed tour of an attrlist will be by attrlist
	   singletons" -- never a stable write address, so this always
	   returns a fresh, detached one-entry AttributeList copy rather
	   than ListAtFunc's own live Attribute* reference into the
	   original (which -- confirmed live -- would otherwise let
	   al.1=val silently overwrite whatever attribute happens to sit
	   at position 1 today, e.g. :b, purely by position; positions
	   shift as attributes are added/removed, so that's not an
	   address worth honoring as an lvalue). Same outcome whether or
	   not lhs_assign() is set: al.1=val falls through to the
	   ordinary "not a symbol or attribute" warning, same as any
	   other non-writable rhs. */
	AttributeList* al = (AttributeList*) listv.obj_val();
	int nvv = after_raw.int_val();
	AttributeList* singleton = nil;
	if (al && nvv < al->Number()) {
	  int count = 0;
	  Iterator it;
	  for (al->First(it); !al->Done(it); al->Next(it)) {
	    if (count==nvv) {
	      Attribute* found = al->GetAttr(it);
	      singleton = new AttributeList();
	      singleton->add_attr(found->SymbolId(), *found->Value());
	      break;
	    }
	    count++;
	  }
	}
	reset_stack();
	if (singleton) {
	  ComValue retval(AttributeList::class_symid(), (void*) singleton);
	  push_stack(retval);
	} else
	  push_stack(ComValue::blankval());
	return;
      }
    } else if (after_raw.is_floatingpoint()) {
      /* lst.0.1 / lst.0.1.2 -- see split_decimal_index()'s own comment
	 for why this is needed at all.  Read-only, like the plain
	 AttributeList case above: no lhs_assign() handling here, so
	 lst.0.1=val falls through to the ordinary "not a symbol or
	 attribute" warning rather than attempting a chained write --
	 out of scope for this recovery, which exists for indexing, not
	 assignment.

	 Rejection (1e0; a value that doesn't reconstruct cleanly) is
	 handled explicitly here rather than by falling through to the
	 generic checks further down -- confirmed live that falling
	 through hits the exact same pre-existing trap the integer path
	 above was already written to avoid (its own "is_integer(), not
	 is_num()" comment): a bare numeric rhs has nids==0, not the -1
	 the malformed-rhs check below is gated on, so it silently skips
	 that check and reaches fire_attrlist_method instead, producing
	 a nonsense "\"<garbage>\" is not a func-valued attribute"
	 warning -- and, worse, left stack residue that corrupted every
	 statement after it in the same script (confirmed live: a
	 correctly-working lst.1 on the very next line printed blank
	 until this was fixed). */
      int intpart, fracpart;
      ComValue listv;
      if (split_decimal_index(after_raw.double_val(), intpart, fracpart) &&
	  resolve_dotted_list(comterp(), before_part, listv)) {
	ComValue mid = read_list_index_once(comterp(), listv, intpart);
	ComValue result = (mid.is_array() || mid.is_attributelist())
	  ? read_list_index_once(comterp(), mid, fracpart)
	  : ComValue::blankval();
	reset_stack();
	push_stack(result);
	return;
      }
      cout << "WARNING: expression after \".\" is not a valid list index ("
	   << after_raw.double_val() << ") -- only positive integers, or "
	      "chained ones merged by a decimal point (lst.0.1), are -- line "
	   << funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
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

boolean DotFunc::check_dbg_keyword() {
    /* Undocumented/internal: get/set dotfunc_debug_expr at runtime via a
       :dbg keyword, so a live session (a running drawserv, say) can turn
       on the raw postfix-token dump ON TOP OF the resolved-value detail
       the malformed-dot warning already always shows, without an
       edit+rebuild -- a fallback for a stranger case than the resolved
       value alone explains, not something to advertise.  Checked first
       and unconditionally: an ordinary a.b expression never supplies a
       :dbg keyword, so this never touches the normal dispatch path
       below. */
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
