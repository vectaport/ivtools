/*
 * Copyright (c) 2001 Scott E. Johnston
 * Copyright (c) 1998 Vectaport Inc.
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

#include <ComTerp/mathfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <math.h>

#define TITLE "MathFunc"

/*****************************************************************************/

ExpFunc::ExpFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ExpFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(exp(operandx.double_val()));
    push_stack(result);
}

LogFunc::LogFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void LogFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(log(operandx.double_val()));
    push_stack(result);
}

Log10Func::Log10Func(ComTerp* comterp) : ComFunc(comterp) {
}

void Log10Func::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(log10(operandx.double_val()));
    push_stack(result);
}

PowFunc::PowFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void PowFunc::execute() {
    ComValue operandx = stack_arg(0);
    ComValue operandy = stack_arg(1);
    reset_stack();
    if (operandx.is_nil() || operandy.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(pow(operandx.double_val(), operandy.double_val()));
    push_stack(result);
}

ACosFunc::ACosFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ACosFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(acos(operandx.double_val()));
    push_stack(result);
}

ASinFunc::ASinFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ASinFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(asin(operandx.double_val()));
    push_stack(result);
}

ATanFunc::ATanFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void ATanFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(atan(operandx.double_val()));
    push_stack(result);
}

ATan2Func::ATan2Func(ComTerp* comterp) : ComFunc(comterp) {
}

void ATan2Func::execute() {
    ComValue operandx = stack_arg(0);
    ComValue operandy = stack_arg(0);
    reset_stack();
    if (operandx.is_nil() || operandy.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(atan2(operandx.double_val(),operandy.double_val()));
    push_stack(result);
}

CosFunc::CosFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void CosFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(cos(operandx.double_val()));
    push_stack(result);
}

SinFunc::SinFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SinFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(sin(operandx.double_val()));
    push_stack(result);
}

TanFunc::TanFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void TanFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(tan(operandx.double_val()));
    push_stack(result);
}

SqrtFunc::SqrtFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SqrtFunc::execute() {
    ComValue operandx = stack_arg(0);
    reset_stack();
    if (operandx.is_nil()) {
      push_stack(ComValue::nullval());
      return;
    }
    ComValue result(sqrt(operandx.double_val()));
    push_stack(result);
}
