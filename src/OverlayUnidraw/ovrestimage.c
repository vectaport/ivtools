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

// #define COMPRESSED_OK

#include <OverlayUnidraw/ovrestimage.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovraster.h>

#include <InterViews/display.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/Xlib.h>

#include <stream.h>

#include <assert.h>

OvRestrictedImage::~OvRestrictedImage() {
}

// -------------------------------------------------------------------------

OvFileImage::OvFileImage() : _file(nil) {

}

OvFileImage::~OvFileImage() {
    if (_file) {
        if (_compressed) {
            pclose(_file);
        } else {
            fclose(_file);
        }
    }
}


int OvFileImage::Width() const {
    return _xend - _xbeg + 1;
}


int OvFileImage::Height() const {
    return _yend - _ybeg + 1;
}


void OvFileImage::initialize() {
}


/* static */ OvFileImage* OvFileImage::create(
    Display* d, const OverlayRasterRect* rr
) {
    int ppm_else_pgm;
    int ncols, nrows;
    boolean compressed;
    boolean tiled;
    int twidth, theight;
    boolean ok = false;

    const char* creator = OvImportCmd::ReadCreator(rr->path());
    if (strcmp(creator, "PGM") == 0) {
        ppm_else_pgm = 0;
        ok = true;
    } 
    else if (strcmp(creator, "PPM") == 0) {
        ppm_else_pgm = 1;
        ok = true;
    }
   
    OvFileImage* fi = nil;

    if (ok) {
        PortableImageHelper* ignored;
        FILE* file = OvImportCmd::Portable_Raster_Open(
           ignored, rr->path(), ppm_else_pgm, ncols, nrows, compressed, tiled,
           twidth, theight
        );

        if (file) {
	    if (tiled) {
                fi = new OvTiledFileImage;
            }
            else {
                fi = new OvPortableFileImage;
            }

            fi->_file = file;
            fi->_bytes_per_pixel = ppm_else_pgm ? 3 : 1;
            fi->_ppm = (ppm_else_pgm == 1) ? true : false;
            fi->_compressed = compressed;
            fi->_file_width = ncols;
            fi->_file_height = nrows;
            fi->_pos = 0;
            fi->_display = d;

            fi->_xbeg = rr->xbeg();
            fi->_xend = rr->xend();
            fi->_ybeg = rr->ybeg();
            fi->_yend = rr->yend();

            fi->initialize();            
       }
    }

    return fi;
}

void OvFileImage::seek_fwd_rel(long count) {

    if (count) {

        _pos += count;

        if (!_compressed) {
            fseek(_file, count, 1);
        } else {
            int i = 0;
            for(; i<count; i++) {
                getc(_file);
            }
        }
    }
}

// ---------------------------------------------------------------------------

OvPortableFileImage::OvPortableFileImage() : OvFileImage() {
}

OvPortableFileImage::~OvPortableFileImage() {
}


void OvPortableFileImage::initialize() {
    seek_fwd_rel(to_offset(0, 0));
}

// as implemented you may not peek twice at the same point

unsigned long OvPortableFileImage::Peek(IntCoord x, IntCoord y) {
    long move = to_offset(x,y) - _pos;

#ifdef COMPRESSED_OK
    if (move < 0) {
        assert(0);
        cerr << "illegal peek pos returning black\n";
        return XBlackPixel(
            _display->rep()->display_, _display->rep()->screen_
        );
    } else {
#endif
        seek_fwd_rel(move);

        if (_ppm) {
            u_short red = (u_short)((float(getc(_file))/0xff)*0xffff);
            u_short green = (u_short)((float(getc(_file))/0xff)*0xffff);
            u_short blue = (u_short)((float(getc(_file))/0xff)*0xffff);
            _pos += 3;

            XColor xc;
            _display->rep()->default_visual_->find_color(
                red, green, blue, xc
            );

            return xc.pixel;

        } else { // pgm

            int byte = getc(_file);
            _pos += 1;
	    return OverlayRaster::gray_lookup(byte);
        }
#ifdef COMPRESSED_OK
    }
#endif
}

// ---------------------------------------------------------------------------

OvTiledFileImage::OvTiledFileImage() : OvFileImage() {
}

OvTiledFileImage::~OvTiledFileImage() {
}

void OvTiledFileImage::initialize() {
    seek_fwd_rel(to_offset(0, 0));
}

// as implemented you may not peek twice at the same point

unsigned long OvTiledFileImage::Peek(IntCoord x, IntCoord y) {
    long move = to_offset(x,y) - _pos;

#ifdef COMPRESSED_OK
    if (move < 0) {
        assert(0);
        cerr << "illegal peek pos returning black\n";
        return XBlackPixel(
            _display->rep()->display_, _display->rep()->screen_
        );
    } else {
#endif
        seek_fwd_rel(move);

        if (_ppm) {
            u_short red = (u_short)((float(getc(_file))/0xff)*0xffff);
            u_short green = (u_short)((float(getc(_file))/0xff)*0xffff);
            u_short blue = (u_short)((float(getc(_file))/0xff)*0xffff);
            _pos += 3;

            XColor xc;
            _display->rep()->default_visual_->find_color(
                red, green, blue, xc
            );

            return xc.pixel;

        } else { // pgm

            int byte = getc(_file);
            _pos += 1;
	    return OverlayRaster::gray_lookup(byte);
        }
#ifdef COMPRESSED_OK
    }
#endif
}

// -------------------------------------------------------------------------

OvMemoryImage::OvMemoryImage(XImage* i) : _image(i) {
}

OvMemoryImage::~OvMemoryImage() {
}

int OvMemoryImage::Width() const {
    return _image->width;
}

int OvMemoryImage::Height() const {
    return _image->height;
}

unsigned long OvMemoryImage::Peek(IntCoord x, IntCoord y) {
    return XGetPixel(_image, x, y);
}

