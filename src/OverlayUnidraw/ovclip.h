/*
 * Copyright (c) 1996-1999 Vectaport Inc.
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

#ifndef ovclip_h
#define ovclip_h

#include <Unidraw/Commands/macro.h>
#include <Unidraw/Tools/tool.h>

class Selection;

//: command for clipping arbitrary graphics with a rectangle.
class ClipRectCmd : public MacroCmd {
public:
    ClipRectCmd(Editor* ed, Selection* sel,
		     Coord l, Coord b, Coord r, Coord t);

    virtual void Execute();
    // replace graphics under box with graphics clipped to box.
    virtual boolean Reversible() { return true; }
    // returns true.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
  Selection* _sel;
  Coord _l, _b, _r, _t;
};

//: tool for clipping arbitrary graphics with a rectangle.
class ClipRectTool : public Tool {
public:
  ClipRectTool(ControlInfo* =nil);

  virtual Manipulator* CreateManipulator(Viewer*, Event&, Transformer* =nil);
  // create rectangular rubber band.
  virtual Command* InterpretManipulator(Manipulator*);
  // use rectangle to clip graphics with ClipRectCmd.

  virtual Tool* Copy();
  virtual ClassId GetClassId();
  virtual boolean IsA(ClassId);
protected:
};

//: command for clipping arbitrary graphics with a polygon.
class ClipPolyCmd : public MacroCmd {
public:
    ClipPolyCmd(Editor* ed, Selection* sel,
		     float* x, float* y, int n);

    virtual void Execute();
    // replace graphics under polygon with graphics clipped by the polygon.
    virtual boolean Reversible() { return true; }
    // returns true.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
  Selection* _sel;
  float* _x;
  float* _y;
  int _n;
};

//: tool for clipping arbitrary graphics with a polygon.
class ClipPolyTool : public Tool {
public:
  ClipPolyTool(ControlInfo* =nil);

  virtual Manipulator* CreateManipulator(Viewer*, Event&, Transformer* =nil);
  // create polygonal rubber band.
  virtual Command* InterpretManipulator(Manipulator*);
  // use polygon to clip graphics with ClipPolyCmd.

  virtual Tool* Copy();
  virtual ClassId GetClassId();
  virtual boolean IsA(ClassId);
protected:
};

#ifdef CLIPPOLY

//: command to clip polygon A with polygon B.
class ClipPolyAMinusBCmd : public MacroCmd {
public:
    ClipPolyAMinusBCmd(ControlInfo*);
    ClipPolyAMinusBCmd(Editor* = nil);

    virtual void Execute();
    // clip first polygon (in selection) with second polygon,
    // leaving the remains of the first.
    virtual boolean Reversible() { return true; }
    // returns true.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
};

//: command to clip polygon B with polygon A.
class ClipPolyBMinusACmd : public MacroCmd {
public:
    ClipPolyBMinusACmd(ControlInfo*);
    ClipPolyBMinusACmd(Editor* = nil);

    virtual void Execute();
    // clip second polygon (in selection) with first polygon,
    // leaving the remains of the second.
    virtual boolean Reversible() { return true; }
    // returns true.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
};

//: command to intersect two polygons.
class ClipPolyAAndBCmd : public MacroCmd {
public:
    ClipPolyAAndBCmd(ControlInfo*);
    ClipPolyAAndBCmd(Editor* = nil);

    virtual void Execute();
    // leave the intersection of the first two polygons in the selection.
    virtual boolean Reversible() { return true; }
    // returns true.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
};

#endif

#endif
