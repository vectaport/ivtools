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

#include <ComTerp/xformfunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterp.h>
#include <ComTerp/mathfunc.h>
#include <ComTerp/numfunc.h>
#include <Attribute/attrlist.h>
#include <Attribute/attribute.h>
#include <Unidraw/iterator.h>
#include <InterViews/transformer.h>
#include <OS/math.h>

#define TITLE "XformFunc"

/*****************************************************************************/

XformFunc::XformFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void XformFunc::execute() {
  ComValue ptslist(stack_arg(0));
  ComValue afflist(stack_arg(1));
  reset_stack();
  
  if (ptslist.is_array() && afflist.is_array()) {
    AttributeValueList* pts_avl = ptslist.array_val();
    AttributeValueList* aff_avl = afflist.array_val();
    if (pts_avl->Number()==2 && aff_avl->Number()==6) {

      /* extract affine transform */
      Iterator it;
      float affine[6];
      aff_avl->First(it);
      for (int i=0; i<6; i++) {
	affine[i] = aff_avl->GetAttrVal(it)->float_val();
	aff_avl->Next(it);
      }
      Transformer t(affine[0], affine[1], affine[2],
		    affine[3], affine[4], affine[5]);
	

      /* check if coordinates are int or float, then apply transform */
      pts_avl->First(it); 
      boolean floatflag = pts_avl->GetAttrVal(it)->is_floatingpoint();
      pts_avl->Next(it);
      floatflag = floatflag || pts_avl->GetAttrVal(it)->is_floatingpoint();
      if (floatflag) {
	pts_avl->First(it);
	float x = pts_avl->GetAttrVal(it)->float_val();
	pts_avl->Next(it);
	float y = pts_avl->GetAttrVal(it)->float_val();
	t.transform(x, y);
	AttributeValueList* avl = new AttributeValueList();
	avl->Append(new ComValue(x));
	avl->Append(new ComValue(y));
	ComValue array(avl);
	push_stack(array);
      } else {
	pts_avl->First(it);
	int x = pts_avl->GetAttrVal(it)->int_val();
	pts_avl->Next(it);
	int y = pts_avl->GetAttrVal(it)->int_val();
	t.Transform(x, y);
	AttributeValueList* avl = new AttributeValueList();
	avl->Append(new ComValue(x));
	avl->Append(new ComValue(y));
	ComValue array(avl);
	push_stack(array);
      }
    } else
      push_stack(ComValue::nullval());
  } else 
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

InvertXformFunc::InvertXformFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void InvertXformFunc::execute() {
  ComValue afflist(stack_arg(0));
  reset_stack();
  
  if (afflist.is_array()) {
    AttributeValueList* aff_avl = afflist.array_val();
    if (aff_avl->Number()==6) {

      /* extract affine transform */
      Iterator it;
      float affine[6];
      aff_avl->First(it);
      for (int i=0; i<6; i++) {
	affine[i] = aff_avl->GetAttrVal(it)->float_val();
	aff_avl->Next(it);
      }
      Transformer t(affine[0], affine[1], affine[2],
		    affine[3], affine[4], affine[5]);
	
      t.Invert();
      t.matrix(affine[0], affine[1], affine[2], 
	       affine[3], affine[4], affine[5]);
      AttributeValueList* avl = new AttributeValueList();
      for (int j=0; j<6; j++)
	avl->Append(new ComValue(affine[j]));
      ComValue array(avl);
      push_stack(array);

    } else
      push_stack(ComValue::nullval());
  } else 
    push_stack(ComValue::nullval());
}

/*****************************************************************************/

XposeFunc::XposeFunc(ComTerp* comterp) : ComFunc(comterp) {
}

void XposeFunc::execute() {
  ComValue listv(stack_arg(0));
  reset_stack();
  
  if (listv.is_array()) {
    int imax = 0;
    int jmax = 0;
    Iterator it;
    listv.array_val()->First(it);
    while (!listv.array_val()->Done(it)) {
      imax++;
      AttributeValue* av = listv.array_val()->GetAttrVal(it);
      if (av->is_array())
	jmax = Math::max(jmax, av->array_val()->Number());
      listv.array_val()->Next(it);
    }

    /* construct tranposed matrix */
    jmax += jmax ? 0 : 1;
    AttributeValueList* new_matrix = new AttributeValueList();
    for(int j=0; j<jmax; j++) 
      new_matrix->Append(new AttributeValue(new AttributeValueList()));
    
    /* save pointer to first new column */
    Iterator jt;
    new_matrix->First(jt);
    AttributeValue* jv = new_matrix->GetAttrVal(jt);

    /* populate new matrix */
    listv.array_val()->First(it);
    while (!listv.array_val()->Done(it)) {
      AttributeValue* av = listv.array_val()->GetAttrVal(it);
      if (av->is_array()) {
	Iterator at;
	av->array_val()->First(at);
	Iterator nt;
	new_matrix->First(nt);
	while (!av->array_val()->Done(at)) {
	  new_matrix->GetAttrVal(nt)->array_val()->Append(new AttributeValue(av->array_val()->GetAttrVal(at)));
	  av->array_val()->Next(at);
	  new_matrix->Next(nt);
	}
      } else
	jv->array_val()->Append(new AttributeValue(av));
      listv.array_val()->Next(it);
    }
    ComValue retval(new_matrix);
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());
}

