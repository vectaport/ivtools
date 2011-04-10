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
 * 
 */

#if !defined(grstatfunc_h)
#define _grstatfunc_h

#include <ComUnidraw/unifunc.h>

//: command to return center of graphics in comdraw.
// xylist=center(compview :xy :yx :x :y :scrn) -- center of compview (dflt :xy)
class CenterFunc : public UnidrawFunc {
public:
    CenterFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "xylist=%s(compview :xy :yx :x :y :scrn) -- center of compview (dflt :xy)"; }
};

//: command to return minimum-bounding rectangle of graphics in comdraw.
// rectlist=mbr(compview :lbrt :lrbt :scrn) -- minimum-bounding rectangle of compview (dflt :lbrt)
class MbrFunc : public UnidrawFunc {
public:
    MbrFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "rectlist=%s(compview :lbrt :lrbt :scrn) -- minimum-bounding rectangle of compview (dflt :lbrt :lrbt)"; }
};

//: command to return point list associated with a graphic
// ptlist=points(compview) -- return point list from compview graphic
class PointsFunc : public UnidrawFunc {
public:
    PointsFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
      return "ptlist=%s(compview) -- return point list from compview graphic"; }
};

#endif /* !defined(_grstatfunc_h) */
