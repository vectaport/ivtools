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

using std::cerr;

#define TITLE "GrDotFunc"

/*****************************************************************************/

int GrDotFunc::_symid = -1;

GrDotFunc::GrDotFunc(ComTerp* comterp) : DotFunc(comterp) {
}

void GrDotFunc::execute() {

    ComValue& before_part(stack_arg(0, true));
    ComValue& after_part(stack_arg(1, true));
    if (!before_part.is_symbol() && 
	!(before_part.is_attribute() && 
          (((Attribute*)before_part.obj_val())->Value()->is_unknown() || 
	  ((Attribute*)before_part.obj_val())->Value()->is_attributelist() ||
          ((Attribute*)before_part.obj_val())->Value()->object_compview())) &&
        !(before_part.is_attributelist()) && 
	!(before_part.object_compview())) {
      cerr << "expression before \".\" needs to evaluate to a symbol or <AttributeList> (instead of "
	   << symbol_pntr(before_part.type_symid());
      if (before_part.is_object())
        cerr << " of class " << symbol_pntr(before_part.class_symid());
      cerr << ") -- grdotfunc.c\n";
      reset_stack();
      return;
    }
    if (!after_part.is_string()) {
      cerr << "expression after \".\" needs to be a symbol or evaluate to a symbol (instead of "
	   << symbol_pntr(after_part.type_symid());
      if (before_part.is_object())
        cerr << " of class " << symbol_pntr(before_part.class_symid());
      cerr << ") -- grdotfunc.c\n";
      reset_stack();
      return;
    }

    /* handle ComponentView case */
    if (before_part.is_symbol()) 
      lookup_symval(before_part);
    if (before_part.is_object() && before_part.object_compview()) {
      ComponentView* compview = (ComponentView*)before_part.obj_val();
      OverlayComp* comp = (OverlayComp*)compview->GetSubject();
      if (comp) {
	ComValue stuffval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
	before_part.assignval(stuffval);
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
	before_part.assignval(stuffval);
      } else {
	cerr << "nil subject on compview value\n";
	reset_stack();
	push_stack(ComValue::nullval());
	return;
      }

    }
    DotFunc::execute();
    
}

/*****************************************************************************/

GrAttrListFunc::GrAttrListFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GrAttrListFunc::execute() {
  ComValue compviewv(stack_arg(0));
  reset_stack();
  if (compviewv.object_compview()) {
    ComponentView* compview = (ComponentView*)compviewv.obj_val();
    OverlayComp* comp = compview ? (OverlayComp*)compview->GetSubject() : nil;
    if (comp) {
      ComValue retval(AttributeList::class_symid(), (void*)comp->GetAttributeList());
      push_stack(retval);
    } else
      push_stack(ComValue::nullval());
  }
}

