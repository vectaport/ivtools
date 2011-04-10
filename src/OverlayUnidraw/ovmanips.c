/*
 * Copyright (c) 1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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

#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovmanips.h>
#include <Unidraw/Graphic/damage.h>
#include <Unidraw/Graphic/graphic.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/viewer.h>
#include <InterViews/transformer.h>
#include <IV-2_6/InterViews/rubcurve.h>
#include <IV-2_6/InterViews/rubline.h>
#include <IV-2_6/InterViews/rubrect.h>
#include <iostream.h>

/****************************************************************************/

OpaqueDragManip::OpaqueDragManip (
    Viewer* v, Rubberband* rub, Transformer* rel, Tool* t, DragConstraint c,
    Graphic* graphic
) : DragManip(v, nil, rel, t, c) {
  Init(graphic, rub);
}

OpaqueDragManip::OpaqueDragManip (
    Viewer* v, Rubberband* rub, Transformer* rel, Tool* t, DragConstraint c,
    Coord x, Coord y, Graphic* graphic
) : DragManip(v, nil, rel, t, c, x, y) {
  Init(graphic, rub);
}

void OpaqueDragManip::Init(Graphic* graphic, Rubberband* rub) {
  _notrans = false;
  _graphic = graphic;
  if (_graphic && OverlayEditor::opaque_flag() && opaqueable_rubband(rub) ) {
    _r = nil;
    _r2 = rub;
    Ref(_r2);
    if (!_graphic->GetTransformer()) {
      _notrans = true;
      _graphic->SetTransformer(new Transformer());
      // cerr << "OpaqueDragManip::Init -- nil transformer set to identity\n";
      // cerr << "_graphic pointer " << _graphic << "\n";
      _origtrans = new Transformer();
    } else
      _origtrans = new Transformer(*_graphic->GetTransformer());
    _totaltrans = new Transformer();
    _graphic->Parent()->TotalTransformation(*_totaltrans);
  } else {
    _graphic = nil;
    _r = rub;
    Ref(_r);
    if (_r != nil) _viewer->InitRubberband(_r);
    _totaltrans = _origtrans = nil;
  }
}

OpaqueDragManip::~OpaqueDragManip() {
  delete _totaltrans;
  delete _origtrans;
}

void OpaqueDragManip::Grasp (Event& e) {
  if (!_graphic) {
    DragManip::Grasp(e);
    return; 
  }

  _grasp_e = e;
  
  Constrain(e);
  if (!_origPreset) {
    _origx = e.x;
    _origy = e.y;
    ClassId id = _r2->GetClassId();
    if (id == SCALINGLINE || id == SCALINGLINELIST || id == SCALINGRECT) {
      _viewer->GetDamage()->Incur(_graphic);
      Track(e.x, e.y);
      _viewer->GetDamage()->Incur(_graphic);
      _viewer->Update();
    } else
      _r2->SetTrack(e.x, e.y);
  }
} 

boolean OpaqueDragManip::Manipulating (Event& e) {
  if (!_graphic) 
    return DragManip::Manipulating(e);

  if (e.eventType == MotionEvent) {
    Constrain(e);
    IntCoord trackx, tracky;
    _r2->GetTrack(trackx, tracky);
    if (e.x!=trackx || e.y!=tracky) {
      _viewer->GetDamage()->Incur(_graphic);
      Track(e.x, e.y);
      _viewer->GetDamage()->Incur(_graphic);
      _viewer->Update();
    }
    
  } else if (e.eventType == UpEvent) {
    /* restore original position and restore default rubberband */
    _viewer->GetDamage()->Incur(_graphic);
    if (_notrans)
      _graphic->SetTransformer(nil);
    else
      *_graphic->GetTransformer() = *_origtrans;
    _graphic->uncacheParents();
    _r = _r2;
    return false;
  }
  return true;
}
  
void OpaqueDragManip::Track (IntCoord x, IntCoord y) {
  IntCoord oldx, oldy;
  Rubberband* rub = _r2;
  ClassId id = rub->GetClassId();

  // cerr << "OpaqueDragManip::Track -- _graphic and GetTransformer() " <<
  //  _graphic << "," << _graphic->GetTransformer() << "\n";
  if (id == SLIDINGLINE || id == SLIDINGLINELIST || id == SLIDINGRECT) {
    *_graphic->GetTransformer() = *_origtrans;
    rub->SetTrack(x, y);
    float f_origx = _origx, f_origy = _origy;
    _totaltrans->InvTransform(f_origx, f_origy, f_origx, f_origy);
    float f_x = x, f_y = y;
    _totaltrans->InvTransform(f_x, f_y, f_x, f_y);
    _graphic->Translate(f_x-f_origx, f_y-f_origy);

  } else if (id == SCALINGLINE || id == SCALINGLINELIST || id == SCALINGRECT) {
    *_graphic->GetTransformer() = *_origtrans;
    rub->SetTrack(x, y);
    float cx, cy;
    _graphic->GetCenter(cx, cy);
    float curscale = current_scaling(rub);
    _graphic->Scale(curscale, curscale, cx, cy);

  } else if (id == RUBBERRECT) {
    *_graphic->GetTransformer() = *_origtrans;
    rub->SetTrack(x, y);

    Coord l0, b0, r0, t0, l1, b1, r1, t1;
    float sx, sy;
    Alignment a;
    Viewer* v = _viewer;
    RubberRect* rr = (RubberRect*) rub;

    rr->GetOriginal(l0, b0, r0, t0);

    if (v->GetOrientation() == Landscape) {
        if (l0 > r0) {
            a = (b0 > t0) ? TopLeft : TopRight;
        } else {
            a = (b0 > t0) ? BottomLeft : BottomRight;
        }
    } else {
        if (l0 > r0) {
            a = (b0 > t0) ? TopRight : BottomRight;
        } else {
            a = (b0 > t0) ? TopLeft : BottomLeft;
        }
    }

    rr->GetCurrent(l1, b1, r1, t1);
    sx = float(r1 - l1) / float(r0 - l0);
    sy = float(t1 - b1) / float(t0 - b0);

    if (v->GetOrientation() == Landscape) {
        float tmp = sx;
        sx = sy;
        sy = tmp;
    }

    float ax, ay;
    GetAlignmentPoint(_graphic, a, ax, ay);
    _graphic->Scale(sx, sy, ax, ay);

  } else if (id == ROTATINGLINE || id == ROTATINGLINELIST || id == ROTATINGRECT) {
    float angle_before = current_angle(rub);
    rub->SetTrack(x, y);
    float angle_after = current_angle(rub);
    float cx, cy;
    _graphic->GetCenter(cx, cy);
    _graphic->Rotate(angle_after-angle_before, cx, cy);

  }

}

boolean OpaqueDragManip::opaqueable_rubband(Rubberband* rub) {
  ClassId id = rub->GetClassId();
  return 
    id == SLIDINGLINE || 
    id == SLIDINGLINELIST || 
    id == SLIDINGRECT || 
    id == SCALINGLINE || 
    id == SCALINGLINELIST || 
    id == SCALINGRECT || 
    id == RUBBERRECT || 
    id == ROTATINGLINE ||
    id == ROTATINGLINELIST ||
    id == ROTATINGRECT;
    ;
}

float OpaqueDragManip::current_angle(Rubberband* rub) {
  ClassId id = rub->GetClassId();
  return 
    id == ROTATINGLINE ? ((RotatingLine*)rub)->CurrentAngle() :
    (id == ROTATINGLINELIST ? ((RotatingLineList*)rub)->CurrentAngle() :
     (id == ROTATINGRECT ? ((RotatingRect*)rub)->CurrentAngle() : 0.0));
}

float OpaqueDragManip::current_scaling(Rubberband* rub) {
  ClassId id = rub->GetClassId();
  return 
    id == SCALINGLINE ? ((ScalingLine*)rub)->CurrentScaling() :
    (id == SCALINGLINELIST ? ((ScalingLineList*)rub)->CurrentScaling() :
     (id == SCALINGRECT ? ((ScalingRect*)rub)->CurrentScaling() : 1.0));
}

