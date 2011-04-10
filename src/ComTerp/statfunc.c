/*
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

#include <ComTerp/statfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/mathfunc.h>
#include <ComTerp/numfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>

#define TITLE "StatFunc"

/*****************************************************************************/

SumFunc::SumFunc(ComTerp* comterp) : ComFunc(comterp) {
  _meanfunc = false;
}

void SumFunc::execute() {
  ComValue vallist(stack_arg(0));
  reset_stack();
  
  if (vallist.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = vallist.array_val();
    AddFunc addfunc(comterp());
    push_stack(ComValue::zeroval());
    Iterator it;
    int count = 0;
    for (avl->First(it); !avl->Done(it); avl->Next(it)) {
      count++;
      push_stack(*avl->GetAttrVal(it));
      addfunc.exec(2,0);
    }
    if (_meanfunc) {
      DivFunc divfunc(comterp());
      ComValue divisor(count, ComValue::IntType);
      push_stack(divisor);
      divfunc.exec(2,0);
    }
  } else {
    push_stack(vallist);
  }
}

/*****************************************************************************/

MeanFunc::MeanFunc(ComTerp* comterp) : SumFunc(comterp) {
  _meanfunc = true;
}

/*****************************************************************************/

VarFunc::VarFunc(ComTerp* comterp) : ComFunc(comterp) {
  _stddevfunc = false;
}

void VarFunc::execute() {
  ComValue vallist(stack_arg(0));
  reset_stack();
  
  if (vallist.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = vallist.array_val();
    AddFunc addfunc(comterp());
    MpyFunc mpyfunc(comterp());
    ComValue sqrsumval(ComValue::zeroval());
    ComValue sumval(ComValue::zeroval());
    Iterator it;
    int count = 0;
    for (avl->First(it); !avl->Done(it); avl->Next(it)) {
      count++;

      /* square value and add to sum of squares */
      push_stack(*avl->GetAttrVal(it));
      push_stack(*avl->GetAttrVal(it));
      mpyfunc.exec(2,0);
      push_stack(sqrsumval);
      addfunc.exec(2,0);
      sqrsumval = comterp()->pop_stack();

      /* add value to running sum */
      push_stack(sumval);
      push_stack(*avl->GetAttrVal(it));
      addfunc.exec(2,0);
      sumval = comterp()->pop_stack();
    }

    /* compute mean squared */
    DivFunc divfunc(comterp());
    push_stack(sumval);
    ComValue countval(count, ComValue::IntType);
    push_stack(countval);
    divfunc.exec(2,0);
    ComValue meanval(comterp()->pop_stack());
    push_stack(meanval);
    push_stack(meanval);
    mpyfunc.exec(2,0);
    ComValue mnsquaredval(comterp()->pop_stack());

    /* subract mean squared from sum of squares to get variance */
    SubFunc subfunc(comterp());
    push_stack(sqrsumval);
    push_stack(mnsquaredval);
    subfunc.exec(2,0);

    /* compute standard deviation if StdDevFunc */
    if (_stddevfunc) {
      SqrtFunc sqrtfunc(comterp());
      sqrtfunc.exec(1,0);
    }

  } else {
    push_stack(ComValue::zeroval());
  }
}

/*****************************************************************************/

StdDevFunc::StdDevFunc(ComTerp* comterp) : VarFunc(comterp) {
  _stddevfunc = true;
}

