/*
 * Copyright 2004 Scott E. Johnston
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
#include <DrawServ/drawlinkcomp.h>

/****************************************************************************/


int DrawLinkComp::_symid = -1;

DrawLinkComp::DrawLinkComp(DrawLink* link) : OverlayComp() {
  _drawlink = link;
  _gr = nil;
}

DrawLinkComp::~DrawLinkComp() {
}

Component* DrawLinkComp::Copy() {
  DrawLinkComp* comp = new DrawLinkComp(_drawlink);
  return comp;
}

ClassId DrawLinkComp::GetClassId () { return DRAWLINK_COMP; }

boolean DrawLinkComp::IsA (ClassId id) {
    return DRAWLINK_COMP == id || OverlayComp::IsA(id);
}

