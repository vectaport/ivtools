/*
 * Copyright (c) 1999 Vectaport Inc.
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
 */

#ifndef ov_manips_h
#define ov_manips_h

#include <Unidraw/manips.h>
#include <InterViews/transformer.h>

class Graphic;
class Transformer;

//: manipulator for opaque move.
class OpaqueDragManip : public DragManip {
public:
    OpaqueDragManip(
        Viewer*, Rubberband*, Transformer* = nil, Tool* = nil, 
        DragConstraint = None, Graphic* = nil
    );
    OpaqueDragManip(
        Viewer*, Rubberband*, Transformer*, Tool*, DragConstraint, 
	IntCoord, IntCoord, Graphic* = nil
    );
    virtual ~OpaqueDragManip();
    void Init(Graphic*, Rubberband*);
    // constructor initialization method.
    
    virtual void Grasp(Event&);
    // down-event method.
    virtual boolean Manipulating(Event&);
    // until up-event method.
    void Track(IntCoord x, IntCoord y);
    // track method implemented here instead of in a rubberband.

protected:
    boolean opaqueable_rubband(Rubberband*); 
    // test to see if this kind of rubberband can be directly visualized instead.
    float current_angle(Rubberband*);
    // utility method for computing current angle of a rotating rubberband.
    float current_scaling(Rubberband*);
    // utility method for computing current factor of a scaling rubberband.

    Graphic* _graphic;
    // graphic for opaque move.
    Rubberband* _r2;
    // save pointer to _r member variable, which gets set to nil.
    boolean _notrans; 
    // flag to say whether graphic had Transformer to start with
    Transformer* _totaltrans;
    // total transformation for parent graphic.
    Transformer* _origtrans;
    // original transformation of graphic.
};    

#endif
