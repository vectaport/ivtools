/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1994,1995,1999,2000 Vectaport Inc.
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

#include <ComTerp/numfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/timefunc.h>
#include <Unidraw/iterator.h>
#include <Attribute/attribute.h>
#include <Attribute/attrlist.h>
#include <Attribute/lexscan.h>
#include <Attribute/paramlist.h>
#include <OS/math.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#define TITLE "NumFunc"

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wswitch"
#endif

/*****************************************************************************/

NumFunc::NumFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NumFunc::promote(ComValue& op1, ComValue& op2) {
    if (op1.type() == op2.type()) return;

#if 0
    if (op1.is_unknown() || op2.is_unknown()) {
      op1.type(ComValue::UnknownType);
      op2.type(ComValue::UnknownType);
      return;
    }
#endif

    boolean op1bigger = op1.type()!=ComValue::BooleanType ? op1.type() > op2.type() : false;
    ComValue* greater = op1bigger ? &op1 : &op2;
    ComValue* lesser =  op1bigger ? &op2 : &op1;

    /* skip promotion if first is string or symbol */
    if (op1.type()==ComValue::StringType || op1.type()==ComValue::SymbolType) return;

    /* first do the integral promotions if necessary */
    switch (greater->type()) {
    case ComValue::CharType:
	greater->int_ref() =  greater->char_val();
	greater->type(ComValue::IntType);
	break;
    case ComValue::UCharType:
	greater->int_ref() =  greater->uchar_val();
	greater->type(ComValue::IntType);
	break;
    case ComValue::ShortType:
	greater->int_ref() =  greater->short_val();
	greater->type(ComValue::IntType);
	break;
    case ComValue::UShortType:
	greater->int_ref() =  greater->ushort_val();
	greater->type(ComValue::IntType);
	break;
    default:
	break;
    }
    switch (lesser->type()) {
    case ComValue::BooleanType:
	lesser->int_ref() =  lesser->boolean_val();
	lesser->type(ComValue::IntType);
	break;
    case ComValue::CharType:
	lesser->int_ref() =  lesser->char_val();
	lesser->type(ComValue::IntType);
	break;
    case ComValue::UCharType:
	lesser->int_ref() =  lesser->uchar_val();
	lesser->type(ComValue::IntType);
	break;
    case ComValue::ShortType:
	lesser->int_ref() =  lesser->short_val();
	lesser->type(ComValue::IntType);
	break;
    case ComValue::UShortType:
	lesser->int_ref() =  lesser->ushort_val();
	lesser->type(ComValue::IntType);
	break;
    default:
	break;
    }

    /* now promote as necessary */
    switch (greater->type()) {
    case ComValue::UIntType:
	switch (lesser->type()) {
	case ComValue::IntType:
	    lesser->uint_ref() = lesser->uint_val();
	    break;
	}
	lesser->type(ComValue::UIntType);
	break;
    case ComValue::LongType:
	switch (lesser->type()) {
	case ComValue::IntType:
	    lesser->long_ref() = lesser->int_val();
	    break;
	case ComValue::UIntType:
	    lesser->long_ref() = lesser->uint_val();
	    break;
	}
	lesser->type(ComValue::LongType);
	break;
    case ComValue::ULongType:
	switch (lesser->type()) {
	case ComValue::IntType:
	    lesser->ulong_ref() = lesser->int_val();
	    break;
	case ComValue::UIntType:
	    lesser->ulong_ref() = lesser->uint_val();
	    break;
	case ComValue::LongType:
	    lesser->ulong_ref() = lesser->long_val();
	    break;
	}
	lesser->type(ComValue::ULongType);
	break;
    case ComValue::FloatType:
	switch (lesser->type()) {
	case ComValue::IntType:
	    lesser->float_ref() = lesser->int_val();
	    break;
	case ComValue::UIntType:
	    lesser->float_ref() = lesser->uint_val();
	    break;
	case ComValue::LongType:
	    lesser->float_ref() = lesser->long_val();
	    break;
	case ComValue::ULongType:
	    lesser->float_ref() = lesser->ulong_val();
	    break;
	}
	lesser->type(ComValue::FloatType);
	break;
    case ComValue::DoubleType:
	switch (lesser->type()) {
	case ComValue::IntType:
	    lesser->double_ref() = lesser->int_val();
	    break;
	case ComValue::UIntType:
	    lesser->double_ref() = lesser->uint_val();
	    break;
	case ComValue::LongType:
	    lesser->double_ref() = lesser->long_val();
	    break;
	case ComValue::ULongType:
	    lesser->double_ref() = lesser->ulong_val();
	    break;
	case ComValue::FloatType:
	    lesser->double_ref() = lesser->float_val();
	    break;
	}
	lesser->type(ComValue::DoubleType);
	break;
    default:
	break;
    }

    return;
}

/*****************************************************************************/

AddFunc::AddFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void AddFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    const char* s1 = operand1.type_name();
    const char* s2 = operand2.type_name();
    promote(operand1, operand2);
    ComValue result(operand2.is_list() ? operand2 : operand1);
    reset_stack();

    if (operand1.is_unknown() || operand2.is_unknown()) {
      fprintf(stderr, "Unknown add operand:  %s+%s (line %d)\n", s1, s2, funcstate()->linenum());
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::BooleanType:
	result.int_ref() = operand1.int_val() + operand2.int_val();
	result.type(ComValue::IntType);
	break;
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() + operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() + operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() + operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() + operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() + operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() + operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() + operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() + operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = operand1.float_val() + operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = operand1.double_val() + operand2.double_val();
	break;
    case ComValue::StringType:
    case ComValue::SymbolType:
        { // braces are work-around for gcc-2.8.1 bug in stack mgmt.
          /* Go-style append (#396): b's bytes go straight into operand1's
             own backing symid, in place, when there's room -- no copy at
             all.  operand1 must be_only_string() (StringType, never
             SymbolType): a symbol's characters are its identity, shared by
             every value holding that symid (#393), so writing through one
             would corrupt everyone else's view.  An ordinary interned
             string literal is excluded too, with no extra check needed --
             its symid was sized to hold exactly its own text
             (symbol_len()==strlen()), zero spare capacity, so the in-place
             branch below never finds room and falls through to copy on its
             own.  The in-place result is handed back as a slice
             (sliceoff()/slicelen(), #395) over operand1's own symid, so a
             caller holding operand1 at a nonzero sliceoff() doesn't have
             its result misread from the front of the shared buffer.

             The fallback copy path intentionally still goes through
             symbol_add() (dedup/intern by content), exactly like the
             pre-#396 concatenation it replaces -- NOT symbol_new()'s
             fresh, non-deduping, unfindable-by-text buffer.  Tried
             symbol_new() with amortized (2x) headroom first, matching
             #396's memory design note; it broke symadd()/global()'s
             existing assumption that a StringType's own symid already
             names a searchable symbol (symadd(global(p1)+global(p2)) --
             global.comt test 11), since a symbol_new() id is deliberately
             absent from symbol_find()'s reverse index (see symbol_new()'s
             own doc comment, symbols.c).  So a plain concat -- not
             starting from an already-over-allocated string()/strcap()
             buffer -- gets no free growth headroom and must copy again on
             its own next append, same as it always did; only appending
             onto a deliberately over-provisioned string() buffer gets the
             true zero-copy path above. */
          std::string scratch1, scratch2;
          const char* s1 = operand1.cstr(scratch1);
          int len1 = operand1.sliced() ? operand1.slicelen() : (int)strlen(s1);
          int base1 = operand1.sliced() ? operand1.sliceoff() : 0;
          int end1 = base1 + len1;
          boolean growable = operand1.is_only_string();
          int cap1 = growable ? symbol_len(operand1.symbol_val()) : 0;

          if (operand2.is_string()) {
            const char* s2 = operand2.cstr(scratch2);
            int len2 = operand2.sliced() ? operand2.slicelen() : (int)strlen(s2);
            if (growable && end1+len2 < cap1) {
              char* buf = (char*)symbol_pntr(operand1.symbol_val());
              /* memmove, not memcpy (Greptile, #440): operand2 can share
                 operand1's own backing symid -- e.g. appending an unsliced
                 buf onto a nonzero-offset slice of that same buf -- in
                 which case s2 (read via cstr() above) points into this
                 very buffer and the [s2,s2+len2) source range can overlap
                 [buf+end1,buf+end1+len2), which memcpy doesn't allow. */
              memmove(buf+end1, s2, len2);
              buf[end1+len2] = '\0';
              result.string_ref() = operand1.symbol_val();
              result.ref_as_needed();
              result.sliceoff(base1);
              result.slicelen(len1+len2);
              result.sliced(1);
            } else {
              int newlen = len1+len2;
              std::vector<char> vbuf(newlen+1);
              memcpy(&vbuf[0], s1, len1);
              memcpy(&vbuf[0]+len1, s2, len2);
              vbuf[newlen] = '\0';
              result.string_ref() = symbol_add(&vbuf[0]);
              result.ref_as_needed();
              result.sliced(0);
            }
          } else {
            if (growable && end1+1 < cap1) {
              char* buf = (char*)symbol_pntr(operand1.symbol_val());
              buf[end1] = operand2.char_val();
              buf[end1+1] = '\0';
              result.string_ref() = operand1.symbol_val();
              result.ref_as_needed();
              result.sliceoff(base1);
              result.slicelen(len1+1);
              result.sliced(1);
            } else {
              int newlen = len1+1;
              std::vector<char> vbuf(newlen+1);
              memcpy(&vbuf[0], s1, len1);
              vbuf[len1] = operand2.char_val();
              vbuf[newlen] = '\0';
              result.string_ref() = symbol_add(&vbuf[0]);
              result.ref_as_needed();
              result.sliced(0);
            }
          }
	}
	break;
    case ComValue::ArrayType: 
        {
	  if (operand1.is_array() && operand2.is_array()) {
	    Resource::unref(result.array_val());
	    result.array_ref() = 
	      AddFunc::matrix_add(operand1.array_val(), operand2.array_val());
	    Resource::ref(result.array_val());
	  }
	  else 
	    result.type(ComValue::UnknownType); // nil
        }
        break;

    case ComValue::ObjectType:
      {
	if (operand1.is_dateobj() && operand2.is_int()) {
	  DateObj *dateobj = new DateObj((DateObj*)operand1.geta(DateObj::class_symid()));
          int addend = operand2.int_val();
	  *dateobj->date() = ((const Date)*dateobj->date()) + addend;
	  result = ComValue(DateObj::class_symid(), (void*)dateobj);

	} else if (operand1.is_attributelist() && operand2.is_attributelist()) {
	  // merge two attrlists: + is to attrlist as + is to string
	  // deep copy al1, then deep copy each attr from al2 into merged
	  AttributeList* al1 = (AttributeList*)operand1.obj_val();
	  AttributeList* al2 = (AttributeList*)operand2.obj_val();
	  AttributeList* merged = new AttributeList(al1);
	  Iterator it;
	  for (al2->First(it); !al2->Done(it); al2->Next(it))
	      merged->add_attribute(new Attribute(*al2->GetAttr(it)));
	  result = ComValue(AttributeList::class_symid(), (void*)merged);
	} else {
	  fprintf(stderr, "Unhandled add operand1 of class %s (line %d)\n", operand1.class_name(), funcstate()->linenum());
	}
      }
      break;

    default: {
        fprintf(stderr, "Unhandled add operand1 type %s (line %d)\n", operand1.type_name(), funcstate()->linenum());
        }
        break;
    }
    push_stack(result);
}


AttributeValueList* AddFunc::matrix_add(AttributeValueList* list1,
					AttributeValueList* list2) {
  AttributeValueList* sum = new AttributeValueList();
  Iterator it1, it2;
  list1->First(it1);
  list2->First(it2);
  while (!list1->Done(it1) && !list2->Done(it2)) {
    push_stack(*list1->GetAttrVal(it1));
    push_stack(*list2->GetAttrVal(it2));
    exec(2, 0);
    ComValue topval(comterp()->pop_stack());
    sum->Append(new AttributeValue(topval));
    list1->Next(it1);
    list2->Next(it2);
  }
  return sum;
}

SubFunc::SubFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void SubFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() - operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() - operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() - operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() - operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() - operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() - operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() - operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() - operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = operand1.float_val() - operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = operand1.double_val() - operand2.double_val();
	break;
    case ComValue::ObjectType:
      {
	if (operand1.is_dateobj() && operand2.is_int()) {
	  DateObj *dateobj = new DateObj((DateObj*)operand1.geta(DateObj::class_symid()));
	  int subtrahend = operand2.int_val();
	  *dateobj->date() = ((const Date)*dateobj->date()) - subtrahend;
	  result = ComValue(DateObj::class_symid(), (void*)dateobj);
	  
	} else if (operand1.is_attributelist() && operand2.is_attributelist()) {
	  // subtract attrlist: remove from al1 any keys present in al2
	  AttributeList* al1 = (AttributeList*)operand1.obj_val();
	  AttributeList* al2 = (AttributeList*)operand2.obj_val();
	  AttributeList* result_al = new AttributeList(al1);
	  Iterator it;
	  for (al2->First(it); !al2->Done(it); al2->Next(it)) {
	    Attribute* attr = al2->GetAttr(it);
	    Attribute* found = result_al->GetAttr(attr->Name());
	    if (found) result_al->Remove(found);
	  }
	  result = ComValue(AttributeList::class_symid(), (void*)result_al);
	  
	} else {
	  fprintf(stderr, "Unhandled subtraction operand1 of class %s (line %d)\n", operand1.class_name(), funcstate()->linenum());
	}
      }
      
      break;
    default: {
      fprintf(stderr, "Unhandled subtraction operand1 type %s (line %d)\n", operand1.type_name(), funcstate()->linenum());
    }
      break;
    }
    reset_stack();
    push_stack(result);
}

MinusFunc::MinusFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MinusFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue result(operand1);

    if (operand1.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = - operand1.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = - operand1.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = - operand1.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = - operand1.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = - operand1.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = - operand1.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = - operand1.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = - operand1.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = - operand1.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = - operand1.double_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

MpyFunc::MpyFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MpyFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand2.is_list() ? operand2 : operand1);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() * operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() * operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() * operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() * operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() * operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() * operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() * operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() * operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = operand1.float_val() * operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = operand1.double_val() * operand2.double_val();
	break;
    case ComValue::ArrayType: 
        {
	  if (operand1.is_array() && operand2.is_array()) {
	    Resource::unref(result.array_val());
	    AttributeValueList* avl = 
	      MpyFunc::matrix_mpy(operand1.array_val(), operand2.array_val());
	    if (avl) {
	      result.array_ref() = avl;
	      Resource::ref(result.array_val());
	    } else
	      result.type(ComValue::UnknownType); // nil
	  }
	  else 
	    result.type(ComValue::UnknownType); // nil
        }
        break;
    }

    reset_stack();
    push_stack(result);
}

AttributeValueList* MpyFunc::matrix_mpy(AttributeValueList* list1,
					AttributeValueList* list2) {

  static AddFunc addfunc(comterp());
  addfunc.funcid(symbol_add("add"));
  Iterator it1, it2;
  list1->First(it1);
  list2->First(it2);
  
  // extract dimensions
  int i1max, j1max;
  int i2max, j2max;
  i1max = list1->Number();
  i2max = list2->Number();
  j1max = list1->GetAttrVal(it1)->is_array() &&
    list1->GetAttrVal(it1)->array_val() ? 
    list1->GetAttrVal(it1)->array_val()->Number() : 0;
  j2max = list2->GetAttrVal(it2)->is_array() &&
    list2->GetAttrVal(it2)->array_val() ? 
    list2->GetAttrVal(it2)->array_val()->Number() : 0;

  /* ensure inner dimension is the same */
  /* allow for vector argument on rhs */
  if (j1max != i2max && (j1max || i2max!=1)) return nil;

  /* ensure each row is of equal length */
  list1->First(it1);
  list1->Next(it1);
  while (!list1->Done(it1)) {
    int jlen = list1->GetAttrVal(it1)->is_array() &&
      list1->GetAttrVal(it1)->array_val() ? 
      list1->GetAttrVal(it1)->array_val()->Number() : 0;
    if (jlen != j1max) return nil;
    list1->Next(it1);
  }
  list2->First(it2);
  list2->Next(it2);
  while (!list2->Done(it2)) {
    int jlen = list2->GetAttrVal(it2)->is_array() &&
      list2->GetAttrVal(it2)->array_val() ? 
      list2->GetAttrVal(it2)->array_val()->Number() : 0;
    if (jlen != j2max) return nil;
    list2->Next(it2);
  }
  

  AttributeValueList* product = new AttributeValueList();

  int i3max = i1max;
  int j3max = j2max;

  /* loop over output rows */
  Iterator iti1, itj1;
  list1->First(iti1);

  for (int i3=0; i3<i3max; i3++) {
    AttributeValueList* prodrow = new AttributeValueList();
    product->Append(new AttributeValue(prodrow));
    AttributeValue* row1v = list1->GetAttrVal(iti1);
    AttributeValueList* row1 = row1v && row1v->is_array() ? 
      row1v->array_val() : nil;

    if (j3max) {
      /* loop over output columns */
      for (int j3=0; j3<j3max; j3++) {
	
	if (row1) row1->First(itj1);
	
	/* generate inner product */
	for (int n=0; n<Math::max(j1max,1); n++) {
	  
	  /* locate the value from the second matrix */
	  Iterator iti2, itj2;
	  list2->First(iti2);
	  for (int i=0; i<n; i++) list2->Next(iti2);
	  AttributeValue* row2v = list2->GetAttrVal(iti2);
	  AttributeValueList* row2 = row2v && row2v->is_array() ? 
	    row2v->array_val() : nil;
	  if (row2) {
	    row2->First(itj2);
	    for (int j=0; j<j3; j++) row2->Next(itj2);
	  }
	  if ((row1 || !j1max) && row2) {
	    if (row1) 
	      comterp()->push_stack(*row1->GetAttrVal(itj1));
	    else
	      comterp()->push_stack(*row1v);
	    comterp()->push_stack(*row2->GetAttrVal(itj2));
	    exec(2,0);
	    if (n) addfunc.exec(2,0);
	  }
	  
	  if (row1) row1->Next(itj1);
	}
	
	ComValue topval(comterp()->pop_stack());
	prodrow->Append(new AttributeValue(topval));
      }
      /* done looping over output columsn */
      
    } else {
      /* handle single output column */

      if (row1) row1->First(itj1);
	
      /* generate inner product */
      for (int n=0; n<Math::max(j1max,1); n++) {
	  
	/* locate the value from the lhs vector */
	Iterator iti2;
	list2->First(iti2);
	for (int i=0; i<n; i++) list2->Next(iti2);
	AttributeValue* val2v = list2->GetAttrVal(iti2);
	if ((row1 || !j1max) && val2v) {
	  if (row1) 
	    comterp()->push_stack(*row1->GetAttrVal(itj1));
	  else
	    comterp()->push_stack(*row1v);
	  comterp()->push_stack(*val2v);
	  exec(2,0);
	  if (n) addfunc.exec(2,0);
	}
	
	if (row1) row1->Next(itj1);
      }
      
      ComValue topval(comterp()->pop_stack());
      prodrow->Append(new AttributeValue(topval));
    }

    list1->Next(iti1);
  }
  /* done looping over output rows */


  return product;
}

DivFunc::DivFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void DivFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	if (operand2.char_val()!=0)
	    result.char_ref() = operand1.char_val() / operand2.char_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::UCharType:
	if (operand2.uchar_val()!=0)
	    result.uchar_ref() = operand1.uchar_val() / operand2.uchar_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::ShortType:
	if (operand2.short_val()!=0)
	    result.short_ref() = operand1.short_val() / operand2.short_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::UShortType:
	if (operand2.ushort_val()!=0)
	    result.ushort_ref() = operand1.ushort_val() / operand2.ushort_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::IntType:
	if (operand2.int_val()!=0)
	    result.int_ref() = operand1.int_val() / operand2.int_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::UIntType:
	if (operand2.uint_val()!=0)
	    result.uint_ref() = operand1.uint_val() / operand2.uint_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::LongType:
	if (operand2.long_val()!=0)
	    result.long_ref() = operand1.long_val() / operand2.long_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::ULongType:
	if (operand2.ulong_val()!=0)
	    result.ulong_ref() = operand1.ulong_val() / operand2.ulong_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::FloatType:
	if (operand2.float_val()!=0)
	    result.float_ref() = operand1.float_val() / operand2.float_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    case ComValue::DoubleType:
	if (operand2.double_val()!=0)
	    result.double_ref() = operand1.double_val() / operand2.double_val();
	else
	    COMERR_SET(ERR_DIV_BY_ZERO);
	break;
    }
    reset_stack();
    push_stack(result);
}

ModFunc::ModFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void ModFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	if (operand2.char_val()!=0)
	    result.char_ref() = operand1.char_val() % operand2.char_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::UCharType:
	if (operand2.uchar_val()!=0)
	    result.uchar_ref() = operand1.uchar_val() % operand2.uchar_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::ShortType:
	if (operand2.short_val()!=0)
	    result.short_ref() = operand1.short_val() % operand2.short_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::UShortType:
	if (operand2.ushort_val()!=0)
	    result.ushort_ref() = operand1.ushort_val() % operand2.ushort_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::IntType:
	if (operand2.int_val()!=0)
	    result.int_ref() = operand1.int_val() % operand2.int_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::UIntType:
	if (operand2.uint_val()!=0)
	    result.uint_ref() = operand1.uint_val() % operand2.uint_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::LongType:
	if (operand2.long_val()!=0)
	    result.long_ref() = operand1.long_val() % operand2.long_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::ULongType:
	if (operand2.ulong_val()!=0)
	    result.ulong_ref() = operand1.ulong_val() % operand2.ulong_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::FloatType:
	if (operand2.float_val()!=0)
	    result.float_ref() = (long) operand1.float_val() % (long) operand2.float_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    case ComValue::DoubleType:
	if (operand2.double_val()!=0)
	    result.double_ref() = (long) operand1.double_val() % (long) operand2.double_val();
	else
	    COMERR_SET(ERR_MOD_BY_ZERO);
	break;
    }
    reset_stack();
    push_stack(result);
}

MinFunc::MinFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MinFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() =  operand1.char_val() < operand2.char_val() 
	  ? operand1.char_val() : operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() =  operand1.uchar_val() < operand2.uchar_val() 
	  ? operand1.uchar_val() : operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() =  operand1.short_val() < operand2.short_val() 
	  ? operand1.short_val() : operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() =  operand1.ushort_val() < operand2.ushort_val() 
	  ? operand1.ushort_val() : operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() =  operand1.int_val() < operand2.int_val() 
	  ? operand1.int_val() : operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() =  operand1.uint_val() < operand2.uint_val() 
	  ? operand1.uint_val() : operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() =  operand1.long_val() < operand2.long_val() 
	  ? operand1.long_val() : operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() =  operand1.ulong_val() < operand2.ulong_val() 
	  ? operand1.ulong_val() : operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() =  operand1.float_val() < operand2.float_val() 
	  ? operand1.float_val() : operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() =  operand1.double_val() < operand2.double_val() 
	  ? operand1.double_val() : operand2.double_val();
	break;
    case ComValue::UnknownType:
	result.assignval(operand2);
	break;
    }
    reset_stack();
    push_stack(result);
}

MaxFunc::MaxFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MaxFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    if (operand1.is_unknown() || operand2.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() =  operand1.char_val() > operand2.char_val() 
	  ? operand1.char_val() : operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() =  operand1.uchar_val() > operand2.uchar_val() 
	  ? operand1.uchar_val() : operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() =  operand1.short_val() > operand2.short_val() 
	  ? operand1.short_val() : operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() =  operand1.ushort_val() > operand2.ushort_val() 
	  ? operand1.ushort_val() : operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() =  operand1.int_val() > operand2.int_val() 
	  ? operand1.int_val() : operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() =  operand1.uint_val() > operand2.uint_val() 
	  ? operand1.uint_val() : operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() =  operand1.long_val() > operand2.long_val() 
	  ? operand1.long_val() : operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() =  operand1.ulong_val() > operand2.ulong_val() 
	  ? operand1.ulong_val() : operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() =  operand1.float_val() > operand2.float_val() 
	  ? operand1.float_val() : operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() =  operand1.double_val() > operand2.double_val() 
	  ? operand1.double_val() : operand2.double_val();
	break;
    case ComValue::UnknownType:
	result.assignval(operand2);
	break;
     
    }
    reset_stack();
    push_stack(result);
}

AbsFunc::AbsFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void AbsFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue result(operand1);

    if (operand1.is_unknown()) {
      reset_stack();
      push_stack(ComValue::nullval());
      return;
    }
    
    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() =  operand1.char_val() < 0 
	  ? -operand1.char_val() : operand1.char_val();
	break;
    case ComValue::ShortType:
	result.short_ref() =  operand1.short_val() < 0
	  ? -operand1.short_val() : operand1.short_val();
	break;
    case ComValue::IntType:
	result.int_ref() =  operand1.int_val() < 0
	  ? -operand1.int_val() : operand1.int_val();
	break;
    case ComValue::LongType:
	result.long_ref() =  operand1.long_val() < 0
	  ? -operand1.long_val() : operand1.long_val();
	break;
    case ComValue::FloatType:
	result.float_ref() =  operand1.float_val() < 0.0
	  ? -operand1.float_val() : operand1.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() =  operand1.double_val() < 0.0
	  ? -operand1.double_val() : operand1.double_val();
	break;
    }
    reset_stack();
    push_stack(result);
}


CharFunc::CharFunc(ComTerp* comterp) : ComFunc(comterp) {}

void CharFunc::execute() {
    ComValue operand(stack_arg(0, false, ComValue::acharval()));
    static int u_symid = symbol_add("u");
    int uval_flag = stack_key(u_symid).is_true(); 
    reset_stack();
    if (operand.is_string()) {
      const char* numstr = operand.symbol_ptr();
      int negflag = *numstr=='-';
      AttributeValue* av = ParamList::lexscan()->get_attrval((char*)numstr+negflag, strlen(numstr+negflag));
      operand = ComValue(av->char_val());
      if (negflag) operand.char_ref() = - operand.char_val();
      delete av;
    }
    ComValue result(operand.char_val(), 
		    operand.is_nil() ? ComValue::UnknownType : 
                    (uval_flag ? ComValue::UCharType : ComValue::CharType));
    push_stack(result);
}

ShortFunc::ShortFunc(ComTerp* comterp) : ComFunc(comterp) {}

void ShortFunc::execute() {
    ComValue operand(stack_arg(0));
    static int u_symid = symbol_add("u");
    int uval_flag = stack_key(u_symid).is_true(); 
    reset_stack();
    if (operand.is_string()) {
      const char* numstr = operand.symbol_ptr();
      int negflag = *numstr=='-';
      AttributeValue* av = ParamList::lexscan()->get_attrval((char*)numstr+negflag, strlen(numstr+negflag));
      operand = ComValue(av->short_val());
      if (negflag) operand.short_ref() = - operand.short_val();
      delete av;
    }
    ComValue result(operand.short_val(), 
		    operand.is_nil() ? ComValue::UnknownType : 
                    (uval_flag ? ComValue::UShortType : ComValue::ShortType));
    push_stack(result);
}

IntFunc::IntFunc(ComTerp* comterp) : ComFunc(comterp) {}

/* Start a string-to-number conversion where atoi/strtol does: skip leading
   whitespace and take an explicit sign, neither of which the lexical scanner
   counts as part of a number.  Returns nil unless the scan really produced
   one -- a non-numeric string lexes as a symbol, and its int_val() is the
   interned symbol id, a wrong answer that reads like a valid parse. */
/* Kinds with no numeric reading at all.  int_val() answered them anyway: an
   object gave something derived from its address, so int(date()) returned a
   different number every call, and a list or a keyword gave 0 -- wrong answers
   wearing the shape of right ones, the same fault as a symbol's interned id.

   Report nil, and let it propagate: nil+1 is nil, where a 0 or -1 marker would
   flow on through arithmetic as ordinary data.  An identity, if one is ever
   wanted, belongs in a command that says so -- symid() already does that for
   symbols.

   Streams are deliberately absent: a stream argument overdrives these
   commands, so it is never converted whole -- int(("1" "2")) is {1,2}, while
   the comma form int(("1","2")) is a list and lands here.
   Keywords likewise: :key in this position is read as a keyword to the
   command, not as a value to convert, so it never arrives here. */
static boolean has_no_numeric_reading(ComValue& operand) {
    return operand.is_object() || operand.is_array();
}

static ComValue scan_number_string(const char* numstr) {
    const char* str = numstr;
    while (isspace((unsigned char)*str)) str++;
    boolean negflag = *str=='-';
    if (*str=='-' || *str=='+') str++;
    AttributeValue* av =
      ParamList::lexscan()->get_attrval((char*)str, strlen(str));
    ComValue result;
    if (av->is_num()) {
      if (av->is_floatingpoint())
	result = ComValue(negflag ? -av->double_val() : av->double_val());
      else
	result = ComValue(negflag ? -av->long_val() : av->long_val());
    }
    delete av;
    return result;
}

void IntFunc::execute() {
    ComValue operand(stack_arg(0, false, ComValue::zeroval()));
    static int u_symid = symbol_add("u");
    int uval_flag = stack_key(u_symid).is_true(); 
    reset_stack();
    if (operand.is_string())
      operand = scan_number_string(operand.symbol_ptr());
    if (has_no_numeric_reading(operand)) operand = ComValue();
    ComValue result(operand.int_val(),  
		    operand.is_nil() ? ComValue::UnknownType :
                    (uval_flag ? ComValue::UIntType : ComValue::IntType));
    push_stack(result);
}

LongFunc::LongFunc(ComTerp* comterp) : ComFunc(comterp) {}

void LongFunc::execute() {
    ComValue operand(stack_arg(0));
    static int u_symid = symbol_add("u");
    int uval_flag = stack_key(u_symid).is_true(); 
    reset_stack();
    if (operand.is_string())
      operand = scan_number_string(operand.symbol_ptr());
    if (has_no_numeric_reading(operand)) operand = ComValue();
    if (operand.is_nil()) { push_stack(ComValue::nullval()); return; }
    ComValue result(operand.long_val());
    if(uval_flag) result.type(ComValue::ULongType);

    push_stack(result);
}

FloatFunc::FloatFunc(ComTerp* comterp) : ComFunc(comterp) {}

void FloatFunc::execute() {
    static ComValue float_zero = ComValue((float)0.0);
    ComValue operand(stack_arg(0, false, float_zero));
    reset_stack();
    if (operand.is_string())
      operand = scan_number_string(operand.symbol_ptr());
    if (has_no_numeric_reading(operand)) operand = ComValue();
    if (operand.is_nil()) { push_stack(ComValue::nullval()); return; }
    ComValue result(operand.float_val());
    push_stack(result);
}

DoubleFunc::DoubleFunc(ComTerp* comterp) : ComFunc(comterp) {}

void DoubleFunc::execute() {
    static ComValue double_zero = ComValue((double)0.0);
    ComValue operand(stack_arg(0, false, double_zero));
    reset_stack();
    if (operand.is_string())
      operand = scan_number_string(operand.symbol_ptr());
    if (has_no_numeric_reading(operand)) operand = ComValue();
    if (operand.is_nil()) { push_stack(ComValue::nullval()); return; }
    ComValue result(operand.double_val());
    push_stack(result);
}

FloorFunc::FloorFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void FloorFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue result(operand1);
    switch (result.type()) {
    case ComValue::CharType:
    case ComValue::UCharType:
    case ComValue::ShortType:
    case ComValue::UShortType:
    case ComValue::IntType:
    case ComValue::UIntType:
    case ComValue::LongType:
    case ComValue::ULongType:
      break;
    case ComValue::FloatType:
      {
	ComValue val((long)floor((double) operand1.float_val()));
	result.assignval(val);
      }
      break;
    case ComValue::DoubleType:
      {
        ComValue val((long)floor(operand1.double_val()));
	result.assignval(val);
      }
      break;
    }
    reset_stack();
    push_stack(result);
}

CeilFunc::CeilFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void CeilFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue result(operand1);
    switch (result.type()) {
    case ComValue::CharType:
    case ComValue::UCharType:
    case ComValue::ShortType:
    case ComValue::UShortType:
    case ComValue::IntType:
    case ComValue::UIntType:
    case ComValue::LongType:
    case ComValue::ULongType:
      break;
    case ComValue::FloatType:
      {
	ComValue val((long)ceil((double) operand1.float_val()));
	result.assignval(val);
      }
      break;
    case ComValue::DoubleType:
      {
        ComValue val((long)ceil(operand1.double_val()));
	result.assignval(val);
      }
      break;
    }
    reset_stack();
    push_stack(result);
}

RoundFunc::RoundFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void RoundFunc::execute() {
    ComValue operand1 = stack_arg(0);
    ComValue result(operand1);
    switch (result.type()) {
    case ComValue::CharType:
    case ComValue::UCharType:
    case ComValue::ShortType:
    case ComValue::UShortType:
    case ComValue::IntType:
    case ComValue::UIntType:
    case ComValue::LongType:
    case ComValue::ULongType:
      break;
    case ComValue::FloatType:
      {
	ComValue val((long)floor((double) operand1.float_val()+0.5));
	result.assignval(val);
      }
      break;
    case ComValue::DoubleType:
      {
        ComValue val((long)floor(operand1.double_val()+0.5));
	result.assignval(val);
      }
      break;
    }
    reset_stack();
    push_stack(result);
}


