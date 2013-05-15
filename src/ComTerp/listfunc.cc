/*
 * Copyright (c) 2011 Wave Semiconductor Inc.
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1999 Vectaport Inc.
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

#include <ComTerp/boolfunc.h>
#include <ComTerp/listfunc.h>
#include <ComTerp/strmfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/postfunc.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <iostream.h>
#include <string.h>

#define TITLE "ListFunc"

/*****************************************************************************/

ListFunc::ListFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ListFunc::execute() {
  ComValue listv(stack_arg_post_eval(0));
  static int strmlst_symid = symbol_add("strmlst"); // hidden debug keyword
  ComValue strmlstv(stack_key_post_eval(strmlst_symid));
  static int attr_symid = symbol_add("attr");
  ComValue attrv(stack_key_post_eval(attr_symid));
  boolean attrflag = attrv.is_true();
  static int size_symid = symbol_add("size");
  ComValue sizev(stack_key_post_eval(size_symid));
  reset_stack();

  if (attrflag) {
      AttributeList* al = new AttributeList();
      Resource::ref(al);
      ComValue retval(AttributeList::class_symid(), al);
      push_stack(retval);
      return;
  }

  AttributeValueList* avl;

  if (listv.is_array()) 
    avl = new AttributeValueList(listv.array_val());
  else {
    avl = new AttributeValueList();
    if (listv.is_stream()) {
      if (strmlstv.is_false()) {

	/* stream to list conversion */
	boolean done = false;
	while (!done) {
	  NextFunc::execute_impl(comterp(), listv);
	  ComValue topval(comterp()->pop_stack());
	  AttributeValue* newval = new AttributeValue(topval);
	  if (newval->is_unknown()) {
	    done = true;
	    delete newval;
	  } else
	    avl->Append(newval);
	}

      } else {
	/* simply return stream's internal list for debug purposes */
	if (listv.stream_list()) {
	  ComValue retval(listv.stream_list());
	  push_stack(retval);
	} else	  
	  push_stack(ComValue::nullval());
	return;
      }

    } else if (sizev.is_int()) {
      for (int i=0; i<sizev.int_val(); i++)
	avl->Append(new AttributeValue());
    } else if (nargs())
      avl->Append(new AttributeValue(listv));
  }
  Resource::ref(avl);
  ComValue retval(avl);
  push_stack(retval);
}

/*****************************************************************************/

ListAtFunc::ListAtFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ListAtFunc::execute() {
  ComValue listv(stack_arg(0));
  ComValue nv(stack_arg(1, false, ComValue::zeroval()));
  static int set_symid = symbol_add("set");
  ComValue setv(stack_key(set_symid, false, ComValue::blankval(), true /* return blank if no :set */));
  boolean setflag = !setv.is_blank();
  static int ins_symid = symbol_add("ins");
  ComValue insv(stack_key(ins_symid, false, ComValue::blankval(), true /* return blank if no :ins */));
  boolean insflag = !insv.is_blank();

  reset_stack();

  if (listv.is_type(ComValue::ArrayType) && !nv.is_nil() && 
      (nv.int_val()>=0 || insflag)) {
    AttributeValueList* avl = listv.array_val();
    if (avl) {
      if (insflag) {
	avl->Insert(nv.int_val(), new AttributeValue(insv));
	push_stack(insv);
	return;
      } else if (setflag) {
	AttributeValue* oldv = avl->Set(nv.int_val(), new AttributeValue(setv));
	delete oldv;
	push_stack(setv);
	return;
      } else {
	AttributeValue* retv = avl->Get(nv.int_val());
	if (retv)
	  push_stack(*retv);
	else
	  push_stack(ComValue::blankval());
	return;
      }
    }
  } else if (listv.is_object(AttributeList::class_symid())) {
    AttributeList* al = (AttributeList*)listv.obj_val();
    if (al && nv.int_val()<al->Number()) {
      int count = 0;
      Iterator it;
      for (al->First(it); !al->Done(it); al->Next(it)) {
	if (count==nv.int_val()) {
	  ComValue retval(Attribute::class_symid(), (void*) al->GetAttr(it));
	  if (insflag) {
	    fprintf(stderr, "Insert not yet supported for AttributeList\n");
	  } else if (setflag)
	    *al->GetAttr(it)->Value() = setv;
	  push_stack(retval);
	  return;
	}
	count++;
      }
    }
  } else if (listv.is_string()) {
    const char* str = listv.string_ptr();
    if(!setflag) {
      if(strlen(str) > nv.int_val()) {
        ComValue retval(*(str+nv.int_val()), ComValue::CharType);
        push_stack(retval);
        return;
      }
    } else {
      if(nv.int_val()<strlen(str) && nv.int_val()>=0) {
	*((char *)str+nv.int_val()) = setv.char_val();
	ComValue retval(setv);
	push_stack(retval);
	return;
      }
    }
  }
  push_stack(ComValue::nullval());
}

/*****************************************************************************/

ListSizeFunc::ListSizeFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ListSizeFunc::execute() {
  ComValue listv(stack_arg(0));
  reset_stack();

  if (listv.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = listv.array_val();
    if (avl) {
      ComValue retval(avl->Number());
      push_stack(retval);
      return;			  
    }
  } else if (listv.is_object(AttributeList::class_symid())) {
    AttributeList* al = (AttributeList*)listv.obj_val();
    if (al) {
      ComValue retval(al->Number());
      push_stack(retval);
      return;			  
    }
  } else if (listv.is_string()) {
    ComValue retval((int)strlen(listv.symbol_ptr()), ComValue::IntType);
    push_stack(retval);
    return;
  } else if (listv.is_object(FuncObj::class_symid())) {
    FuncObj* tokbuf = (FuncObj*)listv.obj_val();
    if (tokbuf) {
      ComValue retval(tokbuf->ntoks());
      push_stack(retval);
      return;			  
    }
  }
  
  push_stack(ComValue::nullval());
}


/*****************************************************************************/

int TupleFunc::_symid;

TupleFunc::TupleFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TupleFunc::execute() {
    ComValue* operand1 = new ComValue(stack_arg(0));
    ComValue* operand2 = new ComValue(stack_arg(1));
    reset_stack();

    if (!operand1->is_array() || 
	operand1->array_val()->nested_insert()) {
	AttributeValueList* avl = new AttributeValueList();
	avl->Append(operand1);
	avl->Append(operand2);
	ComValue retval(avl);
	push_stack(retval);
        if( operand1->is_array())
	  operand1->array_val()->nested_insert(false);
    } else {
        AttributeValueList* avl = operand1->array_val();
	avl->Append(operand2);
	push_stack(*operand1);
	delete operand1;
    }
    
    if (operand2->is_array())
      operand2->array_val()->nested_insert(false);
}

/*****************************************************************************/

ListIndexFunc::ListIndexFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ListIndexFunc::execute() {
  ComValue listorstrv(stack_arg(0));
  ComValue valv(stack_arg(1));
  static int last_symid = symbol_add("last");
  ComValue lastv(stack_key(last_symid));
  boolean lastflag = lastv.is_true();
  static int all_symid = symbol_add("all");
  ComValue allv(stack_key(all_symid));
  boolean allflag = allv.is_true();
  reset_stack();

  AttributeValueList *nvl = allflag ? new AttributeValueList : nil;
  if (listorstrv.is_array()) {  
      AttributeValueList* avl = listorstrv.array_val();
      Iterator it;
      if (lastflag)
        avl->Last(it);
      else
	avl->First(it);
      int index= lastflag ? avl->Number()-1 : 0;
      while(!avl->Done(it)) {
	AttributeValue* testv = avl->GetAttrVal(it);
	comterp()->push_stack(*testv);
	comterp()->push_stack(valv);
	EqualFunc eqfunc(comterp());
	eqfunc.exec(2,0);
	if(comterp()->pop_stack().is_true()) {
	  if (allflag)
	    nvl->Append(new AttributeValue(index, AttributeValue::IntType));
	  else {
	    ComValue retval(index, ComValue::IntType);
	    push_stack(retval);
	    return;
	  }
	}
	if (lastflag)
	  avl->Prev(it);
	else
	  avl->Next(it);
	index += lastflag ? -1 : 1;
      };

  } else if (listorstrv.is_string()) {

      if (valv.is_char()) {
          const char* string = listorstrv.string_ptr();
          int sz=strlen(string);
          int i= lastflag ? sz : 0;
          while(lastflag ? i>=0 : i<sz) {
              if (string[i]==valv.char_val()) {
		if(allflag)
		  nvl->Append(new AttributeValue(i, AttributeValue::IntType));
		else {
                  ComValue retval(i, ComValue::IntType);
                  push_stack(retval);
                  return;
		}
              }
              i = i + (lastflag?-1:1);
          }
      } else if (valv.is_string()) {
          const char* string = listorstrv.string_ptr();          
          const char* foundstr = strstr(string, valv.symbol_ptr());
	  const char* newfoundstr = foundstr;
          if((lastflag||allflag) && foundstr!=NULL) {
	    do {
	      foundstr = newfoundstr;
	      newfoundstr = strstr(foundstr+strlen(valv.symbol_ptr()), valv.symbol_ptr());
              if(allflag) {
                if(lastflag)
		  nvl->Prepend(new AttributeValue((int)(foundstr-string), AttributeValue::IntType));
		else
		  nvl->Append(new AttributeValue((int)(foundstr-string), AttributeValue::IntType));
	      }             
	    } while (newfoundstr!=NULL);
	  }
	  if(foundstr!=NULL && !allflag) {
	    ComValue retval((int)(foundstr-string), ComValue::IntType);
	    push_stack(retval);
	    return;
	  }
      }
  }
  
  if(allflag) {
    ComValue retval(nvl);
    push_stack(retval);
    return;
  }

  push_stack(ComValue::nullval());
  return;
}

