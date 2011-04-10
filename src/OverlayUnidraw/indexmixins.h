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

#ifndef indexmixins_h
#define indexmixins_h

#include <Unidraw/enter-scope.h>

class Graphic;
class MultiLineObj;
class OverlaysComp;
class Picture;

class IndexedGsMixin {
public:
    IndexedGsMixin();

    void grow_indexed_gs(Graphic*);
    Graphic* get_indexed_gs(int);
    void reset_indexed_gs();

protected:
    Picture* _gslist;
};

class IndexedPtsMixin {
public:
    IndexedPtsMixin();

    void grow_indexed_pts(MultiLineObj*);
    MultiLineObj* get_indexed_pts(int);
    void reset_indexed_pts();

protected:
    MultiLineObj** _ptsbuf;
    int _ptsnum;
    int _ptslen;
};

class IndexedPicMixin {
public:
    IndexedPicMixin();

    void grow_indexed_pic(OverlaysComp*);
    OverlaysComp* get_indexed_pic(int);
    void reset_indexed_pic();

protected:
    OverlaysComp** _picbuf;
    int _picnum;
    int _piclen;
};

#endif



