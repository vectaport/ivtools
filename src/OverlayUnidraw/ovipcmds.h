/*
 * Copyright (c) 1998,1999 Vectaport Inc.
 * Copyright (c) 1997 Vectaport Inc. and R.B. Kissh & Associates
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

#ifndef ipcmds_h
#define ipcmds_h

#include <ComTerp/comfunc.h>
#include <ComTerp/comterpserv.h>
#include <Unidraw/Commands/macro.h>

#include <OverlayUnidraw/ovraster.h>

typedef float ColorIntensity;

class RasterOvComp;
class OverlayRaster;
class Raster;
class OverlayRasterRect;

//: ComTerp specialized for image processing operations.
// command interpreter that takes a pointer to an editor, and
// uses it execute interpreted commands on a raster component.
class RasterTerp : public ComTerpServ {
public:
    RasterTerp(Editor*);
    ~RasterTerp();

    int execute(RasterOvComp* comp, const CopyString& exp);
    // parse and interpret 'exp' on 'comp'.
    Editor* editor();
protected:
    Editor* _editor;
};

//: ComFunc base class to hold a raster component.
class RasterFunc : public ComFunc {
public:
    RasterFunc(ComTerp*);
    ~RasterFunc();

    static void SetComp(RasterOvComp*);
    // set raster component for use of derived classes.
protected:
    static Clipboard _comps;
};


//: ComFunc to linearly scale a gray-level raster.
class ScaleGrayFunc : public RasterFunc {
public:
    ScaleGrayFunc(RasterTerp*);
    virtual void execute();
    // execute a ScaleGrayCmd on a gray-level raster.

    static const char* Tag();
    // command name.
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // generates command string.

protected:
    RasterTerp* _rterp;
};

//: ComFunc to pseudo-color a gray-level raster.
class PseudocolorFunc : public RasterFunc {
public:
    PseudocolorFunc(RasterTerp*);
    virtual void execute();
    // execute a PseudocolorCmd on a gray-level raster.

    static const char* Tag();
    // command name.
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // generates command string.
protected:
    RasterTerp* _rterp;
};

//: ComFunc to log-scale a gray-level raster.
class LogScaleFunc : public RasterFunc {
public:
    LogScaleFunc(RasterTerp*);
    virtual void execute();
    // execute a LogScaleCmd on a gray-level raster.

    static const char* Tag();
    // command name.
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );
    // generates command string.
protected:
    RasterTerp* _rterp;
};

//: ComFunc to embed a gray-level ramp in a gray-level raster.
class GrayRampFunc : public RasterFunc {
public:
    GrayRampFunc(RasterTerp*);
    virtual void execute();
    // execute a GrayRampCmd on a gray-level raster.

    static const char* Tag();
    // command name.
    static const char* CommandString(RampAlignment);
    // generates command string.
protected:
    static const char* rpos[];
    // relative position of the gray-level ramp -- ramp alignment.
    RasterTerp* _rterp;
};

// ---------------------------------------------------------------------------

//: base class for image processing commands.
// The classes derived from ProcessingCmd (ScaleGrayCmd, PseudocolorCmd, 
// LogScaleCmd, GrayRampCmd) are image processing operators useful for controlling 
// the appearance of an image in a drawing editor or similar display.  They are 
// not necessarily intended for use in computational imaging algorithms.
class ProcessingCmd : public MacroCmd {
public:
    ProcessingCmd(ControlInfo*);
    ProcessingCmd(Editor*);

    ~ProcessingCmd();

    void Execute();
    // take selected raster, or foremost raster if none is selected,
    // wrap up the image processing command with necessary cut/paste commands,
    // and invoke the whole thing.
    
    virtual boolean PrepareToExecute(GraphicComp*);
    // wrapping the image processing command with necessary cut/paste commands.
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);
    // do the actual processing in derived classes.

    void GetResult(Clipboard&) const;
    // load clipboard with results.

    virtual boolean Reversible();
    // conditional.

protected:
    Clipboard* _comps;
    boolean _prepared;
    boolean _ed_constructor;
    boolean _reversible;
};

//: utility command for storing image processing command string.
class ImageCmd : public Command {
public:
    ImageCmd();
    ImageCmd(ControlInfo*); 
    ImageCmd(ControlInfo* c, const CopyString& str);
    ImageCmd(Editor* ed, const CopyString& str);

    virtual ~ImageCmd();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    const CopyString& Cmd() const;

protected:
    CopyString _cstr;
};

//: command for linear-scaling of gray-level raster.
class ScaleGrayCmd : public ProcessingCmd {
public:

    ScaleGrayCmd(
        ControlInfo*, ColorIntensity mingray, ColorIntensity maxgray
    ); 
    ScaleGrayCmd(ControlInfo*);
    ScaleGrayCmd(
        Editor* ed, ColorIntensity mingray, ColorIntensity maxgray
    );
    ScaleGrayCmd(Editor* ed);

    virtual ~ScaleGrayCmd();
    
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ColorIntensity _mingray, _maxgray;
};

//: command for pseudo-coloring of gray-level raster.
class PseudocolorCmd : public ProcessingCmd {
public:

    PseudocolorCmd(
        ControlInfo*, ColorIntensity mingray, ColorIntensity maxgray
    ); 
    PseudocolorCmd(ControlInfo*);
    PseudocolorCmd(
        Editor* ed, ColorIntensity mingray, ColorIntensity maxgray
    );
    PseudocolorCmd(Editor* ed);
    virtual ~PseudocolorCmd();
    
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    ColorIntensity _mingray, _maxgray;
};

//: command for logarithmic-scaling of gray-level raster.
class LogScaleCmd : public ProcessingCmd {
public:

    LogScaleCmd(ControlInfo*, ColorIntensity mingray, ColorIntensity maxgray); 
    LogScaleCmd(ControlInfo*);
    LogScaleCmd(Editor* ed, ColorIntensity mingray, ColorIntensity maxgray); 
    LogScaleCmd(Editor* ed);
    virtual ~LogScaleCmd();
    
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
  ColorIntensity _mingray;
  ColorIntensity _maxgray;
};

//: command for embedding gray-level ramp in a gray-level raster.
class GrayRampCmd : public ProcessingCmd {
public:
    GrayRampCmd(ControlInfo*, IntCoord x, IntCoord y); 
    GrayRampCmd(ControlInfo*, RampAlignment); 
    GrayRampCmd(Editor* ed, IntCoord x, IntCoord y);
    GrayRampCmd(Editor* ed, RampAlignment);
    virtual ~GrayRampCmd();
    
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    IntCoord _x, _y;
    RampAlignment _align;
    boolean _use_align;
};

#endif


