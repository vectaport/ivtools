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
#include <ImageClasses/Image.h>
#include <ImageClasses/MemoryImage.h>

#include <IueServ/iuecomps.h>
#include <IueServ/iuefunc.h>
#include <ComTerp/comvalue.h>
#include <ComTerp/comterpserv.h>
#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

IueFunc::IueFunc(ComTerp* comterp) : ComFunc(comterp) {
}

IueImageComp* IueFunc::image_comp(ComValue& comval) {
  ComponentView* compview = 
    (ComponentView*)comval.geta(IueImageComp::class_symid());
  if (compview)
    return (IueImageComp*)compview->GetSubject();
  else
    return nil;
}

/*-----------------------------------------------------------------------*/

IueImageFunc::IueImageFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueImageFunc::execute() {
  ComValue path_or_obj(stack_arg(0));
  static int mem_symid = symbol_add("mem");
  ComValue memflag(stack_key(mem_symid));
  static int adrg_symid = symbol_add("adrg");
  ComValue adrgflag(stack_key(adrg_symid));
  reset_stack();

  if (path_or_obj.is_type(ComValue::StringType)) {
    IueImageComp* imagecomp = new IueImageComp(path_or_obj.string_ptr(),
					       adrgflag.boolean_val());
    if (!imagecomp->image()) {
      delete imagecomp;
      push_stack(ComValue::nullval());
      return;  
    }
    ComValue retval(imagecomp->classid(), new ComponentView(imagecomp));
    // retval.obj_type_ref() = IueImageComp::class_symid();
    retval.object_compview(true);
    push_stack(retval);

  } else {
    IueImageComp* comp = image_comp(path_or_obj);
    Image* image = comp ? comp->image() : nil;
    if (image && memflag.is_true()) {
      MemoryImage* memimage = new MemoryImage(image);
      memimage->SetFormat(image->GetFormat());
      int sx = image->GetSizeX(); int sy = image->GetSizeY();
      memimage->PutSection(image->GetSection((void*)0,0,0,sx,sy),0,0,sx,sy);
      IueImageComp* imagecomp = new IueImageComp(memimage);
      ComValue retval(imagecomp->classid(), new ComponentView(imagecomp));
      // retval.obj_type_ref() = IueImageComp::class_symid();
      retval.object_compview(true);
      push_stack(retval);

    } else
      push_stack(ComValue::nullval());

  }
  return;
}

/*-----------------------------------------------------------------------*/

IueGetPixelFunc::IueGetPixelFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueGetPixelFunc::execute() {
  ComValue img(stack_arg(0));
  ComValue xyloc(stack_arg(1));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp && nargs()>1 && xyloc.is_array() && comp) {  
    Image* image = comp->image();
    if (image) {
      AttributeValueList* avl = xyloc.array_val();
      ALIterator it;
      avl->First(it);
      unsigned x = avl->GetAttrVal(it)->uint_val();
      avl->Next(it);
      unsigned y = avl->GetAttrVal(it)->uint_val();
      if (x < image->GetSizeX() && y < image->GetSizeY()) {
	
	ComValue::ValueType type = ComValue::UnknownType;
	Image::PixelType iuetype = image->GetPixelType();
	switch (iuetype) {
	case Image::BYTE:   
	case Image::RGB_BYTE:
	  type = ComValue::UCharType; break;
	case Image::UINT16: 
	case Image::RGB_16:
	  type = ComValue::UShortType; break;
	case Image::UINT32: 
	case Image::RGB_32:
	  type = ComValue::UIntType; break;
	case Image::INT8:   
	  type = ComValue::CharType; break;
	case Image::INT16:  
	  type = ComValue::ShortType; break;
	case Image::INT32:  
	  type = ComValue::IntType; break;
	case Image::FLOAT:  
	case Image::COMPLEX_FLOAT:
	  type = ComValue::FloatType; break;
	case Image::DOUBLE: 
	case Image::COMPLEX_DOUBLE:
	  type = ComValue::DoubleType; break;
	case Image::UINT64: 
	  if (sizeof(unsigned long)==8) type = ComValue::ULongType; break;
	case Image::INT64:  
	  if (sizeof(long)==8) type = ComValue::ULongType; break;
	case Image::RGBA:
	case Image::PYRAMID:
	  break;
	}
	
	if (type != ComValue::UnknownType && iuetype<=Image::DOUBLE) {
	  ComValue retval;
	  image->GetPixel((void *)&retval.double_ref(), x, y);
	  retval.type(type);
	  push_stack(retval);
	  return;
	} else if (iuetype==Image::RGB_BYTE) {
	  unsigned char bytes[3];
	  image->GetPixel(&bytes, x, y);
	  AttributeValueList* avl = new AttributeValueList();
	  for (int i=0; i<3; i++) 
	    avl->Append(new ComValue(bytes[i], ComValue::UCharType));
	  ComValue retval(avl);
	  push_stack(retval);
	  return;
	} else if (iuetype==Image::RGB_16) {
	  unsigned short words[3];
	  image->GetPixel(&words, x, y);
	  AttributeValueList* avl = new AttributeValueList();
	  for (int i=0; i<3; i++) 
	    avl->Append(new ComValue(words[i], ComValue::UShortType));
	  ComValue retval(avl);
	  push_stack(retval);
	  return;
	} else if (iuetype==Image::RGB_32) {
	  unsigned short ints[3];
	  image->GetPixel(&ints, x, y);
	  AttributeValueList* avl = new AttributeValueList();
	  for (int i=0; i<3; i++) 
	    avl->Append(new ComValue(ints[i], ComValue::UIntType));
	  ComValue retval(avl);
	  push_stack(retval);
	  return;
	}
      }
    }
  }
  push_stack(ComValue::nullval());
}
 

/*-----------------------------------------------------------------------*/

IueNcolsFunc::IueNcolsFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueNcolsFunc::execute() {
  ComValue img(stack_arg(0));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {  
    Image* image = comp->image();
    if (image) {
      ComValue retval(image->GetSizeX(), ComValue::UIntType);
      push_stack(retval);
      return;
    }
  }
  push_stack(ComValue::nullval());
}
 
/*-----------------------------------------------------------------------*/

IueNrowsFunc::IueNrowsFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IueNrowsFunc::execute() {
  ComValue img(stack_arg(0));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {  
    Image* image = comp->image();
    if (image) {
      ComValue retval(image->GetSizeY(), ComValue::UIntType);
      push_stack(retval);
      return;
    }
  }
  push_stack(ComValue::nullval());
}

/*-----------------------------------------------------------------------*/

IuePixTypeFunc::IuePixTypeFunc(ComTerp* comterp) : IueFunc(comterp) {
}

void IuePixTypeFunc::execute() {
  ComValue img(stack_arg(0));
  reset_stack();

  IueImageComp* comp = image_comp(img);
  if (comp) {  
    Image* image = comp->image();
    if (image) {
        char* typestr = nil;
	switch (image->GetPixelType()) {
	case Image::UNKNOWN:   
	  typestr = "UNKNOWN"; break;
	case Image::BYTE:   
	  typestr = "BYTE"; break;
	case Image::UINT16: 
	  typestr = "UINT16"; break;
	case Image::UINT32: 
	  typestr = "UINT32"; break;
	case Image::UINT64: 
	  typestr = "UINT64"; break;
	case Image::INT8:   
	  typestr = "INT8"; break;
	case Image::INT16:  
	  typestr = "INT16"; break;
	case Image::INT32:  
	  typestr = "INT32"; break;
	case Image::INT64:  
	  typestr = "INT64"; break;
	case Image::FLOAT:  
	  typestr = "FLOAT"; break;
	case Image::DOUBLE: 
	  typestr = "DOUBLE"; break;
	case Image::COMPLEX_FLOAT:
	  typestr = "COMPLEX_FLOAT"; break;
	case Image::COMPLEX_DOUBLE:
	  typestr = "COMPLEX_DOUBLE"; break;
	case Image::RGB_BYTE:
	  typestr = "RGB_BYTE"; break;
	case Image::RGB_16:
	  typestr = "RGB_16"; break;
	case Image::RGB_32:
	  typestr = "RGB_32"; break;
	case Image::RGBA:
	  typestr = "RGBA"; break;
	case Image::PYRAMID:
	  typestr = "PYRAMID"; break;
	}
      ComValue retval(symbol_add(typestr), ComValue::SymbolType);
      push_stack(retval);
      return;
    }
  }
  push_stack(ComValue::nullval());
}
 
 

 



