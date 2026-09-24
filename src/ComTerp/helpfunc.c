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
#include <ComTerp/postfunc.h>

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Unidraw/iterator.h>

#include <OS/math.h>

#include <iostream.h>
#include <strstream>
#include <fstream.h>
#include <streambuf>
using std::streambuf;

#define TITLE "HelpFunc"

#define HELPOUT 0  // set to 1 if desire to print instead of return help info

static boolean help_is_idchar(char c) {
  return isalnum((unsigned char)c) || c == '_';
}

/* the dockeys() description text for keyname (no leading ':'), or nil if
   func declares no dockey by that name. Only a dockeys entry's own leading
   name (with any "name|alias" aliases, e.g. ":str|:string") is checked --
   never the free-text description, which may itself mention other
   ":name"-shaped keywords in passing (optable's :table entry describes
   itself as returning "(:opr :cmd :pri :rtol :type)", none of which are
   :table's own name). */
static const char* help_dockey_desc(ComFunc* func, const char* keyname) {
  const char** keydoc = func->dockeys();
  if (keydoc == nil) return nil;
  int kwlen = strlen(keyname);
  while (*keydoc != nil) {
    const char* entry = *keydoc;
    if (*entry == ':') {
      const char* p = entry;
      boolean matched = false;
      while (*p == ':') {
        p++;
        const char* namestart = p;
        while (help_is_idchar(*p)) p++;
        if ((int)(p-namestart) == kwlen && strncmp(namestart, keyname, kwlen) == 0)
          matched = true;
        if (*p != '|') break;
        p++;
      }
      if (matched) {
        /* the description starts after the first run of 2+ spaces --
           the column-alignment padding every dockeys entry uses between
           its name (with any placeholder word, e.g. "pri n") and its
           free-text description */
        const char* q = entry;
        while (*q != '\0') {
          if (q[0] == ' ' && q[1] == ' ') {
            while (*q == ' ') q++;
            return q;
          }
          q++;
        }
        while (*p == ' ') p++;
        return p;
      }
    }
    keydoc++;
  }
  return nil;
}

/* true if ":keyname" appears as a bare keyword token in func's docstring
   signature (bounded before " -- ", same as comlint's own keyword-name
   scan) -- a keyword mentioned in the signature but never elaborated in
   dockeys(). A keyword's ':' always sits at a token boundary ('(', '[',
   '|', whitespace, or the start), possibly itself inside "[...]" since
   every keyword is optional (islist's "[:any]"); a colon glued directly
   onto a preceding identifier character (date's "YEAR:MON") is instead a
   value-form separator inside a positional argument, not a keyword. */
static boolean help_signature_has_key(ComFunc* func, int command_symid, const char* keyname) {
  char buffer[8192];  // see the sizing comment where execute() uses this same pattern
  if (func->docstring2() != nil) {
    strncpy(buffer, func->docstring2(), sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';
  } else {
    snprintf(buffer, sizeof(buffer), func->docstring(), symbol_pntr(command_symid));
  }
  /* bounded to before " -- " so a free-text description mentioning a
     ":name"-shaped return-value field is never read as a declared
     keyword -- the signature itself is the only part that means one */
  char* dd = strstr(buffer, " -- ");
  char* end = dd != nil ? dd : buffer + strlen(buffer);
  int kwlen = strlen(keyname);
  char* p = buffer;
  while (p < end) {
    if (*p != ':') { p++; continue; }
    if (p > buffer && help_is_idchar(*(p-1))) { p++; continue; }
    p++;
    char* namestart = p;
    while (p < end && help_is_idchar(*p)) p++;
    if ((int)(p-namestart) == kwlen && strncmp(namestart, keyname, kwlen) == 0) return true;
  }
  return false;
}

/*****************************************************************************/

HelpFunc::HelpFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void HelpFunc::execute() {
  // print list of commands if no arguments given, otherwise print
  // the document string for each command/string/symbol arg on the stack
  static int all_symid = symbol_add("all");
  ComValue allflag(stack_key(all_symid));

  static int posteval_symid = symbol_add("posteval");
  ComValue postevalflag(stack_key(posteval_symid));

  static int aliases_symid = symbol_add("aliases");
  ComValue aliasesflag(stack_key(aliases_symid));
		   
  static int top_symid = symbol_add("top");
  ComValue topflag(stack_key(top_symid));

  static int key_symid = symbol_add("key");
  ComValue keyval(stack_key(key_symid, true));

  boolean noargs = !nargs() && !nkeys();
  ComFunc** comfuncs= nil;
  int* command_ids = nil;
  boolean* str_flags;
  int nfuncs = 0;

  /* help(f) for a bare, unfired FuncObj; parallel to comfuncs[]/command_ids[] but
     only populated in the ordinary branch, since a FuncObj is never a registered command. */
  ComValue* funcobj_help = nil;

  /* build up table of command ids and flags to indicate if its an operator encased in quotes */
  if (allflag.is_false() && postevalflag.is_false() && topflag.is_false()) {

    nfuncs = nargs();
    comfuncs = new ComFunc*[nfuncs];
    command_ids = new int[nfuncs];
    str_flags = new boolean[nfuncs];
    funcobj_help = new ComValue[nfuncs];

    for (int i=0; i<nfuncs; i++) {
      /* stack_arg(i, true) reads symbol-preserving, so a bare FuncObj
         argument reaches the SymbolType branch below completely unfired. */
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
	  command_ids[i] = ((ComValue*)vptr)->command_symid();
	} else {
	  command_ids[i] = val.symbol_val();
	  comfuncs[i] = nil;
	}
	str_flags[i] = true;
      } else {
	comfuncs[i] = nil;
	if (val.is_type(AttributeValue::SymbolType)) {
	  command_ids[i] = val.symbol_val();
	  /* lookup_symval is a pure symbol-table read,
	     so it's safe to check what val names without firing it. */
	  ComValue resolved(comterp()->lookup_symval(val));
	  if (resolved.is_object(FuncObj::class_symid()))
	    funcobj_help[i] = comterp()->describe_funcobj((FuncObj*)resolved.obj_val());
	}
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

  if (keyval.is_known()) {
    /* help(cmd :key name) -- name's dockeys() entry, true if merely named
       bare in cmd's signature, else nil. name may itself be a command. */
    const char* keyname = keyval.is_type(AttributeValue::SymbolType)
      ? symbol_pntr(keyval.symbol_val())
      : keyval.is_type(AttributeValue::CommandType)
      ? symbol_pntr(keyval.command_symid())
      : keyval.is_type(AttributeValue::StringType) ? keyval.string_ptr() : nil;
    ComFunc* keyfunc = (nfuncs>0 && comfuncs) ? comfuncs[0] : nil;
    ComValue keyretval = ComValue::nullval();
    if (keyfunc != nil && keyname != nil) {
      const char* desc = help_dockey_desc(keyfunc, keyname);
      if (desc != nil)
        keyretval = ComValue(desc);
      else if (help_signature_has_key(keyfunc, command_ids[0], keyname))
        keyretval = ComValue::trueval();
    }
    delete [] command_ids;
    delete [] comfuncs;
    delete [] str_flags;
    delete [] funcobj_help;
    push_stack(keyretval);
    return;
  }

  std::strstreambuf sbuf;
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
      if (funcobj_help && funcobj_help[i].is_type(AttributeValue::StringType)) {
	if (first)
	  first = false;
	else
	  *out << '\n';
	/* lead with the name the caller asked about, so a funcobj signature
	   reads like a call site (gcd(arg0 arg1)), since a funcobj itself is nameless. */
	*out << symbol_pntr(command_ids[i]);
	*out << funcobj_help[i].string_ptr();
	printed = true;
      }
      if (!printed && comfuncs[i]) {
	void *vptr = nil;
	comterp()->localtable()->find(vptr, command_ids[i]);
	if (vptr && ((ComValue*)vptr)->type() == ComValue::CommandType) {
	  if (first) 
	    first = false;
	  else
	    *out << '\n';
	  {
	    // BUFSIZ is only 1024 on this platform, too small for a verbose docstring;
	    // this buffer affects only help()'s own rendering.
	    char buffer[8192];
	    if (comfuncs[i]->docstring2()!=NULL) {
	      strncpy(buffer, comfuncs[i]->docstring2(), sizeof(buffer)-1);
	    } else {
	      snprintf(buffer, sizeof(buffer),
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
		if (comfunc->hidden() || comfunc->opr_hidden()) continue;
		if (first) 
		  first = false;
		else
		  out->put('\n');
		{
		  char buffer[8192]; // see the sizing comment above for the sibling case
		  if (comfunc->docstring2()!=NULL) {
		    strncpy(buffer, comfunc->docstring2(), sizeof(buffer)-1);
		  } else {
  		    snprintf(buffer, sizeof(buffer),
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
    *out << '\0';
    // int help_str_symid = symbol_add(sbuf.str());
    ComValue retval(sbuf.str());
    push_stack(retval);
  } else
    out->flush();

  delete command_ids;
  delete comfuncs;
  delete str_flags;
  delete [] funcobj_help;

}

/*****************************************************************************/

// insertion-sort predicate for optable(:table)'s indirect index array:
// true if operator table entry a belongs before entry b.  sort is one of
// OPBY_PRIORITY or OPBY_COMMAND (OPBY_OPERATOR never reaches this -- see
// the comment where it's called).  Matches opr_tbl_print's own two
// directions -- priority highest-first, command name alphabetical --
// which aren't the same direction as each other.
static boolean optable_index_before(int a, int b, int sort) {
  if (sort == OPBY_COMMAND)
    return strcmp(symbol_pntr(opr_tbl_commid(a)), symbol_pntr(opr_tbl_commid(b))) < 0;
  return opr_tbl_priority(a) > opr_tbl_priority(b);
}

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

  /* live operator-table editing: the change takes
     effect for the next parsed expression, like a func definition. */
  static int insert_symid = symbol_add("insert");
  ComValue insertflag(stack_key(insert_symid));
  static int delete_symid = symbol_add("delete");
  ComValue deleteflag(stack_key(delete_symid));
  static int pri_symid = symbol_add("pri");
  ComValue default_pri(80);
  ComValue prival(default_pri);
  ComValue prikeyv(stack_key(pri_symid));
  if (prikeyv.is_known()) prival = prikeyv;
  static int rtol_symid = symbol_add("rtol");
  ComValue rtolflag(stack_key(rtol_symid));
  static int prefix_symid = symbol_add("prefix");
  ComValue prefixflag(stack_key(prefix_symid));
  static int postfix_symid = symbol_add("postfix");
  ComValue postfixflag(stack_key(postfix_symid));
  ComValue opstrv(stack_arg(0));    // operator string (e.g. "%%")
  ComValue commandv(stack_arg(1));  // command it maps to (e.g. "replay"), for :insert

  reset_stack();

  /* optable("%%" "replay" :insert [:pri N] [:rtol] [:prefix|:postfix]) or
     optable("%%" :delete ...) -- returns 1 on success, effective next expression. */
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

  /* :table's contract is natural table order when no sort flag is given at all,
     so track that explicitly rather than defaulting sort to OPBY_PRIORITY. */
  boolean sort_requested = bypriflag.is_true() || bycomflag.is_true() || byoprflag.is_true();
  int sort = OPBY_PRIORITY;
  if (bycomflag.is_true()) { sort = OPBY_COMMAND; }
  if (byoprflag.is_true()) { sort = OPBY_OPERATOR; }
  if (bypriflag.is_true()) { sort = OPBY_PRIORITY; }

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

    unsigned n = opr_tbl_numop_get();

    /* opr_tbl_insert keeps the table sorted by operator string, so :byopr's order is
       already natural; only :bypri/:bycom need an explicit reorder here. */
    int* indirect = new int[n];
    for (unsigned i = 0; i < n; i++) indirect[i] = i;
    if (sort_requested && sort != OPBY_OPERATOR) {
      for (unsigned i = 1; i < n; i++) {
        int key = indirect[i];
        unsigned j = i;
        while (j > 0 && optable_index_before(key, indirect[j-1], sort)) {
          indirect[j] = indirect[j-1];
          j--;
        }
        indirect[j] = key;
      }
    }

    AttributeValueList* avl = new AttributeValueList();
    for (unsigned idx = 0; idx < n; idx++) {
      unsigned i = indirect[idx];
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
    delete [] indirect;
    ComValue retval(avl);
    push_stack(retval);
    return;
  }

  opr_tbl_print(stdout, sort);
  return;
}
