/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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
 * OverlayCatalog - can read and write components in script or postscript
 */

#ifndef ovcatalog_h
#define ovcatalog_h

#include <UniIdraw/idcatalog.h>

class OverlayComp;
class Raster;

class OverlayCatalog : public IdrawCatalog{
public:
    OverlayCatalog(const char*, Creator*);

    virtual boolean Save(Component*, const char*);
    boolean Retrieve (const char*, Component*&);

    virtual GraphicComp* ReadPostScript(istream&);
    virtual void PSReadChildren(istream&, GraphicComp*);

    GraphicComp* ReadPict(istream&);
    GraphicComp* ReadBSpline(istream&);
    GraphicComp* ReadCircle(istream&);
    GraphicComp* ReadClosedBSpline(istream&);
    GraphicComp* ReadEllipse(istream&);
    GraphicComp* ReadLine(istream&);
    GraphicComp* ReadMultiLine(istream&);
    GraphicComp* ReadPolygon(istream&);
    GraphicComp* ReadRect(istream&);
    GraphicComp* ReadText(istream&);
    GraphicComp* ReadSStencil(istream&);
    GraphicComp* ReadFStencil(istream&);
    GraphicComp* ReadRaster(istream&);

    virtual void SetParent(OverlayComp*);
    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);

    virtual OverlayComp* ReadComp(const char*, istream&, OverlayComp* =nil);

    static OverlayCatalog* Instance();
    static void Instance(OverlayCatalog*);
protected:
    boolean _failed;
    OverlayComp* _parent;
    boolean _gs_compacted;
    boolean _pts_compacted;
    boolean _pic_compacted;

    static OverlayCatalog* _instance;
};

#endif
