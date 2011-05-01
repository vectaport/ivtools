/*
 * Copyright (c) 2001 Scott Johnston
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

#include <ComTerp/bitfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <string.h>

#define TITLE "BitFunc"

/*****************************************************************************/

BitAndFunc::BitAndFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void BitAndFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() & operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() & operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() & operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() & operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() & operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() & operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() & operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() & operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() & operand2.boolean_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

BitXorFunc::BitXorFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void BitXorFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() ^ operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() ^ operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() ^ operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() ^ operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() ^ operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() ^ operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() ^ operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() ^ operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() ^ operand2.boolean_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

BitOrFunc::BitOrFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void BitOrFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() | operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() | operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() | operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() | operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() | operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() | operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() | operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() | operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() | operand2.boolean_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

BitNotFunc::BitNotFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void BitNotFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue result(operand1);
    switch (operand1.type()) {
    case ComValue::CharType:
	result.char_ref() = ~ operand1.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = ~ operand1.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = ~ operand1.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = ~ operand1.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = ~ operand1.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = ~ operand1.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = ~ operand1.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = ~ operand1.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = ~ operand1.boolean_val();
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

LeftShiftFunc::LeftShiftFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void LeftShiftFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() << operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() << operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() << operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() << operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() << operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() << operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() << operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() << operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() << operand2.boolean_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

RightShiftFunc::RightShiftFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void RightShiftFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
    case ComValue::CharType:
	result.char_ref() = operand1.char_val() >> operand2.char_val();
	break;
    case ComValue::UCharType:
	result.uchar_ref() = operand1.uchar_val() >> operand2.uchar_val();
	break;
    case ComValue::ShortType:
	result.short_ref() = operand1.short_val() >> operand2.short_val();
	break;
    case ComValue::UShortType:
	result.ushort_ref() = operand1.ushort_val() >> operand2.ushort_val();
	break;
    case ComValue::IntType:
	result.int_ref() = operand1.int_val() >> operand2.int_val();
	break;
    case ComValue::UIntType:
	result.uint_ref() = operand1.uint_val() >> operand2.uint_val();
	break;
    case ComValue::LongType:
	result.long_ref() = operand1.long_val() >> operand2.long_val();
	break;
    case ComValue::ULongType:
	result.ulong_ref() = operand1.ulong_val() >> operand2.ulong_val();
	break;
    case ComValue::FloatType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::DoubleType:
	result.type(ComValue::UnknownType);
	break;
    case ComValue::BooleanType:
        result.boolean_ref() = operand1.boolean_val() >> operand2.boolean_val();
	break;
    }
    reset_stack();
    push_stack(result);
}

