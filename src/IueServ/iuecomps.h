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

#ifndef iuecomps_h
#define iuecomps_h

#include <OverlayUnidraw/ovcomps.h>

// classes from IUE
#undef Image
class ADRGFile;
class Image; 
class FileImage; 

class IueServComp : public OverlayComp {
public:
    IueServComp();
    ~IueServComp();

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    classid("IueServComp");

};

class IueImageComp : public IueServComp {
public:
    IueImageComp(const char* file, boolean adrg=false);
    IueImageComp(Image*);
    virtual ~IueImageComp();

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    Image* image() { return _image; }
    FileImage* fileimage() { return _fileimage; }

protected:
    FileImage* _fileimage;
    ADRGFile* _adrgfile;
    Image* _image;

    classid("IueImageComp");

};

#endif
