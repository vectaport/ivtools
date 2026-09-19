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
#include <algorithm>
#include <vector>

#define TITLE "ListFunc"

/*****************************************************************************/

/* visited tracks every AttributeValueList/AttributeList already walked in
   this search, by pointer, so a container reachable through more than one
   path in 'val' -- or one that is itself part of a cycle unrelated to
   target -- is walked at most once, rather than driving the recursion as
   deep as the structure lets it go. */
static boolean value_contains_container_rec(AttributeValue& val, void* target,
					     boolean target_is_attrlist,
					     std::vector<void*>& visited) {
  if (val.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = val.array_val();
    if (!target_is_attrlist && (void*)avl == target) return true;
    if (avl) {
      if (std::find(visited.begin(), visited.end(), (void*)avl) != visited.end())
	return false;
      visited.push_back((void*)avl);
      ALIterator it;
      for (avl->First(it); !avl->Done(it); avl->Next(it)) {
	AttributeValue* elt = avl->GetAttrVal(it);
	if (elt && value_contains_container_rec(*elt, target, target_is_attrlist, visited))
	  return true;
      }
    }
  } else if (val.is_object(AttributeList::class_symid())) {
    AttributeList* al = (AttributeList*)val.obj_val();
    if (target_is_attrlist && (void*)al == target) return true;
    if (al) {
      if (std::find(visited.begin(), visited.end(), (void*)al) != visited.end())
	return false;
      visited.push_back((void*)al);
      Iterator it;
      for (al->First(it); !al->Done(it); al->Next(it)) {
	Attribute* attr = al->GetAttr(it);
	if (attr && attr->Value() &&
	    value_contains_container_rec(*attr->Value(), target, target_is_attrlist, visited))
	  return true;
      }
    }
  }
  return false;
}

boolean value_contains_container(AttributeValue& val, void* target,
				  boolean target_is_attrlist) {
  std::vector<void*> visited;
  return value_contains_container_rec(val, target, target_is_attrlist, visited);
}

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
  static int colon_symid = symbol_add("colon");
  ComValue colonv(stack_key_post_eval(colon_symid));
  boolean colonflag = colonv.is_true();
  reset_stack();

  if (attrflag) {
      AttributeList* al = new AttributeList();
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
	  if (topval.is_unknown() || StrmFunc::is_delimiter(topval)) {
	    done = true;
	  } else
	    avl->Append(new AttributeValue(topval));
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
  /* no manual Resource::ref(avl) here -- the ComValue ctor already refs it;
     an extra ref would leak an AttributeValueList per list() call */
  ComValue retval(avl);
  /* list(:colon) -- an empty coloned() list, a colon-chain placeholder,
     built elsewhere, still reading as coloned() once populated */
  if (colonflag)
    retval.coloned(1);
  push_stack(retval);
}

/*****************************************************************************/

AttrListFunc::AttrListFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void AttrListFunc::execute() {
    AttributeList* al = stack_keys();
    reset_stack();
    ComValue retval(AttributeList::class_symid(), al);
    push_stack(retval);
}

/*****************************************************************************/

ListAtFunc::ListAtFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ListAtFunc::execute() {
  ComValue listv(stack_arg(0));
  ComValue nv(stack_arg(1, false, ComValue::zeroval()));
  static int raw_symid = symbol_add("raw");
  ComValue rawv(stack_key(raw_symid));
  boolean rawflag = rawv.is_true();
  /* :raw is accepted but a no-op -- nothing dispatches on coloned(),
     so a coloned index reads as an ordinary array index either way */
  (void)rawflag;

  /* str@lo:hi builds a slice sharing str's symid via sliceoff/slicelen;
     hi is exclusive, Go-style; str@lo:hi:cap (Go's full slice expression)
     also bounds how far append() may grow it in place before reallocating;
     slicing a plain list falls through to nil */
  if (listv.is_only_string() && nv.is_type(ComValue::ArrayType) && nv.coloned()) {
    AttributeValueList* range = nv.array_val();
    boolean forwrite = comterp()->stack_top(nkeys()+1).lhs_assign();
    ComValue retval(ComValue::nullval());
    boolean have_cap = range && range->Number()==3;
    if (!forwrite && range && (range->Number()==2 || have_cap)) {
      ComValue loval(*range->Get(0));
      ComValue hival(*range->Get(1));
      loval = comterp()->lookup_symval(loval);
      hival = comterp()->lookup_symval(hival);
      ComValue capval;
      if (have_cap) {
        capval = *range->Get(2);
        capval = comterp()->lookup_symval(capval);
      }
      if (loval.type()==ComValue::IntType && hival.type()==ComValue::IntType &&
          (!have_cap || capval.type()==ComValue::IntType)) {
        int lo = loval.int_val();
        int hi = hival.int_val();
        /* slicing a slice bounds against listv's own window -- its own
           granted extra room included, so re-slicing a capped slice can
           still reach into the room it was given; the offset composes
           onto it, staying off the parent */
        int base = listv.sliced() ? listv.sliceoff() : 0;
        int cap = listv.sliced() ? listv.slicelen()+listv.slicecap() : symbol_len(listv.string_val());
        int room = 0;
        boolean cap_ok = true;
        if (have_cap) {
          int mx = capval.int_val();
          cap_ok = mx>=hi && mx<=cap && (mx-hi)<=0xffff;
          room = cap_ok ? mx-hi : 0;
        }
        if (lo>=0 && hi>=lo && hi<=cap && cap_ok) {
          retval = ComValue(listv.string_val(), ComValue::StringType);
          /* this ctor doesn't ref, unlike (int, ValueType);
             ref_as_needed() stops listv's dtor unrefing the symid */
          retval.ref_as_needed();
          retval.sliceoff(base+lo);
          retval.slicelen(hi-lo);
          retval.sliced(1);
          if (have_cap) retval.slicecap(room);
        }
      }
    }
    reset_stack();
    push_stack(retval);
    return;
  }

  /* a list has no position, and int_val() answers 0 for one,
     so a list index misreads as 0; nil until gather semantics land */
  if (nv.is_array()) {
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  /* lst@N=val: lhs_assign() on this token means hand back [list, idx],
     not a value, then re-drive via :set; index must be non-negative */
  if ((listv.is_type(ComValue::ArrayType) || listv.is_only_string()) &&
      (nv.is_nil() || nv.int_val()>=0) &&
      comterp()->stack_top(nkeys()+1).lhs_assign()) {
    /* a string takes the same route since s@N='c' has somewhere to write;
       is_only_string(), not is_string() -- symbol text is its identity */
    int nvv;
    if (listv.is_only_string()) {
      const char* str = listv.string_ptr();
      nvv = nv.is_nil() ? (int)strlen(str)-1 : nv.int_val();
    } else {
      AttributeValueList* avl = listv.array_val();
      nvv = nv.is_nil() ? (avl ? avl->Number()-1 : 0) : nv.int_val();
    }
    reset_stack();
    AttributeValueList* pair = new AttributeValueList();
    pair->Append(new AttributeValue(listv));
    ComValue nvvval(nvv);
    pair->Append(new AttributeValue(nvvval));
    ComValue retval(pair);
    retval.lhs_assign(1);
    push_stack(retval);
    return;
  }

  static int set_symid = symbol_add("set");
  ComValue setv(stack_key(set_symid, false, ComValue::blankval()));  // bare :set -> blank (nothing to set)
  if (setv.is_unknown()) setv = ComValue::blankval();                 // absent :set -> also blank
  boolean setflag = !setv.is_blank();
  static int ins_symid = symbol_add("ins");
  ComValue insv(stack_key(ins_symid, false, ComValue::blankval()));
  if (insv.is_unknown()) insv = ComValue::blankval();
  boolean insflag = !insv.is_blank();
  static int del_symid = symbol_add("del");
  ComValue delv(stack_key(del_symid));
  boolean delflag = delv.is_true();

  reset_stack();

  if (listv.is_type(ComValue::ArrayType) &&
      (nv.is_nil() || nv.int_val()>=0 )) {
    AttributeValueList* avl = listv.array_val();
    int nvv = nv.is_nil() ? avl->Number()-1 : nv.int_val();
    if (avl) {
      if (insflag) {
	if (value_contains_container(insv, (void*)avl, false)) {
	  fprintf(stderr, "WARNING: refusing to insert a list into itself -- line %d\n",
		  funcstate()->linenum());
	  push_stack(ComValue::nullval());
	  return;
	}
	avl->Insert(nvv, new AttributeValue(insv));
	push_stack(insv);
	return;
      } else if (setflag) {
	/* nvv, not nv.int_val(): a nil index means the last item,
	   same as every other branch here */
	if (value_contains_container(setv, (void*)avl, false)) {
	  fprintf(stderr, "WARNING: refusing to insert a list into itself -- line %d\n",
		  funcstate()->linenum());
	  push_stack(ComValue::nullval());
	  return;
	}
	AttributeValue* oldv = avl->Set(nvv, new AttributeValue(setv));
	delete oldv;
	push_stack(setv);
	return;
      } else if (delflag) {
	AttributeValue* oldv = avl->Get(nvv);
	if (oldv) {
	  ComValue rv = *oldv;
	  if (rv.is_symbol()) rv.bquote(1);
	  avl->Remove(oldv);
	  delete oldv;
	  push_stack(rv);
	} else
	  push_stack(ComValue::blankval());
	return;
      } else {
	AttributeValue* retv = avl->Get(nvv);
	if (retv) {
          ComValue rv = *retv;
          if(rv.is_symbol()) rv.bquote(1);
	  push_stack(rv);
	} else
	  push_stack(ComValue::blankval());
	return;
      }
    }
  } else if (listv.is_object(AttributeList::class_symid())) {
    AttributeList* al = (AttributeList*)listv.obj_val();
    int nvv = nv.is_nil() ? al->Number()-1 : nv.int_val();
    if (al && nvv>=0 && nvv<al->Number()) {
      int count = 0;
      Iterator it;
      for (al->First(it); !al->Done(it); al->Next(it)) {
	if (count==nvv) {
	  Attribute* attr = al->GetAttr(it);
	  if (insflag) {
	    fprintf(stderr, "Insert not yet supported for AttributeList\n");
	  } else if (setflag) {
	    if (value_contains_container(setv, (void*)al, true)) {
	      fprintf(stderr, "WARNING: refusing to insert an attrlist into itself -- line %d\n",
		      funcstate()->linenum());
	      push_stack(ComValue::nullval());
	      return;
	    }
	    *attr->Value() = setv;
	  }
	  /* return a detached single-entry attrlist, e.g. (:y 20),
	     not a live handle, since al@n=val must never write through */
	  AttributeList* singleton = new AttributeList();
	  singleton->add_attribute(new Attribute(attr->SymbolId(), new AttributeValue(*attr->Value())));
	  ComValue retval(AttributeList::class_symid(), (void*)singleton);
	  push_stack(retval);
	  return;
	}
	count++;
      }
    }
  } else if (listv.is_string()) {
    const char* str = listv.string_ptr();
    /* a sliced listv indexes via its own window into the parent buffer;
       bound by symbol_len(), not strlen() -- string(cap) starts all NUL */
    boolean isslice = listv.sliced();
    int base = isslice ? listv.sliceoff() : 0;
    int cap = isslice ? listv.slicelen() : symbol_len(listv.string_val());
    /* nil means the last character: the slice's last index when sliced,
       otherwise the parent's strlen()-based last character */
    int nvv = nv.is_nil() ? (isslice ? cap-1 : (int)strlen(str)-1) : nv.int_val();
    if(!setflag) {
      if(nvv>=0 && nvv<cap) {
        ComValue retval(*(str+base+nvv), ComValue::CharType);
        push_stack(retval);
        return;
      }
    } else if (listv.is_only_string()) {
      /* is_string() also matches symbols, whose chars are their identity,
         so writing here would edit every value sharing the symbol */
      if(nvv<cap && nvv>=0) {
	*((char *)str+base+nvv) = setv.char_val();
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
      /* wrap the count in the counted value's delimiter, {n} for a list,
         so the readings of size() are told apart on sight */
      comterp()->stack_top().wrapper(AttributeValue::BraceWrapper);
      return;			  
    }
  } else if (listv.is_object(AttributeList::class_symid())) {
    AttributeList* al = (AttributeList*)listv.obj_val();
    if (al) {
      ComValue retval(al->Number());
      push_stack(retval);
      comterp()->stack_top().wrapper(AttributeValue::ParenWrapper);
      return;			  
    }
  } else if (listv.is_string() || listv.is_symbol()) {
    /* a slice's own length, not its shared parent's --
       strlen() would run past the slice's window into the parent */
    int len = listv.sliced() ? listv.slicelen() : (int)strlen(listv.symbol_ptr());
    ComValue retval(len, ComValue::IntType);
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


TupleFunc::TupleFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TupleFunc::execute() {
    ComValue* operand1 = new ComValue(stack_arg(0));
    ComValue* operand2 = new ComValue(stack_arg(1));
    reset_stack();

    /* trailing comma -- wrap operand1 in a single-element list */
    if (operand2->is_blank()) {
        AttributeValueList* avl = new AttributeValueList();
        avl->Append(operand1);
        ComValue retval(avl);
        push_stack(retval);
        delete operand2;
        return;
    }

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

int ColonListFunc::_symid = -1;

ColonListFunc::ColonListFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ColonListFunc::execute() {
  /* symbol=true: eager like any other operator,
     but a bare identifier arrives as its own symbol, not looked up */
  ComValue lo(stack_arg(0, true));
  ComValue hi(stack_arg(1, true));
  reset_stack();
  /* chained ':' flattens, not nests (1:2:3 -> {1,2,3}, not {{1,2},3});
     guarded by nested_insert(), like TupleFunc; unsafe on shared lists */
  if (lo.is_array() && lo.coloned() && !lo.array_val()->nested_insert()) {
    AttributeValueList* avl = lo.array_val();
    avl->Append(new AttributeValue(hi));
    push_stack(lo);
  } else {
    AttributeValueList* avl = new AttributeValueList();
    avl->Append(new AttributeValue(lo));
    avl->Append(new AttributeValue(hi));
    ComValue retval(avl);
    retval.coloned(1);
    push_stack(retval);
    if (lo.is_array())
      lo.array_val()->nested_insert(false);
  }
}

/*****************************************************************************/

NextCommandIsFunc::NextCommandIsFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NextCommandIsFunc::execute() {
  /* a backquoted command name (e.g. `at) arrives as CommandType, not
     SymbolType -- same distinction global()/local()/exists() already
     make for a name that collides with a registered command */
  ComValue symv(stack_arg(0, true));
  reset_stack();
  int target = symv.is_symbol() ? symv.symbol_val()
    : symv.is_command() ? symv.command_symid() : -1;
  boolean found = target>=0 && comterp()->next_command_is(target);
  push_stack(found ? ComValue::trueval() : ComValue::falseval());
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
  static int substr_symid = symbol_add("substr");
  ComValue substrv(stack_key(substr_symid));
  boolean substrflag = substrv.is_true();
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
        int match;
	AttributeValue* testv = avl->GetAttrVal(it);
        if(!substrflag) {
	  comterp()->push_stack(*testv);
	  comterp()->push_stack(valv);
	  EqualFunc eqfunc(comterp());
	  eqfunc.funcid(symbol_add("eq"));
	  eqfunc.exec(2,0);
	  match =  comterp()->pop_stack().is_true();
	} else {
	  /* cstr(), not string_ptr() -- testv or valv can each be a slice,
	     string_ptr() would search the whole parent, not just its window */
	  std::string tscratch, vscratch;
	  ComValue testcv(*testv);
	  match = strstr(testcv.cstr(tscratch), valv.cstr(vscratch)) != NULL;
	}
	if(match) {
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
      /* cstr(), not string_ptr() -- listorstrv can be a slice;
         the index stays within the slice's window, same as split()/at() */
      std::string sscratch;
      const char* string = listorstrv.cstr(sscratch);

      if (valv.is_char()) {
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
          std::string nscratch;
          const char* needle = valv.cstr(nscratch);
          const char* foundstr = strstr(string, needle);
	  const char* newfoundstr = foundstr;
          if((lastflag||allflag) && foundstr!=NULL) {
	    do {
	      foundstr = newfoundstr;
	      newfoundstr = strstr(foundstr+strlen(needle), needle);
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

