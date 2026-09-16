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
    if (before_part.is_symbol())
      lookup_symval(before_part);
    if (before_part.is_object() && before_part.object_compview()) {
      ComponentView* compview = (ComponentView*)before_part.obj_val();
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

    } else if (before_part.is_object() && before_part.is_attribute() &&
	       ((Attribute*)before_part.obj_val())->Value()->object_compview()) {
      AttributeValue* av = ((Attribute*)before_part.obj_val())->Value();
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
    AttributeList* al = stack_keys();
    reset_stack();
    ComValue retval(AttributeList::class_symid(), al);
    push_stack(retval);
  }
}

