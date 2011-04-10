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

#ifndef frameviews_h
#define frameviews_h

#include <OverlayUnidraw/ovviews.h>

class FrameComp;
class FramesComp;
class FrameIdrawComp;

class FrameOverlaysView : public OverlaysView {
public:
    FrameOverlaysView();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class FrameView : public OverlaysView {
public:
    FrameView(FrameComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:

};

class FramesView : public FrameView {
public:
    FramesView(FramesComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    void UpdateFrame(FrameView* curr, FrameView* prev,
		     int curr_other = 0, int prev_other=0);
};

class FrameIdrawView : public FramesView {
public:

    FrameIdrawView(FrameIdrawComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual GraphicView* GetGraphicView(Component*);
    virtual Selection* SelectAll();
    virtual Selection* ViewContaining(Coord, Coord);
    virtual Selection* ViewsContaining(Coord, Coord);


    virtual Selection* ViewIntersecting(Coord, Coord, Coord, Coord);
    virtual Selection* ViewsIntersecting(Coord, Coord, Coord, Coord);
    virtual Selection* ViewsWithin(Coord, Coord, Coord, Coord);
    virtual ConnectorView* ConnectorIntersecting(Coord, Coord, Coord, Coord);

protected:
};

#endif
