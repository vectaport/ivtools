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

#include <ComTerp/comhandler.h>

#include <ComTerp/symbolfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/iterator.h>

#include <iostream.h>
#include <string.h>
#include <ctype.h>
#include <vector>

using std::cout;

#define TITLE "SymbolFunc"

/*****************************************************************************/

SymIdFunc::SymIdFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymIdFunc::execute() {
  static int max_symid = symbol_add("max");
  boolean max_flag = stack_key(max_symid).is_true();
  static int cnt_symid = symbol_add("cnt");
  boolean cnt_flag = stack_key(cnt_symid).is_true();
  if(max_flag) {
    reset_stack();
    ComValue retval(symbol_max(), ComValue::IntType);
    push_stack(retval);    
    return;
  }
  if(cnt_flag) {
    reset_stack();
    ComValue retval(symbol_cnt(), ComValue::IntType);
    push_stack(retval);    
    return;
  }

  // return id of each symbol in the arguments
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) return;
  std::vector<int> symbol_ids(numargs);
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
  std::vector<int> symbol_ids(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i);
    std::string scratch;
    if (val.is_type(AttributeValue::CommandType))
      symbol_ids[i] = val.command_symid();
    else if (val.is_type(AttributeValue::StringType))
      /* symbol_add(), not val.string_val() -- a writable string's
         symid isn't guaranteed findable, and cstr() is slice-aware */
      symbol_ids[i] = symbol_add(val.cstr(scratch));
    else if (val.is_type(AttributeValue::SymbolType))
      symbol_ids[i] = val.symbol_val();
    else 
      symbol_ids[i] = -1;
    if(symbol_ids[i]!=-1) 
      symbol_reference(symbol_ids[i]);
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      ComValue* av = new ComValue(symbol_ids[i], AttributeValue::SymbolType);
      // av->bquote(1);
      if (symbol_ids[i]<0) av->type(ComValue::UnknownType);
      avl->Append(av);
    }
    push_stack(retval);
  } else {
    ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
    if (symbol_ids[0]<0) retval.type(ComValue::UnknownType);
    // retval.bquote(1);
    push_stack(retval);
  }

  // releases the temporary ref taken above; the returned
  // value's own ref (from ref_as_needed()) is permanent, never auto-unref'd
  for (int i=0; i<numargs; i++)
    if(symbol_ids[i]!=-1)
      symbol_unref(symbol_ids[i]);

}

/*****************************************************************************/

SymbolFunc::SymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymbolFunc::execute() {
  // return symbol for each id argument
  int numargs = nargs();
  if (!numargs) return;
  std::vector<int> symbol_ids(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_symbol()) {
      lookup_symval(val);
    }
    if (val.is_char() || val.is_short() || val.is_int()) {
      symbol_ids[i] = val.int_val();
    } else 
      symbol_ids[i] = -1;
  }
  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      ComValue* av = new ComValue(symbol_ids[i], AttributeValue::SymbolType);
      av->bquote(1);
      avl->Append(av);
    }
    push_stack(retval);
  } else {
    ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
    retval.bquote(1);
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
  std::vector<ComValue*> varvalues(numargs);
  for (int i=0; i<numargs; i++) {

    // return fully-evaluated value: expression --> symbol --> value
    varvalues[i] = &stack_arg(i, false); 
    lookup_symval(*varvalues[i]);
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

SymVarFunc::SymVarFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymVarFunc::execute() {
  ComValue symv(stack_arg(0));
  reset_stack();
  push_stack(symv);
}


/*****************************************************************************/

SymStrFunc::SymStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymStrFunc::execute() {
  ComValue symv(stack_arg(0));
  reset_stack();
  symv.type(ComValue::StringType);
  push_stack(symv);
}


/*****************************************************************************/

StrRefFunc::StrRefFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void StrRefFunc::execute() {
  ComValue strv(stack_arg(0));
  reset_stack();
  if (strv.type()==ComValue::StringType) {
    ComValue retval(symbol_refcount(strv.symbol_val()), ComValue::IntType);
    push_stack(retval);
  } 
  else if (strv.type()==ComValue::IntType) {
    ComValue retval(symbol_refcount(strv.int_val()), ComValue::IntType);
    push_stack(retval);
  } else
    push_stack(ComValue::nullval());
  return;  
}


/*****************************************************************************/

StringFunc::StringFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void StringFunc::execute() {
  ComValue capv(stack_arg(0));
  static int spaces_symid = symbol_add("spaces");
  ComValue spacesv(stack_key(spaces_symid));
  boolean spacesflag = spacesv.is_true();
  static int raw_symid = symbol_add("raw");
  ComValue rawv(stack_key(raw_symid));
  boolean rawflag = rawv.is_true();
  reset_stack();

  /* string(str) is a copy, not a capacity request: :raw copies the full
     slice (append_str()'s own byte range), bare stops at the first NUL
     (cstr()'s C-string contract) -- see SLICES.md for the distinction. */
  if (capv.is_string()) {
    if (rawflag) {
      ComValue dest("");
      ComValue retval = dest.append_str(capv, true);
      push_stack(retval);
    } else {
      std::string scratch;
      ComValue retval(capv.cstr(scratch));
      push_stack(retval);
    }
    return;
  }

  int cap = capv.int_val();
  int newid = cap>=0 ? symbol_new((unsigned)cap, spacesflag) : -1;
  if (newid<0) {
    push_stack(ComValue::nullval());
    return;
  }
  ComValue retval((unsigned int)newid, ComValue::StringType);
  push_stack(retval);
}

/*****************************************************************************/

StrCapFunc::StrCapFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void StrCapFunc::execute() {
  ComValue strv(stack_arg(0));
  reset_stack();
  if (strv.type()==ComValue::StringType) {
    ComValue retval(symbol_len(strv.symbol_val()), ComValue::IntType);
    push_stack(retval);
  } else
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

SplitStrFunc::SplitStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

/* A bare string delimiter falls through char_val()'s default case
   (attrvalue.c) as '\0', so a StringType :tokstr/:tokval used to
   silently match nothing and split() never split at all.  A
   one-character string is accepted as the obvious equivalent of the
   CharType form.  A real multi-character (substring) delimiter isn't
   supported -- the scan in SplitStrFunc::execute() compares one
   character at a time throughout (the delimiter test, :keep's
   reinserted delimiter value, and the isspace-as-alternate-delimiter
   rule all assume a single char), so matching a whole substring would
   mean rewriting that scan, not just this coercion -- reported and
   refused outright (nil) rather than silently treated as a harmless
   one-token split. */
static boolean coerce_split_string_delim(ComValue& delimv, const char* keyword,
                                          ComFunc* func) {
  std::string scratch;
  const char* str = delimv.cstr(scratch);
  if (strlen(str) == 1) {
    delimv = ComValue(str[0]);
    return true;
  }
  fprintf(stderr, "Error: split() :%s \"%s\" is not a single character (line %d)\n",
          keyword, str, func->funcstate()->linenum());
  return false;
}

void SplitStrFunc::execute() {
  ComValue zerov(0, ComValue::IntType);
  ComValue commav(',');
  ComValue symvalv(stack_arg(0));
  static int tokstr_symid = symbol_add("tokstr");
  ComValue tokstrv(stack_key(tokstr_symid, false, zerov));
  boolean tokstrflag = tokstrv.is_known();
  static int tokval_symid = symbol_add("tokval");
  ComValue tokvalv(stack_key(tokval_symid, false, zerov));
  boolean tokvalflag = tokvalv.is_known();
  static int keep_symid = symbol_add("keep");
  ComValue keepflagv(stack_key(keep_symid));
  boolean keepflag = keepflagv.is_true();
  static int reverse_symid = symbol_add("reverse");
  ComValue reverseflagv(stack_key(reverse_symid));
  boolean reverseflag = reverseflagv.is_true();
  reset_stack();

  boolean tokstr_charflag = tokstrv.is_type(ComValue::CharType);
  if(tokstrv.is_type(ComValue::IntType)) tokstrv = commav;
  if(tokvalv.is_type(ComValue::IntType)) tokvalv = commav;

  if (tokstrflag && tokstrv.is_string()) {
    if (!coerce_split_string_delim(tokstrv, "tokstr", this)) {
      push_stack(ComValue::nullval());
      return;
    }
    /* a one-char string delimiter must behave like its coerced CharType form,
       so set tokstr_charflag now */
    tokstr_charflag = true;
  }
  if (tokvalflag && tokvalv.is_string() &&
      !coerce_split_string_delim(tokvalv, "tokval", this)) {
    push_stack(ComValue::nullval());
    return;
  }

  if (symvalv.is_string()) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    /* cstr(), not symbol_ptr() -- symvalv can be a slice,
       and everything below reads through this one str pointer */
    std::string scratch;
    const char* str = symvalv.cstr(scratch);
    const char* strbase = str;
    int len = strlen(str);
    char delim = tokstrv.char_val();
    char delimstr[2];
    delimstr[0] = delim;
    delimstr[1] = '\0';
    if (!tokstrflag && !tokvalflag) {
      for (int i=0; i<len; i++)
        if (reverseflag) 
	  avl->Prepend(new AttributeValue(str[i]));
	else
	  avl->Append(new AttributeValue(str[i]));
    } else if (tokstrflag) {
      char buffer[BUFSIZ];
      int bufoff = 0;
      while (*str) {
        int delim1=0;
        boolean delim_seen=false;
        while(*str && isspace(*str) || *str==delim) {
          if (*str==delim) {
            if ((delim1 || avl->Number()==0) && delim!=' ') {
	      if (!keepflag) {
		ComValue* comval = new ComValue(ComValue::nullval());
		if (reverseflag) 
		  avl->Prepend(comval);
		else
		  avl->Append(comval);
	      }
            } else {
              delim1=1;
              delim_seen=true;
            }
          }
          str++;
        }
	if (!*str) {
          if (delim1 && !isspace(delim)) {
	    if (!keepflag) {
	      ComValue* comval = new ComValue(ComValue::nullval());
	      if (reverseflag) 
		avl->Prepend(comval);
	      else
		avl->Append(comval);
	    }
          }
          break;
        }
	if (keepflag && delim_seen && avl->Number()>0) {
	  if (reverseflag)
	    avl->Prepend(new ComValue(delimstr));
	  else
	    avl->Append(new ComValue(delimstr));
	}
        // some uses need this, can't remember which
         while (*str && (/**/tokstr_charflag?(*str!='\n'&&*str!='\r'):/**/!isspace(*str)) && *str!=delim && bufoff<BUFSIZ-1) {
          if(*str=='"') {
            buffer[bufoff++] = *str++;
            while(*str && *str!='"' && bufoff<BUFSIZ-1) {
               buffer[bufoff++] = *str++;
	    }
          }
          buffer[bufoff++] = *str++;
        }
	buffer[bufoff] = '\0';
	if (reverseflag)
	  avl->Prepend(new ComValue(buffer));
	else
	  avl->Append(new ComValue(buffer));
	bufoff=0;
      }
    } else {
      char buffer[BUFSIZ];
      int bufoff = 0;
      char delim = tokvalv.char_val();
      while (*str) {
        int delim1=0;
        boolean delim_seen=false;
	while(*str && (isspace(*str) || *str==delim)) {
          if (*str==delim) {
              if((delim1 || avl->Number()==0) && !isspace(delim)) {
	      if (!keepflag) {
                ComValue* comval = new ComValue(ComValue::nullval());
	        if (reverseflag) 
		  avl->Prepend(comval);
	        else
		  avl->Append(comval);
	      }
            } else {
              delim1=1;
              delim_seen=true;
            }
          }
          str++;
        }
	if (!*str) {
            if (delim1 && !isspace(delim)) {
	    if (!keepflag) {
              ComValue* comval = new ComValue(ComValue::nullval());
	      if (reverseflag) 
	        avl->Prepend(comval);
	      else
	        avl->Append(comval);
	    }
	    }
          break;
        }
	if (keepflag && delim_seen && avl->Number()>0) {
	  ComValue* comval = new ComValue(delim);
	  if (reverseflag)
	    avl->Prepend(comval);
	  else
	    avl->Append(comval);
	}
	while (*str && !isspace(*str) && *str!=delim && bufoff<BUFSIZ-1) {
	  buffer[bufoff++] = *str++;
	}
	buffer[bufoff] = '\0';
        ComValue* comval = new ComValue(((ComTerpServ*)_comterp)->run(buffer, true /*nested*/));
	if (reverseflag) 
	  avl->Prepend(comval);
	else
	  avl->Append(comval);
	bufoff=0;
      }
    }
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
      std::vector<char> cbuf(avl->Number()+1);
      Iterator i;
      int cnt=0;
      for (avl->First(i); !avl->Done(i); avl->Next(i)) {
	cbuf[cnt] = avl->GetAttrVal(i)->char_val();
	cnt++;
      }
      cbuf[cnt] = '\0';

    ComValue retval(symbol_add(&cbuf[0]), symflag ? ComValue::SymbolType : ComValue::StringType);
    push_stack(retval);
    return;
    }
  }
  push_stack(ComValue::nullval());
}


/*****************************************************************************/

GlobalSymbolFunc::GlobalSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void GlobalSymbolFunc::execute() {
  static int clear_symid = symbol_add("clear");
  ComValue clearflagv(stack_key(clear_symid));
  boolean clearflag = clearflagv.is_true();
  static int cnt_symid = symbol_add("cnt");
  ComValue cntflagv(stack_key(cnt_symid));
  boolean cntflag = cntflagv.is_true();

  if (cntflag) {
    reset_stack();
    TableIterator(ComValueTable) it(*comterp()->globaltable());
    int cnt=0;
    while(it.more()) {
      cnt++;
      it.next();
    }
    ComValue retval(cnt);
    push_stack(retval);
    return;
  }

  // return symbol(s) with global flag set
  boolean noargs = !nargs() && !nkeys();
  int numargs = nargs();
  if (!numargs) {
    reset_stack();
    return;
  }
  std::vector<int> symbol_ids(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_symbol())
      symbol_ids[i] = val.symbol_val();
    else if (val.is_command()) {
      /* only reachable via an explicit backquote, which
         suppresses self-invoke but doesn't permit a command name as a variable */
      cout << "WARNING:  \"" << val.command_name() << "\" is a command"
		   " -- global() can't use it as a variable name -- line "
		<< funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    } else {
      /* val resolved to neither symbol nor command --
         fail loudly rather than silently key off a shared, meaningless -1 slot */
      cout << "WARNING:  global() argument did not resolve to a symbol"
		   " (if its name collides with a command, backquote it to"
		   " confirm) -- line " << funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
  }
  boolean assign_next = comterp()->stack_top(nargs()+nkeys()).lhs_assign();

  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      if (!clearflag) {
	ComValue* av =
	  new ComValue(symbol_ids[i], AttributeValue::SymbolType);
	if (assign_next) {
	  av->global_flag(true);
	  av->bquote(1);
	} else {
	  ComValue* gval = comterp()->globalvalue(symbol_ids[i]);
	  if (gval && !gval->is_unknown())
	    *av = *gval;
	  else
	    av->type(ComValue::UnknownType);
	}
	avl->Append(av);
      } else {
	void* oldval = nil;
	comterp()->globaltable()->find_and_remove(oldval, symbol_ids[i]);
	if (oldval) delete (ComValue*)oldval;
      }
    }
    push_stack(retval);
  } else {
    
    if (!clearflag) {
      if (assign_next) {
        ComValue retval(symbol_ids[0], AttributeValue::SymbolType);
        retval.global_flag(true);
        retval.bquote(1);
        push_stack(retval);
      } else {
        ComValue* gval = comterp()->globalvalue(symbol_ids[0]);
        if (gval && !gval->is_unknown())
          push_stack(*gval);
        else
          push_stack(ComValue::nullval());
      }
    } else {
      void* oldval = nil;
      comterp()->globaltable()->find_and_remove(oldval, symbol_ids[0]);
      if (oldval) delete (ComValue*)oldval;
    }
  }

}

/*****************************************************************************/

LocalSymbolFunc::LocalSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void LocalSymbolFunc::execute() {
  static int clear_symid = symbol_add("clear");
  ComValue clearflagv(stack_key(clear_symid));
  boolean clearflag = clearflagv.is_true();
  static int cnt_symid = symbol_add("cnt");
  ComValue cntflagv(stack_key(cnt_symid));
  boolean cntflag = cntflagv.is_true();

  if (cntflag) {
    reset_stack();
    TableIterator(ComValueTable) it(*comterp()->localtable());
    int cnt=0;
    while(it.more()) {
      cnt++;
      it.next();
    }
    ComValue retval(cnt);
    push_stack(retval);
    return;
  }

  /* local() names the per-instance symbol table directly,
     with no func-frame shadow or globaltable fallback */
  int numargs = nargs();
  if (!numargs) {
    reset_stack();
    return;
  }
  std::vector<int> symbol_ids(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_symbol())
      symbol_ids[i] = val.symbol_val();
    else if (val.is_command()) {
      /* only reachable via an explicit backquote, which
         suppresses self-invoke but doesn't permit a command name as a variable */
      cout << "WARNING:  \"" << val.command_name() << "\" is a command"
		   " -- local() can't use it as a variable name -- line "
		<< funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    } else {
      /* val resolved to neither symbol nor command --
         fail loudly rather than silently key off a shared, meaningless -1 slot */
      cout << "WARNING:  local() argument did not resolve to a symbol"
		   " (if its name collides with a command, backquote it to"
		   " confirm) -- line " << funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
  }
  boolean assign_next = comterp()->stack_top(nargs()+nkeys()).lhs_assign();

  reset_stack();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      if (!clearflag) {
	ComValue* av =
	  new ComValue(symbol_ids[i], AttributeValue::SymbolType);
	if (assign_next) {
	  av->local_flag(true);
	  av->bquote(1);
	} else {
	  ComValue* lval = comterp()->localvalue(symbol_ids[i]);
	  if (lval && !lval->is_unknown())
	    *av = *lval;
	  else
	    av->type(ComValue::UnknownType);
	}
	avl->Append(av);
      } else {
	void* oldval = nil;
	comterp()->localtable()->find_and_remove(oldval, symbol_ids[i]);
	if (oldval) delete (ComValue*)oldval;
      }
    }
    push_stack(retval);
  } else {

    if (!clearflag) {
      if (assign_next) {
        ComValue retval(symbol_ids[0], AttributeValue::SymbolType);
        retval.local_flag(true);
        retval.bquote(1);
        push_stack(retval);
      } else {
        ComValue* lval = comterp()->localvalue(symbol_ids[0]);
        if (lval && !lval->is_unknown())
          push_stack(*lval);
        else
          push_stack(ComValue::nullval());
      }
    } else {
      void* oldval = nil;
      comterp()->localtable()->find_and_remove(oldval, symbol_ids[0]);
      if (oldval) delete (ComValue*)oldval;
    }
  }

}


/*****************************************************************************/

TempSymbolFunc::TempSymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TempSymbolFunc::execute() {
  /* names a variable private to the current func call's temp frame -- see
     ComTerp::get_tempframe() and run_funcobj_body().  Only the assignment
     that creates the variable needs the temp() wrapper; lookup_symval()
     checks the temp frame ahead of the func's own attrlist on every bare
     reference afterward, for the life of this call. */
  int numargs = nargs();
  if (!numargs) {
    reset_stack();
    return;
  }
  std::vector<int> symbol_ids(numargs);
  for (int i=0; i<numargs; i++) {
    ComValue& val = stack_arg(i, true);
    if (val.is_symbol())
      symbol_ids[i] = val.symbol_val();
    else if (val.is_command()) {
      /* only reachable via an explicit backquote, which
         suppresses self-invoke but doesn't permit a command name as a variable */
      cout << "WARNING:  \"" << val.command_name() << "\" is a command"
		   " -- temp() can't use it as a variable name -- line "
	    << funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    } else {
      /* val resolved to neither symbol nor command --
         fail loudly rather than silently key off a shared, meaningless -1 slot */
      cout << "WARNING:  temp() argument did not resolve to a symbol"
		   " (if its name collides with a command, backquote it to"
		   " confirm) -- line " << funcstate()->linenum() << "\n";
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
  }
  boolean assign_next = comterp()->stack_top(nargs()+nkeys()).lhs_assign();

  reset_stack();

  AttributeList* tempframe = comterp()->get_tempframe();

  if (numargs>1) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    for (int i=0; i<numargs; i++) {
      ComValue* av = new ComValue(symbol_ids[i], AttributeValue::SymbolType);
      if (assign_next) {
	av->temp_flag(true);
	av->bquote(1);
      } else {
	AttributeValue* tval = tempframe ? tempframe->find(symbol_ids[i]) : nil;
	if (tval && !tval->is_unknown())
	  *av = *tval;
	else
	  av->type(ComValue::UnknownType);
      }
      avl->Append(av);
    }
    push_stack(retval);
  } else {
    if (assign_next) {
      ComValue retval(symbol_ids[0], AttributeValue::SymbolType);
      retval.temp_flag(true);
      retval.bquote(1);
      push_stack(retval);
    } else {
      AttributeValue* tval = tempframe ? tempframe->find(symbol_ids[0]) : nil;
      if (tval && !tval->is_unknown())
	push_stack(*tval);
      else
	push_stack(ComValue::nullval());
    }
  }

}


/*****************************************************************************/

SubStrFunc::SubStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SubStrFunc::execute() {
  ComValue strv(stack_arg(0));
  ComValue nv(stack_arg(1));
  static int after_symid = symbol_add("after");
  ComValue afterflagv(stack_key(after_symid));
  boolean afterflag = afterflagv.is_true();
  static int nonil_symid = symbol_add("nonil");
  ComValue nonilflagv(stack_key(nonil_symid));
  boolean nonilflag = nonilflagv.is_true();
  reset_stack();

  if (strv.is_unknown()) {
    push_stack(ComValue::nullval());
    return;
  }

  const char* string = strv.symbol_ptr();
  int n;
  int offset;
  if(!nv.is_string()) {
    n = !afterflag ? nv.int_val() : strlen(string)-nv.int_val();
    offset = afterflag ? nv.int_val() : 0;
  }
  else {
    const char* foundstr = strstr(string, nv.symbol_ptr());
    if(foundstr==NULL) {
      if(nonilflag)
	push_stack(strv);
      else
	push_stack(ComValue::nullval());
      return;
    }
    n = afterflag ?  strlen(string)-(foundstr-string)-strlen(nv.symbol_ptr()) : foundstr-string;
    offset = afterflag ? foundstr-string+strlen(nv.symbol_ptr()) : 0;
  };
  if(n>0) { 
    std::vector<char> buffer(n+1);
    strncpy(&buffer[0], string+offset, n);
    buffer[n] = '\0';
    ComValue retval(&buffer[0]);
    push_stack(retval);
  } else
    if(nonilflag)
      push_stack(strv);
    else
      push_stack(ComValue::nullval());
}

/*****************************************************************************/

AppendFunc::AppendFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void AppendFunc::execute() {
  /* stack_arg(0,true) peeks arg0 unresolved -- only a bare symbol carries
     a name to write the grown result back under. */
  ComValue arg0(stack_arg(0, true));
  boolean have_name = arg0.is_symbol();
  ComValue dest;
  if (have_name) {
    ComValue probe(arg0);
    dest = comterp()->lookup_symval(probe);
  } else {
    dest = arg0;
  }
  ComValue addend(stack_arg(1));
  reset_stack();

  if (!dest.is_only_string() || !(addend.is_string() || addend.is_char())) {
    push_stack(ComValue::nullval());
    return;
  }

  ComValue result = dest.append_str(addend, true /* headroom */);

  /* write back only on success, scoped exactly like a bare read of arg0:
     inside a func frame that's AssignFunc's own attrlist branch, not
     assign_symval() (local/global only, no frame) -- same split AssignFunc
     makes on its bare '=' path. */
  if (have_name && result.is_only_string()) {
    AttributeList* attrlist = comterp()->get_attributes();
    if (attrlist) {
      Resource::ref(attrlist);
      Attribute* attr = new Attribute(arg0.symbol_val(), new ComValue(result));
      attrlist->add_attribute(attr);
      Unref(attrlist);
    } else {
      comterp()->assign_symval(arg0.symbol_val(), new ComValue(result));
    }
  }

  push_stack(result);
}


