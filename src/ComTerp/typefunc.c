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

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/iterator.h>

#include <iostream.h>

#define TITLE "TypeFunc"

/*****************************************************************************/

TypeSymbolFunc::TypeSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TypeSymbolFunc::execute() {
  // return type symbol for each argumen
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  int type_syms[numargs];
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
  // return type symbol for each argumen
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  int class_syms[numargs];
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
