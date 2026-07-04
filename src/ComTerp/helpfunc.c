/*
 * Copyright (c) 2001,2005 Scott E. Johnston
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
vv * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#include <ComTerp/comhandler.h>

#include <ComTerp/helpfunc.h>
#include <ComTerp/comterp.h>
#include <ComTerp/comvalue.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Unidraw/iterator.h>

#include <OS/math.h>

#include <iostream.h>
#include <sstream>
#include <fstream.h>
#include <streambuf>
using std::streambuf;

#define TITLE "HelpFunc"

#define HELPOUT 0  // set to 1 if desire to print instead of return help info

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

  static int posteval_symid = symbol_add("posteval");
  ComValue postevalflag(stack_key(posteval_symid));

  static int aliases_symid = symbol_add("aliases");
  ComValue aliasesflag(stack_key(aliases_symid));
		   
  static int top_symid = symbol_add("top");
  ComValue topflag(stack_key(top_symid));
		   
  boolean noargs = !nargs() && !nkeys();
  ComFunc** comfuncs= nil;
  int* command_ids = nil;
  boolean* str_flags;
  int nfuncs = 0;
  
  /* build up table of command ids and flags to indicate if its an operator encased in quotes */
  if (allflag.is_false() && postevalflag.is_false() && topflag.is_false()) {

    nfuncs = nargs();
    comfuncs = new ComFunc*[nfuncs];
    command_ids = new int[nfuncs];
    str_flags = new boolean[nfuncs];

    for (int i=0; i<nfuncs; i++) {
      ComValue val = stack_arg(i, true);
      if (val.is_type(AttributeValue::CommandType)) {
	comfuncs[i] = (ComFunc*)val.obj_val();
	command_ids[i] = val.command_symid();
	str_flags[i] = false;
      } else if (val.is_type(AttributeValue::StringType)) {
 	void *vptr = nil;
	comterp()->localtable()->find(vptr, val.string_val());
	if (vptr && ((ComValue*)vptr)->is_command()) {
	  comfuncs[i] = (ComFunc*)((ComValue*)vptr)->obj_val();
	} else {
	  command_ids[i] = val.symbol_val();
	  comfuncs[i] = nil;
	}
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

  } else if (topflag.is_true()) {
      
    AttributeValueList* avl = comterp()->top_commands();
    nfuncs = avl->Number();
    command_ids = new int[nfuncs];
    comfuncs = new ComFunc*[nfuncs];
    str_flags = new boolean[nfuncs];
    Iterator it;
    avl->First(it);
    for (int j=0; j<nfuncs; j++) {

      int command_id = avl->GetAttrVal(it)->symbol_val();
      avl->Next(it);
      command_ids[j] = command_id;

      void* vptr;
      comterp()->localtable()->find(vptr, command_id);
      if (vptr && ((ComValue*)vptr)->is_command()) {
	comfuncs[j] = (ComFunc*)((ComValue*)vptr)->obj_val();
	if (postevalflag.is_true() && !comfuncs[j]->post_eval())
	  comfuncs[j] = nil;
      } else
	comfuncs[j] = nil;
      str_flags[j] = false;
    }

  } else {
    command_ids = comterp()->get_commands(nfuncs, true);
    comfuncs = new ComFunc*[nfuncs];
    str_flags = new boolean[nfuncs];
    for (int j=0; j<nfuncs; j++) {

      /* check for aliases, and negate the symbol id if needed */
      int command_id;
      if (command_ids[j]<0) {
	if (aliasesflag.is_false()) continue;
	command_id = -command_ids[j];
      } else 
	command_id = command_ids[j];

      void* vptr=NULL;
      comterp()->localtable()->find(vptr, command_id);
      if (vptr && ((ComValue*)vptr)->is_command()) {
	comfuncs[j] = (ComFunc*)((ComValue*)vptr)->obj_val();
	if (postevalflag.is_true() && !comfuncs[j]->post_eval())
	  comfuncs[j] = nil;
      } else
	comfuncs[j] = nil;
      str_flags[j] = false;
    }
  }
  
  reset_stack();

  std::stringbuf sbuf;
  #if HELPOUT
  FILEBUF(fbuf, comterp()->handler() && HELPOUT && comterp()->handler()->wrfptr()
	       ? comterp()->handler()->wrfptr() : stdout, ios_base::out);
  ostream outs((comterp()->handler() && HELPOUT) ? (streambuf*)&fbuf : (streambuf*)&sbuf);
  #else
  ostream outs((streambuf*)&sbuf);
  #endif
  ostream *out = &outs;

  if (noargs) {

    *out << "help available on these operators and commands:\n";
    comterp()->list_commands(*out, true);
    *out << "\n(provide any of the above, operators in quotes, as arguments to help,\ni.e. help(help) or help(\"++\"))\n";

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
	    *out << '\n';
	  {
	    char buffer[BUFSIZ];
	    if (comfuncs[i]->docstring2()!=NULL) {
	      strncpy(buffer, comfuncs[i]->docstring2(), BUFSIZ-1);
	    } else {
	      snprintf(buffer, BUFSIZ, 
		       comfuncs[i]->docstring(), symbol_pntr(command_ids[i]));
	    }
	    *out << buffer;
	    if (comfuncs[i]->docstring2()==NULL) {
	      const char** keydoc = comfuncs[i]->dockeys();
	      if (keydoc != nil) {
		while(*keydoc !=nil) {
		  *out << '\n';
		  *out << "        " << keydoc[0];
		  keydoc++;
		}
	      }
	    }
	  }
	  printed = true;
	}
      }
     if (!printed && command_ids[i]>=0) {

        /* if symid is smaller than the highest operator (and it doesnt' start with a letter) it must be one */
        const char* opstr = symbol_pntr(command_ids[i]);
	if (command_ids[i]>=0 && command_ids[i]<=opr_tbl_topstr() && !isalpha(opstr[0])) {
	  int op_ids[OPTYPE_NUM];
	  unsigned int charcnt;
	  opr_tbl_entries((char*)opstr, op_ids, OPTYPE_NUM, &charcnt);
	  for (int j=0; j<OPTYPE_NUM; j++) {
	    if (op_ids[j]>=0) {
	      ComValue* value = comterp()->localvalue(opr_tbl_commid(op_ids[j]));
	      if (value) {
		ComFunc* comfunc = (ComFunc*)value->obj_val();
		if (postevalflag.is_true() && !comfunc->post_eval()) continue;
		if (first) 
		  first = false;
		else
		  out->put('\n');
		{
		  char buffer[BUFSIZ];
		  if (comfunc->docstring2()!=NULL) {
		    strncpy(buffer, comfunc->docstring2(), BUFSIZ-1);
		  } else {
  		    snprintf(buffer, BUFSIZ, 
			     comfunc->docstring(), symbol_pntr(value->command_symid()));
		  }
		  *out << buffer;
		}
	      } else 
		out_form((*out), "unknown operator: %s\n", symbol_pntr(command_ids[i]));

	    }
	  }
	} else if (comfuncs[i]) {
	  if (first) 
	    first = false;
	  else
	    out->put('\n');
	  if (str_flags[i]) out->put('"');
	  *out << symbol_pntr(command_ids[i]);
	  if (str_flags[i]) out->put('"');
	  *out << " unknown";
	}
      }
    }
  }
  
  if (!comterp()->handler() || !HELPOUT) {
    // int help_str_symid = symbol_add(sbuf.str().c_str());
    ComValue retval(sbuf.str().c_str());
    push_stack(retval);
  } else
    out->flush();

  delete command_ids;
  delete comfuncs;
  delete str_flags;

}

/*****************************************************************************/

OptableFunc::OptableFunc(ComTerp* comterp) : ComFunc(comterp) {
}

  void OptableFunc::execute() {

  static int bypri_symid = symbol_add("bypri");
  ComValue bypriflag(stack_key(bypri_symid));
  static int byopr_symid = symbol_add("byopr");
  ComValue byoprflag(stack_key(byopr_symid));
  static int bycom_symid = symbol_add("bycom");
  ComValue bycomflag(stack_key(bycom_symid));
  static int table_symid = symbol_add("table");
  ComValue tableflag(stack_key(table_symid));

  /* live operator-table editing (reprogram the parser at runtime): the change
     takes effect for the NEXT parsed expression, like a func definition. */
  static int insert_symid = symbol_add("insert");
  ComValue insertflag(stack_key(insert_symid));
  static int delete_symid = symbol_add("delete");
  ComValue deleteflag(stack_key(delete_symid));
  static int pri_symid = symbol_add("pri");
  ComValue default_pri(80);
  ComValue prival(stack_key(pri_symid, false, default_pri, true));
  static int rtol_symid = symbol_add("rtol");
  ComValue rtolflag(stack_key(rtol_symid));
  static int prefix_symid = symbol_add("prefix");
  ComValue prefixflag(stack_key(prefix_symid));
  static int postfix_symid = symbol_add("postfix");
  ComValue postfixflag(stack_key(postfix_symid));
  ComValue opstrv(stack_arg(0));    // operator string (e.g. "%%")
  ComValue commandv(stack_arg(1));  // command it maps to (e.g. "replay"), for :insert

  reset_stack();

  /* optable("%%" "replay" :insert [:pri N] [:rtol] [:prefix|:postfix])
     optable("%%" :delete [:prefix|:postfix])
     -- returns 1 on success, 0 on failure.  Effective next expression. */
  if (insertflag.is_true() || deleteflag.is_true()) {
    if (!opstrv.is_string()) {
      fprintf(stderr, "optable: :insert/:delete needs an operator string\n");
      ComValue zero(0);
      push_stack(zero);
      return;
    }
    unsigned optype = prefixflag.is_true()  ? OPTYPE_UNARY_PREFIX
                    : postfixflag.is_true() ? OPTYPE_UNARY_POSTFIX
                    : OPTYPE_BINARY;
    int rc;
    if (deleteflag.is_true()) {
      rc = opr_tbl_remove(opstrv.string_ptr(), optype);
    } else {
      if (!commandv.is_string()) {
        fprintf(stderr, "optable: :insert needs a command name\n");
        ComValue zero(0);
        push_stack(zero);
        return;
      }
      rc = opr_tbl_insert(opstrv.string_ptr(), commandv.string_ptr(),
                          (unsigned)prival.int_val(),
                          (BOOLEAN)rtolflag.is_true(), optype);
    }
    ComValue result(rc == 0 ? 1 : 0);
    push_stack(result);
    return;
  }

  if (tableflag.is_true()) {
    // return list of attrlists, one per operator entry
    static int opr_symid = symbol_add("opr");
    static int cmd_symid = symbol_add("cmd");
    static int pri_symid = symbol_add("pri");
    static int rtol_symid = symbol_add("rtol");
    static int type_symid = symbol_add("type");
    static int binary_symid = symbol_add("BINARY");
    static int prefix_symid = symbol_add("UNARY PREFIX");
    static int postfix_symid = symbol_add("UNARY POSTFIX");

    AttributeValueList* avl = new AttributeValueList();
    unsigned n = opr_tbl_numop_get();
    for (unsigned i = 0; i < n; i++) {
      AttributeList* al = new AttributeList();
      AttributeValue opr_val((const char *)symbol_pntr(opr_tbl_operid(i)));
      AttributeValue cmd_val((const char *)symbol_pntr(opr_tbl_commid(i)));
      AttributeValue pri_val((int)opr_tbl_priority(i), AttributeValue::IntType);
      AttributeValue rtol_val((boolean)opr_tbl_rtol(i), AttributeValue::BooleanType);
      al->add_attr(opr_symid, opr_val);
      al->add_attr(cmd_symid, cmd_val);
      al->add_attr(pri_symid,  pri_val);
      al->add_attr(rtol_symid, rtol_val);
      int optype = opr_tbl_optype(i);
      int type_symval = optype == OPTYPE_UNARY_POSTFIX ? postfix_symid
                      : optype == OPTYPE_UNARY_PREFIX  ? prefix_symid
                      : binary_symid;
      AttributeValue type_val(type_symval, AttributeValue::SymbolType);
      al->add_attr(type_symid, type_val);
      AttributeValue* av = new AttributeValue(AttributeList::class_symid(), al);
      avl->Append(av);
    }
    ComValue retval(avl);
    push_stack(retval);
    return;
  }

  int sort = OPBY_PRIORITY;
  if (bycomflag.is_true()) { sort = OPBY_COMMAND; }
  if (byoprflag.is_true()) { sort = OPBY_OPERATOR; }
  if (bypriflag.is_true()) { sort = OPBY_PRIORITY; }

  opr_tbl_print(stdout, sort);
  return;
}
