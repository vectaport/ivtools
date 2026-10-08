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

/* KwPending, apply_kw, and restore_kw_if_unwritten are declared in
   dotfunc.h -- ComTerp::fire_funcobj (comterp.c) reuses them for a bare
   call to a func with a home attrlist, the same "ephemeral unless
   written" injection this file uses below for obj.method(args). */
void apply_kw(AttributeList* al, int symid, AttributeValue& newval, KwPending& pending) {
  pending.symid = symid;
  Attribute* existing = al->GetAttr(symid);
  pending.existed = existing!=nil;
  if (pending.existed) pending.oldval = *existing->Value();
  pending.injectedval = newval;
  al->add_attr(symid, newval);
}

void restore_kw_if_unwritten(ComTerp* comterp, AttributeList* al, KwPending& pending) {
  Attribute* now = al->GetAttr(pending.symid);
  if (!now || !values_equal(comterp, *now->Value(), pending.injectedval))
    return;   // written during the call (or vanished outright) -- leave it
  if (pending.existed)
    *now->Value() = pending.oldval;
  else
    al->Remove(now);
}

/* a FuncObj used as a dot target exposes its own captures as a sealed
   attrlist: existing entries read/write, no new ones can be added. */
static AttributeList* funcobj_dot_attrlist(AttributeValue* funcval) {
  FuncObj* fo = (FuncObj*) funcval->obj_val();
  AttributeList* caps;
  if (fo->captures().is_object(AttributeList::class_symid()))
    caps = (AttributeList*) fo->captures().obj_val();
  else {
    caps = new AttributeList();
    fo->captures() = ComValue(AttributeList::class_symid(), (void*)caps);
  }
  caps->sealed(true);
  return caps;
}

/* the parser's placeholder for "nothing here" (e.g. a trailing dot with no
   field after it) -- DotFunc treats it as if the rhs were simply absent.
   nids()==0 tells this apart from a genuine call to the "empty" command
   with no args (e.g. "f.empty()"): the placeholder's token is emitted with
   a literal 0 there (_parser.c), while an actual parsed call always gets
   a nonzero nids. */
static boolean is_blank_rhs(ComValue& after_raw) {
  static int empty_symid = symbol_add("empty");
  return after_raw.is_command() && after_raw.command_symid()==empty_symid &&
    after_raw.narg()==0 && after_raw.nkey()==0 && after_raw.nids()==0;
}

/* the counterpart for captures rather than keywords: a capture was never
   something the caller passed at this call site, so its injected entry
   always reverts or is removed from obj -- it never leaves a permanent
   field there.  Otherwise obj.bump=func(c=c+1) on a never-before-seen c
   leaves a :c field behind on obj instead of staying private to the
   closure.  Before reverting, the body's possibly-mutated value is written
   back into the FuncObj's own captures list, so the closure's next call
   starts from wherever this call left it. */
void restore_capture(AttributeList* al, KwPending& pending, AttributeList* captures) {
  Attribute* now = al->GetAttr(pending.symid);
  if (now) {
    Attribute* capattr = captures->GetAttr(pending.symid);
    if (capattr) *capattr->Value() = *now->Value();
  }
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
/* fires fo once, self-bound to al, with one resolved set of positional/
   keyword args -- the inject-fire-revert dispatch shared by a direct
   obj.method(args) call and, per pulled element, a streamed one. */
/* add_attr() never consults sealed() -- dispatch straight at the real
   list, seal untouched, so a nested named-field write (even through an
   alias) still rejects for the whole call. */
/* seal_snapshot/seal_strip_new bracket each individual firing, not just
   the call that creates a deferred stream, so a per-pull body run through
   DotMethodNextFunc is covered too. */
/* presymids rides in an ArrayType AttributeValue, not a raw int*, since
   dup_as_needed() (attrvalue.c) deep-copies ArrayType on a $$ stream copy --
   each copy then frees its own list instead of sharing one raw pointer. */
static AttributeValueList* seal_snapshot(AttributeList* al, boolean& was_sealed) {
  was_sealed = al && al->sealed();
  if (!was_sealed) return nil;
  AttributeValueList* presymids = new AttributeValueList();
  ALIterator pit;
  for (al->First(pit); !al->Done(pit); al->Next(pit))
    presymids->Append(new AttributeValue(al->GetAttr(pit)->SymbolId(), AttributeValue::IntType));
  return presymids;
}

static void seal_strip_new(AttributeList* al, boolean was_sealed, AttributeValueList* presymids) {
  if (!was_sealed) return;
  Attribute** newattrs = new Attribute*[al->Number()];
  int nnewattrs = 0;
  ALIterator it;
  for (al->First(it); !al->Done(it); al->Next(it)) {
    Attribute* attr = al->GetAttr(it);
    int symid = attr->SymbolId();
    boolean was_present = false;
    int npresymids = presymids ? presymids->Number() : 0;
    for (int i=0; i<npresymids; i++)
      if (presymids->Get(i)->int_val()==symid) { was_present = true; break; }
    if (!was_present) newattrs[nnewattrs++] = attr;
  }
  for (int i=0; i<nnewattrs; i++)
    al->Remove(newattrs[i]);
  delete [] newattrs;
}

/* was_sealed/presymids: a seal_snapshot the caller took before evaluating
   any argument, since an argument's own side effect (e.g.
   eval(... :alist dot(obj))) can add a field to al before this call starts. */
static ComValue fire_attrlist_method_once(ComFunc* self, ComTerp* comterp,
					   AttributeList* al, FuncObj* fo,
					   int method_nkey,
					   AttributeValueList* poslist,
					   AttributeList* kwlist, int npos,
					   boolean was_sealed,
					   AttributeValueList* presymids) {
  ComValue* posvals = npos>0 ? new ComValue[npos] : nil;
  if (npos>0) {
    for (int i=0; i<npos; i++)
      posvals[i] = *poslist->Get(i);
  }

  /* captures are applied via the same inject-fire-revert mechanism as
     keywords, but first, so an explicit :x still overrides a capture */
  int* kwsymids = method_nkey>0 ? new int[method_nkey] : nil;
  if (method_nkey>0) {
    if (poslist) {
      for (int i=0; i<method_nkey; i++) {
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
      /* skip a capture whose name obj already owns as a real field,
         so self-bound reads/writes see obj's current value, not stale */
      if (al->GetAttr(capsymid)) continue;
      /* skip the capture when the caller also supplied it as a keyword,
         so apply_kw's existed/oldval reflects the true pre-call state */
      boolean also_keyword = false;
      for (int k=0; k<method_nkey; k++)
	if (kwsymids[k]==capsymid) { also_keyword = true; break; }
      if (also_keyword) continue;
      apply_kw(al, capsymid, *capattr->Value(), cappending[ncap]);
      ncap++;
    }
  }
  delete [] kwsymids;

  /* keyword args are ephemeral unless the method's body writes that
     name -- apply, fire, then revert whatever the call left unchanged */
  int nkw = method_nkey;
  KwPending* kwpending = nkw>0 ? new KwPending[nkw] : nil;
  if (nkw>0) {
    if (poslist) {
      for (int i=0; i<nkw; i++) {
	AttributeList* singleton = (AttributeList*) poslist->Get(npos+i)->obj_val();
	ALIterator it;
	singleton->First(it);
	Attribute* a = singleton->GetAttr(it);
	ComValue kwval(*a->Value());
	kwval.kwoverride(1);
	apply_kw(al, a->SymbolId(), kwval, kwpending[i]);
      }
    } else if (kwlist) {
      ALIterator it;
      int i = 0;
      for (kwlist->First(it); !kwlist->Done(it); kwlist->Next(it), i++) {
	Attribute* a = kwlist->GetAttr(it);
	ComValue kwval(*a->Value());
	kwval.kwoverride(1);
	apply_kw(al, a->SymbolId(), kwval, kwpending[i]);
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

  ComValue result(self->comterpserv()->run_funcobj_body(fo));

  comterp->set_funcobj_args(saved_argvals, saved_nargs, saved_active);
  delete [] posvals;

  comterp->set_attributes(old_alist);
  Unref(old_alist);

  /* a keyword that also names a capture persists here, bypassing the
     ephemeral-keyword revert below; !existed excludes a name that is a
     real pre-existing field on obj rather than pure closure state. */
  AttributeList* fo_captures = fo->captures().is_object(AttributeList::class_symid()) ?
    (AttributeList*) fo->captures().obj_val() : nil;
  for (int i=0; i<nkw; i++) {
    if (fo_captures && !kwpending[i].existed) {
      Attribute* capattr = fo_captures->GetAttr(kwpending[i].symid);
      Attribute* now = capattr ? al->GetAttr(kwpending[i].symid) : nil;
      /* an untouched keyword override skips persistence -- kwoverride()
         is cleared by any ordinary write, this call's own included */
      if (now && !((ComValue*)now->Value())->kwoverride())
        *capattr->Value() = *now->Value();
    }
    restore_kw_if_unwritten(comterp, al, kwpending[i]);
  }
  delete [] kwpending;

  for (int i=0; i<ncap; i++)
    restore_capture(al, cappending[i], fo_captures);
  delete [] cappending;

  /* a written keyword persists onto the callee's own captures above, but
     must never also become a permanent field of the sealed receiver. */
  seal_strip_new(al, was_sealed, presymids);

  return result;
}

/* a stream-valued arg makes echo() (an eager command) overdrive just like
   any other eager command with a stream operand (comterp.c's general scan)
   -- echoresult arrives as a deferred external stream of per-element echo()
   results, instead of one resolved list/attrlist, so fire_attrlist_method
   hands it to DotMethodNextFunc to drive lazily, one fo firing per pull,
   rather than dropping it. */
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

  /* snapshot before evaluating any argument -- an argument's own side
     effect (e.g. eval(... :alist dot(obj))) can add a field to a sealed
     al, and that must count as pre-existing, not as this call's own. */
  boolean was_sealed;
  AttributeValueList* presymids = seal_snapshot(al, was_sealed);

  /* echoresult owns poslist's/kwlist's storage -- keep it alive across
     this whole block, not just the extraction below */
  ComValue echoresult;
  AttributeValueList* poslist = nil;
  AttributeList* kwlist = nil;
  int npos = 0;
  if (method_narg>0 || method_nkey>0) {
    static int echo_symid = symbol_add("echo");
    method_tok.v.symbolid = echo_symid;
    echoresult = self->comterpserv()->run(argtoks, nargtoks);
    if (echoresult.is_stream()) {
      delete [] argtoks;
      static DotMethodNextFunc* dmnfunc = nil;
      if (!dmnfunc) {
	dmnfunc = new DotMethodNextFunc(comterp);
	dmnfunc->funcid(symbol_add("dotmethodnext"));
      }
      AttributeValueList* avl = new AttributeValueList();
      avl->Append(new AttributeValue(echoresult));
      avl->Append(new AttributeValue(AttributeList::class_symid(), (void*)al));
      avl->Append(new AttributeValue(FuncObj::class_symid(), (void*)fo));
      avl->Append(new AttributeValue(method_nkey, AttributeValue::IntType));
      avl->Append(new AttributeValue(was_sealed, AttributeValue::BooleanType));
      avl->Append(new AttributeValue(presymids));
      ComValue stream(dmnfunc, avl);
      stream.stream_mode(STREAM_INTERNAL);
      self->push_stack(stream);
      return;
    } else if (echoresult.is_list()) {
      /* positionals present -- echo appends one singleton attrlist
         per keyword, so trailing entries are those, not positionals */
      poslist = echoresult.list_val();
      npos = poslist->Number() - method_nkey;
      if (npos<0) npos = 0;
    } else if (echoresult.is_attributelist()) {
      /* no positionals -- echo returns the keywords bare, as one
         multi-attribute attrlist */
      kwlist = (AttributeList*) echoresult.obj_val();
    }
  }
  delete [] argtoks;

  ComValue result(fire_attrlist_method_once(self, comterp, al, fo, method_nkey,
					     poslist, kwlist, npos,
					     was_sealed, presymids));
  delete presymids;
  self->push_stack(result);
}

void DotFunc::peek_and_fire(ComValue& before_part, ComValue& after_raw, int& after_nids,
			     std::string& before_expr_text, std::string& after_expr_text) {
    /* capture both args' source text before either evaluates -- firing
       arg 0 moves the bookmark print_stack_arg_post_eval relies on */
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
      /* a command reference (e.g. node.left.val's inner dot) -- fire it
         for its value; symbol=false needs pop_stack's full finalization */
      before_part = stack_arg_post_eval(0);
    } else if (before_part.type()==ComValue::BlankType) {
      /* a never-pulled stream peeks as BlankType -- fire it to get the
         real StreamType value the LHS-stream support below needs */
      before_part = stack_arg_post_eval(0);
    }
    after_raw = stack_arg(1, true);
    after_nids = after_raw.nids();
}

void DotFunc::execute_core(ComValue before_part, ComValue after_raw, int after_nids,
			    const std::string& before_expr_text, const std::string& after_expr_text,
			    boolean force_named_field, int after_stack_idx) {
    /* a variable bound to a stream arrives as a raw SymbolType -- resolve
       a copy to test is_stream(), leaving before_part itself untouched */
    ComValue before_resolved = before_part;
    if (before_resolved.is_symbol()) {
      AttributeValue* rv = comterp()->lookup_symval(&before_resolved, false);
      if (rv) before_resolved = ComValue(*rv);
    }
    if (before_resolved.is_stream() && after_nids==-1) {
      /* (stream).field -- lazy, pulling .field from each element on
         demand; stream.method(args) falls through to the warning below */
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
	!before_part.is_attributelist() &&
	!before_part.is_object(FuncObj::class_symid())) {

      /* a list before "." is common (e.g. zoo.who("Ellie").moves) --
         give a specific error, not the generic type-mismatch one below */
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
    /* an arglist-attached rhs (after_nids != -1) is validated later in
       fire_attrlist_method -- only bare/string rhs needs this check */
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
    boolean al_from_funcobj = false;
    if (before_part.is_object(FuncObj::class_symid())) {
      al = funcobj_dot_attrlist(&before_part);
      al_from_funcobj = true;
    } else if (!before_part.is_attribute() && !before_part.is_attributelist()) {
      int before_symid = before_part.symbol_val();
      boolean global = before_part.global_flag();
      /* func scope (_alist) is checked before local/global, same order
         as ComTerp::lookup_symval -- so func bodies see their own vars */
      AttributeList* funcscope = !global ? comterp()->get_attributes() : nil;
      AttributeValue* fsval = funcscope ? funcscope->find(before_symid) : nil;
      if (fsval) {
	if (fsval->is_attributelist())
	  al = (AttributeList*) fsval->obj_val();
	else if (fsval->is_object(FuncObj::class_symid())) {
	  al = funcobj_dot_attrlist(fsval);
	  al_from_funcobj = true;
	} else {
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
	} else if (vptr && ((ComValue*) vptr)->is_object(FuncObj::class_symid())) {
	  al = funcobj_dot_attrlist((ComValue*) vptr);
	  al_from_funcobj = true;
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

    /* a funcobj's dot with a blank rhs (e.g. "(f.)", the parser's "empty"
       placeholder for a closing delimiter with nothing before it) acts as
       if the rhs weren't there at all -- fall through to the bare form. */
    boolean blank_rhs = al_from_funcobj && is_blank_rhs(after_raw);

    /* al.method(args) attached to a unary self-bound dot (selfdot) carries
       its call as a CommandType after_raw instead of nargs()>1 -- selfdot
       always has exactly one real stack arg (the whole after-expression),
       so nargs()>1 can't distinguish it the way it does for binary dot. */
    boolean selfbound_call = force_named_field && after_raw.is_command();
    /* a string key (dot(al "z")) can only ever mean a plain field lookup
       -- method-call syntax (.name(args)) always carries the name as a
       symbol, never a runtime string -- so it skips method dispatch
       regardless of after_nids, straight to the named-field branch below. */
    if (!blank_rhs && after_nids!=-1 && after_raw.type()!=ComValue::StringType && (nargs()>1 || selfbound_call)) {
      /* al.method(args) -- fire, self-bound; copy_stack_arg_post_eval runs
         before reset_stack(); nargs()>1 + after_nids excludes dot(name) */
      int nargtoks;
      postfix_token* argtoks = copy_stack_arg_post_eval(after_stack_idx, nargtoks);
      reset_stack();
      /* sealed-field cleanup lives inside fire_attrlist_method/_once, not
         here, so it covers every firing a streamed call defers to
         DotMethodNextFunc, not just the one that creates the stream. */
      fire_attrlist_method(this, comterp(), al, argtoks, nargtoks);
    } else if (!blank_rhs && (force_named_field || nargs()>1)) {
      int after_symid = after_raw.symbol_val();
      if (after_raw.type()==ComValue::StringType) {
        symbol_reference(after_symid);
      }
      reset_stack();
      Attribute* attr = al ? al->GetAttr(after_symid) :  nil;
      if (!attr && al && al->sealed()) {
	/* a sealed attrlist (e.g. a func's own captures via dot) names no
	   such entry -- report nil rather than growing the list. */
	ComValue retval(ComValue::nullval());
	retval.lhs_assign(1);
	push_stack(retval);
	return;
      }
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
    /* internal: get/set dotfunc_debug_expr at runtime via a :dbg keyword;
       checked first, since an ordinary a.b expression never supplies :dbg */
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

SelfDotFunc::SelfDotFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void SelfDotFunc::execute() {
    if (check_dbg_keyword()) return;

    AttributeList* home = comterp()->get_attributes();
    if (!home) {
      /* no identifiable frame/scope to be "self" of -- e.g. .f at top level */
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    ComValue before_part(AttributeList::class_symid(), home);
    ComValue after_raw(stack_arg(0, true));
    int after_nids = after_raw.nids();
    /* selfdot has exactly one real stack arg (the whole after-expression,
       field or call) -- pass 0 so execute_core's self-bound method-call
       branch pulls the call's raw tokens from the right slot (it defaults
       to 1, binary dot's "before, after" stack layout). */
    execute_core(before_part, after_raw, after_nids, "", "", true, 0);
}

/*****************************************************************************/

DotStreamNextFunc::DotStreamNextFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void DotStreamNextFunc::execute() {
    /* invoked by the next mechanism -- our own stream (arg 0) carries
       [0] the before-stream and [1] the after-dot symbol, in stream_list() */
    /* deliberately no reset_stack() here: execute_core() below does its
       own single reset, and a second one here would cancel out push_stack() */
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
      /* copy-then-drive pattern from ConcatNextFunc/ReplayNextFunc -- the
         copy shares stream_list(), so advancing persists via *beforeval */
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
      /* not exercised by the LHS-only case this lands in -- kept generic
         so a future RHS/zip extension can reuse this next-func directly */
      before_next = ComValue(*beforeval);
    }

    ComValue after_raw(afterval->symbol_val(), ComValue::SymbolType);
    execute_core(before_next, after_raw, -1, "", "", true);

    /* execute_core()'s named-field branch pushes the raw dotted-pair
       Attribute* wrapper; this per-pull call has no caller to unwrap it */
    ComValue unwrapped(comterp()->pop_stack(true));
    push_stack(unwrapped);
}

/*****************************************************************************/

DotMethodNextFunc::DotMethodNextFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotMethodNextFunc::execute() {
    /* our own stream (arg 0) carries, in stream_list(): [0] the arg-eval
       stream, [1] receiver attrlist, [2] FuncObj, [3] nkey, [4] was-sealed,
       [5] presymids ([5] is ArrayType, so $$ deep-copies it -- see above). */
    ComValue selfstream(stack_arg(0));
    reset_stack();

    AttributeValueList* avl = selfstream.stream_list();
    Iterator i;
    avl->First(i);
    AttributeValue* streamval = avl->GetAttrVal(i); avl->Next(i);
    AttributeValue* alval = avl->GetAttrVal(i); avl->Next(i);
    AttributeValue* foval = avl->GetAttrVal(i); avl->Next(i);
    AttributeValue* nkeyval = avl->GetAttrVal(i); avl->Next(i);
    AttributeValue* sealedval = avl->GetAttrVal(i); avl->Next(i);
    AttributeValue* presymidsval = avl->GetAttrVal(i);

    AttributeList* al = (AttributeList*) alval->obj_val();
    FuncObj* fo = (FuncObj*) foval->obj_val();
    int method_nkey = nkeyval->int_val();
    boolean was_sealed = sealedval->boolean_val();
    AttributeValueList* presymids = presymidsval->array_val();

    /* copy-then-drive pattern from DotStreamNextFunc -- the copy shares
       stream_list(), so advancing persists via *streamval */
    ComValue streamcopy(*streamval);
    NextFunc::execute_impl(comterp(), streamcopy);
    if (comterp()->stack_top().is_unknown()) {
      comterp()->pop_stack();
      push_stack(ComValue::nullval());
      return;
    }
    ComValue elem(comterp()->pop_stack());
    *streamval = streamcopy;

    AttributeValueList* poslist = nil;
    AttributeList* kwlist = nil;
    int npos = 0;
    if (elem.is_list()) {
      poslist = elem.list_val();
      npos = poslist->Number() - method_nkey;
      if (npos<0) npos = 0;
    } else if (elem.is_attributelist()) {
      kwlist = (AttributeList*) elem.obj_val();
    }

    ComValue result(fire_attrlist_method_once(this, comterp(), al, fo, method_nkey,
					       poslist, kwlist, npos,
					       was_sealed, presymids));
    push_stack(result);
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
    /* stack_arg's symbol=true skips lookup_symval()'s dotted-pair unwrap,
       so attrname(x) arrives as raw SymbolType -- resolve while still one */
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
       comment above -- same fix, same reasoning */
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
