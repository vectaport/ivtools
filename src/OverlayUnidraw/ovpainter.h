/*
 * Copyright (c) 1994-1995,1999 Vectaport Inc.
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

/*
 * OverlayPainter -- painter extended to support OverlayUnidraw
 *             
 */

#ifndef ovpainter_h
#define ovpainter_h

#include <InterViews/painter.h>

class OverlayRaster;
class OverlayRasterRect;

//: specialized Painter for use with OverlayUnidraw.
class OverlayPainter : public Painter {
public:
#ifdef RasterRect
#undef RasterRect
    virtual void RasterRect(Canvas*, Coord x, Coord y, Raster*);
    virtual void RasterRect(Canvas*, Coord x, Coord y, OverlayRasterRect*);
#define RasterRect _lib_iv(RasterRect)
#else
    virtual void RasterRect(Canvas*, Coord x, Coord y, Raster*);
    virtual void RasterRect(Canvas*, Coord x, Coord y, OverlayRasterRect*);
#endif /* RasterRect */
    void MapRoundUp(Canvas*c, IntCoord x, IntCoord y, IntCoord& mx, IntCoord& my);
    static void Uncache(Raster*);
    static void FreeCache();

protected:
    virtual void DoRasterRect(
        Canvas*, Coord x, Coord y, OverlayRaster*, OverlayRasterRect*
    );
};

/*
 * Per-display shared painter information.
 */

#include <OS/table2.h>
#include <OS/list.h>
#include <IV-X11/xfont.h>

class Bitmap;
class RasterRep;

declareTable2(BitmapTable,XFont,int,Bitmap*)
declareTable2(RasterTable,const Raster*,int,RasterRep*)

class PainterDpyInfoList;

class PainterDpyInfo {
public:
    Display* display_;
    BitmapTable* btable_;
    BitmapTable* txtable_;
    RasterTable* tx_rasters_;
    static PainterDpyInfoList* info_list_;

    enum { TxFontsDefault, TxFontsOff, TxFontsOn, TxFontsCache } txfonts;
    enum {
	TxImagesDefault, TxImagesAuto, TxImagesDest, TxImagesSource
    } tximages;

    static PainterDpyInfo* find(Display*);

    int tx_key(const Transformer&, Coord x, Coord y);

    Bitmap* get_char_bitmap(const Font*, int c, int k, const Transformer&);
    RasterRep* tx_raster(const Raster*, const Transformer&);
};

declarePtrList(PainterDpyInfoList,PainterDpyInfo);

#endif


