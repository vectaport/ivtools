/*
 * Copyright (c) 1998,1999 Vectaport Inc.
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

#ifndef ovhull_h
#define ovhull_h

#include <Unidraw/Commands/command.h>
#include <Unidraw/Tools/tool.h>

class Clipboard;

//: command to generate generate and paste a convex hull given a polygon.
// relies on the qhull utilility from the University of Minnesota Geometry Center
// (http://geom.umn.edu).
class ConvexHullCmd : public Command {
public:
  ConvexHullCmd(Editor*, Clipboard*);
  virtual void Execute();
  static int ConvexHull(int np, float* fx, float* fy, float*& hx, float*& hy);
  // static method useful for computing a convex hull elsewhere.
};

//: tool to draw a polygon and invoke the ConvexHullCmd.
class ConvexHullTool : public Tool {
public:
  ConvexHullTool(ControlInfo* =nil);
  virtual Manipulator* CreateManipulator(Viewer*, Event&, Transformer* =nil);
  // create a GrowingPolygon manipulator.
  virtual Command* InterpretManipulator(Manipulator*);
  // interpret GrowingPolygon manipulator to generate and paste a convex hull.
};

#endif
