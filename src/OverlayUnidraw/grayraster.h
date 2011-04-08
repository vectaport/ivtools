/*
 * Copyright (c) 1998 Vectaport Inc.
 * Copyright (c) 1997 R.B. Kissh & Associates, Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc.
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

/*
 * Gray Raster component declarations.
 */

#ifndef gray_rastercomp_h
#define gray_rastercomp_h

#include <OverlayUnidraw/ovraster.h>
#include <Attribute/attrvalue.h>

/* class GrayRaster */

// gray image optimized for "point" operations

class GrayRaster : public OverlayRaster {
public:
    GrayRaster(unsigned long width, unsigned long height, 
	       AttributeValue::ValueType type = AttributeValue::UCharType,
	       void* data = nil);
    GrayRaster(const GrayRaster& raster);
    virtual ~GrayRaster();

    virtual OverlayRaster* copy() const;

    virtual void poke(
	unsigned long x, unsigned long y,
	ColorIntensity red, ColorIntensity green, ColorIntensity blue,
	float alpha
    );
    virtual void graypeek(unsigned long x, unsigned long y, unsigned int&);
    virtual void graypeek(unsigned long x, unsigned long y, unsigned long&);
    virtual void graypeek(unsigned long x, unsigned long y, float&);
    virtual void graypeek(unsigned long x, unsigned long y, double&);
    virtual void graypeek(unsigned long x, unsigned long y, AttributeValue&);

    virtual void graypoke(unsigned long x, unsigned long y, unsigned int);
    virtual void graypoke(unsigned long x, unsigned long y, unsigned long);
    virtual void graypoke(unsigned long x, unsigned long y, float);
    virtual void graypoke(unsigned long x, unsigned long y, double);
    virtual void graypoke(unsigned long x, unsigned long y, AttributeValue);

    virtual void highlight(unsigned long x, unsigned long y);
    virtual void unhighlight();

    virtual void flush() const;
    virtual int status() const;

    virtual OverlayRaster* scale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    virtual OverlayRaster* logscale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    int value_size();
    virtual AttributeValue::ValueType value_type() const { return _type; }

    virtual boolean write(ostream& out, boolean gray=false);
    virtual boolean read(istream& in, boolean gray=false);
    virtual boolean gray_flag() { return true; }

    boolean top2bottom() const { return _t2b; }
    void top2bottom(boolean t2b) { _t2b = t2b; }

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, RampAlignment = R_LT
    );

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, IntCoord x, IntCoord y
    );

    void set_minmax(double minval, double maxval, boolean fixminmax = false); 

protected:
    void init(AttributeValue::ValueType=AttributeValue::UCharType,
	      void* data=nil);

    unsigned char ipeek(unsigned long x, unsigned long y);
    void ipoke(unsigned long x, unsigned long y, int byte);
    void vpeek(unsigned long x, unsigned long y, AttributeValue&);
    void vpoke(unsigned long x, unsigned long y, AttributeValue&);


    virtual void scale(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    virtual void logscale(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    virtual void paintgrayramp(
        IntCoord left, IntCoord bottom, unsigned width, unsigned height,
	boolean horiz
    );

    void gainbias_minmax(double& gain, double& bias, 
			 double& dmin, double& dmax) const;

    virtual boolean grayraster() { return true; }

protected:
    unsigned char* _pixel_map;
    void* _data;
    AttributeValue::ValueType _type;
    boolean _t2b;
    double _minval;
    double _maxval;
    int _minmax_set;
};


inline void GrayRaster::ipoke(
    unsigned long x, unsigned long y, int byte
) {
  assert((x >=0) && (x < pwidth()));
  assert((y >=0) && (y < pheight()));

  ((unsigned char*)_data)[rep()->pwidth_ * y + x] 
    = (unsigned char)byte;
}


inline unsigned char GrayRaster::ipeek(
    unsigned long x, unsigned long y
) {
  assert((x >=0) && (x < pwidth()));
  assert((y >=0) && (y < pheight()));

  return (unsigned char)((unsigned char*)_data)[rep()->pwidth_ * y + x];
}

#endif

