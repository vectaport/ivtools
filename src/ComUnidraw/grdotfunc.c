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
#include <fstream>
#include <iostream>

using std::cout;
using std::cerr;

#define TITLE "GrDotFunc"

/*****************************************************************************/


GrDotFunc::GrDotFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void GrDotFunc::execute() {
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
    if (before_resolved.is_object() && before_resolved.object_compview()) {
      ComponentView* compview = (ComponentView*)before_resolved.obj_val();
      OverlayComp* comp = (OverlayComp*)compview->GetSubject();
      if (comp) {
	ComValue stuffval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
	before_part = stuffval;
      } else {
	cerr << "nil subject on compview value\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }

    } else if (before_resolved.is_object() && before_resolved.is_attribute() &&
	       ((Attribute*)before_resolved.obj_val())->Value()->object_compview()) {
      AttributeValue* av = ((Attribute*)before_resolved.obj_val())->Value();
      ComponentView* compview = (ComponentView*)av->obj_val();
      OverlayComp* comp = (OverlayComp*)compview->GetSubject();
      if (comp) {
	ComValue stuffval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
	before_part = stuffval;
      } else {
	cerr << "nil subject on compview value\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }

    }
    execute_core(before_part, after_part, after_nids, before_expr_text, after_expr_text);
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

    /* al must be ref'd by retval before stamp_home_attrs runs -- see
       AttrListFunc::execute()'s identical ordering fix (listfunc.c). */
    ComValue retval(AttributeList::class_symid(), al);
    AttrListFunc::stamp_home_attrs(al);
    push_stack(retval);
  }
}

