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
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <iostream>
#include <fstream>

#define TITLE "DotFunc"

using std::cerr;

/*****************************************************************************/

int DotFunc::_symid = -1;

DotFunc::DotFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotFunc::execute() {
    ComValue before_part(stack_arg(0, true));
    ComValue after_part(stack_arg(1, true));
    reset_stack();

    if (!before_part.is_symbol() && 
	!(before_part.is_attribute() && 
	  (((Attribute*)before_part.obj_val())->Value()->is_unknown() || 
	  ((Attribute*)before_part.obj_val())->Value()->is_attributelist())) &&
	!before_part.is_attributelist()) {
      cerr << "expression before \".\" needs to evaluate to a symbol or <AttributeList> (instead of "
	   << symbol_pntr(before_part.type_symid());
      if (before_part.is_object())
        cerr << " of class " << symbol_pntr(before_part.class_symid());
      cerr << ")\n";
      return;
    }
    if (nargs()>1 && !after_part.is_string()) {
      cerr << "expression after \".\" needs to be a symbol or evaluate to a symbol (instead of "
	   << symbol_pntr(after_part.type_symid());
      if (before_part.is_object())
        cerr << " for class " << symbol_pntr(before_part.class_symid());
      cerr << ")\n";
      return;
    }

    /* lookup value of before variable */
    void* vptr = nil; 
    AttributeList* al = nil;
    if (!before_part.is_attribute() && !before_part.is_attributelist()) {
      int before_symid = before_part.symbol_val();
      boolean global = before_part.global_flag();
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
	Resource::ref(al);
	ComValue* comval = new ComValue(AttributeList::class_symid(), (void*)al);
	if (!global)
	  comterp()->localtable()->insert(before_symid, comval);
	else
	  comterp()->globaltable()->insert(before_symid, comval);
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

    if (nargs()>1) {
      int after_symid = after_part.symbol_val();
      Attribute* attr = al ? al->GetAttr(after_symid) :  nil;
      if (!attr) {
	attr = new Attribute(after_symid, new AttributeValue());
	al->add_attribute(attr);
      }
      ComValue retval(Attribute::class_symid(), attr);
      push_stack(retval);
    } else {
      ComValue retval(AttributeList::class_symid(), al);
      push_stack(retval);
    }
}

/*****************************************************************************/

DotNameFunc::DotNameFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotNameFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    reset_stack();
    if (dotted_pair.class_symid() != Attribute::class_symid()) return;
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
    if (dotted_pair.class_symid() != Attribute::class_symid()) return;
    Attribute *attr = (Attribute*)dotted_pair.obj_val();
    push_stack(*attr->Value());
}





