/*
 * Copyright (c) 1996-1997 R.B. Kissh & Associates
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

#ifndef ov_rest_image_h
#define ov_rest_image_h

#include <InterViews/color.h>
#include <InterViews/coord.h>

#include <IV-X11/Xlib.h>
#include <stdio.h>

class OvRestrictedImage {
public:
    virtual ~OvRestrictedImage();

    // N.B. 0,0 is the upper left corner, NOT IV conventions

    virtual unsigned long Peek(IntCoord x, IntCoord y) = 0;

    virtual int Width() const = 0;
    virtual int Height() const = 0;
};

class OverlayRasterRect;

class OvFileImage : public OvRestrictedImage {
public:
    virtual ~OvFileImage();
    static OvFileImage* create(Display*, const OverlayRasterRect*);

    virtual unsigned long Peek(IntCoord x, IntCoord y) = 0;

    virtual int Width() const;
    virtual int Height() const;

protected:
    OvFileImage();

    virtual void initialize();    
    void seek_fwd_rel(long count);

protected:
    FILE* _file;

    int _bytes_per_pixel;
    boolean _ppm;  // else pgm
    boolean _compressed;
    IntCoord _file_width;
    IntCoord _file_height;

    long _pos;
    Display* _display;

    IntCoord _xbeg;  // these are IV coords
    IntCoord _xend;
    IntCoord _ybeg;
    IntCoord _yend;
};

class OvPortableFileImage : public OvFileImage {
friend class OvFileImage;
public:
    virtual ~OvPortableFileImage();
    virtual unsigned long Peek(IntCoord x, IntCoord y);

protected:
    OvPortableFileImage();
    virtual void initialize();

    long to_offset(IntCoord x, IntCoord y) const;
};

inline long OvPortableFileImage::to_offset(IntCoord x, IntCoord y) const {
    x = _xbeg + x;
    y = (_file_height - 1) - _yend + y;
    return (y * _file_width * _bytes_per_pixel) + (x * _bytes_per_pixel);
}



// Note that this class is optimized for iterations that begin at 0,0 and
// proceed 1,0 2,0 3,0 ... 0,1 1,1 2,1 ... 

class OvTiledFileImage : public OvFileImage {
friend class OvFileImage;
public:
    virtual ~OvTiledFileImage();
    virtual unsigned long Peek(IntCoord x, IntCoord y);

protected:
    OvTiledFileImage();
    virtual void initialize();

    long to_offset(IntCoord x, IntCoord y) const;
};

inline long OvTiledFileImage::to_offset(IntCoord x, IntCoord y) const {
    return( 
        ((_xbeg * (_yend + 1)) + ((_file_width - _xbeg) * _ybeg) // to the tile
        + (y * (_xend - _xbeg + 1) + x))                         // to x, y
        * _bytes_per_pixel
    );
}


class OvMemoryImage : public OvRestrictedImage {
public:
    OvMemoryImage(XImage*);
    virtual ~OvMemoryImage();

    virtual unsigned long Peek(IntCoord x, IntCoord y);

    virtual int Width() const;
    virtual int Height() const;
protected:
    XImage* _image;
};


#endif

