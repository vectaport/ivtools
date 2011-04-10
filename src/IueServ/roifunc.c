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


// from the IUE
#include <IUE-IM/Base/generic-image.h>
#include <IUE-IM/DataOrg/tjr-image-data-2d.h>
#include <IUE-IM/DataOrg/tjr-image-data-2d-rep.h>
#include <ImageClasses/Image.h>
#include <ImageClasses/MemoryImage.h>
#include <RegionsOfInterest/aai-connected-components.h>

#include <IueServ/roifunc.h>
#include <IueServ/iuecomps.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

/*-----------------------------------------------------------------------*/

IueConnCompFunc::IueConnCompFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueConnCompFunc::execute() {
  ComValue img(stack_arg(0));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {
    Image* src = comp->image();
    if (src) {
      IUE_scalar_image_2d* inimage =
	new IUE_scalar_image_2d_of<IUE_UINT8>
	(new IUE_tjr_image_data_2d<IUE_UINT8>(src));
      IUE_scalar_image_2d_of<int>* outimage;
      AAI_connected_components(inimage, outimage);

      int ncols = outimage->x_size();
      int nrows = outimage->y_size();
      ImageTemplate it;
      it.SetSizeX(ncols);
      it.SetSizeY(nrows);
      it.SetFormat('U');
      it.SetBitsPixel(4*8);
      MemoryImage* dst = new MemoryImage(&it);
      for (int row=0; row<nrows; row++) {
	int rowvals[ncols];
	outimage->getrow(rowvals, row, 0, ncols);
	dst->PutSection((void*)rowvals,0,row,ncols,1);
      }
      IueImageComp* imagecomp = new IueImageComp(dst);
      ComValue retval(ComValue::ObjectType, new ComponentView(imagecomp));
      retval.obj_type_ref() = IueImageComp::class_symid();
      push_stack(retval);
      return;

    }
  } 
  push_stack(ComValue::nullval());
}
