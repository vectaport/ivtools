/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1997 R.B. Kissh & Associates, Vectaport Inc.
 * Copyright (c) 1994-1996 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * Overlay Raster component declarations.
 */

#ifndef overlay_rastercomp_h
#define overlay_rastercomp_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>
#include <Unidraw/Graphic/rasterrect.h>
#include <InterViews/raster.h>
#include <IV-X11/Xlib.h>
#include <IV-X11/xraster.h>
#include <OS/list.h>
#include <OS/string.h>
#include <assert.h>
#undef None

class OvImportCmd;
class OverlayRaster;
class OverlayRasterRect;
class RasterScript;
#include <iosfwd>

//: alignment of gray-level ramp when embbeded in a OverlayRaster.
enum RampAlignment { R_LB, R_LT, R_TL, R_TR, R_RT, R_RB, R_BR, R_BL };

declareList(CopyStringList,CopyString)

//: clone of RasterComp derived from OverlayComp.
class RasterOvComp : public OverlayComp {
public:
    RasterOvComp(OverlayRasterRect* = nil, const char* pathname = nil,
		 OverlayComp* parent = nil);
    RasterOvComp(istream&, OverlayComp* parent = nil);

    virtual ~RasterOvComp();

    OverlayRasterRect* GetOverlayRasterRect();
    // return pointer to graphic.
    virtual void SetPathName(const char*);
    // set pathname associated with raster.
    virtual const char* GetPathName();
    // return pathname associated with raster.
    virtual boolean GetByPathnameFlag() { return _import_flags & bypath_mask;}
    // return flag that indicates whether component will be serialized
    // by data or by pathname.
    virtual void SetByPathnameFlag(boolean flag) 
      { _import_flags = flag ? _import_flags |= bypath_mask : _import_flags &= ~bypath_mask;}
    // set flag that indicates whether component will be serialized
    // by data or by pathname.
    virtual boolean GetFromCommandFlag() { return _import_flags & fromcomm_mask;}
    // return flag that indicates whether component will be serialized
    // by data or by pathname.
    virtual void SetFromCommandFlag(boolean flag) 
      { _import_flags = flag ? _import_flags |= fromcomm_mask : _import_flags &= ~fromcomm_mask;}
    // set flag that indicates whether component will be serialized
    // by data or by pathname.

    virtual Component* Copy();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);

    virtual void Interpret(Command*);
    // interpret an image command, otherwise pass to base class.
    virtual void Uninterpret(Command*);
    // uninterpret an image command, otherwise pass to base class.

    virtual void Configure(Editor*);
    // hook for initializing component after Editor is constructed.

    static boolean UseGrayRaster() { return _use_gray_raster; }
    // return flag that indicates whether GrayRaster or OverlayRaster should be used.
    static void UseGrayRaster(boolean flag) { _use_gray_raster = flag; }
    // set flag that indicates whether GrayRaster or OverlayRaster should be used.

    static ParamList* get_param_list();
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovraster_params;

    char* _pathname;
    int _import_flags;

    CopyStringList _commands;    
    CopyString _com_exp;

    static boolean _use_gray_raster;
    static boolean _warned;

friend class RasterScript;

    CLASS_SYMID("RasterComp");
};

//: graphical view of RasterOvComp.
class RasterOvView : public OverlayView {
public:
    RasterOvView(RasterOvComp* = nil);

    virtual void Update();

    RasterOvComp* GetRasterOvComp();
    virtual Graphic* GetGraphic();
    OverlayRasterRect* GetOverlayRasterRect() 
      {return (OverlayRasterRect*)GetGraphic();}
    // return pointer to graphic.
    OverlayRaster* GetOverlayRaster ();
    // return pointer to raster inside graphic.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

};

//: "PostScript" view of RasterOvComp.
class RasterPS : public OverlayPS {
public:
    RasterPS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

//: serialized view of RasterOvComp.
class RasterScript : public OverlayScript {
public:
    RasterScript(RasterOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.
    static int ReadRaster(istream&, void*, void*, void*, void*);
    // read raster pathname if it exists, and construct OverlayRasterRect 
    // a OverlayRaster or GrayRaster inside.
    static int ReadRGB(istream&, void*, void*, void*, void*);
    // read RGB pixel values directly from serialized file.
    static int ReadGrayChar(istream&, void*, void*, void*, void*);
    // read 8-bit pixel values directly from serialized file.
    static int ReadGrayUChar(istream&, void*, void*, void*, void*);
    // read unsigned 8-bit pixel values directly from serialized file.
    static int ReadGrayShort(istream&, void*, void*, void*, void*);
    // read 16-bit pixel values directly from serialized file.
    static int ReadGrayUShort(istream&, void*, void*, void*, void*);
    // read unsigned 16-bit pixel values directly from serialized file.
    static int ReadGrayInt(istream&, void*, void*, void*, void*);
    // read integer pixel values directly from serialized file.
    static int ReadGrayUInt(istream&, void*, void*, void*, void*);
    // read unsigned integer pixel values directly from serialized file.
    static int ReadGrayLong(istream&, void*, void*, void*, void*);
    // read long pixel values directly from serialized file.
    static int ReadGrayULong(istream&, void*, void*, void*, void*);
    // read unsigned long pixel values directly from serialized file.
    static int ReadGrayFloat(istream&, void*, void*, void*, void*);
    // read floating point pixel values directly from serialized file.
    static int ReadGrayDouble(istream&, void*, void*, void*, void*);
    // read double floating point pixel values directly from serialized file.
    static int ReadSub(istream&, void*, void*, void*, void*);
    // read sub-image specification.
    static int ReadProcess(istream&, void*, void*, void*, void*);
    // read image-processing command string.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean GetByPathnameFlag();
    boolean GetFromCommandFlag();

};

class MultiLineObj;
class OverlayRaster;

//: derived RasterRect Graphic for use with OverlayRaster and GrayRaster.
class OverlayRasterRect : public RasterRect {
public:
    OverlayRasterRect(OverlayRaster* = nil, Graphic* = nil);
    virtual ~OverlayRasterRect();

    void clippts(MultiLineObj*); 
    // set polygon used for additional clipping 
    void clippts(int* x, int* y, int n);
    // set polygon used for additional clipping 
    MultiLineObj* clippts(); 
    // return polygon used for additional clipping 

    virtual Graphic* Copy();

    OverlayRaster* GetOriginal() { return (OverlayRaster*)_raster; }
    // return raster associated with this graphic.
    virtual void SetRaster(OverlayRaster*);
    // set raster associated with this graphic.

    OverlayRaster* GetOverlayRaster();
    // return raster associated with this graphic.
    const char* path() const;
    // return pathname associated with the raster.
    void load_image(const char* pathname = nil);
    // method for deferred loading of raster by pathname.
    void draw(Canvas *c, Graphic* gs);
    // drawing method.

    void xbeg(IntCoord);
    // set xbeg of subimage specification (disabled == -1).
    void xend(IntCoord);
    // set xend of subimage specification (disabled == -1).
    void ybeg(IntCoord);
    // set ybeg of subimage specification (disabled == -1).
    void yend(IntCoord);
    // set yend of subimage specification (disabled == -1).
    IntCoord xbeg() const;
    // xbeg of subimage specification (disabled == -1).
    IntCoord xend() const;
    // xend of subimage specification (disabled == -1).
    IntCoord ybeg() const;
    // ybeg of subimage specification (disabled == -1).
    IntCoord yend() const;
    // yend of subimage specification (disabled == -1).

    virtual OverlayRasterRect& operator = (OverlayRasterRect&);
    // assignment operator.

    boolean damage_done() { return _damage_done; }
    // indicates if a damage rectangle has been set for this raster.
    
    void damage_done(boolean flag) { _damage_done = flag; }
    // set flag that indicates damage rectangle specified for this
    // raster.
    
    void damage_flush();
    // if a damage rectangle is set this does a partial flush
    // by calling Raster::flushrect.  This clears the flag
    // returned by ::damage_done.

    void damage_rect(IntCoord l, IntCoord b, IntCoord r, IntCoord t);
    // set rectangle used by ::damage_flush for calling Raster::flushrect.

protected:
    IntCoord _xbeg;
    IntCoord _xend;
    IntCoord _ybeg;
    IntCoord _yend;

    boolean _damage_done;
    IntCoord _damage_l;
    IntCoord _damage_b;
    IntCoord _damage_r;
    IntCoord _damage_t;
    
    MultiLineObj* _clippts;

friend class RasterOvComp;
friend class RasterOvView;
};

#include <Attribute/attrvalue.h>

//: specialized Raster object for use with RasterOvComp.
class OverlayRaster : public Raster {
public:
    OverlayRaster(unsigned long width, unsigned long height);
    // construct an empty raster ready to accept width*height of pixel values.
    OverlayRaster(
      unsigned long width, unsigned long height, unsigned long bwidth
    );
    // initialize with a border of width bwidth using the default fg/bg colors
    OverlayRaster(const OverlayRaster& raster);
    // copy constructor
    OverlayRaster(const Raster& raster);
    // conversion constructor

    virtual ~OverlayRaster();

    virtual boolean initialized();
    // get initialized flag.
    virtual void initialized(boolean);
    // set initialized flag.
    virtual void initialize();
    // creates pixmap_ with correct data

    virtual void poke(
	unsigned long x, unsigned long y,
	ColorIntensity red, ColorIntensity green, ColorIntensity blue,
	float alpha
    );
    // lookup Color that best matches a given 'red', 'green', 'blue', and
    // 'alpha' value, and poke corresponding entry in the colormap into
    // the raster.

    virtual void graypeek(unsigned long x, unsigned long y, unsigned int&);
    // get green pixel value at 'x','y' and convert to an unsigned int.
    virtual void graypeek(unsigned long x, unsigned long y, unsigned long&);
    // get green pixel value at 'x','y' and convert to an unsigned long.
    virtual void graypeek(unsigned long x, unsigned long y, float&);
    // get green pixel value at 'x','y' and convert to a float.
    virtual void graypeek(unsigned long x, unsigned long y, double&);
    // get green pixel value at 'x','y' and convert to a double.
    virtual void graypeek(unsigned long x, unsigned long y, AttributeValue&);
    // get green pixel value at 'x','y' and place in an AttributeValue.

    virtual void graypoke(unsigned long x, unsigned long y, unsigned int);
    // set rgb pixel values at 'x','y' with an unsigned int.
    virtual void graypoke(unsigned long x, unsigned long y, unsigned long);
    // set rgb pixel values at 'x','y' with an unsigned long.
    virtual void graypoke(unsigned long x, unsigned long y, float);
    // set rgb pixel values at 'x','y' with a float.
    virtual void graypoke(unsigned long x, unsigned long y, double);
    // set rgb pixel values at 'x','y' with a double.
    virtual void graypoke(unsigned long x, unsigned long y, AttributeValue);
    // set rgb pixel values at 'x','y' with the contents of an AttributeValue.

    virtual void highlight(unsigned long x, unsigned long y) {}
    // saturate the red value at 'x','y' to highlight a pixel.  Implemented
    // only in GrayRaster.
    virtual void unhighlight() {}
    // clear the highlighted pixels.  Implemented only in GrayRaster.

    virtual void flush() const;
    // flush internal XImage data structure to a pixmap on the X server.

    virtual void flushrect(IntCoord l, IntCoord b, IntCoord t, IntCoord t) const;
    // flush rectangular region of internal XImage data structure 
    // to a pixmap on the X server.

    virtual int status() const;
    virtual OverlayRaster* copy() const;

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

    virtual boolean write(ostream& out);
    // write pixel values of raster to ostream as comma-separated 
    // parenthesized rgb triples in ASCII.
    virtual boolean read(istream& in);
    // read pixels values written out by write method.
    virtual void gray_flag(boolean flag) { _grayflag = flag; }
    // set flag that indicates this a graylevel image (r==g==b).
    virtual boolean gray_flag() { return _grayflag; }
    // return flag that indicates this a graylevel image.
    virtual boolean grayraster() { return false; }
    // return flag that indicates whether this is a GrayRaster
    // (always false for OverlayRaster).

    virtual AttributeValue::ValueType value_type() const 
      { return AttributeValue::UCharType; } 
    // arbitrary type is not enabled in this class, only GrayRaster.
    virtual boolean is_type(AttributeValue::ValueType type) 
      { return value_type() == type; }
    // arbitrary type is not enabled in this class, only GrayRaster.

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, RampAlignment = R_LT
    );
    // embed gray-level ramp in raster at given alignment, and return 'cmd'
    // string to reproduce this effect after save/restore.

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, IntCoord x, IntCoord y
    );
    // embed gray-level ramp in raster at given alignment, and return 'cmd'
    // string to reproduce this effect after save/restore.

    virtual void paintgrayramp(
        IntCoord left, IntCoord bottom, unsigned width, unsigned height,
	boolean horiz
    );
    // utility method for painting the grayramp.

// protected:   

    static int gray_init();
    // setup colormap for viewing graylevel imagery with good rubberband
    // visibility.
    static int gray_init(int nbits);
    // setup colormap for viewing graylevel imagery with good rubberband
    // visibility, using a specified number of gray-level bits.
    static boolean gray_initialized() {return _gray_initialized;}
    // returns true if graylevel colormap has been initialized.
    static int color_init(int nlevels);
    // setup colormap for 'nlevels' of R, G, B, i.e. if 'nlevels'==6, 
    // the number of entries set up in the colormap is 216 (6*6*6).

    static long gray_lookup(int byte);
    // lookup the original gray-level value associated with a colormap entry.

protected:
    void construct(const Raster&);
    void init_rep(unsigned long width, unsigned long height);
    virtual void init_space();

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    virtual void scale(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    virtual void logscale(
        ColorIntensity mingray, ColorIntensity maxgray
    );

    void _addgrayramp(
        RampAlignment algn, IntCoord w = 0, IntCoord h = 0
    );
    void computeramp(boolean vert, RampAlignment, IntCoord& w, IntCoord& h);
    RampAlignment ramppos(IntCoord x, IntCoord y);

protected:
    static XColor* _gray_map;
    static int _unique_grays;
    static boolean _gray_initialized;
    static XColor* _color_map;
    static int _unique_colors;
    boolean _grayflag;
    boolean _init;

friend class OvImportCmd;
friend class RasterOvComp;
};


#endif

