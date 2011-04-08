/*
 * Copyright (c) 2007 Scott E. Johnston
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
 * implementation of LeafWalker
 */

#include <OverlayUnidraw/leafwalker.h>
#include <OverlayUnidraw/ovclasses.h>

/*****************************************************************************/

LeafWalker::LeafWalker(OverlaysComp* start) {
  _before = nil;
  _curr = start;
  _start = start;
}

OverlayComp* LeafWalker::NextLeaf() {
  do {
    OverlayComp* after = _curr->DepthNext(_before);
    _before = _curr;
    _curr = after;
  } while (_curr && _curr->IsA(OVERLAYS_COMP) && _curr != _start->GetParent());
  if (_curr == _start->GetParent()) return nil;
  else return _curr;
}
  
