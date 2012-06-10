/*
 * Copyright (c) 2002 Scott E. Johnston
 * Copyright (c) 1997 Vectaport Inc. and R.B. Kissh & Associates
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

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/rastercmds.h>

#include <Unidraw/unidraw.h>
#include <Unidraw/iterator.h>

#include <IVGlyph/stredit.h>

#include <InterViews/window.h>

// -----------------------------------------------------------------------


ClassId ReplaceRasterCmd::GetClassId () { return REPLACE_RASTER_CMD; }

boolean ReplaceRasterCmd::IsA (ClassId id) {
    return REPLACE_RASTER_CMD == id || Command::IsA(id);
}


ReplaceRasterCmd::ReplaceRasterCmd (ControlInfo* c) 
  : Command(c) {
}


ReplaceRasterCmd::ReplaceRasterCmd () 
    : Command((Editor*)nil) 
{ 
}


ReplaceRasterCmd::ReplaceRasterCmd (
  Editor* ed, RasterOvComp* comp, OverlayRaster* nras
)
    : Command(ed), _comp(comp), _nras(nras), _orig(nil) 
{ 
    _nras->ref();
}


ReplaceRasterCmd::ReplaceRasterCmd( 
  ControlInfo* c, RasterOvComp* comp, OverlayRaster* nras
)
    : Command(c), _comp(comp), _nras(nras), _orig(nil)
{
    _nras->ref();
}


ReplaceRasterCmd::~ReplaceRasterCmd() {
    _orig->unref();
    _nras->unref();
}


void ReplaceRasterCmd::Execute() {

    OverlayRasterRect* rr = _comp->GetOverlayRasterRect();

    if (!_orig) {
        _orig = rr->GetOriginal();
        _orig->ref();
    }
 
    rr->SetRaster( _nras );

    _comp->Notify();
    unidraw->Update();

}


void ReplaceRasterCmd::Unexecute() {

    OverlayRasterRect* rr = _comp->GetOverlayRasterRect();

    if (_orig)
        rr->SetRaster((OverlayRaster*)_orig);

    _comp->Notify();
    unidraw->Update();
}


Command* ReplaceRasterCmd::Copy () {
    ReplaceRasterCmd* copy = new ReplaceRasterCmd(
        CopyControlInfo(), _comp, _nras
    );
    InitCopy(copy);
    return copy;
}


boolean ReplaceRasterCmd::Reversible() {
    return true;
}

/*-----------------------------------------------------------------*/

UnhighlightRasterCmd::UnhighlightRasterCmd(ControlInfo* ci) : Command(ci) {
}

Command* UnhighlightRasterCmd::Copy() {
    Command* copy = new UnhighlightRasterCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

ClassId UnhighlightRasterCmd::GetClassId () { return UNHIGHLIGHT_RASTER_CMD; }

boolean UnhighlightRasterCmd::IsA (ClassId id) {
    return UNHIGHLIGHT_RASTER_CMD == id || Command::IsA(id);
}

boolean UnhighlightRasterCmd::Reversible() {
    return false;
}

void UnhighlightRasterCmd::Execute() {
  // find the rasters in the current comp and unhighlight

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlayViewer* v = ed->GetOverlayViewer();
  OverlayView* views = v->GetCurrent(); 
  RasterOvView* rastview = nil;
  Iterator i;
  for (views->Last(i); !views->Done(i); views->Prev(i)) {
    GraphicView* view = views->GetView(i);
    if (view->IsA(OVRASTER_VIEW)) {
      rastview = (RasterOvView*)view;
      if (rastview) {
	OverlayRaster* raster = rastview->GetOverlayRaster();
	if (raster) {
	  raster->unhighlight();
	}
      }
    }
  }
}

/*-----------------------------------------------------------------*/

AlphaTransparentRasterCmd::AlphaTransparentRasterCmd(ControlInfo* ci) : Command(ci) {
  _alpha_set = false;
}

Command* AlphaTransparentRasterCmd::Copy() {
    Command* copy = new AlphaTransparentRasterCmd(CopyControlInfo());
    InitCopy(copy);
    ((AlphaTransparentRasterCmd*)copy)->_alpha = _alpha;
    ((AlphaTransparentRasterCmd*)copy)->_oldalpha = _oldalpha;
    ((AlphaTransparentRasterCmd*)copy)->_alpha_set = _alpha_set;
    return copy;
}

ClassId AlphaTransparentRasterCmd::GetClassId () { return ALPHATRANSPARENT_CMD; }

boolean AlphaTransparentRasterCmd::IsA (ClassId id) {
    return ALPHATRANSPARENT_CMD == id || Command::IsA(id);
}

boolean AlphaTransparentRasterCmd::Reversible() {
    return true;
}

void AlphaTransparentRasterCmd::Execute() {

  if (!_alpha_set) {
    char* newalpha = StrEditDialog::post
      (GetEditor()->GetWindow(), 
       "Enter alpha value", "0.5");
    if (newalpha)
      _alpha = atof(newalpha);
    else
      _alpha = 1.0;
    _alpha_set = true;
  }

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlaySelection* sel = (OverlaySelection*) ed->GetSelection();
  Iterator i;
  for (sel->First(i); !sel->Done(i); sel->Next(i)) {
    GraphicView* view = sel->GetView(i);
    if (view->IsA(OVRASTER_VIEW)) {
      RasterOvView* rastview = (RasterOvView*)view;
      if (rastview) {
	RasterOvComp* rastcomp = (RasterOvComp*)rastview->GetSubject();
	OverlayRasterRect* rr = rastcomp->GetOverlayRasterRect();
	if (rr) {
	  _oldalpha = rr->alphaval();
	  rr->alphaval(_alpha);
	  rastcomp->Notify();
	  unidraw->Update();
	}
      }
    }
  }
}

void AlphaTransparentRasterCmd::Unexecute() {

  OverlayEditor* ed = (OverlayEditor*)GetEditor();
  OverlaySelection* sel = (OverlaySelection*) ed->GetSelection();
  Iterator i;
  for (sel->First(i); !sel->Done(i); sel->Next(i)) {
    GraphicView* view = sel->GetView(i);
    if (view->IsA(OVRASTER_VIEW)) {
      RasterOvView* rastview = (RasterOvView*)view;
      if (rastview) {
	RasterOvComp* rastcomp = (RasterOvComp*)rastview->GetSubject();
	OverlayRasterRect* rr = rastcomp->GetOverlayRasterRect();
	if (rr) {
	  rr->alphaval(_oldalpha);
	  rastcomp->Notify();
	  unidraw->Update();
	}
      }
    }
  }
}

