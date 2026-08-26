/*
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1998,1999,2000 Vectaport Inc.
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

#include <ComTerp/typefunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/postfunc.h>

#include <Attribute/_comutil.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/iterator.h>

#include <iostream.h>
#include <string.h>
#include <algorithm>
#include <vector>

#define TITLE "TypeFunc"

/*****************************************************************************/

TypeSymbolFunc::TypeSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TypeSymbolFunc::execute() {
  // return type symbol for each argument
  static int all_symid = symbol_add("all");
  boolean all_flag = stack_key(all_symid).is_true();
  int numargs = nargs();

  if (all_flag) {
    /* the whole closed set, in enum order -- every value in the language has
       one of these, so unlike class() this list is complete by construction.
       ListType and ArrayType are one type under two names; the symbol is
       ListType, so ArrayType never appears. */
    reset_stack();
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int t=AttributeValue::UnknownType; t<=AttributeValue::BlankType; t++) {
      ComValue* av = new ComValue
	(AttributeValue::type_symid((AttributeValue::ValueType)t),
	 AttributeValue::SymbolType);
      av->bquote(1);
      avl->Append(av);
    }
    push_stack(retval);
    return;
  }

  if (!numargs) {
    /* no value named at all -- blank, the "nothing was asked" answer, the same
       distinction class() draws.  nil stays the answer about a named value. */
    reset_stack();
    push_stack(ComValue::blankval());
    return;
  }
  std::vector<int> type_syms(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i);
    type_syms[i] = val.type_symid();
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      if (type_syms[i]<0)
	avl->Append(new AttributeValue());
      else {
	ComValue* av = new ComValue(type_syms[i], AttributeValue::SymbolType);
	av->bquote(1);
	avl->Append(av);
      }
    push_stack(retval);
  } else {
    if (type_syms[0]<0)
      push_stack(ComValue::nullval());
    else {
      ComValue retval (type_syms[0], AttributeValue::SymbolType);
      retval.bquote(1);
      push_stack(retval);
    }
  }

}

/*****************************************************************************/

ClassSymbolFunc::ClassSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ClassSymbolFunc::execute() {
  // return class symbol for each argument
  static int all_symid = symbol_add("all");
  static int comps_symid = symbol_add("comps");
  boolean all_flag = stack_key(all_symid).is_true();
  boolean comps_flag = stack_key(comps_symid).is_true();

  if (all_flag || comps_flag) {
    /* every class that used CLASS_SYMID, enrolled before main() -- so this is
       what the binary linked, not what it happens to have touched.  :comps
       narrows to the CLASS_SYMID2 classes, the ones carrying a Unidraw
       ClassId.  Sorted by name: the registry is in dynamic-initializer order,
       which no standard pins down. */
    std::vector<const char*> names;
    for (ClassSymid* node = class_symid_list(); node; node = node->next)
      if (!comps_flag || node->iscomp) names.push_back(node->classname);
    std::sort(names.begin(), names.end(), [](const char* a, const char* b)
	      { return strcmp(a, b) < 0; });
    reset_stack();
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<names.size(); i++) {
      /* the registry holds names, so the ids are made here -- symbol_add() is
	 idempotent, so this is the same id class_symid() hands back */
      ComValue* av = new ComValue(symbol_add(names[i]), AttributeValue::SymbolType);
      av->bquote(1);
      avl->Append(av);
    }
    push_stack(retval);
    return;
  }

  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) {
    /* no value named at all -- blank, the "nothing was asked" answer.  nil is
       reserved for the value that was named but has no class to report. */
    reset_stack();
    push_stack(ComValue::blankval());
    return;
  }
  std::vector<int> class_syms(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue val = stack_arg(i);
    if (val.is_object()) 
      class_syms[i] = val.class_symid();
    else
      class_syms[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      if (class_syms[i]<0)
	avl->Append(new AttributeValue());
      else {
	ComValue* av = new ComValue(class_syms[i], AttributeValue::SymbolType);
	av->bquote(1);
	avl->Append(av);
      }
    push_stack(retval);
  } else {
    if (class_syms[0]<0)
      push_stack(ComValue::nullval());
    else {
      ComValue retval (class_syms[0], AttributeValue::SymbolType);
      retval.bquote(1);
      push_stack(retval);
    }
  }

}

/*****************************************************************************/

/* shared by IsTypeFunc/IsClassFunc/IsCommFunc/IsFuncFunc: resolve a raw,
   post_eval-read argument to what it's actually bound to, without ever
   firing it -- a raw arg that's already resolved (a CommandType token like
   `pi`, a literal, another command's own raw call token) is its own
   answer; a raw arg that's still a bare SymbolType token (an ordinary
   variable reference) gets exactly one non-invoking table lookup
   (ComTerp::lookup_symval, the same primitive ComValue::is_funcobj()
   already uses internally) to see what it's bound to.  Returns nil for an
   unbound bare symbol. */
static AttributeValue* resolve_no_fire(ComTerp* comterp, ComValue& raw) {
  if (raw.type() != AttributeValue::SymbolType) return &raw;
  return comterp->lookup_symval(&raw);
}

/* shared by the two-argument forms of istype()/isclass(): the type/class
   name argument is always just a symbol, bare (CommandType) or bquoted
   (`CommandType) -- neither form should ever be resolved as a variable
   reference (there usually isn't one bound to that name).  A bare token
   is read directly; anything else (a bquote call being the expected
   case) is safe to evaluate outright, since bquote itself never fires
   anything and has no side effects. */
static int arg_symbol_id(ComFunc* func, ComValue& raw) {
  if (raw.type() == AttributeValue::SymbolType) return raw.symbol_val();
  ComValue evaluated(func->stack_arg_post_eval(1, true));
  return evaluated.symbol_val();
}

/* shared by all four: push the quoted symbol for a resolved symid, or nil
   if there isn't one -- the :sym-flag counterpart to the plain boolean
   push these commands otherwise do. */
static void push_symid_or_nil(ComFunc* func, int symid) {
  if (symid<0) {
    func->push_stack(ComValue::nullval());
  } else {
    ComValue retval(symid, AttributeValue::SymbolType);
    retval.bquote(1);
    func->push_stack(retval);
  }
}

/*****************************************************************************/

IsTypeFunc::IsTypeFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void IsTypeFunc::execute() {
  ComValue arg0(stack_arg(0, true));
  boolean two_arg = nargsfixed()>1;
  static int sym_symid = symbol_add("sym");
  ComValue symflag(stack_key(sym_symid));
  AttributeValue* resolved = resolve_no_fire(comterp(), arg0);
  int subj_symid = resolved ? resolved->type_symid() : -1;

  if (symflag.is_true()) {
    reset_stack();
    push_symid_or_nil(this, subj_symid);
  } else if (two_arg) {
    ComValue arg1(stack_arg(1, true));
    int type_symid = arg_symbol_id(this, arg1);
    reset_stack();
    push_stack(subj_symid==type_symid ? ComValue::trueval() : ComValue::falseval());
  } else {
    reset_stack();
    boolean is_obj = resolved && resolved->is_object();
    push_stack(!is_obj ? ComValue::trueval() : ComValue::falseval());
  }
}

/*****************************************************************************/

IsClassFunc::IsClassFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void IsClassFunc::execute() {
  ComValue arg0(stack_arg(0, true));
  boolean two_arg = nargsfixed()>1;
  static int sym_symid = symbol_add("sym");
  ComValue symflag(stack_key(sym_symid));
  AttributeValue* resolved = resolve_no_fire(comterp(), arg0);
  boolean is_obj = resolved && resolved->is_object();
  int subj_class_symid = is_obj ? resolved->class_symid() : -1;

  if (symflag.is_true()) {
    reset_stack();
    push_symid_or_nil(this, subj_class_symid);
  } else if (two_arg) {
    ComValue arg1(stack_arg(1, true));
    int class_symid = arg_symbol_id(this, arg1);
    reset_stack();
    push_stack((is_obj && subj_class_symid==class_symid) ? ComValue::trueval() : ComValue::falseval());
  } else {
    reset_stack();
    push_stack(is_obj ? ComValue::trueval() : ComValue::falseval());
  }
}

/*****************************************************************************/

IsCommFunc::IsCommFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void IsCommFunc::execute() {
  ComValue arg0(stack_arg(0, true));
  static int sym_symid = symbol_add("sym");
  ComValue symflag(stack_key(sym_symid));
  reset_stack();
  AttributeValue* resolved = resolve_no_fire(comterp(), arg0);
  boolean match = resolved && resolved->is_type(AttributeValue::CommandType);
  if (symflag.is_true())
    push_symid_or_nil(this, match ? resolved->type_symid() : -1);
  else
    push_stack(match ? ComValue::trueval() : ComValue::falseval());
}

/*****************************************************************************/

IsFuncFunc::IsFuncFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void IsFuncFunc::execute() {
  ComValue arg0(stack_arg(0, true));
  static int sym_symid = symbol_add("sym");
  ComValue symflag(stack_key(sym_symid));
  reset_stack();
  AttributeValue* resolved = resolve_no_fire(comterp(), arg0);
  boolean match = resolved && resolved->is_object(FuncObj::class_symid());
  if (symflag.is_true())
    push_symid_or_nil(this, match ? resolved->class_symid() : -1);
  else
    push_stack(match ? ComValue::trueval() : ComValue::falseval());
}
