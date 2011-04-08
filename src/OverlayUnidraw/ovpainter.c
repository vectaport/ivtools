/*
 * Copyright (c) 1994-1995 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
 * Copyright (c) 1996, 1999 R.B. Kissh & Associates
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


#define NDEBUG

#include <OverlayUnidraw/ovpainter.h>

#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovrestimage.h>

#include <Unidraw/Graphic/rasterrect.h>

#include <InterViews/canvas.h>
#include <InterViews/raster.h>
#include <InterViews/transformer.h>
#include <InterViews/session.h>

#include <OS/table.h>
#include <OS/math.h>

#include <IV-X11/Xlib.h>
#include <IV-X11/Xutil.h>
#include <IV-X11/xcanvas.h>
#include <IV-X11/xdisplay.h>
#include <IV-X11/xpainter.h>
#include <IV-X11/xraster.h>

#include <assert.h>
#include <iostream.h>

#undef RasterRect

static inline int pround(float x) {
    return (x > 0. ? int(x + 0.5) : -int(-x + 0.499));
}


// ---------------------------------------------------------------------------

class Mapper {
public:
    Mapper(
        IntCoord coff_x, IntCoord coff_y, IntCoord goff_x, IntCoord goff_y
    );
    void Transform(
        const Transformer&, IntCoord x, IntCoord y, IntCoord& tx, IntCoord& ty
    ) const;
    void Transform(
        const Transformer&, float x, float y, float& tx, float& ty
    ) const;
    void Transform(
        const Transformer&, float& x, float& y
    ) const;

    void InverseTransform(
        const Transformer&, IntCoord x, IntCoord y, IntCoord& tx, IntCoord& ty
    ) const;
    void InverseTransform(
        const Transformer&, float x, float y, float& tx, float& ty
    ) const;
    void InverseTransform(
        const Transformer&, float& x, float& y
    ) const;

protected:
    float _coff_x;
    float _coff_y;

    float _goff_x;
    float _goff_y;
};

Mapper::Mapper(
    IntCoord coff_x, IntCoord coff_y, IntCoord goff_x, IntCoord goff_y
) 
    : _coff_x(float(coff_x)), _coff_y(float(coff_y)), _goff_x(float(goff_x)), 
      _goff_y(float(goff_y))
{
}

inline void Mapper::Transform(
    const Transformer& t, float x, float y, float& tx, float& ty
) const {
    x += _goff_x;
    y += _goff_y;

    t.transform(x, y, tx, ty);

    tx += _coff_x;
    ty += _coff_y;
}

inline void Mapper::Transform(
    const Transformer& t, IntCoord x, IntCoord y, IntCoord& tx, IntCoord& ty
) const {
    float ftx, fty;
    Transform(t, float(x), float(y), ftx, fty);
    tx = pround(ftx);
    ty = pround(fty);
}

inline void Mapper::Transform(
    const Transformer& t, float& x, float& y
) const {
    Transform(t, x, y, x, y);
}

inline void Mapper::InverseTransform(
    const Transformer& t, float x, float y, float& tx, float& ty
) const {
    x -= _coff_x;
    y -= _coff_y;

    t.inverse_transform(x, y, tx, ty);

    tx -= _goff_x;
    ty -= _goff_y;
}

inline void Mapper::InverseTransform(
    const Transformer& t, IntCoord x, IntCoord y, IntCoord& tx, IntCoord& ty
) const {
    float ftx, fty;
    InverseTransform(t, float(x), float(y), ftx, fty);
    tx = pround(ftx);
    ty = pround(fty);
}

inline void Mapper::InverseTransform(
    const Transformer& t, float& x, float& y
) const {
    InverseTransform(t, x, y, x, y);
}

// --------------------------------------------------------------------------


static void TransformedInfo(
    const Mapper* mpr, IntCoord swidth, IntCoord sheight, 
    const Transformer& t, IntCoord* dxmin, IntCoord* dxmax = nil, 
    IntCoord* dymin = nil, IntCoord* dymax = nil, IntCoord* pwidth = nil, 
    IntCoord* pheight = nil, boolean* rotated = nil
) {
    float x1 = 0;
    float y1 = 0;
    float x2 = 0;
    float y2 = sheight;
    float x3 = swidth;
    float y3 = sheight;
    float x4 = swidth;
    float y4 = 0;

    if (mpr) {
        mpr->Transform(t, x1, y1);
        mpr->Transform(t, x2, y2);
        mpr->Transform(t, x3, y3);
        mpr->Transform(t, x4, y4);
    } else {
        t.transform(x1, y1);
        t.transform(x2, y2);
        t.transform(x3, y3);
        t.transform(x4, y4);
    }

    IntCoord xmin, ymin, xmax, ymax;
    xmin = pround(Math::min(x1, x2, x3, x4));
    xmax = pround(Math::max(x1, x2, x3, x4)) - 1;
    ymin = pround(Math::min(y1, y2, y3, y4));
    ymax = pround(Math::max(y1, y2, y3, y4)) - 1;
 
    if (dxmin) {
        *dxmin = xmin;
    }
    if (dxmax) {
        *dxmax = xmax;
    }
    if (dymin) {
        *dymin = ymin;
    }
    if (dymax) {
        *dymax = ymax;
    }
    if (pwidth) {
        *pwidth = xmax - xmin + 1;
    }
    if (pheight) {
        *pheight = ymax - ymin + 1;
    }
    if (rotated) {
        *rotated = !(
            (pround(x1)==pround(x2) || pround(y1)==pround(y2)) && 
            (pround(x1)==pround(x4) || pround(y1)==pround(y4))
        );
    }
}


static void DrawSourceTransformedImage (
    XImage* s, int sx0, int sy0,
    XImage* m, int mx0, int my0,
    XDrawable d, unsigned int height, int dx0, int dy0,
    boolean stencil, unsigned long fg, unsigned long bg,
    XDisplay* display, GC gc, const Transformer* matrix,
    int xmin, int ymin, int xmax, int ymax
) {
    unsigned long lastdrawnpixel = fg;

    for (int xx = xmin; xx <= xmax; ++xx) {
        float lx, ly;
        float rx, ry;
        float tx, ty;
        matrix->Transform(float(xx), float(ymin), lx, ly);
        matrix->Transform(float(xx + 1), float(ymin), rx, ry);
        matrix->Transform(float(xx), float(ymax+1), tx, ty);
        float dx = (tx - lx) / float(ymax - ymin + 1);
        float dy = (ty - ly) / float(ymax - ymin + 1);
        int ilx = 0, ily = 0;
        int irx = 0, iry = 0;
        boolean lastmask = false, mask;
        unsigned long lastpixel = fg, pixel, source;
        for (int yy = ymin; yy <= ymax+1; ++yy) {
            mask = (
                yy <= ymax
                && (m == nil || XGetPixel(m, xx-mx0, m->height-1-(yy-my0)))
            );
            if (
                yy<sy0 || yy>=sy0+s->height || xx<sx0 || xx>=sx0+s->width
            ) {
                source = bg;
            } else {
                source = XGetPixel(s, xx-sx0, s->height-1-(yy-sy0));
            }
            if (stencil) {
                pixel = (source != 0) ? fg : bg;
            } else {
                pixel = source;
            }
            if (mask != lastmask || lastmask && pixel != lastpixel) {
                int iilx = pround(lx), iily = pround(ly);
                int iirx = pround(rx), iiry = pround(ry);
                if (lastmask) {
                    if (lastpixel != lastdrawnpixel) {
                        XSetForeground(display, gc, lastpixel);
                        lastdrawnpixel = lastpixel;
                    }
                    if (
                        (ilx==iilx || ily==iily) && (irx==ilx || iry==ily)
                    ) {
                        XFillRectangle(
                            display, d, gc,
                            Math::min(ilx, iirx) - dx0,
                            height - (Math::max(ily, iiry) - dy0),
                            Math::abs(ilx - iirx), Math::abs(ily - iiry)
                        );
                    } else {
                        XPoint v[4];
                        v[0].x = ilx-dx0; v[0].y = height - (ily-dy0);
                        v[1].x = iilx-dx0; v[1].y = height - (iily-dy0);
                        v[2].x = iirx-dx0; v[2].y = height - (iiry-dy0);
                        v[3].x = irx-dx0; v[3].y = height - (iry-dy0);
                        XFillPolygon(
                            display, d, gc,
                            v, 4, Convex, CoordModeOrigin
                        );
                    }
                }
                ilx = iilx; ily = iily;
                irx = iirx; iry = iiry;
                lastpixel = pixel;
                lastmask = mask;
            }
            lx += dx; ly += dy;
            rx += dx; ry += dy;
        }
    }
    XSetForeground(display, gc, fg);
}


// ---------------------------------------------------------------------------


class ImageHolder {
public:
    ImageHolder();
    ~ImageHolder();

    XImage* _image;
    boolean _shared_memory;
#ifdef XSHM
    XShmSegmentInfo _shminfo;
#endif
};

ImageHolder::ImageHolder() : _image(nil), _shared_memory(false) {
}

ImageHolder::~ImageHolder() {
    XDestroyImage(_image);

#ifdef XSHM
    if (_shared_memory) {
        Display& d = *Session::instance()->default_display();
        RasterRep::free_shared_memory(d, _shminfo);
    }
#endif
}


declareTable(ImageTable,Pixmap,ImageHolder*)
implementTable(ImageTable,Pixmap,ImageHolder*)

class ImageCache {
public:
    ImageCache();
    ~ImageCache();

    XImage* get(const Raster* r);
    XImage* get(
        Display&, Pixmap, int width, int height, const Raster* r = nil
    );
    void remove(Pixmap);

protected:
    ImageTable _table;
};


static ImageCache* icache_;

ImageCache::ImageCache() : _table(100) {
}

ImageCache::~ImageCache() {
    TableIterator(ImageTable) i(_table);
    for (; i.more(); i.next()) {
        delete i.cur_value();
    }
}

XImage* ImageCache::get(
    Display& d, Pixmap pix, int width, int height, const Raster* ras
) {
    XImage* ret = nil;

    if (ras && (pix == ras->rep()->pixmap_)) {
        ret = ras->rep()->image_;
    }
    else {
        ImageHolder* ih = nil;
        if (!_table.find(ih, pix)) {
            XDisplay* dpy = d.rep()->display_;
            ih = new ImageHolder;

#ifdef XSHM
            RasterRep::init_shared_memory(
                ih->_shared_memory, d, ih->_shminfo, width, height, ih->_image,
                pix
            );
#endif

            if (!ih->_shared_memory) {
                ih->_image = XGetImage(
                    dpy, pix, 0, 0, width, height, AllPlanes, ZPixmap
                );
            }

            _table.insert(pix, ih);
        }
        ret = ih->_image;
    }

    return ret;
}

XImage* ImageCache::get(const Raster* r) {
    RasterRep* srep = r->rep();
    return get(
        *srep->display_, srep->pixmap_, srep->pwidth_, srep->pheight_, r
    );
}

void ImageCache::remove(Pixmap p) {
    ImageHolder* doomed;
    if (_table.find(doomed, p)) {
        _table.remove(p);
        delete doomed;
    }
}


// ---------------------------------------------------------------------------

// return the bounding box of the intersection of an axis aligned rectangle
// and a polygon region

static void BoundingRectIntersect(
  const XRectangle& r1, const Region& r2, XRectangle& ins
) {
    Region tmp = XCreateRegion();
    XUnionRectWithRegion((XRectangle*)&r1, tmp, tmp);
    XIntersectRegion(r2, tmp, tmp);

    XClipBox(tmp, &ins);
    XDestroyRegion(tmp);
}


static inline void check_dim(IntCoord& w, IntCoord& h) {
   if (w <= 0) {
        w = 1;
    }

    if (h <= 0) {
       h = 1;
    }
}


static Pixmap CreateSourceClippedRaster(
    const Mapper& mpr, unsigned long fg, unsigned long bg, Display& d, 
    const Raster* r, Pixmap smap, IntCoord swidth, IntCoord sheight, 
    const Transformer& tx, XRectangle* bb, IntCoord& dwidth, IntCoord& dheight
) {
    float llx, lux, rlx, rux;
    float lly, luy, rly, ruy;

    mpr.InverseTransform(tx, float(bb->x), float(bb->y), llx, lly);
    mpr.InverseTransform(
        tx, float(bb->x), float(bb->y + bb->height), lux, luy
    );
    mpr.InverseTransform(
        tx, float(bb->x + bb->width), float(bb->y), rlx, rly
    );
    mpr.InverseTransform(
        tx, float(bb->x + bb->width), float(bb->y + bb->height), rux, ruy
    );

    IntCoord xmin, ymin, xmax, ymax;
    xmin = pround(Math::min(llx, lux, rlx, rux));
    xmax = pround(Math::max(llx, lux, rlx, rux)) - 1;
    ymin = pround(Math::min(lly, luy, rly, ruy));
    ymax = pround(Math::max(lly, luy, rly, ruy)) - 1;

    DisplayRep& dr = *d.rep();
    XDisplay* dpy = dr.display_;

    XImage* source = icache_->get(d, smap, swidth, sheight, r);

    dwidth = bb->width;
    dheight = bb->height;

    check_dim(dwidth, dheight);

    Pixmap dmap = XCreatePixmap(
        dpy, dr.root_, dwidth, dheight, dr.default_visual_->depth()
    );
    GC xgc = XCreateGC(dpy, dmap, 0, nil);

    DrawSourceTransformedImage(
        source, 0, 0,
        nil, 0, 0,
        dmap, dheight, bb->x, bb->y,
        false, fg, bg,
        dpy, xgc, &tx,
        xmin - 1, ymin - 1, xmax + 1, ymax + 1  // out of range is ok
    );

    XFreeGC(dpy, xgc);
    return dmap;
}


static Pixmap CreateSourceRaster(
    const Mapper& mpr, unsigned long fg, unsigned long bg, Display& d, 
    const Raster* r, Pixmap smap, IntCoord swidth, IntCoord sheight, 
    const Transformer& tx, IntCoord& xmin, IntCoord& ymin, IntCoord& dwidth, 
    IntCoord& dheight
) {
    Transformer v(tx);

    IntCoord dxmin, dymin;
    TransformedInfo(
        &mpr, swidth, sheight, tx, &dxmin, nil, &dymin, nil, &dwidth, &dheight
    );

    xmin = dxmin;
    ymin = dymin;

    check_dim(dwidth, dheight);

    DisplayRep& dr = *d.rep();
    XDisplay* dpy = dr.display_;

    XImage* source = icache_->get(d, smap, swidth, sheight, r);

    Pixmap dmap = XCreatePixmap(
        dpy, dr.root_, dwidth, dheight, dr.default_visual_->depth()
    );
    GC xgc = XCreateGC(dpy, dmap, 0, nil);

    DrawSourceTransformedImage(
        source, 0, 0,
        nil, 0, 0,
        dmap, dheight, dxmin, dymin,
        false, fg, bg,
        dpy, xgc, &v,
        0, 0, swidth - 1, sheight - 1
    );

    XFreeGC(dpy, xgc);
    return dmap;
}


static Pixmap DrawDestTransformedImage(
    Display& d, OvRestrictedImage& img, const Transformer& tx, 
    IntCoord dwidth, IntCoord dheight, IntCoord dx0, IntCoord dy0
) {
    DisplayRep& dr = *d.rep();
    XDisplay* dpy = d.rep()->display_;

    Pixmap map = XCreatePixmap(
        dpy, dr.root_, dwidth, dheight, dr.default_visual_->depth()
    );
    GC xgc = XCreateGC(dpy, map, 0, nil);

    XImage* dest = nil;
    boolean shared_mem = false;
#ifdef XSHM
    XShmSegmentInfo shminfo;
    RasterRep::init_shared_memory(
        shared_mem, d, shminfo, dwidth, dheight, dest, map
    );
#endif

    if (!shared_mem) {
        dest = XGetImage(
            dpy, map, 0, 0, dwidth, dheight, AllPlanes, ZPixmap
        );
    }

    int sx0 = 0;
    int sy0 = 0;

    for (int dy = dheight - 1; dy >= 0; --dy) {
        float tx1, ty1, tx2, ty2;
        tx.inverse_transform(- dx0, dy - dy0, tx1, ty1);
        tx.inverse_transform(dwidth - dx0, dy - dy0, tx2, ty2);
        float delta_x = (tx2 - tx1) / dwidth;
        float delta_y = (ty2 - ty1) / dwidth;
        int sx, sy;
        for (int dx = 0; dx < dwidth; ++dx) {
            sx = int(tx1) + sx0;
            sy = int(ty1) + sy0;
            if (
                sx >= 0 && sx < img.Width() &&
                sy >= 0 && sy < img.Height()
            ) {
                XPutPixel(
                    dest, dx, dheight - 1 - dy,
                    img.Peek(sx, img.Height() - 1 - sy)
                );
            }
            tx1 = tx1 + delta_x;
            ty1 = ty1 + delta_y;
        }
    }

    XPutImage(dpy, map, xgc, dest, 0, 0, 0, 0, dwidth, dheight);
    XFreeGC(dpy, xgc);

    XDestroyImage(dest);

#ifdef XSHM
    if (shared_mem) {
        RasterRep::free_shared_memory(d, shminfo);
    }
#endif

    return map;
}


static Pixmap CreateDestClippedRaster(
    const Mapper& mpr, Display& dis, OvRestrictedImage& img, 
    const Transformer& tx, XRectangle* bb, IntCoord& dwidth, IntCoord& dheight
) {
    Transformer v(tx);

    float xt, yt; 
    mpr.InverseTransform(v, (float)bb->x, (float)bb->y, xt, yt);

    float x0, y0;
    v.transform(0, 0, x0, y0);
    v.translate(-x0, -y0);

    v.transform(xt, yt, xt, yt);

    int xorig = -pround(xt);
    int yorig = -pround(yt);

    dwidth = bb->width;
    dheight = bb->height;

    check_dim(dwidth, dheight);

    return DrawDestTransformedImage(
        dis, img, v, dwidth, dheight, xorig, yorig
    );
}


static Pixmap CreateDestRaster(
    const Mapper& mpr, Display& dis, OvRestrictedImage& img, 
    const Transformer& tx, IntCoord& xmin, IntCoord& ymin, IntCoord& dwidth, 
    IntCoord& dheight
) {
    TransformedInfo(
       &mpr, img.Width(), img.Height(), tx, &xmin, nil, &ymin, nil, nil, nil
    );

    Transformer v(tx);

    float x0, y0;
    v.transform(0, 0, x0, y0);
    v.translate(-x0, -y0);

    IntCoord dxmin, dymin;
    TransformedInfo(
       &mpr, img.Width(), img.Height(), v, &dxmin, nil, &dymin, nil, &dwidth, 
       &dheight
    );

    check_dim(dwidth, dheight);

    int dx0 = -dxmin;
    int dy0 = -dymin;

    return DrawDestTransformedImage(dis, img, v, dwidth, dheight, dx0, dy0);
}

static void TransformFromSource(
    IntCoord swidth, IntCoord sheight, const Transformer& tx, 
    boolean& fromsource, boolean& overlimit
) {
    boolean rotated;
    IntCoord dwidth, dheight;

    TransformedInfo(
        nil, swidth, sheight, tx, nil, nil, nil, nil, &dwidth, &dheight, 
        &rotated
    );

    // this doesn't consider non-uniform scaling
    boolean zoomed_out = dwidth * dheight < swidth * sheight;

    fromsource = !(rotated || zoomed_out);

    overlimit = (dwidth * dheight) > 1000000;
}

static inline signed char _ovtxkey (int i) {
    if (i >= 0) {
        return (
            i<32 ? i : i<160 ? 24 + (i>>2) : i<672 ? 54 + (i>>4) : 127
        );
    } else {
        return (
            i>-32 ? i : i>-160 ? -24 - (i>>2) : i>-672 ? -54 - (i>>4) : -127
        );
    }
}

static int ovtx_key(const Transformer& tx, float x, float y) {
    float x1, y1, x2, y2, x3, y3;
    tx.transform(0, 0, x1, y1);
    tx.transform(0, y, x2, y2);
    tx.transform(x, 0, x3, y3);
    int k1 = _ovtxkey(int(x2 - x1)) & 0xff;
    int k2 = _ovtxkey(int(y2 - y1 - y)) & 0xff;
    int k3 = _ovtxkey(int(x3 - x1 - x)) & 0xff;
    int k4 = _ovtxkey(int(y3 - y1)) & 0xff;
    return (k1 << 24) + (k2 << 16) + (k3 << 8) + k4;
}

// -------------------------------------------------------------------------

class RasterKey {
public:
   RasterKey();
   RasterKey(const Transformer&, float width, float height);
   operator long() const; // hash
   boolean operator ==(const RasterKey&) const;

protected:
    Transformer _tx;
    float _width;
    float _height;
};

RasterKey::RasterKey(const Transformer& t, float w, float h) 
    : _tx(t), _width(w), _height(h)
{ 
    float x, y;
    _tx.transform(0., 0., x, y);
    _tx.translate(-x, -y);
}

RasterKey::RasterKey()
{ 
}

RasterKey::operator long() const { // hash
    return (long)ovtx_key(_tx, _width, _height);
}

static inline boolean eq_tol(float a, float b) {
    static float tol = 1e-6;
    float diff = a - b;
    return ((diff >= -tol) && (diff <= tol));
}

boolean RasterKey::operator ==(const RasterKey& r) const {
    float m00, m01, m10, m11, m20, m21;
    _tx.GetEntries(m00, m01, m10, m11, m20, m21);

    Transformer ct(r._tx);
    float x, y;
    ct.transform(0., 0., x, y);
    ct.translate(-x, -y);
    
    float tm00, tm01, tm10, tm11, tm20, tm21;
    ct.GetEntries(tm00, tm01, tm10, tm11, tm20, tm21);
    return(
        eq_tol(m00, tm00) && eq_tol(m01, tm01) && eq_tol(m10, tm10) &&
        eq_tol(m11, tm11) && eq_tol(m20, tm20) && eq_tol(m21, tm21)
    );
}

// -------------------------------------------------------------------------

declareTable2(OvPixmapTableBase,const Raster*,RasterKey,Pixmap)
implementTable2(OvPixmapTableBase,const Raster*,RasterKey,Pixmap)

class OvPixmapTable : public OvPixmapTableBase {
public:
    OvPixmapTable(int size);
    void remove(const Raster*);
};

OvPixmapTable::OvPixmapTable(int size) : OvPixmapTableBase(size) {
}

// having only one key we are forced to delete by iterating through the entire
// table
void OvPixmapTable::remove(const Raster* r) {
    Display& d = *Session::instance()->default_display();
    XDisplay* dpy = d.rep()->display_;

    OvPixmapTableBase_Entry** a;
    register OvPixmapTableBase_Entry* e;

    for (a = first_; a <= last_; a++) {
        e = *a;
        while (e != nil) {
            if (e->key1_ == r) {
                *a = e->chain_;
                XFreePixmap(dpy, e->value_);
                delete e;
                e = *a;
            } else {
                register OvPixmapTableBase_Entry* prev;
                do {
                    prev = e;
                    e = e->chain_;
                } while (e != nil && (e->key1_ != r));
                if (e != nil) {
                    prev->chain_ = e->chain_;
                    XFreePixmap(dpy, e->value_);
                    delete e;
                    e = prev->chain_;
                }
            }
        }
    }
}

static OvPixmapTable* tx_pixmaps_;

// -------------------------------------------------------------------------

class SourceRep {
public:
    SourceRep(Pixmap, float hs, float vs, IntCoord w, IntCoord h);
    ~SourceRep();

    Pixmap map_;
    float hscale_;
    float vscale_;
    IntCoord pwidth_;
    IntCoord pheight_;
};

SourceRep::SourceRep(Pixmap pm, float hs, float vs, IntCoord w, IntCoord h)
    : map_(pm), hscale_(hs), vscale_(vs), pwidth_(w), pheight_(h)
{
}

SourceRep::~SourceRep() {
}

declareTable(OvSourceTable,const Raster*,SourceRep*)
implementTable(OvSourceTable,const Raster*,SourceRep*)
static OvSourceTable* source_table_;

// swidth, sheight and key are always set
// if dpm is set then use that else either sri or spm will be set
// if spm or sri is set then hscale, vscale will be set

static void ImageSetup(
    const OverlayRaster* o_r, OverlayRasterRect* r_r, const Transformer& tx,
    OvFileImage*& sri, Pixmap& spm, IntCoord& swidth, IntCoord& sheight, 
    float& hscale, float& vscale, int& key, Pixmap& dpm
) {
    const OverlayRaster* r = o_r ? o_r : r_r->GetOriginal();
    Display* d = r->rep()->display_;
    sri = nil;
    spm = dpm = nil;

    key = ovtx_key(tx, r->width(), r->height());

#if 0
    if (key == 0) {  // no scale or rotation
#else
    if (!(tx.Rotated() || tx.Scaled())) {
#endif
        if (!r->rep()->pixmap_) {
            r_r->load_image();
        }
        dpm = r->rep()->pixmap_;
        swidth = r->rep()->pwidth_;   
        sheight = r->rep()->pheight_;   

    } else {  // rotated or scaled

        RasterKey rkey(tx, r->width(), r->height());
        if (!tx_pixmaps_->find(dpm, r, rkey)) {
            if (r->rep()->pixmap_) {
                spm = r->rep()->pixmap_;
                swidth = r->rep()->pwidth_;   
                sheight = r->rep()->pheight_;   
                hscale = vscale = 1.;
            } else {
                if (tx.Rotated()) {
                    r_r->load_image();
                    spm = r->rep()->pixmap_;
                    swidth = r->rep()->pwidth_;   
                    sheight = r->rep()->pheight_;   
                    hscale = vscale = 1.;
                } else {
                    // -- scaled only

                    float m00, m01, m10, m11, m20, m21;
                    tx.GetEntries(hscale, m01, m10, vscale, m20, m21);

                    SourceRep* srep;
                    if (!source_table_->find(srep, r)) {
                        if ((hscale < 1.) && (vscale < 1.)) {
                            // need to store this later in source_table
                            sri = OvFileImage::create(d, r_r);
                            assert(sri);
                            swidth = sri->Width(); 
                            sheight = sri->Height();
                        } else {
                            r_r->load_image();
                            spm = r->rep()->pixmap_;
                            swidth = r->rep()->pwidth_;   
                            sheight = r->rep()->pheight_;   
                            hscale = vscale = 1.;
                        }
                    } else {
                        assert( srep->hscale_ < 1. );
                        // -- ensure that the cached image has more detail
                        if (
                            (srep->hscale_ > hscale) && 
                            (srep->vscale_ > vscale) 
                        ) {
                            spm = srep->map_;
                            swidth = srep->pwidth_;
                            sheight = srep->pheight_;
                            hscale = srep->hscale_;
                            vscale = srep->vscale_;
                        } else if ((hscale < 1.) && (vscale < 1.)) {
                            // we need some sort of hueristic, we may just want
                            // to read the 1x res
                            // need to store this later in source_table
                            sri = OvFileImage::create(d, r_r);
                            assert(sri);
                            swidth = sri->Width(); 
                            sheight = sri->Height();
                        } else {  // normal or zoomed in scale
                            r_r->load_image();
                            spm = r->rep()->pixmap_;
                            swidth = r->rep()->pwidth_;   
                            sheight = r->rep()->pheight_;   
                            hscale = vscale = 1.;
                        }
                    }
                }
            }
        } else {
            swidth = r->rep()->pwidth_;   
            sheight = r->rep()->pheight_;   
        }
    }
}


static Pixmap DetermineImage(
    const Mapper& mpr, unsigned long fg, unsigned long bg, 
    const OverlayRaster* o_r, OverlayRasterRect* r_r, 
    const Transformer& tx, XRectangle* bb, 
    boolean& free_pixmap, IntCoord& xmin, IntCoord& ymin, IntCoord& dwidth, 
    IntCoord& dheight
) {
    Pixmap map;
    const OverlayRaster* r = o_r ? o_r : r_r->GetOriginal();
    Display& dis = *r->rep()->display_;

    Pixmap spm = nil;
    Pixmap dpm = nil;
    IntCoord swidth, sheight;
    float hscale, vscale;
    OvFileImage* sri = nil;
    int key;

    ImageSetup(
        r, r_r, tx, sri, spm, swidth, sheight, hscale, vscale, key, dpm
    );

    boolean from_source, over_maxpixels;
    TransformFromSource(swidth, sheight, tx, from_source, over_maxpixels);

    if (dpm) {
        map = dpm;
        free_pixmap = false;

        TransformedInfo(
            &mpr, swidth, sheight, tx, &xmin, nil, &ymin, nil, &dwidth, 
            &dheight
        );
    } else if (sri) { // not rotated, hscale && vscale < 1

        if (over_maxpixels) {  // just do visable area
            free_pixmap = true;
            map = CreateDestClippedRaster(
                mpr, dis, *sri, tx, bb, dwidth, dheight 
            );

            xmin = bb->x;
            ymin = bb->y;
        } else {
            free_pixmap = false;
            map = CreateDestRaster(
                mpr, dis, *sri, tx, xmin, ymin, dwidth, dheight
            );

            SourceRep* srep;
            if (source_table_->find(srep, r)) {
                source_table_->remove(r);
                delete srep;
            }

            srep = new SourceRep(
                map, hscale, vscale, dwidth, dheight
            );
            source_table_->insert(r,srep);

            RasterKey rkey(tx, r->width(), r->height());
            tx_pixmaps_->insert(r, rkey, map);
        }

        delete sri;

    } else {  // spm is set

        if (r->rep()->pixmap_) {  // this raster has no need for source_table
            SourceRep* srep;
            if (source_table_->find(srep, r)) {
                source_table_->remove(r);
                delete srep;
            }
        }

        float m00, m01, m10, m11, m20, m21;
        tx.GetEntries(m00, m01, m10, m11, m20, m21);
        Transformer stx(m00/hscale, m01, m10, m11/vscale, m20, m21);

        if (over_maxpixels) {  // just do visable area
            free_pixmap = true;
            if (from_source) {
                map = CreateSourceClippedRaster(
                    mpr, fg, bg, dis, r, spm, swidth, sheight, stx, bb, dwidth,
                    dheight
                );
            } else {
                OvMemoryImage mem(icache_->get(dis, spm, swidth, sheight, r));
                map = CreateDestClippedRaster(
                    mpr, dis, mem, stx, bb, dwidth, dheight
                );
            }

            xmin = bb->x;
            ymin = bb->y;

        } else {  // full image

            free_pixmap = false;

            if (from_source) {
               map = CreateSourceRaster(
                   mpr, fg, bg, dis, r, spm, swidth, sheight, stx, xmin, 
                   ymin, dwidth, dheight
               );
            } else {  // from destination
                OvMemoryImage mem(icache_->get(dis, spm, swidth, sheight,r));
                map = CreateDestRaster(
                    mpr, dis, mem, stx, xmin, ymin, dwidth, dheight
                );
            }

            RasterKey rkey(tx, r->width(), r->height());
            tx_pixmaps_->insert(r, rkey, map);
        }
    }

    return map;
}

void OverlayPainter::RasterRect(
    Canvas* c, IntCoord x, IntCoord y, Raster* r
) {
    DoRasterRect(c, x, y, (OverlayRaster*)r, nil);
}

void OverlayPainter::RasterRect(
    Canvas* c, IntCoord x, IntCoord y, OverlayRasterRect* r_r
) {
    DoRasterRect(c, x, y, nil, r_r);
}

void OverlayPainter::DoRasterRect(
    Canvas* c, IntCoord x, IntCoord y, OverlayRaster* o_r, OverlayRasterRect* r_r
) {
    if (c == nil || r_r == nil) {
	return;
    }

    r_r->load_image();

    const OverlayRaster* r = o_r ? o_r : r_r->GetOriginal();
    r_r->damage_flush();  // was r->flush()

    if (!icache_) {
        icache_ = new ImageCache();
    }

    if (!source_table_) {
        source_table_ = new OvSourceTable(1024);
    }

    if (!tx_pixmaps_) {
        tx_pixmaps_ = new OvPixmapTable(1024);
    }

    // -- transform the corners of the image
   
    IntCoord rw = (unsigned int)r->pwidth();
    IntCoord rh = (unsigned int)r->pheight();

    IntCoord x1, y1, x2, y2, x3, y3, x4, y4;
    MapRoundUp(c, x, y, x1, y1);             
    MapRoundUp(c, x, y + rh, x2, y2);        
    MapRoundUp(c, x + rw, y + rh, x3, y3);   
    MapRoundUp(c, x + rw, y, x4, y4);

    XPoint rast_area[4];
    rast_area[0].x = x1; rast_area[0].y = y1;
    rast_area[1].x = x2; rast_area[1].y = y2;
    rast_area[2].x = x3; rast_area[2].y = y3;
    rast_area[3].x = x4; rast_area[3].y = y4;
    Region rg = XPolygonRegion(rast_area, 4, EvenOddRule);

    static const Transformer tt;
    const Transformer* tmatrix = GetTransformer() ? GetTransformer() : &tt;

    XRectangle bb;

    if (Rep()->clipped) {
        BoundingRectIntersect(Rep()->xclip[0], rg, bb);
	// cerr << "used clipbox of " 
	     // << Rep()->xclip[0].x << "," << Rep()->xclip[0].y << ","
	     // << Rep()->xclip[0].width << "," << Rep()->xclip[0].height << "\n";
    }
    else {
        XRectangle canvas_rect;
        canvas_rect.x = 0;
        canvas_rect.y = 0;
        canvas_rect.height = c->pheight();
        canvas_rect.width = c->pwidth();

        BoundingRectIntersect(canvas_rect, rg, bb);
    }

    Region tmp = XCreateRegion();
    XUnionRectWithRegion(&bb, tmp, tmp);
    XIntersectRegion(rg, tmp, rg);
    XDestroyRegion(tmp);

    if (r_r && r_r->clippts()) {
      MultiLineObj* mlo = r_r->clippts();
      XPoint polypts[mlo->count()];
      for (int i=0; i<mlo->count(); i++) {
	IntCoord x, y;
	MapRoundUp(c, mlo->x()[i], mlo->y()[i], x, y);
	polypts[i].x = x;
	polypts[i].y = y;
      }
      Region poly = XPolygonRegion(polypts, mlo->count(), EvenOddRule);
      XIntersectRegion(rg, poly, rg);
      XDestroyRegion(poly);
    }

    boolean free_pmap;
    IntCoord xmin, ymin;

    // map from ul to ll and the X origin to IV origin
    XRectangle iv_bb(bb);
    iv_bb.y = bb.y + bb.height;
    iv_bb.y = c->pheight() - 1 - iv_bb.y;

    IntCoord pwidth, pheight;
    IntCoord cx, cy;
    GetOrigin(cx, cy);
    Mapper mapper(cx, cy, x, y);

    Pixmap dest = DetermineImage(
        mapper, GetFgColor()->PixelValue(), GetBgColor()->PixelValue(),
        o_r, r_r, *tmatrix, &iv_bb, free_pmap, xmin, ymin, pwidth, pheight
    );

    Display& d = *r->rep()->display_;
    XDisplay* dpy = d.rep()->display_;
    XDrawable xid = c->rep()->xdrawable_;

    if (xid != CanvasRep::unbound) {
        XSetRegion(dpy, Rep()->fillgc, rg);
        XSetGraphicsExposures(dpy, Rep()->fillgc, False);
        XCopyArea(
    	    dpy, dest, xid, Rep()->fillgc,
	    0, 0, pwidth, pheight, xmin, 
            c->pheight() - 1 - (ymin + pheight)
        );
        XSetGraphicsExposures(dpy, Rep()->fillgc, True);
        XDestroyRegion(rg);

        if (Rep()->clipped) {
	    XSetClipRectangles(
              dpy, Rep()->fillgc, 0, 0, Rep()->xclip, 1, Unsorted
            );
        } else {
    	    NoClip();
        }
    }

    if (free_pmap) {
        XFreePixmap(dpy, dest);
        icache_->remove(dest);
    }
}


void OverlayPainter::MapRoundUp(
    Canvas* c, IntCoord x, IntCoord y, IntCoord& mx, IntCoord& my
) {
    Transformer* matrix = GetTransformer();
    int xoff, yoff;

    if (matrix == nil) {
	mx = x; my = y;
    } else {
        float fx, fy;
        matrix->Transform(float(x), float(y), fx, fy);
        mx = pround(fx);
        my = pround(fy);
    }
    GetOrigin(xoff, yoff);
    mx += xoff;
    my = c->pheight() - 1 - (my + yoff);
}

// ---------------------------------------------------------------------------

/* static */ void OverlayPainter::Uncache(Raster* r) {

    Display& d = *r->rep()->display_;
    XDisplay* dpy = d.rep()->display_;

    if (icache_) {
        icache_->remove(r->rep()->pixmap_);  // deletes ximage
    }

    // tx_pixmaps_ is a superset of source_table_

    if (tx_pixmaps_) {
        tx_pixmaps_->remove(r);  // frees pixmap
    }

    if (source_table_) {
        SourceRep* dum;
        while(source_table_->find_and_remove(dum, r));
    }
}


/* static */ void OverlayPainter::FreeCache() {

    Display& d = *Session::instance()->default_display();
    XDisplay* dpy = d.rep()->display_;

    if (icache_) {
        delete icache_;  // will free up ximages
        icache_ = nil;
    }

    // tx_pixmaps_ is a superset of source_table_
        
    if (tx_pixmaps_) {
        Table2Iterator(OvPixmapTableBase) i(*tx_pixmaps_);
        
        for (; i.more(); i.next()) {
            XFreePixmap(dpy, i.cur_value());
        }
        delete tx_pixmaps_;
        tx_pixmaps_ = nil;
    }

    delete source_table_;
    source_table_ = nil;
}











