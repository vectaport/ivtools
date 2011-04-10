/*
 * Copyright 1994, 1995, 1999 Vectaport Inc.
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

#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovstates.h>
#include <OverlayUnidraw/ovviewer.h>
#include <stdio.h>

#include <iostream>
#include <fstream>

using std::cerr;

/*****************************************************************************/

PtrLocState::PtrLocState(PixelCoord x, PixelCoord y, OverlayEditor* ed) :NameState(nil)
{
  init(ed);
  ptrcoords(x, y);
}

PtrLocState::PtrLocState() : NameState(nil) {
  _buf = nil;
  _ed = nil;
}

void PtrLocState::init(OverlayEditor* ed) {
  _buf = (char*) new char[256];
  _ed = ed;
}

PtrLocState::~PtrLocState() {
  delete _buf;
}

void PtrLocState::ptrcoords(PixelCoord x, PixelCoord y) {
  ((OverlayViewer*)_ed->GetViewer())->ScreenToDrawing(x, y, _x, _y);
  sprintf(_buf, "%.2f %.2f", _x, _y);
  name(_buf, true);
}
