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

#include <ComUnidraw/comeditor.h>
#include <ComUnidraw/grstatfunc.h>
#include <ComUnidraw/pixelfunc.h>

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovviewer.h>
#include <Unidraw/Graphic/damage.h>
#include <Unidraw/iterator.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>
#include <Attribute/attrlist.h>
#include <IV-2_6/InterViews/world.h>
#include <vector>

/*****************************************************************************/
PixelPokeLineFunc::PixelPokeLineFunc(ComTerp* comterp, Editor* ed) : UnidrawFunc(comterp, ed) {
}

void PixelPokeLineFunc::execute() {
  Viewer* viewer = _ed->GetViewer();
  
  ComValue rastcompv(stack_arg(0));
  ComValue xv(stack_arg(1));
  ComValue yv(stack_arg(2));
  ComValue vallistv(stack_arg(3));
  int xval = xv.int_val();
  int yval = yv.int_val();
  
  if(!vallistv.is_type(ComValue::ArrayType) || vallistv.array_len() <= 0){
    reset_stack();
    push_stack(ComValue::nullval());
    return;
  }

  reset_stack();
  
  RasterOvComp* rastcomp = (RasterOvComp*) rastcompv.geta(RasterOvComp::class_symid());
  OverlayRasterRect* rastrect = rastcomp ? rastcomp->GetOverlayRasterRect() : nil;
  OverlayRaster* raster = rastrect ? rastrect->GetOriginal() : nil;
  
  if (raster) {
    ALIterator i;
    AttributeValueList* avl = vallistv.array_val();
    avl->First(i);
    int wval = avl->Number();

    for (int j = 0; j < wval && !avl->Done(i); j++) {
      AttributeValue* elem = avl->GetAttrVal(i);
      avl->Next(i);

      ColorIntensity r, g, b;
      float alpha = 1.0;

      if (elem->is_array()) {
        // tuple form: r,g,b or r,g,b,alpha  (values in 0.0..1.0)
        AttributeValueList* tavl = elem->array_val();
        int n = tavl->Number();
        if (n < 3) continue;
        ALIterator ti;
        tavl->First(ti);
        r = (ColorIntensity) tavl->GetAttrVal(ti)->float_val(); tavl->Next(ti);
        g = (ColorIntensity) tavl->GetAttrVal(ti)->float_val(); tavl->Next(ti);
        b = (ColorIntensity) tavl->GetAttrVal(ti)->float_val(); tavl->Next(ti);
        if (n >= 4 && !tavl->Done(ti))
          alpha = tavl->GetAttrVal(ti)->float_val();
      } else {
        // legacy packed-int form: 0xRRGGBB
        int pixelcolor = elem->int_val();
        char colorname[8];
        snprintf(colorname, sizeof(colorname),"#%06x",pixelcolor);
        Color::find(World::current()->display(),colorname, r, g, b);
      }

      raster->poke(xval+j, yval, r, g, b, alpha);
    }
    push_stack(rastcompv);
  } 
  else 
    push_stack(ComValue::nullval());
}

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
    ColorIntensity r, g, b;
    float alpha = 1.0;

    if (valv.is_array()) {
      // tuple form: r,g,b or r,g,b,alpha  (values in 0.0..1.0)
      AttributeValueList* avl = valv.array_val();
      int n = avl->Number();
      if (n < 3) {
        push_stack(ComValue::nullval());
        return;
      }
      ALIterator it;
      avl->First(it);
      r = (ColorIntensity) avl->GetAttrVal(it)->float_val(); avl->Next(it);
      g = (ColorIntensity) avl->GetAttrVal(it)->float_val(); avl->Next(it);
      b = (ColorIntensity) avl->GetAttrVal(it)->float_val(); avl->Next(it);
      if (n >= 4 && !avl->Done(it))
        alpha = avl->GetAttrVal(it)->float_val();
    } else {
      // legacy packed-int form: 0xRRGGBB
      int pixelcolor = valv.int_val();
      char colorname[8];
      snprintf(colorname, sizeof(colorname),"#%06x",pixelcolor);
      Color::find(World::current()->display(),colorname, r, g, b);
    }

    raster->poke(xv.int_val(), yv.int_val(), r, g, b, alpha);
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
    if (raster->gray_flag()) {
      raster->graypeek(xv.int_val(), yv.int_val(), retval);
      push_stack(retval);
    }
    else {
      ColorIntensity r, g, b;
      float alpha;
      raster->peek(xv.int_val(), yv.int_val(), r, g, b, alpha);
      
      // put r,g,b,alpha in a list return
      AttributeValueList* al = new AttributeValueList();
      al->Append(new AttributeValue((float)r));
      al->Append(new AttributeValue((float)g));
      al->Append(new AttributeValue((float)b));
      al->Append(new AttributeValue(alpha));

      ComValue retval(al);		   
      push_stack(retval);
    }
      
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
    std::vector<IntCoord> x(n); std::vector<IntCoord> y(n);
    Iterator it;
    AttributeValueList* avl = ptsv.array_val();
    avl->First(it);
    for( int i=0; i<n; i++ ) {
      x[i] = avl->GetAttrVal(it)->int_val();
      avl->Next(it);
      y[i] = avl->GetAttrVal(it)->int_val();
      avl->Next(it);
    }
    rastrect->clippts(&x[0], &y[0], n);
    rastcomp->Notify();
    push_stack(rastcompv);
    return;
  }
  
  if (rastrect && ptsv.is_compview()) {
    OverlayComp* ovcomp = (OverlayComp*)ptsv.geta(OverlayComp::class_symid(), OVERLAY_COMP);
    Graphic* pgr = ovcomp->GetGraphic();
    OverlayEditor* ed = (OverlayEditor*)GetEditor();
    OverlayViewer* viewer = ed ? (OverlayViewer*)ed->GetViewer() : nil;
    
    push_stack(ptsv);
    PointsFunc ptsfunc(comterp(), editor());
    ptsfunc.exec(1,0);
    ComValue newptsv = comterp()->pop_stack();

    if (newptsv.is_array()) {
      int n = newptsv.array_val()->Number()/2;
      std::vector<IntCoord> x(n); std::vector<IntCoord> y(n);
      AttributeValueList *alist = newptsv.array_val();
      ALIterator it;
      int i=0;
      alist->First(it);
      while (!alist->Done(it)) {
	float gx = alist->GetAttrVal(it)->float_val();
	alist->Next(it);
	float gy = alist->GetAttrVal(it)->float_val();
	alist->Next(it);
	float sx, sy, dx, dy, rx, ry;
	viewer->GraphicToScreen(pgr, gx, gy, sx, sy);
	viewer->ScreenToGraphic(sx, sy, rastrect, rx, ry);
	x[i] = int(rx);
	y[i] = int(ry);
	i++;
      }
      rastrect->clippts(&x[0], &y[0], n);
      rastcomp->Notify();
      push_stack(rastcompv);
      unidraw->Update();
      return;
    }
  }

  rastrect->clippts(NULL, NULL, 0);
  rastcomp->Notify();
  push_stack(rastcompv);
  unidraw->Update();

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


