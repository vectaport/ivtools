/*
 * Copyright (c) 1998 Vectaport Inc.
 * Copyright (c) 1997 Vectaport Inc., R.B. Kissh & Associates
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

#define NOREF

/*
 * GrayRaster definitions.
 */

#include <OS/math.h>
#include <math.h>
#include <nan.h>

#include <OverlayUnidraw/grayraster.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovipcmds.h>
#include <Attribute/attrvalue.h>
#include <OS/math.h>
#include <OS/memory.h>
#include <IV-X11/xdisplay.h>

#undef max
#undef min

/*****************************************************************************/

GrayRaster::GrayRaster
(unsigned long width, unsigned long height, AttributeValue::ValueType type, void* data) 
  : OverlayRaster(width, height)
{
  init(type, data);
}


GrayRaster::GrayRaster(const GrayRaster& image) 
    : OverlayRaster(image)
{
  init(image.value_type(), image._data);
  for(int i = 0; i < 256; i++) {
    _pixel_map[i] = image._pixel_map[i];
  }
  top2bottom(image.top2bottom());
  _minmax_set = image._minmax_set;
  _minval = image._minval;
  _maxval = image._maxval;
}


GrayRaster::~GrayRaster() {
  delete [] _data;
  delete [] _pixel_map;
}

void GrayRaster::init(AttributeValue::ValueType type, void* data) {
  _grayflag = true;
  _minmax_set = 0x00;
  _t2b = true;
  _type = type;
  _pixel_map = new unsigned char[256];
  int i;
  for(i = 0; i < 256; i++)
    _pixel_map[i] = i;
  
  int size = AttributeValue::type_size(value_type());
  unsigned long nbytes = pwidth() * pheight() * size;
  _data = new unsigned char[nbytes];
  
  if (data) {
    unsigned char* srcptr = (unsigned char*) data;
    unsigned char* destptr = (unsigned char*) _data;
    for (unsigned long j=0; j<nbytes; j++) 
      *destptr++ = *srcptr++;
    rep()->modified_ = true;
    
  }
  else 
    Memory::zero(_data, nbytes);

}


OverlayRaster* GrayRaster::copy() const {
  return new GrayRaster(*this);
}


void GrayRaster::poke(
    unsigned long x, unsigned long y,
    ColorIntensity red, ColorIntensity green, ColorIntensity blue, float
) {
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? y : (unsigned long)rep()->pheight_ - y - 1;

  // compute an equivalent gray
  // From ppmtopgm.c, netpbm
  // color to lumin. value: 0.299 r + 0.587 g + 0.114 b.

  if (AttributeValue::is_char(value_type())) {
    unsigned char gray;
    gray = (unsigned char)(0xff * (0.299 * red + 0.587 * green + 0.114 * blue));
    ipoke(xloc, yloc, gray);
  }
  else {
    float gray;
    gray = 0xff * (0.299 * red + 0.587 * green + 0.114 * blue);
    AttributeValue grayval(gray);
    vpoke(xloc, yloc, grayval);
  }

  rep()->modified_ = true;
}


void GrayRaster::graypeek(unsigned long x, unsigned long y, unsigned int& val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    val = _pixel_map[ipeek(xloc, yloc)];
  else {
    AttributeValue av;
    vpeek(xloc, yloc, av);
    val = av.uint_val();
  }
}


void GrayRaster::graypeek(unsigned long x, unsigned long y, unsigned long& val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    val = _pixel_map[ipeek(xloc, yloc)];
  else {
    AttributeValue av;
    vpeek(xloc, yloc, av);
    val = av.ulong_val();
  }
}


void GrayRaster::graypeek(unsigned long x, unsigned long y, float& val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    val = (float)_pixel_map[ipeek(xloc, yloc)];
  else {
    AttributeValue av;
    vpeek(xloc, yloc, av);
    val = av.float_val();
  }
}


void GrayRaster::graypeek(unsigned long x, unsigned long y, double& val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    val = (double)_pixel_map[ipeek(xloc, yloc)];
  else {
    AttributeValue av;
    vpeek(xloc, yloc, av);
    val = av.double_val();
  }
}


void GrayRaster::graypeek(unsigned long x, unsigned long y, AttributeValue& val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) {
    val.char_ref() = _pixel_map[ipeek(xloc, yloc)];
  } else {
    vpeek(xloc, yloc, val);
  }
}


void GrayRaster::graypoke(unsigned long x, unsigned long y, unsigned int i)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    ipoke(xloc, yloc, i);
  else {
    AttributeValue av(i, AttributeValue::UIntType);
    vpoke(xloc, yloc, av);
  }
  rep()->modified_ = true;
}


void GrayRaster::graypoke(unsigned long x, unsigned long y, unsigned long l)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    ipoke(xloc, yloc, (unsigned char)l);
  else {
    AttributeValue av(l);
    vpoke(xloc, yloc, av);
  }
  rep()->modified_ = true;
}

void GrayRaster::graypoke(unsigned long x, unsigned long y, float f)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    ipoke(xloc, yloc, (unsigned char)f);
  else {
    AttributeValue av(f);
    vpoke(xloc, yloc, av);
  }
  rep()->modified_ = true;
}

void GrayRaster::graypoke(unsigned long x, unsigned long y, double d)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    ipoke(xloc, yloc, (unsigned char)d);
  else {
    AttributeValue av(d);
    vpoke(xloc, yloc, av);
  }
  rep()->modified_ = true;
}

void GrayRaster::graypoke(unsigned long x, unsigned long y, AttributeValue val)
{
  unsigned long xloc = x;
  unsigned long yloc = 
    top2bottom() ? (unsigned long)rep()->pheight_ - y - 1 : y;
  if (AttributeValue::is_char(value_type())) 
    ipoke(xloc, yloc, val.uchar_val());
  else {
    vpoke(xloc, yloc, val);
  }
  rep()->modified_ = true;
}

void GrayRaster::highlight(unsigned long x, unsigned long y) {
  RasterRep* rr = rep();
  float r, g, b, a;
  Raster::peek(x, y, r, g, b, a);
  OverlayRaster::poke(x, y, 1., g, b, a);
  rr->modified_ = false;
}

void GrayRaster::unhighlight() {
  flush();
}

void GrayRaster::flush() const {
  RasterRep* r = rep();
  GrayRaster* me = (GrayRaster*)this;
  
  if (r->modified_) {
    if (!r->pixmap_) {
      me->init_space();
    }
    
    if (!gray_initialized()) {
      int status = me->gray_init();
      assert( status != -1 );
    }
    
    if (!_gray_map) {	
      return; 
    }

    // determine gain/bias to scale to 8 bit range
    double gain, bias;
    double min, max;
    gainbias_minmax(gain, bias, min, max);
    
    // sync our rep with the XImage
    int w = pwidth();
    int h = pheight();
    for(int x=0; x < w; x++) {
      for(int y=0; y < h; y++) {
	int pixel;
	if (AttributeValue::is_char(value_type()))
	  pixel = _gray_map[_pixel_map[me->ipeek(x, y)]].pixel;
	else {
	  AttributeValue av;
	  me->vpeek(x, y, av);
	  int ival = (int)(av.double_val()*gain+bias);
	  ival = ival < 0 ? 0 : (ival > 255 ? 255 : ival);
	  pixel = _gray_map[_pixel_map[ival]].pixel;
	}
	unsigned int xloc = x;
	unsigned int yloc = top2bottom() ? y : h-y-1;
	XPutPixel(r->image_, xloc, yloc, pixel); // replace with direct access
      }
    }
  }
  
  OverlayRaster::flush();
}

int GrayRaster::status() const {
  if (!gray_initialized()) 
    gray_init();
  if (!_gray_map) 
    return -1;
  else 
    return 0;
}


OverlayRaster* GrayRaster::scale
( ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd ) 
{
  GrayRaster* nrast = new GrayRaster(*this);
  nrast->scale(mingray, maxgray);
  nrast->flush();
  cmd = ScaleGrayFunc::CommandString(mingray,maxgray);
  return nrast;
}


OverlayRaster* GrayRaster::pseudocolor
( ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd ) {
  // this hardcoded scoping override should be unnecessary?
  OverlayRaster* nrast = pseudocolor(mingray, maxgray);
  cmd = PseudocolorFunc::CommandString(mingray,maxgray);
  return nrast;
}


void GrayRaster::scale
( ColorIntensity mingray, ColorIntensity maxgray )
{
  float fmin, fmax;
  int min, max;
  fmin = mingray * 0xff;
  fmax = maxgray * 0xff;
  min = Math::round(mingray * 0xff);
  max = Math::round(maxgray * 0xff);

  float ratio = ((fmax - fmin) == 0) ? 0. : (0xff / (fmax - fmin));
  
  int byte;
  int i;
  for (i = 0; i <= 255; i++) {
    byte = _pixel_map[i];
    if (byte < min) byte = min;
    if (byte > max) byte = max;
    _pixel_map[i] = Math::round((byte - min) * ratio);
  }
}


void GrayRaster::logscale 
( ColorIntensity mingray, ColorIntensity maxgray )
{
  int n = 255;
  int min, max;
  min = Math::round(mingray * 0xff);
  max = Math::round(maxgray * 0xff);

#ifdef DEBUG
  cerr << "logscale between " << mingray << " and " << maxgray << "\n";
#endif
  int i;
  int nvals = max-min+1;
  for(i = 0; i<=n; i++) {
    int byte = _pixel_map[i];
    if (byte < min) byte = min;
    if (byte > max) byte = max;
    double val = (byte-((double)min)) / nvals * (exp(1.0) - 1.0) + 1.0;
    _pixel_map[i] = (unsigned char) (log(val)*n);
#ifdef DEBUG
    cerr << "_pixel_map[" << i << "]  " << (int) _pixel_map[i] << "\n";
#endif
  }
}


OverlayRaster* GrayRaster::logscale(
    ColorIntensity minintensity, ColorIntensity maxintensity, CopyString& cmd 
) {
  GrayRaster* nrast = new GrayRaster(*this);
  nrast->logscale(minintensity, maxintensity);
  nrast->flush();
  cmd = LogScaleFunc::CommandString(minintensity, maxintensity);
  return nrast;
}

OverlayRaster* GrayRaster::pseudocolor(
    ColorIntensity mingray, ColorIntensity maxgray
) {

    if (AttributeValue::is_integer(value_type()))
	return OverlayRaster::pseudocolor(mingray, maxgray);

    OverlayRaster* color = new OverlayRaster(pwidth(), pheight());

    float ratio = (1.0 / (maxgray - mingray));

    float gray;

    RasterRep* rp = rep();
    unsigned int width = rp->pwidth_;
    unsigned int height = rp->pheight_;
    int w,h;

    for (w = 0; w < width; w++) {
	for (h = 0; h < height; h++) {

	    AttributeValue val;
	    graypeek(w, h, val);
            gray = val.double_val();
	    if (gray < mingray) gray = mingray;
	    if (gray > maxgray) gray = maxgray;
 	    float grayfract = (gray - mingray) * ratio;

	    float newr, newg, newb;

	    newr = grayfract < 0.5 ? 0.0 : (grayfract-.5)*2;
	    newg = grayfract < 0.5 ? grayfract*2 : 1.0 - (grayfract-.5)*2;
	    newb = grayfract < 0.5 ? 1.0 - (grayfract-.5)*2 : 0.0;
	    
	    newr = Math::max((float)0.0, newr);
	    newg = Math::max((float)0.0, newg);
	    newb = Math::max((float)0.0, newb);

	    color->poke(w, h, newr, newg, newb, 1.0);
        }
    }
    return color;
}


void GrayRaster::vpoke(unsigned long x, unsigned long y, AttributeValue& val)
{
  int size = AttributeValue::type_size(value_type());
  unsigned char* srcptr = nil;
  unsigned char c;  unsigned short s;  unsigned int i;  unsigned long l;  float f;  double d;
  switch (value_type()) {
    case AttributeValue::CharType:
    case AttributeValue::UCharType:
	c = val.uchar_val();
	srcptr = &c;
	break;
    case AttributeValue::ShortType:
    case AttributeValue::UShortType:
	s = val.ushort_val();
	srcptr = (unsigned char*)&s;
	break;
    case AttributeValue::IntType:
    case AttributeValue::UIntType:
	i = val.uint_val();
	srcptr = (unsigned char*)&i;
	break;
    case AttributeValue::LongType:
    case AttributeValue::ULongType:
	l = val.ulong_val();
	srcptr = (unsigned char*)&l;
	break;
    case AttributeValue::FloatType:
	f = val.float_val();
	srcptr = (unsigned char*)&f;
	break;
    case AttributeValue::DoubleType:
	d = val.double_val();
	srcptr = (unsigned char*)&d;
	break;
    }

  unsigned char* destptr = (unsigned char*)_data + pwidth()*y*size + x*size;
  for (int j=0; j<size; j++)
    *destptr++ = *srcptr++;
}


void GrayRaster::vpeek(unsigned long x, unsigned long y, AttributeValue& val)
{
  val.type(value_type());
  int size = val.type_size();
  unsigned char* srcptr = (unsigned char*)_data + pwidth()*y*size + x*size;
  unsigned char* destptr = (unsigned char*)val.value_ptr();
  for (int i=0; i<size; i++)
    *destptr++ = *srcptr++;
}

boolean GrayRaster::write (ostream& out, boolean gray) {
  Coord w = Width();
  Coord h = Height();

  out << w << "," << h << ",\n";
  unsigned int byte;
  int x=0;
  const int rowsiz=10;
  for (int y=0; y < h; y++) {
    for (int xstep=0; xstep < w; xstep+=rowsiz) {
      int xstop = w < xstep+rowsiz ? w : xstep+rowsiz;
      int xloc = x;
      int yloc = top2bottom() ? h-y-1 : y;
      for (x=xstep; x < xstop; x++) {
	if (AttributeValue::is_char(value_type())) {
	  graypeek(x, y, byte);
	  out << byte;
	} else {
	  AttributeValue av;
	  vpeek(x, yloc, av);
	  out << av;
	}
	
	if (x != w-1)
	  out << ",";
	xloc++;
      }
      /* end of x inner step for loop */
      if (x != w)
	out << "\n";
    }
    /* end of x outer step for loop */
    if (y != h-1)
      out << ",";
    out << "\n";
  }
  /* end of y for loop */

  return out.good();
}

boolean GrayRaster::read(istream& in, boolean gray) {
  int w = Width();
  int h = Height();
  char delim;

  if (is_type(AttributeValue::CharType) || is_type(AttributeValue::UCharType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	int data;
	in >> data;
	ipoke(x, y, data);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::ShortType) || is_type(AttributeValue::UShortType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	int data;
	in >> data;
	AttributeValue av(data, value_type());
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::IntType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	int data;
	in >> data;
	AttributeValue av(data, AttributeValue::IntType);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::UIntType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	unsigned int data;
	in >> data;
	AttributeValue av(data, AttributeValue::UIntType);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::LongType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	long data;
	in >> data;
	AttributeValue av(data);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::ULongType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	unsigned long data;
	in >> data;
	AttributeValue av(data);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::FloatType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	float data;
	in >> data;
	AttributeValue av(data);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  } else if (is_type(AttributeValue::DoubleType)) {
    for (int y=0; y < h; y++) {
      for (int x=0; x < w; x++) {
	double data;
	in >> data;
	AttributeValue av(data);
	vpoke(x, y, av);
	if (x != w-1)
	  in >> delim;
      }
      if ( y != h-1 ) 
	in >> delim;
    }
  }

  if (in.good()) {
    rep()->modified_ = true;
    return true;
  } else
    return false;
}


void GrayRaster::gainbias_minmax(double& gain, double& bias, 
				 double& dmin, double& dmax) const
{
  GrayRaster* me = (GrayRaster*)this;
  gain = 1.0;
  bias = 0.0;
  int w = pwidth();
  int h = pheight();
  if (!AttributeValue::is_char(value_type())) {
    AttributeValue av;
    me->vpeek(0, 0, av);
    dmin = av.double_val();
    dmax = av.double_val();
    for(int x=0; x < w; x++) {
      for(int y=0; y < h; y++) {
	me->vpeek(x, h-y-1, av);
	double dval = av.double_val();
        if (isnanorinf(dval)) continue;
	if (dval<dmin) dmin = dval;
	if (dval>dmax) dmax = dval;
      }
    }
    if (_minmax_set) {
      if (dmin<_minval || _minmax_set>1) dmin = _minval;
      if (dmax>_maxval || _minmax_set>1) dmax = _maxval;
    }
    gain = 256.0/(dmax-dmin);
    bias = -dmin*gain;
#ifdef DEBUG
    cerr << "grayraster min,max " << dmin << "," << dmax << "\n";
    cerr << "grayraster gain,bias " << gain << "," << bias << "\n";
#endif
  }
  return;
}

void GrayRaster::paintgrayramp
(IntCoord l, IntCoord b, unsigned int w, unsigned int h, boolean horiz) {

    IntCoord rows = b + h;
    IntCoord cols = l + w;

    // determine gain/bias to scale to 8 bit range
    double gain, bias;
    double min, max;
    gainbias_minmax(gain, bias, min, max);

    float gray;
    IntCoord row;
    IntCoord col;
    for (row = b; row < rows; row++) {
        for (col = l; col < cols; col++) {

	    if (horiz) {
  	        gray = (float(col) - l) / 
                    (((cols - l) == 1) ? 1 : ((cols - l) - 1));
	    }
            else {
  	        gray = (float(row) - b) / 
                    (((rows - b) == 1) ? 1 : ((rows - b) - 1));
            }   

	    AttributeValue av((gray*0xff-bias)/gain);
            vpoke(col, row, av);
        }
    }
}

OverlayRaster* GrayRaster::addgrayramp(
    CopyString& cmd, RampAlignment algn
) {
    GrayRaster* nrast = new GrayRaster(*this);
    nrast->_addgrayramp(algn);
    cmd = GrayRampFunc::CommandString(algn);
    return nrast;
}

OverlayRaster* GrayRaster::addgrayramp(
    CopyString& cmd, Coord x, Coord y
) {
    GrayRaster* nrast = new GrayRaster(*this);
    RampAlignment algn = ramppos(x, y);
    nrast->_addgrayramp(algn);
    cmd = GrayRampFunc::CommandString(algn);
    return nrast;
}


void GrayRaster::set_minmax(double minval, double maxval, boolean fixminmax) {
  _minval = minval; 
  _maxval = maxval; 
  _minmax_set = 0x1; 
  _minmax_set = _minmax_set | (fixminmax ? 0x02 : 0x00); 
}

