/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 2000 IET Inc.
 * Copyright (c) 1994-1997 Vectaport Inc.
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

#include <Unidraw/Components/grview.h>
#include <ComTerp/boolfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <Attribute/attrlist.h>
#include <string.h>

#define TITLE "BoolFunc"

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wswitch"
#endif

static int sym_symid = symbol_add("sym");
static int n_symid = symbol_add("n");

/*****************************************************************************/

AndFunc::AndFunc(ComTerp* comterp, boolean pre) : NumFunc(comterp) {
    _pre = pre;
}

void AndFunc::execute() {
    ComValue operand1(_pre ? stack_arg(0) : stack_arg_post_eval(0));
    if (operand1.is_false()) {
      reset_stack();
      push_stack(operand1);
      return;
    }
    ComValue operand2(_pre ? stack_arg(1) : stack_arg_post_eval(1));
    reset_stack();

    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() && operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() && operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() && operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() && operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() && operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() && operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() && operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() && operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = operand1.float_val() && operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = operand1.double_val() && operand2.double_val();
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() && operand2.boolean_val();
	break;
    }
    push_stack(result);
}

OrFunc::OrFunc(ComTerp* comterp, boolean pre) : NumFunc(comterp) {
    _pre = pre;
}

void OrFunc::execute() {
    ComValue operand1(_pre ? stack_arg(0) : stack_arg_post_eval(0));
    if (operand1.is_true()) {
      reset_stack();
      push_stack(operand1);
      return;
    }
    ComValue operand2(_pre ? stack_arg(1) : stack_arg_post_eval(1));
    reset_stack();

    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() || operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() || operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() || operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() || operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() || operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() || operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() || operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() || operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = operand1.float_val() || operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = operand1.double_val() || operand2.double_val();
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() || operand2.boolean_val();
	break;
    }
    push_stack(result);
}

NegFunc::NegFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void NegFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);
    switch (operand1.type()) {
    case ComValue::CharType:
	result.char_ref() = ! operand1.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = ! operand1.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = ! operand1.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = ! operand1.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = ! operand1.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = ! operand1.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = ! operand1.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = ! operand1.ulong_val();
	break;
    case ComValue::FloatType:
	result.float_ref() = ! operand1.float_val();
	break;
    case ComValue::DoubleType:
	result.double_ref() = ! operand1.double_val();
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = ! operand1.boolean_val();
	break;
    case ComValue::UnknownType:
        result.boolean_ref() = true;
	break;
    case ComValue::ArrayType:
    case ComValue::ObjectType:
        result.boolean_ref() = false;
	break;
    case ComValue::SymbolType:
    case ComValue::StringType:
        result.boolean_ref() = operand1.symbol_val()==0;
	break;
    case ComValue::StreamType:
        result.boolean_ref() = !operand1.stream_mode();
        break;
    }
    reset_stack();
    push_stack(result);
}

EqualFunc::EqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void EqualFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0, symflag);
    ComValue& operand2 = stack_arg(1, symflag);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    if (operand2.type()==ComValue::UnknownType && operand1.type()!=ComValue::UnknownType)
      result.boolean_ref() = 0;
    
    else if (operand2.type()==ComValue::BlankType && operand1.type()!=ComValue::BlankType)
      result.boolean_ref() = 0;

    else if (operand1.is_numeric() && !operand2.is_numeric())
      result.boolean_ref() = 0;
    
    else {
      switch (operand1.type()) {
      case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() == operand2.char_val();
	break;
      case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() == operand2.uchar_val();
	break;
      case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() == operand2.short_val();
	break;
      case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() == operand2.ushort_val();
	break;
      case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() == operand2.int_val();
	break;
      case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() == operand2.uint_val();
	break; 
      case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() == operand2.long_val();
	break;
      case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() == operand2.ulong_val();
	break;
      case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() == operand2.float_val();
	break;
      case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() == operand2.double_val();
	break;
      case ComValue::StringType:
      case ComValue::SymbolType:
	if (nval.is_unknown()) 
	  result.boolean_ref() = operand1.symbol_val() == operand2.symbol_val();
	else {
	  const char* str1 = operand1.symbol_ptr();
	  const char* str2 = operand2.symbol_ptr();
	  result.boolean_ref() = strncmp(str1, str2, nval.int_val())==0;
	}
	break;
      case ComValue::ArrayType: 
	result.boolean_ref() = operand2.type() == ComValue::ArrayType && 
          (operand1.array_val() == operand2.array_val() ||
           operand1.array_val()->Equal(operand2.array_val()));
	break;
      case ComValue::ObjectType:
	if (!operand1.object_compview())
	  result.boolean_ref() = operand2.type() == ComValue::ObjectType && 
	    operand1.obj_val() == operand2.obj_val() &&
	    operand1.class_symid() == operand2.class_symid();
	else
	  result.boolean_ref() = operand2.type() == ComValue::ObjectType && 
	    operand1.class_symid() == operand2.class_symid() &&
	    operand2.object_compview() &&
	    ((ComponentView*)operand1.obj_val())->GetSubject() == 
	    ((ComponentView*)operand2.obj_val())->GetSubject();
	break;
      default:
        result.boolean_ref() = 
	  operand1.is_type(ComValue::UnknownType) && operand2.is_type(ComValue::UnknownType) ||
	  operand1.is_type(ComValue::BlankType) && operand2.is_type(ComValue::BlankType);
	break;
      }
    }
    reset_stack();
    push_stack(result);
}

NotEqualFunc::NotEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void NotEqualFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    if (operand1.type() != operand2.type()) {
      result.boolean_ref() = true;
      reset_stack();
      push_stack(result);
      return;
    }

    switch (operand1.type()) {
    case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() != operand2.char_val();
	break;
    case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() != operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() != operand2.short_val();
	break;
    case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() != operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() != operand2.int_val();
	break;
    case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() != operand2.uint_val();
	break; 
    case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() != operand2.long_val();
	break;
    case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() != operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() != operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() != operand2.double_val();
	break;
    case ComValue::StringType:
    case ComValue::SymbolType:
      if (nval.is_unknown()) 
	result.boolean_ref() = operand1.symbol_val() != operand2.symbol_val();
      else {
	const char* str1 = operand1.symbol_ptr();
	const char* str2 = operand2.symbol_ptr();
	result.boolean_ref() = strncmp(str1, str2, nval.int_val())!=0;
      }
      break;
    case ComValue::ArrayType: 
      result.boolean_ref() = operand2.type() != ComValue::ArrayType || 
	operand1.array_val() != operand2.array_val() &&
        !operand1.array_val()->Equal(operand2.array_val());
      break;
      case ComValue::ObjectType:
	if (!operand1.object_compview())
	  result.boolean_ref() = operand2.type() != ComValue::ObjectType || 
	    operand1.obj_val() != operand2.obj_val() ||
	    operand1.class_symid() != operand2.class_symid();
	else
	  result.boolean_ref() = operand2.type() != ComValue::ObjectType || 
	    operand1.class_symid() != operand2.class_symid() ||
	    !operand2.object_compview() ||
	    ((ComponentView*)operand1.obj_val())->GetSubject() != 
	    ((ComponentView*)operand2.obj_val())->GetSubject();
	break;
    case ComValue::UnknownType:
	result.boolean_ref() = operand2.is_known();
	break;
    }
    reset_stack();
    push_stack(result);
}

GreaterThanFunc::GreaterThanFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void GreaterThanFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (operand1.type()) {
    case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() > operand2.char_val();
	break;
    case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() > operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() > operand2.short_val();
	break;
    case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() > operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() > operand2.int_val();
	break;
    case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() > operand2.uint_val();
	break; 
    case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() > operand2.long_val();
	break;
    case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() > operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() > operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() > operand2.double_val();
	break;
    case ComValue::SymbolType:
	const char* str1 = operand1.symbol_ptr();
	const char* str2 = operand2.symbol_ptr();
	if (nval.is_unknown())
	  result.boolean_ref() = strcmp(str1, str2)>0;
	else
	  result.boolean_ref() = strncmp(str1, str2, nval.int_val())>0;
	break;
    }
    reset_stack();
    push_stack(result);
}

GreaterThanOrEqualFunc::GreaterThanOrEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void GreaterThanOrEqualFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (operand1.type()) {
    case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() >= operand2.char_val();
	break;
    case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() >= operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() >= operand2.short_val();
	break;
    case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() >= operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() >= operand2.int_val();
	break;
    case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() >= operand2.uint_val();
	break; 
    case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() >= operand2.long_val();
	break;
    case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() >= operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() >= operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() >= operand2.double_val();
	break;
    case ComValue::SymbolType:
	const char* str1 = operand1.symbol_ptr();
	const char* str2 = operand2.symbol_ptr();
	if (nval.is_unknown())
	  result.boolean_ref() = strcmp(str1, str2)>=0;
	else
	  result.boolean_ref() = strncmp(str1, str2, nval.int_val())>=0;
	break;
    }
    reset_stack();
    push_stack(result);
}

LessThanFunc::LessThanFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void LessThanFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (operand1.type()) {
    case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() < operand2.char_val();
	break;
    case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() < operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() < operand2.short_val();
	break;
    case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() < operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() < operand2.int_val();
	break;
    case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() < operand2.uint_val();
	break; 
    case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() < operand2.long_val();
	break;
    case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() < operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() < operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() < operand2.double_val();
	break;
    case ComValue::SymbolType:
	const char* str1 = operand1.symbol_ptr();
	const char* str2 = operand2.symbol_ptr();
	if (nval.is_unknown())
	  result.boolean_ref() = strcmp(str1, str2)<0;
	else
	  result.boolean_ref() = strncmp(str1, str2, nval.int_val())<0;
	break;
    }
    reset_stack();
    push_stack(result);
}

LessThanOrEqualFunc::LessThanOrEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void LessThanOrEqualFunc::execute() {
    boolean symflag = stack_key(sym_symid).is_true(); 
    ComValue& nval = stack_key(n_symid); 

    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    switch (operand1.type()) {
    case ComValue::CharType:
	result.boolean_ref() = operand1.char_val() <= operand2.char_val();
	break;
    case ComValue::UCharType:
	result.boolean_ref() = operand1.uchar_val() <= operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.boolean_ref() = operand1.short_val() <= operand2.short_val();
	break;
    case ComValue::UShortType:
	result.boolean_ref() = operand1.ushort_val() <= operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.boolean_ref() = operand1.int_val() <= operand2.int_val();
	break;
    case ComValue::UIntType:
	result.boolean_ref() = operand1.uint_val() <= operand2.uint_val();
	break; 
    case ComValue::LongType:
	result.boolean_ref() = operand1.long_val() <= operand2.long_val();
	break;
    case ComValue::ULongType:
	result.boolean_ref() = operand1.ulong_val() <= operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.boolean_ref() = operand1.float_val() <= operand2.float_val();
	break;
    case ComValue::DoubleType:
	result.boolean_ref() = operand1.double_val() <= operand2.double_val();
	break;
    case ComValue::SymbolType:
	const char* str1 = operand1.symbol_ptr();
	const char* str2 = operand2.symbol_ptr();
	if (nval.is_unknown())
	  result.boolean_ref() = strcmp(str1, str2)<=0;
	else
	  result.boolean_ref() = strncmp(str1, str2, nval.int_val())<=0;
	break;
    }
    reset_stack();
    push_stack(result);
}



