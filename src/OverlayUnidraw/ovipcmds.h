/*
 * Copyright (c) 1998 Vectaport Inc.
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

class RasterTerp : public ComTerpServ {
public:
    RasterTerp(Editor*);
    ~RasterTerp();

    int execute(RasterOvComp* comp, const CopyString& exp);
    Editor* editor();
protected:
    Editor* _editor;
};


class RasterFunc : public ComFunc {
public:
    RasterFunc(ComTerp*);
    ~RasterFunc();

    static void SetComp(RasterOvComp*);
protected:
    static Clipboard _comps;
};


class ScaleGrayFunc : public RasterFunc {
public:
    ScaleGrayFunc(RasterTerp*);
    virtual void execute();

    static const char* Tag();
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );

protected:
    RasterTerp* _rterp;
};

class PseudocolorFunc : public RasterFunc {
public:
    PseudocolorFunc(RasterTerp*);
    virtual void execute();

    static const char* Tag();
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );
protected:
    RasterTerp* _rterp;
};


class LogScaleFunc : public RasterFunc {
public:
    LogScaleFunc(RasterTerp*);
    virtual void execute();

    static const char* Tag();
    static const char* CommandString(
        ColorIntensity mingray, ColorIntensity maxgray
    );
protected:
    RasterTerp* _rterp;
};


class GrayRampFunc : public RasterFunc {
public:
    GrayRampFunc(RasterTerp*);
    virtual void execute();

    static const char* Tag();
    static const char* CommandString(RampAlignment);

protected:
    static const char* rpos[];
    RasterTerp* _rterp;
};

// ---------------------------------------------------------------------------

class ProcessingCmd : public MacroCmd {
public:
    ProcessingCmd(ControlInfo*);
    ProcessingCmd(Editor*);

    ~ProcessingCmd();

    void Execute();
    
    virtual boolean PrepareToExecute(GraphicComp*);
    virtual OverlayRaster* Process(OverlayRaster*, CopyString& scmd);

    void GetResult(Clipboard&) const;

    virtual boolean Reversible();

protected:
    Clipboard* _comps;
    boolean _prepared;
    boolean _ed_constructor;
    boolean _reversible;
};


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


