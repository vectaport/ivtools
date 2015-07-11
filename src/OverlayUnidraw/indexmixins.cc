/*
 * Copyright (c) 1998 Vectaport Inc.
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
 * implementation of gs, pts, and pic indexing
 */

#include <OverlayUnidraw/indexmixins.h>
#include <OverlayUnidraw/ovcomps.h>
#include <Unidraw/Graphic/picture.h>
#include <Unidraw/iterator.h>

/*****************************************************************************/

IndexedGsMixin::IndexedGsMixin() {
  _gslist = nil;
}

void IndexedGsMixin::grow_indexed_gs(Graphic* gs) {
  if (!_gslist) _gslist = new Picture();
  _gslist->Append(gs);
}

Graphic* IndexedGsMixin::get_indexed_gs(int index) {
  if (_gslist) {
    Iterator i;
    for (_gslist->First(i); !_gslist->Done(i); _gslist->Next(i)) {
      if (index==0) return _gslist->GetGraphic(i);
      index--;
    }
  }
  return nil;
}

void IndexedGsMixin::reset_indexed_gs() {
  delete _gslist;
  _gslist = nil;
}

/*****************************************************************************/

IndexedPtsMixin::IndexedPtsMixin() {
  _ptsbuf = nil;
}

void IndexedPtsMixin::grow_indexed_pts(MultiLineObj* mlo) {
  if (!_ptsbuf) {
    _ptslen = 64;
    _ptsbuf = new MultiLineObj*[_ptslen];
    _ptsnum = 0;
    for (int i=0; i<_ptslen; i++) 
      _ptsbuf[i] = nil;
  }
  if (_ptsnum==_ptslen) {
    MultiLineObj** newbuf = new MultiLineObj*[_ptslen*2];
    int i;
    for (i=0; i<_ptslen; i++) 
      newbuf[i] = _ptsbuf[i];
    for (;i<_ptslen*2; i++)
      newbuf[i] = nil;
    _ptslen *= 2;
    delete _ptsbuf;
    _ptsbuf = newbuf;
  }
  Resource::ref(mlo);
  _ptsbuf[_ptsnum++] = mlo;
}

MultiLineObj* IndexedPtsMixin::get_indexed_pts(int index) {
  if (index >= 0  && index < _ptsnum) 
    return _ptsbuf[index];
  else
    return nil;
}

void IndexedPtsMixin::reset_indexed_pts() {
  if (_ptsbuf) {
    for (int i=0; i<_ptsnum; i++) 
      Unref(_ptsbuf[i]);
    delete _ptsbuf;
    _ptsbuf = nil;
  }
}

/*****************************************************************************/

IndexedPicMixin::IndexedPicMixin() {
  _picbuf = nil;
}

void IndexedPicMixin::grow_indexed_pic(OverlaysComp* pic) {
  if (!_picbuf) {
    _piclen = 64;
    _picbuf = new OverlaysComp*[_piclen];
    _picnum = 0;
    for (int i=0; i<_piclen; i++) 
      _picbuf[i] = nil;
  }
  if (_picnum==_piclen) {
    OverlaysComp** newbuf = new OverlaysComp*[_piclen*2];
    int i;
    for (i=0; i<_piclen; i++) 
      newbuf[i] = _picbuf[i];
    for (;i<_piclen*2; i++)
      newbuf[i] = nil;
    _piclen *= 2;
    delete _picbuf;
    _picbuf = newbuf;
  }
  _picbuf[_picnum++] = pic;
}

OverlaysComp* IndexedPicMixin::get_indexed_pic(int index) {
  if (index >= 0  && index < _picnum) 
    return _picbuf[index];
  else
    return nil;
}

void IndexedPicMixin::reset_indexed_pic() {
  if (_picbuf) {
    for (int i=0; i<_picnum; i++)
      delete _picbuf[i];
    delete _picbuf;
    _picbuf = nil;
  }
}

