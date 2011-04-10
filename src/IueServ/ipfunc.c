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
#include <Basics/bufferxy.h>
#include <ImageClasses/Image.h>
#include <ImageClasses/MemoryImage.h>
#include <ImageProcessing/FloatOperators.h>
#include <ImageProcessing/pixel.h>

#include <IueServ/ipfunc.h>
#include <IueServ/iuecomps.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

FloatOperators ops;

/*-----------------------------------------------------------------------*/

static BufferXY* GetFloatBuffer(MemoryImage const* src) {
  if (src->GetImageClass() != 'M') { cerr << "Not a monochrome image\n"; return 0; }
  BufferXY* fsrc = new BufferXY(src->GetSizeX(), src->GetSizeY(), 8*sizeof(float));
  float* fbuf = (float*)(fsrc->GetBuffer());
  int sz = src->GetBytesPixel();
  if (src->GetFormat() == 'A') { // already float (or double)
    if (sz == sizeof(float)) {
      memcpy(fsrc, src->GetBufferPtr(), src->GetArea()*sz);
      return fsrc;
    } else { cerr << "Very strange: float image with "<<sz<<" bytes/pixel!\n"; return 0; }
  }
  else if (src->GetFormat() == 'L') {
    if (sz == 1) {
      byte* buf = (byte*)(src->GetBufferPtr());
      for (int i=0; i<src->GetArea(); ++i) fbuf[i] = buf[i];
      return fsrc;
    }
    else if (sz == 2) {
      short* buf = (short*)(src->GetBufferPtr());
      for (int i=0; i<src->GetArea(); ++i) fbuf[i] = buf[i];
      return fsrc;
    }
    else if (sz == 4) {
      int* buf = (int*)(src->GetBufferPtr());
      for (int i=0; i<src->GetArea(); ++i) fbuf[i] = buf[i];
      return fsrc;
    } else { cerr << "Strange: image with "<<sz<<" bytes/pixel!\n"; return 0; }
  }
  else return 0;
}

static void PutFloatBuffer(MemoryImage* src, BufferXY* fsrc) {
  float const* fbuf = (float*)(fsrc->GetBuffer());
  int sz = src->GetBytesPixel();
  void* ptr = (void*)(((MemoryImage const*)src)->GetBufferPtr());
  if (src->GetFormat() == 'L') {
    if (sz == 1) {
      byte* buf = (byte*)ptr;
      ops.Normalize(*fsrc, 0, 255);
      for (int i=0; i<src->GetArea(); ++i) buf[i] = (byte)(fbuf[i]+0.5);
    }
    else if (sz == 2) {
      short* buf = (short*)ptr;
      ops.Normalize(*fsrc, -0x8000, 0xf777);
      for (int i=0; i<src->GetArea(); ++i) buf[i] = (short)(fbuf[i]+0.5);
    }
    else if (sz == 4) {
      int* buf = (int*)ptr;
      ops.Normalize(*fsrc, -0x80000000, 0xf7777777);
      for (int i=0; i<src->GetArea(); ++i) buf[i] = (int)(fbuf[i]+0.5);
    }
  }
  else memcpy ((float*)ptr, fbuf, src->GetArea()*sz); // type 'A' (float)
}

/*-----------------------------------------------------------------------*/

IueGaussianFunc::IueGaussianFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueGaussianFunc::execute() {
  ComValue img(stack_arg(0));
  ComValue ptfive(.5);
  ComValue sigma(stack_arg(1, false, ptfive));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {
    Image* image = comp->image();
    if (image) {
      int sx = image->GetSizeX(); int sy = image->GetSizeY();
      MemoryImage* src = new MemoryImage(image); 
      src->SetFormat(image->GetFormat());
      src->PutSection(image->GetSection((void*)0,0,0,sx,sy),0,0,sx,sy);
      BufferXY* fsrc = GetFloatBuffer(src);
      if (!fsrc) {
	delete src;
	push_stack(ComValue::nullval());
	return;
      }
      ops.Gaussian(*fsrc, fsrc, sigma.float_val());
      PutFloatBuffer(src, fsrc);
      IueImageComp* imagecomp = new IueImageComp(src);
      ComValue retval(ComValue::ObjectType, new ComponentView(imagecomp));
      retval.obj_type_ref() = IueImageComp::class_symid();
      push_stack(retval);
      return;
    }
  }
}

/*-----------------------------------------------------------------------*/

IueThresholdFunc::IueThresholdFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueThresholdFunc::execute() {
  ComValue img(stack_arg(0));
  ComValue thresh(stack_arg(1));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {
    Image* src = comp->image();
    if (src) {
      int sx = src->GetSizeX(); int sy = src->GetSizeY();
      MemoryImage* dst = new MemoryImage(src); 
      dst->SetFormat(src->GetFormat());
      dst->PutSection(src->GetSection((void*)0,0,0,sx,sy),0,0,sx,sy);
      BufferXY* fdst = GetFloatBuffer(dst);
      if (!fdst) {
	delete dst;
	push_stack(ComValue::nullval());
	return;
      }
      float* threshs;
      int nthresh;
      if (!thresh.is_array()) {
	nthresh = 1;
	threshs = new float[1];
	threshs[0] = thresh.float_val();
      } else {
	AttributeValueList* avl = thresh.array_val();
	nthresh = avl->Number();
	threshs = new float[nthresh];
	ALIterator it;
	avl->First(it);
	for (int i=0; i<nthresh; i++) {
	  threshs[i] = avl->GetAttrVal(it)->float_val();
	  avl->Next(it);
	}
	// need to do ascending sort of threshs here
      }
      for (unsigned y=0; y<sy; y++) {
	for (unsigned x=0; x<sx; x++) {
	  float ival = floatPixel(*fdst, x, y);
	  float oval = 0.0;
	  int tcnt = 0;
	  while (tcnt<nthresh) {
	    if (threshs[tcnt]<=ival) oval=threshs[tcnt];
	    tcnt++;
	  }
	  floatPixel(*fdst, x, y) = oval;
	}
      }
      PutFloatBuffer(dst, fdst);
      IueImageComp* imagecomp = new IueImageComp(dst);
      ComValue retval(ComValue::ObjectType, new ComponentView(imagecomp));
      retval.obj_type_ref() = IueImageComp::class_symid();
      push_stack(retval);
      return;
    }
  }
}

