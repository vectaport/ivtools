/*
 * Copyright (c) 1994,1995,1999 Vectaport Inc.
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

#define TITLE "NumFunc"

/*****************************************************************************/

NumFunc::NumFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void NumFunc::promote(ComValue& op1, ComValue& op2) {
    if (op1.type() == op2.type()) return;

    boolean op1bigger = op1.type() > op2.type();
    ComValue* greater = op1bigger ? &op1 : &op2;
    ComValue* lesser =  op1bigger ? &op2 : &op1;

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
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

    switch (result.type()) {
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
        { // braces are work-around for gcc-2.8.1 bug in stack mgmt.
	  int len1 = strlen(operand1.string_ptr()); 
	  int len2 = strlen(operand2.string_ptr()); 
	  char buffer[len1+len2+1];
	  strcpy(buffer, operand1.string_ptr());
	  strcpy(buffer+len1, operand2.string_ptr());
	  result.string_ref()  = symbol_add(buffer);
	}
	break;
    }
    reset_stack();
    push_stack(result);
}

SubFunc::SubFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void SubFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

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
    }
    reset_stack();
    push_stack(result);
}

MinusFunc::MinusFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MinusFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue result(operand1);
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
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

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
    }
    reset_stack();
    push_stack(result);
}

DivFunc::DivFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void DivFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
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
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

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
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

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
    }
    reset_stack();
    push_stack(result);
}

MaxFunc::MaxFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void MaxFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue& operand2 = stack_arg(1);
    promote(operand1, operand2);
    ComValue result(operand1);

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
    }
    reset_stack();
    push_stack(result);
}

AbsFunc::AbsFunc(ComTerp* comterp) : NumFunc(comterp) {
}

void AbsFunc::execute() {
    ComValue& operand1 = stack_arg(0);
    ComValue result(operand1);

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
    ComValue& operand = stack_arg(0);
    ComValue result(operand.char_val(), ComValue::CharType);
    reset_stack();
    push_stack(result);
}

ShortFunc::ShortFunc(ComTerp* comterp) : ComFunc(comterp) {}

void ShortFunc::execute() {
    ComValue& operand = stack_arg(0);
    ComValue result(operand.short_val(), ComValue::ShortType);
    reset_stack();
    push_stack(result);
}

IntFunc::IntFunc(ComTerp* comterp) : ComFunc(comterp) {}

void IntFunc::execute() {
    ComValue& operand = stack_arg(0);
    ComValue result(operand.int_val(), ComValue::IntType);
    reset_stack();
    push_stack(result);
}

LongFunc::LongFunc(ComTerp* comterp) : ComFunc(comterp) {}

void LongFunc::execute() {
    ComValue& operand = stack_arg(0);
    ComValue result(operand.long_val());
    reset_stack();
    push_stack(result);
}

FloatFunc::FloatFunc(ComTerp* comterp) : ComFunc(comterp) {}

void FloatFunc::execute() {
    ComValue& operand = stack_arg(0);
    ComValue result(operand.float_val());
    reset_stack();
    push_stack(result);
}

DoubleFunc::DoubleFunc(ComTerp* comterp) : ComFunc(comterp) {}

void DoubleFunc::execute() {
    ComValue& operand = stack_arg(0);
    ComValue result(operand.double_val());
    reset_stack();
    push_stack(result);
}

