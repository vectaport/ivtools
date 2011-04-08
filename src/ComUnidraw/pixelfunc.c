/*
 * Copyright (c) 2001 Scott E. Johnston
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

#include <ComUnidraw/pixelfunc.h>

#include <OverlayUnidraw/ovraster.h>
#include <Unidraw/Graphic/damage.h>
#include <Unidraw/iterator.h>
#include <Unidraw/viewer.h>
#include <Attribute/attrlist.h>

/*****************************************************************************/

PixelPokeFunc::PixelPokeFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelPokeFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  ComValue xv(stack_arg(1));
  ComValue yv(stack_arg(2));
  ComValue valv(stack_arg(3));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    raster->poke(xv.int_val(), yv.int_val(), valv.float_val(), valv.float_val(), valv.float_val(), 1.0);
    push_stack(rastcompv);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelPeekFunc::PixelPeekFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelPeekFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  ComValue xv(stack_arg(1));
  ComValue yv(stack_arg(2));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    ComValue retval;
    raster->graypeek(xv.int_val(), yv.int_val(), retval);
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelColsFunc::PixelColsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelColsFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    ComValue retval(raster->pwidth());
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelRowsFunc::PixelRowsFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelRowsFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    ComValue retval(raster->pheight());
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelFlushFunc::PixelFlushFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelFlushFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (raster) {
    raster->flush();
    viewer->GetDamage()->Incur(rastrect);
    ComValue retval(rastcompv);
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());


}

/*****************************************************************************/

PixelClipFunc::PixelClipFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelClipFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  ComValue ptsv(stack_arg(1));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;

  if (rastrect && ptsv.is_array() && ptsv.array_val()->Number()>2 ) {
    int n = ptsv.array_val()->Number()/2;
    IntCoord x[n], y[n];
    Iterator it;
    AttributeValueList* avl = ptsv.array_val();
    avl->First(it);
    for( int i=0; i<n; i++ ) {
      x[i] = avl->GetAttrVal(it)->int_val();
      avl->Next(it);
      y[i] = avl->GetAttrVal(it)->int_val();
      avl->Next(it);
    }
    rastrect->clippts(x, y, n);
    rastcomp->Notify();
  } else 
    push_stack(ComValue::nullval());

}

/*****************************************************************************/

AlphaTransFunc::AlphaTransFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void AlphaTransFunc::execute() {
  Viewer* viewer = _ed->GetViewer();

  ComValue rastcompv(stack_arg(0));
  ComValue alphav(stack_arg(1));
  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;

  if (rastrect ) {
    if (alphav.is_numeric()) {
      rastrect->alphaval(alphav.float_val());
      rastcomp->Notify();
    }
    ComValue retval(rastrect->alphaval());
    push_stack(retval);
  } else 
    push_stack(ComValue::nullval());

}

