/*
 * Copyright 1998-1999 Vectaport Inc.
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

#ifndef geoloc_h
#define geoloc_h

#include <Unidraw/Tools/tool.h>

//: tool to display coordinates relative to a graphic.
// For a raster or stencil the origin is 0,0.  
// For all other graphics the origin is relative to the coordinates used to
// construct the graphic, i.e. the lower-left corner of the original screen
// canvas if created from mouse events.
class GrLocTool : public Tool {
public:
    GrLocTool(ControlInfo* = nil);

    virtual Manipulator* CreateManipulator(Viewer*, Event&, Transformer* =nil);
    // use event to select a component and determine graphic-relative coordinates
    // to display in a pop-up window.

    virtual Tool* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

};

#endif
