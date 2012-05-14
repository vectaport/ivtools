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

#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/iterator.h>

#include <iostream.h>
#include <string.h>
#include <ctype.h>

#define TITLE "SymbolFunc"

/*****************************************************************************/

SymIdFunc::SymIdFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymIdFunc::execute() {
  static int max_symid = symbol_add("max");
  boolean max_flag = stack_key(max_symid).is_true();
  if(max_flag) {
    reset_stack();
    ComValue retval(symbol_max(), ComValue::IntType);
    push_stack(retval);    
    return;
  }

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
    ComValue& val = stack_arg(i);
    if (val.is_type(AttributeValue::CommandType))
      symbol_ids[i] = val.command_symid();
    else if (val.is_type(AttributeValue::StringType))
      symbol_ids[i] = val.string_val();
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

  for (int i=0; i<numargs; i++)
    if(symbol_ids[i]!=-1) 
      symbol_del(symbol_ids[i]);
    
}

/*****************************************************************************/

SymbolFunc::SymbolFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SymbolFunc::execute() {
  // return symbol for each id argument
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
  ComValue* varvalues[numargs];
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
  ComValue symv(stack_arg(0));  // will only show up here if backquoted
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

SplitStrFunc::SplitStrFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SplitStrFunc::execute() {
  ComValue commav(',');
  ComValue symvalv(stack_arg(0));
  static int tokstr_symid = symbol_add("tokstr");
  ComValue tokstrv(stack_key(tokstr_symid, false, commav));
  boolean tokstrflag = tokstrv.is_known();
  static int tokval_symid = symbol_add("tokval");
  ComValue tokvalv(stack_key(tokval_symid, false, commav));
  boolean tokvalflag = tokvalv.is_known();
  reset_stack();

  if (symvalv.is_string()) {
    AttributeValueList* avl = new AttributeValueList();
    ComValue retval(avl);
    const char* str = symvalv.symbol_ptr();
    int len = strlen(str);
    if (!tokstrflag && !tokvalflag) {
      for (int i=0; i<len; i++)
	avl->Append(new AttributeValue(str[i]));
    } else if (tokstrflag) {
      char buffer[BUFSIZ];
      int bufoff = 0;
      char delim = tokstrv.char_val();
      while (*str) {
        int delim1=0;
        while(*str && (isspace(*str) || *str==delim)) {
          if (*str==delim) {
            if ((delim1 || avl->Number()==0) && !isspace(delim) ) {
              ComValue* comval = new ComValue(ComValue::nullval());
              avl->Append(comval);
            } else
              delim1=1;
          }
          str++;
        }
	if (!*str) {
          if (delim1 && !isspace(delim)) {
            ComValue* comval = new ComValue(ComValue::nullval());
            avl->Append(comval);
          }
          break;
        }
        while (*str && !isspace(*str) && *str!=delim && bufoff<BUFSIZ-1) {
          if(*str=='"') {
            while(*str && (*str!='"' || *(str-1)!='\\') && bufoff<BUFSIZ-1) 
              buffer[bufoff++] = *str++;
          }
          buffer[bufoff++] = *str++;
        }
	buffer[bufoff] = '\0';
	avl->Append(new AttributeValue(buffer));
	bufoff=0;
      }
    } else {
      char buffer[BUFSIZ];
      int bufoff = 0;
      char delim = tokvalv.char_val();
      while (*str) {
        int delim1=0;
	while(*str && (isspace(*str) || *str==delim)) {
          if (*str==delim) {
              if((delim1 || avl->Number()==0) && !isspace(delim)) {
              ComValue* comval = new ComValue(ComValue::nullval());
              avl->Append(comval);
            } else 
              delim1=1;
          }
          str++;
        }
	if (!*str) {
            if (delim1 && !isspace(delim)) {
            ComValue* comval = new ComValue(ComValue::nullval());
            avl->Append(comval);
          }
          break;
        }
	while (*str && !isspace(*str) && *str!=delim && bufoff<BUFSIZ-1) {
	  buffer[bufoff++] = *str++;
	}
	buffer[bufoff] = '\0';
        ComValue* comval = new ComValue(((ComTerpServ*)_comterp)->run(buffer, true /*nested*/));
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
      if (!clearflag) {
	ComValue* av = 
	  new ComValue(symbol_ids[i], AttributeValue::SymbolType);
	av->global_flag(true);
	av->bquote(1);
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
      ComValue retval (symbol_ids[0], AttributeValue::SymbolType);
      retval.global_flag(true);
      retval.bquote(1);
      push_stack(retval);
    } else {
      void* oldval = nil;
      comterp()->globaltable()->find_and_remove(oldval, symbol_ids[0]);
      if (oldval) delete (ComValue*)oldval;
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
  reset_stack();

  if (strv.is_unknown()) {
    push_stack(ComValue::nullval());
    return;
  }

  const char* string = strv.symbol_ptr();
  const int n = !afterflag ? nv.int_val()+1 : strlen(string)-nv.int_val()+1;
  char buffer[n];
  strncpy(buffer, string+(afterflag?nv.int_val():0), n-1);
  buffer[n-1] = '\0';

  ComValue retval(buffer);
  push_stack(retval);
}


