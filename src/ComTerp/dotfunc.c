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

#include <ComTerp/dotfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>

#define TITLE "DotFunc"

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
	  ((Attribute*)before_part.obj_val())->Value()->is_attributelist())) {
      cerr << "expression before \".\" needs to evaluate to a symbol or <AttributeList>\n";
      return;
    }
    if (!after_part.is_symbol()) {
      cerr << "expression after \".\" needs to be a symbol or evaluate to a syymbol\n";
      return;
    }

    /* lookup value of before variable */
    void* vptr = nil; 
    AttributeList* al = nil;
    if (!before_part.is_attribute()) {
      int before_symid = before_part.symbol_val();
      comterp()->localtable()->find(vptr, before_symid);
      if (vptr &&((ComValue*) vptr)->class_symid() == AttributeList::class_symid()) {
	al = (AttributeList*) ((ComValue*) vptr)->obj_val();
      } else {
	al = new AttributeList();
	Resource::ref(al);
	ComValue* comval = new ComValue(AttributeList::class_symid(), (void*)al);
	comterp()->localtable()->insert(before_symid, comval);
      }
    } else
      al = (AttributeList*) ((Attribute*) before_part.obj_val())->Value()->obj_val();

    int after_symid = after_part.symbol_val();
    Attribute* attr = al->GetAttr(after_symid);
    if (!attr) {
      attr = new Attribute(after_symid, new AttributeValue());
      al->add_attribute(attr);
    }
    ComValue retval(Attribute::class_symid(), attr);
    push_stack(retval);
}

/*****************************************************************************/

DotNameFunc::DotNameFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotNameFunc::execute() {
    ComValue dotted_pair(stack_arg(0, true));
    reset_stack();
    if (dotted_pair.class_symid() != Attribute::class_symid()) return;
    Attribute *attr = (Attribute*)dotted_pair.obj_val();
    ComValue retval(attr->SymbolId(), ComValue::SymbolType);
    push_stack(retval);
}

/*****************************************************************************/

DotValFunc::DotValFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void DotValFunc::execute() {
    ComValue dotted_pair(stack_arg(0));
    reset_stack();
    if (dotted_pair.class_symid() != Attribute::class_symid()) return;
    Attribute *attr = (Attribute*)dotted_pair.obj_val();
    push_stack(*attr->Value());
}





