/*
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

#include <ComTerp/boolfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>

#define TITLE "BoolFunc"

/*****************************************************************************/

AndFunc::AndFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void AndFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
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
    reset_stack();
    push_stack(result);
}

OrFunc::OrFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void OrFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
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
    reset_stack();
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
        result.boolean_ref() = operand1.symbol_val()<0;
	break;
    }
    reset_stack();
    push_stack(result);
}

EqualFunc::EqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void EqualFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

    if (operand2.type()==ComValue::UnknownType && operand1.type()!=ComValue::UnknownType)
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
	result.boolean_ref() = operand1.string_val() == operand2.string_val();
	break;
      case ComValue::SymbolType:
	result.boolean_ref() = operand1.symbol_val() == operand2.symbol_val();
	break;
      default:
        result.boolean_ref() = operand1.is_type(ComValue::UnknownType) && 
	  operand2.is_type(ComValue::UnknownType);
	break;
      }
    }
    reset_stack();
    push_stack(result);
}

NotEqualFunc::NotEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void NotEqualFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);
    result.type(ComValue::BooleanType);

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
    }
    reset_stack();
    push_stack(result);
}

GreaterThanFunc::GreaterThanFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void GreaterThanFunc::execute() {
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
    }
    reset_stack();
    push_stack(result);
}

GreaterThanOrEqualFunc::GreaterThanOrEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void GreaterThanOrEqualFunc::execute() {
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
    }
    reset_stack();
    push_stack(result);
}

LessThanFunc::LessThanFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void LessThanFunc::execute() {
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
    }
    reset_stack();
    push_stack(result);
}

LessThanOrEqualFunc::LessThanOrEqualFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void LessThanOrEqualFunc::execute() {
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
    }
    reset_stack();
    push_stack(result);
}



