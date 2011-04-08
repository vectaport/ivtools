/*
 * Copyright 1998 Vectaport Inc.
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

// from IUE
#include <ImageClasses/ADRGImage.h>
#include <ImageClasses/Image.h>
#include <EasyImage/FileImage.h>

#include <IueServ/iueclasses.h>
#include <IueServ/iuecomps.h>

/****************************************************************************/


int IueServComp::_symid = -1;

IueServComp::IueServComp() : OverlayComp() {
  _gr = nil;
}

IueServComp::~IueServComp() {
  _gr = nil;
}

Component* IueServComp::Copy() {
  IueServComp* comp = new IueServComp();
  return comp;
}

ClassId IueServComp::GetClassId () { return IUESERV_COMP; }

boolean IueServComp::IsA (ClassId id) {
  return IUESERV_COMP == id || OverlayComp::IsA(id);
}

/****************************************************************************/


int IueImageComp::_symid = -1;

IueImageComp::IueImageComp(const char* name, boolean adrg) : IueServComp() {
  _image = nil;
  _fileimage = nil;
  if (!adrg) {
    _fileimage = new FileImage(name);
    if (_fileimage) _image = *_fileimage;
  } else 
    _image = MakeADRGImage((char *)name);
  if (_image) _image->ref();
}

IueImageComp::IueImageComp(Image* image) : IueServComp() {
  _fileimage = nil;
  _image = image;
  if (_image) _image->ref();
}

IueImageComp::~IueImageComp() {
  delete _fileimage;
  if (_image) _image->unref();
}

Component* IueImageComp::Copy() {
    IueImageComp* comp = new IueImageComp(_image);
    return comp;
}

ClassId IueImageComp::GetClassId () { return IUEIMAGE_COMP; }

boolean IueImageComp::IsA (ClassId id) {
    return IUEIMAGE_COMP == id || IueServComp::IsA(id);
}
