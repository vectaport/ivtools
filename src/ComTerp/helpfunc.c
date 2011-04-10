/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include <ComTerp/helpfunc.h>
#include <ComTerp/comhandler.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <iostream.h>

#define TITLE "HelpFunc"

/*****************************************************************************/

HelpFunc::HelpFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void HelpFunc::execute() {
  // print list of commands if no arguments provided, otherwise
  // take everything off the stack and check commands, strings, and symbols
  // for valid command names and print their document string
  // (check if an ordinary symbol is less than the largest symbol id of an operator
  // because that is what it would be (an operator)
  static int all_symid = symbol_add("all");
  ComValue allflag(stack_key(all_symid));
		   
  boolean noargs = !nargs() && !nkeys();
  ComFunc** comfuncs= nil;
  int* command_ids = nil;
  boolean* str_flags;
  int nfuncs = 0;
  
  /* build up table of command ids and flags to indicate if its an operator encased in quotes */
  if (allflag.is_false()) {

    nfuncs = nargs();
    comfuncs = new ComFunc*[nfuncs];
    command_ids = new int[nfuncs];
    str_flags = new boolean[nfuncs];

    for (int i=0; i<nfuncs; i++) {
      ComValue& val = stack_arg(i, true);
      if (val.is_type(AttributeValue::CommandType)) {
	comfuncs[i] = (ComFunc*)val.obj_val();
	command_ids[i] = val.command_symid();
	str_flags[i] = false;
      } else if (val.is_type(AttributeValue::StringType)) {
	void *vptr = nil;
	comterp()->localtable()->find(vptr, val.string_val());
	if (vptr) {
	  comfuncs[i] = (ComFunc*)((ComValue*)vptr)->obj_val();
	} else
	  comfuncs[i] = nil;
	command_ids[i] = val.string_val();
	str_flags[i] = true;
      } else {
	comfuncs[i] = nil;
	if (val.is_type(AttributeValue::SymbolType))
	  command_ids[i] = val.symbol_val();
	else 
	  command_ids[i] = -1;
	str_flags[i] = false;
      }
    }
  } else {
    command_ids = comterp()->get_commands(nfuncs, true);
    comfuncs = new ComFunc*[nfuncs];
    str_flags = new boolean[nfuncs];
    for (int j=0; j<nfuncs; j++) {
      void* vptr;
      comterp()->localtable()->find(vptr, command_ids[j]);
      if (vptr) {
	comfuncs[j] = (ComFunc*)((ComValue*)vptr)->obj_val();
      } else
	comfuncs[j] = nil;
      str_flags[j] = false;
    }
  }
  
  reset_stack();

  filebuf fbuf;
  if (comterp()->handler()) {
    int fd = max(1, comterp()->handler()->get_handle());
    fbuf.attach(fd);
  } else
    fbuf.attach(fileno(stdout));
  ostream out(&fbuf);

  if (noargs) {

    out << "help available on these commands:\n";
    comterp()->list_commands(out, true);
    out << "\n(provide any of the above, operators in quotes, as arguments to help,\ni.e. help(help) or help(\"++\"))\n";

  } else {
    boolean first=true;
    for (int i=0; i<nfuncs; i++) {
      boolean printed = false;
      if (comfuncs[i]) {
	void *vptr = nil;
	comterp()->localtable()->find(vptr, command_ids[i]);
	if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType) {
	  if (first) 
	    first = false;
	  else
	    out.put('\n');
	  ;
	  out.form(comfuncs[i]->docstring(), symbol_pntr(command_ids[i]));
	  printed = true;
	}
      }
      if (!printed && command_ids[i]>=0) {
	/* if symid is smaller than the highest operator it must be one */
	if (command_ids[i] && command_ids[i]<=opr_tbl_topstr()) {
	  int op_ids[OPTYPE_NUM];
	  char* opstr = symbol_pntr(command_ids[i]);
	  unsigned int charcnt;
	  opr_tbl_entries(opstr, op_ids, OPTYPE_NUM, &charcnt);
	  for (int j=0; j<OPTYPE_NUM; j++) {
	    if (op_ids[j]>=0) {
	      ComValue* value = comterp()->localvalue(opr_tbl_commid(op_ids[j]));
	      if (value) {
		ComFunc* comfunc = (ComFunc*)value->obj_val();
		if (first) 
		  first = false;
		else
		  out.put('\n');
		out.form(comfunc->docstring(), symbol_pntr(value->command_symid()));
	      } else 
		out.form("unknown operator: %s\n", symbol_pntr(command_ids[i]));

	    }
	  }
	} else {
	  if (first) 
	    first = false;
	  else
	    out.put('\n');
	  if (str_flags[i]) out.put('"');
	  out << symbol_pntr(command_ids[i]);
	  if (str_flags[i]) out.put('"');
	  out << " unknown";
	}
      }
    }
  }

  delete command_ids;
  delete comfuncs;
  delete str_flags;

}

/*****************************************************************************/

SymIdFunc::SymIdFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymIdFunc::execute() {
  // print symbol id of arguments
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
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

  AttributeValueList* avl = new AttributeValueList();
  ComValue retval(avl);
  for (int i=0; i<numargs; i++)
    avl->Append(new AttributeValue(symbol_ids[i], AttributeValue::IntType));
  push_stack(retval);
}

/*****************************************************************************/

SymValFunc::SymValFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymValFunc::execute() {
  // print symbol id of arguments
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  int symbol_ids[numargs];
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_char() || val.is_short() || val.is_int())
      symbol_ids[i] = val.int_val();
    else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  AttributeValueList* avl = new AttributeValueList();
  ComValue retval(avl);
  for (int i=0; i<numargs; i++)
    avl->Append(new AttributeValue(symbol_ids[i], AttributeValue::StringType));
  push_stack(retval);
}
