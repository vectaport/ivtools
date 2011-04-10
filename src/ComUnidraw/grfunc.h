/*
 * Copyright (c) 1994-1997,1999 Vectaport Inc.
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

#if !defined(grfunc_h)
#define _grfunc_h

#include <ComUnidraw/unifunc.h>

//: rectangle drawing command for comdraw.
// compview=rect(x0,y0,x1,y1) -- create a rectangle <br>
// compview=rectangle(x0,y0,x1,y1) -- same as rect
class CreateRectFunc : public UnidrawFunc {
public:
    CreateRectFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0,x1,y1) -- create a rectangle"; }
};

//: line drawing command for comdraw.
// compview=line(x0,y0,x1,y1) -- create a line <br>
// compview=arrowline(x0,y0,x1,y1) -- same as line
class CreateLineFunc : public UnidrawFunc {
public:
    CreateLineFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0,x1,y1) -- create a line"; }
};

//: ellipse drawing command for comdraw.
// compview=ellipse(x0,y0,r1,r2) -- create an ellipse
class CreateEllipseFunc : public UnidrawFunc {
public:
    CreateEllipseFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0,r1,r2) -- create an ellipse"; }
};

//: text drawing command for comdraw.
// compview=text(x0,y0 textstr) -- create a text string
class CreateTextFunc : public UnidrawFunc {  // doesn\'t work
public:
    CreateTextFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0 textstr) -- create a text string"; }
};

//: multiline drawing command for comdraw.
// compview=multiline(x0,y0[,x1,y1,...]) -- create a multiline <br>
// compview=arrowmultiline(x0,y0[,x1,y1,...]) -- same as multiline
class CreateMultiLineFunc : public UnidrawFunc {
public:
    CreateMultiLineFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0[,x1,y1,...]) -- create a multiline"; }
};

//: open spline drawing command for comdraw.
// compview=openspline(x0,y0[,x1,y1,...]) -- create an open spline <br>
// compview=arrowspline(x0,y0[,x1,y1,...]) -- same as openspline
class CreateOpenSplineFunc : public UnidrawFunc {
public:
    CreateOpenSplineFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0[,x1,y1,...]) -- create an open spline"; }
};

//: polygon drawing command for comdraw.
// compview=polygon(x0,y0[,x1,y1,...]) -- create a polygon
class CreatePolygonFunc : public UnidrawFunc {
public:
    CreatePolygonFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0[,x1,y1,...]) -- create a polygon"; }
};

//: closed spline drawing command for comdraw.
// compview=closedspline(x0,y0[,x1,y1,...]) -- create a closed spline
class CreateClosedSplineFunc : public UnidrawFunc {
public:
    CreateClosedSplineFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "compview=%s(x0,y0[,x1,y1,...]) -- create a closed spline"; }
};

//: command for setting font state variable in comdraw.
// font(fontnum) -- set current font from menu order
class FontFunc : public UnidrawFunc {
public:
    FontFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(fontnum) -- set current font from menu order"; }
};

//: command for setting brush state variable in comdraw.
// brush(brushnum) -- set current brush from menu order
class BrushFunc : public UnidrawFunc {
public:
    BrushFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(brushnum) -- set current brush from menu order"; }
};

//: command for setting pattern state variable in comdraw.
// pattern(patternnum) -- set current pattern from menu order
class PatternFunc : public UnidrawFunc {
public:
    PatternFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(patternnum) -- set current pattern from menu order"; }
};

//: command for setting color state variables in comdraw.
// colors(fgcolornum bgcolornum) -- set current colors from menu order
class ColorFunc : public UnidrawFunc {
public:
    ColorFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(fgcolornum bgcolornum) -- set current colors from menu order"; }
};

//: command to select graphics in comdraw.
// select(compview [compview ...]) -- make these graphics the current selection
class SelectFunc : public UnidrawFunc {
public:
    SelectFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(compview [compview ...]) -- make these graphics the current selection"; }
};

//: command to move current selection in comdraw
// move(dx dy) -- move current selection
class MoveFunc : public UnidrawFunc {
public:
    MoveFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(dx dy) -- move current selection"; }
};

//: command to scale current selection in comdraw
// scale(xflt yflt) -- scale current selection
class ScaleFunc : public UnidrawFunc {
public:
    ScaleFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(xflt yflt) -- scale current selection"; }
};

//: command to rotate current selection in comdraw
// rotate(degflt) -- rotate current selection
class RotateFunc : public UnidrawFunc {
public:
    RotateFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(degflt) -- rotate current selection"; }
};

//: command to pan viewer in comdraw.
// pan(px py) -- pan viewer
class PanFunc : public UnidrawFunc {
public:
    PanFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(px py) -- pan viewer"; }
};

//: command to pan viewer upward a small fixed amount in comdraw.
// smallpanup() -- small pan up
class PanUpSmallFunc : public UnidrawFunc {
public:
    PanUpSmallFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- small pan up"; }
};

//: command to pan viewer downward a small fixed amount in comdraw.
// smallpandown() -- small pan down
class PanDownSmallFunc : public UnidrawFunc {
public:
    PanDownSmallFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- small pan down"; }
};

//: command to pan viewer to the left a small fixed amount in comdraw.
// smallpanleft() -- small pan left
class PanLeftSmallFunc : public UnidrawFunc {
public:
    PanLeftSmallFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- small pan left"; }
};

//: command to pan viewer to the right a small fixed amount in comdraw.
// smallpanright() -- small pan right
class PanRightSmallFunc : public UnidrawFunc {
public:
    PanRightSmallFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() - small pan right"; }
};

//: command to pan viewer upward a large fixed amount in comdraw.
// largepanup() -- large pan up
class PanUpLargeFunc : public UnidrawFunc {
public:
    PanUpLargeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- large pan up"; }
};

//: command to pan viewer downward a large fixed amount in comdraw.
// largepandown() -- large pan down
class PanDownLargeFunc : public UnidrawFunc {
public:
    PanDownLargeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- large pan down"; }
};

//: command to pan viewer to the left a large fixed amount in comdraw.
// largepanleft() -- large pan left
class PanLeftLargeFunc : public UnidrawFunc {
public:
    PanLeftLargeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- large pan left"; }
};

//: command to pan viewer to the right a large fixed amount in comdraw.
// largepanright() -- large pan right
class PanRightLargeFunc : public UnidrawFunc {
public:
    PanRightLargeFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- large pan right"; }
};

//: command to zoom viewer in comdraw.
// zoom(zoomflt) -- zoom by factor
class ZoomFunc : public UnidrawFunc {
public:
    ZoomFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(zoomflt) -- zoom by factor"; }
};

//: command to zoom-in by 2 in comdraw.
// zoomin() -- zoom-in by 2.
class ZoomInFunc : public UnidrawFunc {
public:
    ZoomInFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- zoom-in by 2"; }
};

//: command to zoom-out by 2 in comdraw.
// zoomout() -- zoom-out by 2.
class ZoomOutFunc : public UnidrawFunc {
public:
    ZoomOutFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s() -- zoom-out by 2"; }
};

//: command to generate an internally tiled PGM/PPM file for drawtool or comdraw.
// tilefile(inpath outpath [xsize] [ysiz]) -- tile pgm or ppm image file
class TileFileFunc : public UnidrawFunc {
public:
    TileFileFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(inpath outpath [xsize] [ysiz]) -- tile pgm or ppm image file"; }
};

#endif /* !defined(_grfunc_h) */
