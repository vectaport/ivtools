/*
 * Copyright (c) 1998-1999 Vectaport Inc.
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

//: gray-level image with arbitary numeric values.
class GrayRaster : public OverlayRaster {
public:
    GrayRaster(unsigned long width, unsigned long height, 
	       AttributeValue::ValueType type = AttributeValue::UCharType,
	       void* data = nil);
    // construct a GrayRaster of 'width','height' and 'type', with an optional
    // copy of 'data', a linear array of 'width'*'height' values of 'type'.
    GrayRaster(const GrayRaster& raster);
    // construct a GrayRaster by copying 'raster'.
    virtual ~GrayRaster();

    virtual OverlayRaster* copy() const;
    // interface to GrayRaster copy constructor.

    virtual void poke(
	unsigned long x, unsigned long y,
	ColorIntensity red, ColorIntensity green, ColorIntensity blue,
	float alpha
    );
    // Convert 'red','green','blue' to graylevel value using 
    // this equation: 0.299 r + 0.587 g + 0.114 b, then poke at 'x','y'.

    virtual void graypeek(unsigned long x, unsigned long y, unsigned int& v);
    // convert value of value_type() at 'x','y' to an unsigned int.
    virtual void graypeek(unsigned long x, unsigned long y, unsigned long& v);
    // convert value of value_type() at 'x','y' to an unsigned long.
    virtual void graypeek(unsigned long x, unsigned long y, float& v);
    // convert value of value_type() at 'x','y' to a float.
    virtual void graypeek(unsigned long x, unsigned long y, double& v);
    // convert value of value_type() at 'x','y' to a double.

    virtual void graypeek(unsigned long x, unsigned long y, AttributeValue& v);
    // insert value of value_type() at 'x','y' into an AttributeValue.
    virtual void graypoke(unsigned long x, unsigned long y, unsigned int v);
    // convert unsigned int to value_type(), then poke at 'x','y'.
    virtual void graypoke(unsigned long x, unsigned long y, unsigned long v);
    // convert unsigned long to value_type(), then poke at 'x','y'.
    virtual void graypoke(unsigned long x, unsigned long y, float v);
    // convert unsigned float to value_type(), then poke at 'x','y'.
    virtual void graypoke(unsigned long x, unsigned long y, double v);
    // convert double to value_type(), then poke at 'x','y'.
    virtual void graypoke(unsigned long x, unsigned long y, AttributeValue v);
    // convert numeric AttributeValue to value_type(), then poke at 'x','y'.

    virtual void highlight(unsigned long x, unsigned long y);
    // highlight pixel at 'x','y' by setting red value to 1.0.
    virtual void unhighlight();
    // clear pixel highlighting by restoring red value.

    virtual void flush() const;
    // scale pixel values to onscreen graylevels and flush to display.
    virtual int status() const;
    // 0 if ok, -1 if error.

    virtual OverlayRaster* scale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );
    // create new raster scaled between 'mingray' and 'maxgray', and return
    // command string to reproduce this effect after save/restore.

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );
    // create new raster pseudo-colored between 'mingray' and 'maxgray', 
    // and return command string to reproduce this effect after save/restore.

    virtual OverlayRaster* logscale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );
    // create new raster logarithmically scaled between 'mingray' and 'maxgray', 
    // and return command string to reproduce this effect after save/restore.

    virtual AttributeValue::ValueType value_type() const { return _type; }
    // pixel type.

    virtual boolean write(ostream& out, boolean gray=false);
    // write raster to ostream.
    virtual boolean read(istream& in, boolean gray=false);
    // read raster from istream.
    virtual boolean gray_flag() { return true; }
    // always true.

    boolean top2bottom() const { return _t2b; }
    // orientation of pixel rows.  Default is true.
    void top2bottom(boolean t2b) { _t2b = t2b; }
    // set orientation of pixel rows.

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, RampAlignment = R_LT
    );
    // embed gray-level ramp in raster at given alignment, and return 'cmd'
    // string to reproduce this effect after save/restore.

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, IntCoord x, IntCoord y
    );
    // embed gray-level ramp in raster at 'x', 'y',  and return 'cmd' string 
    // to reproduce this effect after save/restore.

    void set_minmax(double minval, double maxval, boolean fixminmax = false); 
    // set 'minval' and 'maxval' used for flush().

protected:
    void init(AttributeValue::ValueType type =AttributeValue::UCharType,
	      void* data=nil);
    // initialize raster of 'type' with optional 'data'.

    unsigned char ipeek(unsigned long x, unsigned long y);
    // raw byte peek.
    void ipoke(unsigned long x, unsigned long y, int byte);
    // raw byte poke.
    void vpeek(unsigned long x, unsigned long y, AttributeValue&);
    // raw AttributeValue peek.
    void vpoke(unsigned long x, unsigned long y, AttributeValue&);
    // raw AttributeValue poke.

    virtual void scale(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // immediate-mode linear scaling.

    virtual void logscale(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // immediate-mode logarithmic scaling.

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // immediate-mode pseudocoloring.

    virtual void paintgrayramp(
        IntCoord left, IntCoord bottom, unsigned width, unsigned height,
	boolean horiz
    );
    // immediate-mode grayramp embedding.

    void gainbias_minmax(double& gain, double& bias, 
			 double& dmin, double& dmax) const;
    // determine 'dmin' and 'dmax', then compute 'gain' and 'bias' for 
    // mapping to 0 to 255.

    virtual boolean grayraster() { return true; }
    // always true.

protected:
    unsigned char* _pixel_map;        // map from intensity-byte to colormap-byte
    unsigned char* _data;             // raw pixel data of _type
    AttributeValue::ValueType _type;  // numeric type of pixels
    boolean _t2b;                     // top2bottom() flag
    double _minval;                   // cached minimum pixel value
    double _maxval;                   // cached maximum pixel value
    int _minmax_set;                  // true when _minval,_maxval computed
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

