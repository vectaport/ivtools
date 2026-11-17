/*
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

#include <ComUnidraw/grdotfunc.h>
#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Components/compview.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/listfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>
#include <fstream>
#include <iostream>

using std::cout;
using std::cerr;

#define TITLE "GrDotFunc"

/*****************************************************************************/


GrDotFunc::GrDotFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void GrDotFunc::execute() {
    /* read first, before anything touches the stack: the flag AssignFunc set
       on this call's command token survives only in the stale slot just above
       our own arguments (ARCHITECTURE.md, "at()'s lhs flag"). */
    boolean for_write = comterp()->stack_top(nkeys()+1).lhs_assign();

    /* Overrides DotFunc::execute() entirely (peek_and_fire needs before_part
       threaded through first), so dot(:dbg true) is checked explicitly here. */
    if (check_dbg_keyword()) return;

    /* peek_and_fire fires arg 0 once if unfired, leaving before_part a real
       value threaded through explicitly to execute_core() below. */
    ComValue before_part, after_part;
    int after_nids;
    std::string before_expr_text, after_expr_text;
    peek_and_fire(before_part, after_part, after_nids, before_expr_text, after_expr_text);

    /* unwrap a ComponentView (or an Attribute wrapping one) to its attr
       list -- dot-dispatch only understands symbols/attributes/attrlists. */
    /* resolved into a copy so before_part stays a raw symbol for
       execute_core's own lookup, which may need to auto-vivify it. */
    ComValue before_resolved = before_part;
    if (before_resolved.is_symbol())
      lookup_symval(before_resolved);

    /* the compview itself, or the one an Attribute's value wraps */
    AttributeValue* compviewval = nil;
    if (before_resolved.is_object() && before_resolved.object_compview())
      compviewval = &before_resolved;
    else if (before_resolved.is_object() && before_resolved.is_attribute() &&
	     ((Attribute*)before_resolved.obj_val())->Value()->object_compview())
      compviewval = ((Attribute*)before_resolved.obj_val())->Value();

    if (compviewval) {
      ComponentView* compview = (ComponentView*)compviewval->obj_val();
      OverlayComp* comp = (OverlayComp*)compview->GetSubject();
      if (!comp) {
	cerr << "nil subject on compview value\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }
      if (for_write && write_pair(*compviewval, after_part, after_nids)) return;
      ComValue stuffval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
      before_part = stuffval;
    }
    execute_core(before_part, after_part, after_nids, before_expr_text, after_expr_text);
}

boolean GrDotFunc::write_pair(AttributeValue& compviewval, ComValue& after_part,
			       int after_nids) {
    /* a named field only, by execute_core's own two tests: nargs()>1 tells
       "comp.field" from the rare one-arg dot() form, and a non-string
       after-part with real nids is a method call (comp.method(args)), whose
       result is the call's, not a field to write. */
    boolean is_string_key = after_part.type() == ComValue::StringType;
    if (nargs() <= 1 || (after_nids != -1 && !is_string_key)) return false;
    if (!after_part.is_symbol() && !is_string_key) return false;

    /* registered by ComEditor; without it (a comterp built with no editor
       commands) the write falls through to the live property list. */
    static int dotsetattr_symid = symbol_add("dotsetattr");
    ComValue* writefunc = comterp()->localvalue(dotsetattr_symid);
    if (!writefunc || !writefunc->is_type(ComValue::CommandType)) return false;

    int field_symid = after_part.symbol_val();
    /* a string key becomes a symbol the list can be indexed by, same as
       execute_core's own named-field branch does with one. */
    if (is_string_key) symbol_reference(field_symid);
    reset_stack();

    AttributeValueList* pair = new AttributeValueList();
    pair->Append(new AttributeValue(*writefunc));
    /* the reference ctor, which shares -- the pointer one dup_as_needed()s,
       and duplicating a ComponentView would hand the command a view of its
       own while unreferencing the one the script holds. */
    pair->Append(new AttributeValue(compviewval));
    ComValue fieldval(field_symid, ComValue::SymbolType);
    pair->Append(new AttributeValue(fieldval));
    ComValue retval(pair);
    retval.lhs_assign(1);
    push_stack(retval);
    return true;
}

/*****************************************************************************/

GrAttrListFunc::GrAttrListFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GrAttrListFunc::execute() {
  ComValue compviewv(stack_arg(0));
  if (compviewv.object_compview()) {
    reset_stack();
    ComponentView* compview = (ComponentView*)compviewv.obj_val();
    OverlayComp* comp = compview ? (OverlayComp*)compview->GetSubject() : nil;
    if (comp) {
      ComValue retval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
      push_stack(retval);
    } else
      push_stack(ComValue::nullval());
  } else {
    /* not a component view -- a bare attrlist literal, e.g. zoo=(:a 1 :b 2);
       stack_keys() must run before reset_stack(), which reads the live stack. */
    static int bincnt_symid = symbol_add("bincnt");
    /* caps a script-supplied :bincnt so it can't drive AttributeTable's
       size-doubling loop past what a bucket array can hold */
    static const int max_bincnt = 1<<20;
    ComValue bincntv(stack_key(bincnt_symid));
    AttributeList* raw = stack_keys();
    reset_stack();

    /* :bincnt sizes the index (0 disables it) and is always stripped from
       the result; a bare or non-int value falls back to -1, the default. */
    AttributeList* al = raw;
    if (raw->GetAttr(bincnt_symid) != nil) {
      int bincnt = -1;
      if (bincntv.is_int()) {
        bincnt = bincntv.int_val();
        if (bincnt < 0) bincnt = -1;
        if (bincnt > max_bincnt) bincnt = max_bincnt;
      }
      al = new AttributeList(nil, bincnt);
      ALIterator i;
      for (raw->First(i); !raw->Done(i); raw->Next(i)) {
        Attribute* attr = raw->GetAttr(i);
        if (attr->SymbolId() != bincnt_symid)
          al->add_attribute(new Attribute(*attr));
      }
      delete raw;
    }

    /* retval must hold al's permanent ref before stamp_home_attrs runs --
       same ordering requirement as AttrListFunc::execute() (listfunc.c). */
    ComValue retval(AttributeList::class_symid(), al);
    AttrListFunc::stamp_home_attrs(al);
    push_stack(retval);
  }
}

/*****************************************************************************/

GrDotSetAttrFunc::GrDotSetAttrFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void GrDotSetAttrFunc::execute() {
  ComValue viewval(stack_arg(0));
  /* no lookup: the field is a symbol naming a property, not a variable to
     resolve -- plain stack_arg() would read whatever `hp` is bound to. */
  ComValue fieldval(stack_arg(1, true));
  ComValue writeval(stack_arg(2));
  reset_stack();

  /* GrDotFunc builds the call, so a bad shape here is a bug rather than
     something a script can provoke -- nil, not a warning. */
  if (!viewval.object_compview() || !fieldval.is_symbol()) {
    push_stack(ComValue::nullval());
    return;
  }
  ComponentView* view = (ComponentView*)viewval.obj_val();
  OverlayComp* comp = (OverlayComp*)view->GetSubject();
  if (!comp) {
    push_stack(ComValue::nullval());
    return;
  }

  /* one keyword's worth of list, merged: the command carries the field it
     writes, and the target's other attributes are none of its business. */
  AttributeList* al = new AttributeList();
  al->add_attribute(new Attribute(fieldval.symbol_val(), new AttributeValue(writeval)));
  if (!execute_setattr(comp, al)) {
    push_stack(ComValue::nullval());
    return;
  }

  /* the value, so comp.field=val reports what an assignment reports */
  push_stack(writeval);
}
