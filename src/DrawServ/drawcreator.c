/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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

#include <DrawServ/drawclasses.h>
#include <DrawServ/drawcomps.h>
#include <DrawServ/drawcreator.h>
#include <DrawServ/drawviews.h>

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphcreator.h>
#include <GraphUnidraw/nodecomp.h>

#include <FrameUnidraw/framecreator.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovpspict.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/catalog.h>

DrawCreator::DrawCreator () { }

void* DrawCreator::Create (ClassId id) {
    void* view = DrawCreator::create(id);
    if (!view) 
      view = FrameCreator::create(id);
    if (!view) 
      view = GraphCreator::create(id);
    return view ? view : OverlayCreator::Create(id);
}

void* DrawCreator::create (ClassId id) {

    if (id == DRAW_IDRAW_VIEW)   return new DrawIdrawView;

    if (id == DRAW_IDRAW_PS)     return new OverlayIdrawPS;

    if (id == DRAW_IDRAW_SCRIPT) return new DrawIdrawScript;

    return nil;
}
