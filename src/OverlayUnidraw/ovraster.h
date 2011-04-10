/*
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
class istream;

enum RampAlignment { R_LB, R_LT, R_TL, R_TR, R_RT, R_RB, R_BR, R_BL };

declareList(CopyStringList,CopyString)

class RasterOvComp : public OverlayComp {
public:
    RasterOvComp(OverlayRasterRect* = nil, const char* pathname = nil,
		 OverlayComp* parent = nil);
    RasterOvComp(istream&, OverlayComp* parent = nil);

    virtual ~RasterOvComp();

    OverlayRasterRect* GetOverlayRasterRect();
    virtual void SetPathName(const char*);
    virtual const char* GetPathName();
    virtual boolean GetByPathnameFlag() { return _by_pathname;}
    virtual void SetByPathnameFlag(boolean flag) { _by_pathname = flag;}

    virtual Component* Copy();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual void Configure(Editor*);

    static boolean UseGrayRaster() { return _use_gray_raster; }
    static void UseGrayRaster(boolean flag) { _use_gray_raster = flag; }

    static ParamList* get_param_list();
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovraster_params;

    char* _pathname;
    boolean _by_pathname;

    CopyStringList _commands;    
    CopyString _com_exp;

    static boolean _use_gray_raster;
    static boolean _warned;

friend RasterScript;
};

class RasterOvView : public OverlayView {
public:
    RasterOvView(RasterOvComp* = nil);

    virtual void Update();

    RasterOvComp* GetRasterOvComp();
    virtual Graphic* GetGraphic();
    OverlayRasterRect* GetOverlayRasterRect() 
      {return (OverlayRasterRect*)GetGraphic();}
    OverlayRaster* GetOverlayRaster ();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class RasterPS : public OverlayPS {
public:
    RasterPS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class RasterScript : public OverlayScript {
public:
    RasterScript(RasterOvComp* = nil);

    virtual boolean Definition(ostream&);
    static int ReadRaster(istream&, void*, void*, void*, void*);
    static int ReadRGB(istream&, void*, void*, void*, void*);
    static int ReadGrayChar(istream&, void*, void*, void*, void*);
    static int ReadGrayUChar(istream&, void*, void*, void*, void*);
    static int ReadGrayShort(istream&, void*, void*, void*, void*);
    static int ReadGrayUShort(istream&, void*, void*, void*, void*);
    static int ReadGrayInt(istream&, void*, void*, void*, void*);
    static int ReadGrayUInt(istream&, void*, void*, void*, void*);
    static int ReadGrayLong(istream&, void*, void*, void*, void*);
    static int ReadGrayULong(istream&, void*, void*, void*, void*);
    static int ReadGrayFloat(istream&, void*, void*, void*, void*);
    static int ReadGrayDouble(istream&, void*, void*, void*, void*);
    static int ReadSub(istream&, void*, void*, void*, void*);
    static int ReadProcess(istream&, void*, void*, void*, void*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    boolean GetByPathnameFlag();

};

class OverlayRaster;

class OverlayRasterRect : public RasterRect {
public:
    OverlayRasterRect(OverlayRaster* = nil, Graphic* = nil);
    virtual ~OverlayRasterRect();

    virtual Graphic* Copy();

    OverlayRaster* GetOriginal() { return (OverlayRaster*)_raster; }
    virtual void SetRaster(OverlayRaster*);

    OverlayRaster* GetOverlayRaster();
    const char* path() const;
    void load_image(const char* pathname = nil);
    void draw(Canvas *c, Graphic* gs);

    void xbeg(IntCoord);
    void xend(IntCoord);
    void ybeg(IntCoord);
    void yend(IntCoord);
    IntCoord xbeg() const;
    IntCoord xend() const;
    IntCoord ybeg() const;
    IntCoord yend() const;

    virtual OverlayRasterRect& operator = (OverlayRasterRect&);

protected:
    IntCoord _xbeg;
    IntCoord _xend;
    IntCoord _ybeg;
    IntCoord _yend;

friend RasterOvComp;
};

#include <Attribute/attrvalue.h>

class OverlayRaster : public Raster {
public:
    OverlayRaster(unsigned long width, unsigned long height);
    OverlayRaster(const OverlayRaster& raster);
    OverlayRaster(const Raster& raster);
    virtual ~OverlayRaster();

    virtual boolean initialized();
   // creates pixmap_ with correct data
    virtual void initialize();

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

    virtual void highlight(unsigned long x, unsigned long y) {}
    virtual void unhighlight() {}

    virtual void flush() const;
    virtual int status() const;
    virtual OverlayRaster* copy() const;

    virtual OverlayRaster* scale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    virtual OverlayRaster* pseudocolor(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    virtual OverlayRaster* logscale(
        ColorIntensity mingray, ColorIntensity maxgray, CopyString& cmd
    );

    virtual boolean write(ostream& out, boolean gray=false);
    virtual boolean read(istream& in, boolean gray=false);
    virtual void gray_flag(boolean flag) { _grayflag = flag; }
    virtual boolean gray_flag() { return _grayflag; }
    virtual boolean grayraster() { return false; }

    // arbitrary type is not enabled in this class
    virtual AttributeValue::ValueType value_type() const 
      { return AttributeValue::UnknownType; } 
    virtual boolean is_type(AttributeValue::ValueType type) 
      { return value_type() == type; }

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, RampAlignment = R_LT
    );

    virtual OverlayRaster* addgrayramp(
        CopyString& cmd, IntCoord x, IntCoord y
    );

    virtual void paintgrayramp(
        IntCoord left, IntCoord bottom, unsigned width, unsigned height,
	boolean horiz
    );

// protected:   

    static int gray_init();
    static int gray_init(int nbits);
    static boolean gray_initialized() {return _gray_initialized;}
    static int color_init(int nlevels);

    static long gray_lookup(int byte);

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

friend OvImportCmd;
friend RasterOvComp;
};


#endif

