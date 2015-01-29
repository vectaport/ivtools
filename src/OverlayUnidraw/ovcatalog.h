/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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

class Editor;
class OverlayComp;
class Raster;

//: catalog for read/write of "PostScript" and ASCII script documents
// One of these is used by the editor to save and restore drawing editor
// documents in idraw "PostScript" format or in an ASCII variable-length
// record format (each record enclosed in parenthesis separated by commas,
// with nesting for hierarchical objects).  Reading in "PostScript" is done
// with the balance of "Read" methods below.  Reading in the ASCII script 
// files is done with the istream constructors of classes derived from OverlayComp.
// Only writing in script is supported by this class.  Writing in idraw "PostScript"
// is handled by IdrawCatalog.
class OverlayCatalog : public IdrawCatalog{
public:
    OverlayCatalog(const char* name, Creator* creator);
    // 'creator' used to construct objects as they are read in.

    virtual boolean Save(Component* comp, const char* path);
    // save 'comp' to 'path'.
    boolean Retrieve (const char* path, Component*& comp);
    // retrieve 'comp' from 'path'.

    virtual GraphicComp* ReadPostScript(istream&);
    // read body of an idraw format "PostScript" document.
    virtual void PSReadChildren(istream&, GraphicComp*);
    // read individual graphics (and composite-graphics) from an idraw
    // "PostScript" file.

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
    // set parent component to pass to OverlayIdrawComp istream constructor.

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);
    // set compaction flags used for writing ASCII script files.

    virtual OverlayComp* ReadComp(const char*, istream&, OverlayComp* =nil);
    // read an arbitrary graphic in ASCII script form.

    Editor* GetEditor() { return _ed;}
    void SetEditor(Editor* ed) { _ed = ed; }

    static OverlayCatalog* Instance();
    // return default instance.
    static void Instance(OverlayCatalog*);
    // set default instance.
protected:
    boolean _failed;
    OverlayComp* _parent;
    boolean _gs_compacted;
    boolean _pts_compacted;
    boolean _pic_compacted;
    Editor* _ed;

    static OverlayCatalog* _instance;
};

#endif
