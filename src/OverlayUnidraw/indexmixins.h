/*
 * Copyright (c) 1998-1999 Vectaport Inc.
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

//: mixin for graphic-state indexing mechanism used by OverlayIdrawComp, etc..
// When a tree of graphical components is saved to disk (translated to ASCII), the 
// graphic states are output first as 'gs' records, to allow for replicated graphic
// states to appear only once in a file.  When reading these documents back in,
// the istream constructor of OverlayIdrawComp (and other top-level components) uses 
// this mixin to manage a table of graphic states, building it up as each 'gs'
// record is read, and accessing entries for each ":gs id" field found in subsequent
// records.
class IndexedGsMixin {
public:
    IndexedGsMixin();

    void grow_indexed_gs(Graphic*);
    // add graphic state to table.
    Graphic* get_indexed_gs(int id);
    // retrieve graphic state by 'id'.
    void reset_indexed_gs();
    // reset graphic state table.

protected:
    Picture* _gslist;
};

//: mixin for point-list indexing mechanism used by OverlayIdrawComp, etc..
// When a tree of graphical components is saved to disk (translated to ASCII), it is
// possible to enable the output of point-lists first as 'pts' records, to allow
// for compression of replicated point lists within a single document.  When reading 
// these documents back in, the istream constructor of OverlayIdrawComp (and other 
// top-level components) uses this mixin to manage a table of point lists, building 
// it up as each 'pts' record is read, and accessing entries for each ":pts id" field 
// found in subsequent records.
class IndexedPtsMixin {
public:
    IndexedPtsMixin();

    void grow_indexed_pts(MultiLineObj*);
    // add point list to table.
    MultiLineObj* get_indexed_pts(int id);
    // retrieve point list by 'id'.
    void reset_indexed_pts();
    // reset point list table.

protected:
    MultiLineObj** _ptsbuf;
    int _ptsnum;
    int _ptslen;
};

//: mixin for composite-graphic (pic) indexing mechanism used by OverlayIdrawComp, etc..
// When a tree of graphical components is saved to disk (translated to ASCII), it is
// possible to enable the output of common composite-graphics as 'pic' records, 
// to allow for compression of replicated composite-graphics within a single document.
// When reading these documents back in, the istream constructor of OverlayIdrawComp 
// (and other top-level components) uses this mixin to manage a table of pics, 
// building it up as each 'pic' record is read, and accessing entries for each 
// ":pics id" field found in subsequent records.
class IndexedPicMixin {
public:
    IndexedPicMixin();

    void grow_indexed_pic(OverlaysComp*);
    // add pic to table.
    OverlaysComp* get_indexed_pic(int id);
    // retrieve pic by 'id'.
    void reset_indexed_pic();
    // reset pic table.

protected:
    OverlaysComp** _picbuf;
    int _picnum;
    int _piclen;
};

#endif



