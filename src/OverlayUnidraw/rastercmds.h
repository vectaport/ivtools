/*
 * Copyright (c) 1999 Vectaport Inc.
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

#ifndef rastercmds_h
#define rastercmds_h

#include <Unidraw/Commands/command.h>

class ControlInfo;
class RasterOvComp;
class OverlayRaster;

//: command to replace raster in a component.
class ReplaceRasterCmd : public Command {
public:
    ReplaceRasterCmd();
    ReplaceRasterCmd(ControlInfo*); 
    ReplaceRasterCmd(ControlInfo* c, RasterOvComp* comp, OverlayRaster* nras);
    ReplaceRasterCmd(Editor* ed, RasterOvComp* comp, OverlayRaster* nras);

    virtual ~ReplaceRasterCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual boolean Reversible();
protected:
    OverlayRaster* _orig;

    RasterOvComp* _comp;
    OverlayRaster* _nras;
};

//: command to unhighlight a raster.
class UnhighlightRasterCmd : public Command {
public:
    UnhighlightRasterCmd();
    UnhighlightRasterCmd(ControlInfo*); 

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual boolean Reversible();
};



#endif

