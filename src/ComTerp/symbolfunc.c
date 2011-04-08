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

#include <ComTerp/symbolfunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/iterator.h>

#include <iostream.h>

#define TITLE "SymbolFunc"

/*****************************************************************************/

SymIdFunc::SymIdFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymIdFunc::execute() {
  // return id of each symbol in the arguments
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  int symbol_ids[numargs];
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_type(AttributeValue::CommandType))
      symbol_ids[i] = val.command_symid();
    else if (val.is_type(AttributeValue::StringType))
      symbol_ids[i] = val.string_val();
    else if (val.is_type(AttributeValue::SymbolType))
      symbol_ids[i] = val.symbol_val();
    else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      avl->Append(new AttributeValue(symbol_ids[i], AttributeValue::IntType));
    push_stack(retval);
  } else {
    ComValue retval (symbol_ids[0], AttributeValue::IntType);
    push_stack(retval);
  }

}

/*****************************************************************************/

SymAddFunc::SymAddFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymAddFunc::execute() {
  // return each symbol in the arguments as is
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  int symbol_ids[numargs];
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_type(AttributeValue::CommandType))
      symbol_ids[i] = val.command_symid();
    else if (val.is_type(AttributeValue::StringType))
      symbol_ids[i] = val.string_val();
    else if (val.is_type(AttributeValue::SymbolType))
      symbol_ids[i] = val.symbol_val();
    else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      avl->Append(new AttributeValue(symbol_ids[i], AttributeValue::SymbolType));
    push_stack(retval);
  } else {
    ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
    push_stack(retval);
  }

}

/*****************************************************************************/

SymbolFunc::SymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymbolFunc::execute() {
  // return symbol for each id argument
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  int symbol_ids[numargs];
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_char() || val.is_short() || val.is_int())
      symbol_ids[i] = val.int_val();
    else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      avl->Append(new AttributeValue(symbol_ids[i], AttributeValue::SymbolType));
    push_stack(retval);
  } else {
    ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
    push_stack(retval);
  }

}


/*****************************************************************************/

SymValFunc::SymValFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymValFunc::execute() {
  // return value for each symbol variable
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  ComValue* varvalues[numargs];
  for (int i=0; i<numargs; i++) {

    // return fully-evaluated value: expression --> symbol --> value
    varvalues[i] = &stack_arg(i, false); 
    //    lookup_symval(*varvalues[i]);
  }

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++)
      avl->Append(new ComValue(*varvalues[i]));
    reset_stack();
    push_stack(retval);
  } else {
    ComValue retval (*varvalues[0]);
    reset_stack();
    push_stack(retval);
  }
}

/*****************************************************************************/

SplitStrFunc::SplitStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SplitStrFunc::execute() {
  ComValue symvalv(stack_arg(0));
  reset_stack();

  if (symvalv.is_string()) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    const char* str = symvalv.symbol_ptr();
    int len = strlen(str);
    for (int i=0; i<len; i++)
      avl->Append(new AttributeValue(str[i]));
    push_stack(retval);
  } else
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

JoinStrFunc::JoinStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void JoinStrFunc::execute() {
  ComValue listv(stack_arg(0));
  static int sym_symid = symbol_add("sym");
  ComValue symflagv(stack_key(sym_symid));
  boolean symflag = symflagv.is_true();
  reset_stack();

  if (listv.is_array()) {
    AttributeValueList* avl = listv.array_val();
    if (avl) {
      char cbuf[avl->Number()+1];
      Iterator i;
      int cnt=0;
      for (avl->First(i); !avl->Done(i); avl->Next(i)) {
	cbuf[cnt] = avl->GetAttrVal(i)->char_val();
	cnt++;
      }
      cbuf[cnt] = '\0';

    ComValue retval(symbol_add(cbuf), symflag ? ComValue::SymbolType : ComValue::StringType);
    push_stack(retval);
    return;
    }
  }
  push_stack(ComValue::nullval());
}


GlobalSymbolFunc::GlobalSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GlobalSymbolFunc::execute() {
  // return symbol(s) with global flag set
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) {
    reset_stack();
    return;
  }
  int symbol_ids[numargs];
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_symbol())
      symbol_ids[i] = val.symbol_val();
    else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      AttributeValue* av = 
	new AttributeValue(symbol_ids[i], AttributeValue::SymbolType);
      av->global_flag(true);
      avl->Append(av);
    }
    push_stack(retval);
  } else {
    
    ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
    retval.global_flag(true);
    push_stack(retval);
  }

}


