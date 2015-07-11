/*
 * Copyright (c) 1994 Vectaport Inc.
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

/*
 * Overlay object constructor class implementation.
 */

#include <OverlayUnidraw/annotate.h>
#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcreator.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovfile.h>
#include <OverlayUnidraw/ovline.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovpspict.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovspline.h>
#include <OverlayUnidraw/ovstencil.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/textfile.h>
#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/scriptview.h>

#include <UniIdraw/idclasses.h>
#include <UniIdraw/idcomp.h>

#include <Unidraw/catalog.h>

#include <Unidraw/Components/psview.h>

/*****************************************************************************/

OverlayCreator::OverlayCreator () { }

void* OverlayCreator::Create (
    ClassId id, istream& in, ObjectMap* objmap, int objid
) {
    switch (id) {
        case PUSH_CMD:    CREATE(PushCmd, in, objmap, objid);
        case PULL_CMD:    CREATE(PullCmd, in, objmap, objid);
    default:                   return IdrawCreator::Create(id, in,objmap,objid);
    }
}

void* OverlayCreator::Create (ClassId id) {

    if (id == OVERLAYS_VIEW)	     return new OverlaysView;
    if (id == OVERLAY_IDRAW_VIEW)    return new OverlayIdrawView;

    if (id == OVARROWLINE_VIEW)	     return new ArrowLineOvView;
    if (id == OVARROWMULTILINE_VIEW) return new ArrowMultiLineOvView;
    if (id == OVARROWSPLINE_VIEW)    return new ArrowSplineOvView;
    if (id == OVCLOSEDSPLINE_VIEW)   return new ClosedSplineOvView;
    if (id == OVELLIPSE_VIEW)	     return new EllipseOvView;
    if (id == OVLINE_VIEW)	     return new LineOvView;
    if (id == OVMULTILINE_VIEW)	     return new MultiLineOvView;
    if (id == OVPOLYGON_VIEW)	     return new PolygonOvView;
    if (id == OVRASTER_VIEW)	     return new RasterOvView;
    if (id == OVRECT_VIEW)	     return new RectOvView;
    if (id == OVSPLINE_VIEW)	     return new SplineOvView;
    if (id == OVSTENCIL_VIEW)	     return new StencilOvView;
    if (id == OVTEXT_VIEW)	     return new TextOvView;
    if (id == TEXTFILE_VIEW)	     return new TextFileView;
    if (id == OVFILE_VIEW)	     return new OverlayFileView;
	
    if (id == OVERLAYS_PS)	     return new OverlaysPS;
    if (id == OVERLAY_IDRAW_PS)	     return new OverlayIdrawPS;
    if (id == PICTURE_PS)	     return new PicturePS;

    if (id == ARROWLINE_PS)          return new ArrowLinePS;
    if (id == ARROWMULTILINE_PS)     return new ArrowMultiLinePS;
    if (id == ARROWSPLINE_PS)        return new ArrowSplinePS;
    if (id == CLOSEDSPLINE_PS)       return new ClosedSplinePS;
    if (id == ELLIPSE_PS)	     return new EllipsePS;
    if (id == LINE_PS)	             return new LinePS;
    if (id == MULTILINE_PS)          return new MultiLinePS;
    if (id == POLYGON_PS)	     return new PolygonPS;
    if (id == RASTER_PS)	     return new RasterPS;
    if (id == RECT_PS)	             return new RectPS;
    if (id == SPLINE_PS)	     return new SplinePS;
    if (id == STENCIL_PS)	     return new StencilPS;
    if (id == TEXT_PS)	             return new TextPS;
    if (id == TEXTFILE_PS)	     return new TextPS;
    if (id == OVFILE_PS)             return new OverlaysPS;
	
    if (id == OVERLAYS_SCRIPT)	     return new OverlaysScript;
    if (id == OVERLAY_IDRAW_SCRIPT)  return new OverlayIdrawScript;

    if (id == ARROWLINE_SCRIPT)	     return new ArrowLineScript;
    if (id == ARROWMULTILINE_SCRIPT) return new ArrowMultiLineScript;
    if (id == ARROWSPLINE_SCRIPT)    return new ArrowSplineScript;
    if (id == CLOSEDSPLINE_SCRIPT)   return new ClosedSplineScript;
    if (id == ELLIPSE_SCRIPT)        return new EllipseScript;
    if (id == LINE_SCRIPT)	     return new LineScript;
    if (id == MULTILINE_SCRIPT)      return new MultiLineScript;
    if (id == POLYGON_SCRIPT)        return new PolygonScript;
    if (id == RASTER_SCRIPT)         return new RasterScript;
    if (id == RECT_SCRIPT)	     return new RectScript;
    if (id == SPLINE_SCRIPT)	     return new SplineScript;
    if (id == STENCIL_SCRIPT)        return new StencilScript;
    if (id == TEXT_SCRIPT)           return new TextScript;
    if (id == TEXTFILE_SCRIPT)       return new TextFileScript;
    if (id == OVFILE_SCRIPT)         return new OverlayFileScript;

    return IdrawCreator::Create(id);
}











