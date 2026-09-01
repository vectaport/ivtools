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
  /* no manual Resource::ref(avl) here: the ComValue constructor refs the
     list (AttributeValue(AttributeValueList*) does Resource::ref), and an
     extra unmatched ref pinned every list() result in memory forever --
     ~27 years of one leaked AttributeValueList per list() call. */
  ComValue retval(avl);
  /* list(:colon) -- an empty coloned() list, the same tag ':' itself
     stamps on what it builds (ColonListFunc, coloned(1)).  Useful as a
     genuine "empty" starting point for a colon-chain built up
     elsewhere (e.g. programmatically, one at() :ins at a time) that
     still wants to read as coloned() once populated, not just an
     ordinary list that happens to hold the same elements. */
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
  /* :raw is accepted but currently a no-op -- nothing below dispatches on
     coloned(), so a coloned index reads as an ordinary array index
     either way. */
  (void)rawflag;

  /* str@lo:hi: a coloned 2-element index into a string builds a slice -- a
     StringType ComValue sharing str's own symid, with an offset/length window
     recorded via sliceoff()/slicelen() rather than a copy.  lo and hi may
     arrive as unresolved symbols, so each goes through lookup_symval().  hi is
     exclusive, Go-style: length is hi-lo, and hi==cap is in bounds.  Writing
     through a slice, and slicing a plain list, are not supported and fall
     through to nil. */
  if (listv.is_only_string() && nv.is_type(ComValue::ArrayType) && nv.coloned()) {
    AttributeValueList* range = nv.array_val();
    boolean forwrite = comterp()->stack_top(nkeys()+1).lhs_assign();
    ComValue retval(ComValue::nullval());
    if (!forwrite && range && range->Number()==2) {
      ComValue loval(*range->Get(0));
      ComValue hival(*range->Get(1));
      loval = comterp()->lookup_symval(loval);
      hival = comterp()->lookup_symval(hival);
      if (loval.type()==ComValue::IntType && hival.type()==ComValue::IntType) {
        int lo = loval.int_val();
        int hi = hival.int_val();
        /* slicing a slice (nesting): bound against listv's own window, not
           the full backing allocation, and compose the new offset onto
           listv's own -- otherwise a re-slice both exposes whatever the
           parent holds past listv's own end and reads from the wrong
           place entirely (sl@0:1 read the parent's own index 0 instead of
           listv's). */
        int base = listv.sliced() ? listv.sliceoff() : 0;
        int cap = listv.sliced() ? listv.slicelen() : symbol_len(listv.string_val());
        if (lo>=0 && hi>=lo && hi<=cap) {
          retval = ComValue(listv.string_val(), ComValue::StringType);
          /* the (unsigned int, ValueType) ctor doesn't ref -- unlike the
             (int, ValueType) one, it has no "used as a StringType
             constructor" case (attrvalue.c).  Without this, listv's own
             destructor unrefs the symid this slice still points at when
             ListAtFunc::execute() returns, and a later allocation can
             reuse that freed memory out from under the still-live slice. */
          retval.ref_as_needed();
          retval.sliceoff(base+lo);
          retval.slicelen(hi-lo);
          retval.sliced(1);
        }
      }
    }
    reset_stack();
    push_stack(retval);
    return;
  }

  /* a list has no position in it, and int_val() answers 0 for one -- so a
     list-valued index used to read as index 0 and hand back the first item,
     a wrong answer wearing a right one's clothes.  A stream of indices does
     fan out, through ordinary overdrive; whether the list spelling should
     mean the same gather is undecided.  Until then, say nil. */
  if (nv.is_array()) {
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  /* the @ operator: lst@N=val.  AssignFunc flags this call's token with
     lhs_assign() when it is the before-part of an assignment.  A plain list
     element is a bare value, not a live handle the way an attrlist position's
     Attribute* is, so an ordinary read would leave AssignFunc no way back to
     lst and N -- hand back a [list, idx] pair instead.  AssignFunc then
     completes the write by re-driving this command with a real :set keyword,
     reusing the logic below.  Guarded on a non-negative index: a negative one
     here is always the sub-expression "-N", and that combination trips a
     stack-corruption bug elsewhere in the dispatch path, so it falls through
     to the ordinary read, which answers nil without mutating. */
  if ((listv.is_type(ComValue::ArrayType) || listv.is_only_string()) &&
      (nv.is_nil() || nv.int_val()>=0) &&
      comterp()->stack_top(nkeys()+1).lhs_assign()) {
    /* a string takes the same route: its characters are writable in place, so s@N='c' has somewhere to write, and the pair carries the
       resolved index just as the list case does.  is_only_string(), not
       is_string(), keeps a symbol out -- its text is its identity. */
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
	avl->Insert(nvv, new AttributeValue(insv));
	push_stack(insv);
	return;
      } else if (setflag) {
	/* nvv, not nv.int_val(): a nil index means the last item, and every
	   other branch here already reads it that way -- this one wrote the
	   first item instead. */
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
	  } else if (setflag)
	    *attr->Value() = setv;
	  /* return a detached single-entry attrlist, e.g. (:y 20), not a live
	     handle into al: al@n=val must never write through, and handing back
	     a live Attribute* would make that unenforceable, since AssignFunc
	     writes through any dotted pair on sight.  A plain AttributeList has
	     none of that machinery, so al@n=val reaches AssignFunc's ordinary
	     non-writable-lvalue warning.  attrname()/attrval() accept this
	     shape directly. */
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
    /* a sliced listv indexes relative to its own window into the shared
       parent buffer: base offsets every access, and cap is the slice's own
       length rather than the parent's capacity, so a slice cannot reach past
       its bound into neighboring bytes.  Storage is still shared, so a write
       stays visible through the parent and any other slice.  Bounds are
       checked against symbol_len(), the allocation's byte count, not
       strlen() -- a strlen() bound would make every byte past the first NUL
       unreachable, and a fresh string(cap) buffer is all NUL. */
    boolean isslice = listv.sliced();
    int base = isslice ? listv.sliceoff() : 0;
    int cap = isslice ? listv.slicelen() : symbol_len(listv.string_val());
    /* nil means the last logical character -- the slice's own last index
       when sliced (its length is exact, not NUL-delimited), otherwise the
       parent's strlen()-based last character, unchanged. */
    int nvv = nv.is_nil() ? (isslice ? cap-1 : (int)strlen(str)-1) : nv.int_val();
    if(!setflag) {
      if(nvv>=0 && nvv<cap) {
        ComValue retval(*(str+base+nvv), ComValue::CharType);
        push_stack(retval);
        return;
      }
    } else if (listv.is_only_string()) {
      /* is_string() is StringType||SymbolType, and a symbol's characters are
	 its identity: every value holding that symid names the same text, so
	 a write here would edit the symbol out from under all of them.
	 Reads above stay open to both. */
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
      /* echo the count wrapped in the delimiter of whatever was counted --
	 {n} for a list, (n) for an attrlist, bare for a string -- so the
	 four readings of size() are told apart on sight.  Stamped on the
	 pushed slot because the wrapper never survives a copy. */
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
    /* a slice's own length, not its shared parent's -- strlen()
       would run past the slice's window into whatever the parent holds
       beyond it. */
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
  /* symbol=true: eager like any other operator (no post_eval()), but a
     bare identifier operand is captured as its own unresolved symbol
     instead of being looked up -- see stack_arg()'s own "if (!symbol)"
     branch, comfunc.c.  Anything else (a literal, a parenthesized
     sub-expression, ...) still evaluates normally; there was never a
     symbol table entry standing in its way to begin with. */
  ComValue lo(stack_arg(0, true));
  ComValue hi(stack_arg(1, true));
  reset_stack();
  /* chained ':' flattens rather than nests.  1:2:3 parses left-associatively
     as (1:2):3, so lo is already the coloned 2-element list by the time this
     runs; appending hi to it, rather than wrapping lo again, yields one flat
     {1,2,3} instead of {{1,2},3} -- which is what hr:min:sec needs to reach a
     consumer as three elements.

     Guarded by nested_insert(), the same flag TupleFunc uses and for the same
     reason: flattening in place is safe only within one chain being built, not
     onto a list another variable still points at.  x=1:2; y=(x):3 would
     otherwise turn x into {1,2,3} as well, since both share the underlying
     list.  eval_expr_internals already marks any array read back off the
     symbol table, so ':' gets the signal for free. */
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
	  /* cstr(), not string_ptr() -- testv (a list element) or valv (the
	     search value) can each independently be a slice; a raw
	     string_ptr() would search the whole shared parent instead of
	     just testv's/valv's own window. */
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
	 the returned index stays relative to the slice's own window
	 (position 0 of cstr()'s text), same convention split()/at()
	 already use, not the shared parent's. */
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

