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

#include <ComTerp/randfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attrvalue.h>
#include <Unidraw/iterator.h>

#define TITLE "RandFunc"

/*****************************************************************************/

RandFunc::RandFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void RandFunc::execute() {
  ComValue minmaxlist(stack_arg(0));
  reset_stack();
  
  /* set min and max bounds for random number */
  double minval = 0.0;
  double maxval = 1.0;
  if (minmaxlist.is_type(ComValue::ArrayType)) {
    AttributeValueList* avl = minmaxlist.array_val();
    if (avl->Number()==2) {
      Iterator it;
      avl->First(it);
      minval = avl->GetAttrVal(it)->double_val();
      avl->Next(it);
      maxval = avl->GetAttrVal(it)->double_val();
    }
  } 

#ifndef RAND_MAX
#include <sys/limits.h>
#define RAND_MAX INT_MAX
#endif

  double gain = (maxval-minval)/RAND_MAX;
  double bias = minval;

  int rnum = rand();
  double rval = rnum*gain+bias;
  ComValue retval(rval);
  push_stack(retval);

}

/*****************************************************************************/

SRandFunc::SRandFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void SRandFunc::execute() {
  ComValue seedval(stack_arg(0));
  reset_stack();
  srand(seedval.uint_val());
}


