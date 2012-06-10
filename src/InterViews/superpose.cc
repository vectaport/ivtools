/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * Superpose - composite layout
 */

#include <InterViews/superpose.h>

Superpose::Superpose(
    Layout* l0, Layout* l1, Layout* l2, Layout* l3, Layout* l4
) : Layout() {
    Layout* arg[6];
    arg[0] = l0; arg[1] = l1; arg[2] = l2; arg[3] = l3; arg[4] = l4;
    arg[5] = nil;
    for (count_ = 0; arg[count_] != nil; count_++) { }
    layout_ = new Layout* [count_];
    for (long i = 0; i < count_; i++) {
	layout_[i] = arg[i];
    }
}

Superpose::~Superpose() {
    for (long i = 0; i < count_; ++i) {
        delete layout_[i];
    }
    delete layout_;
    layout_ = nil;
}

void Superpose::request(
    GlyphIndex count, const Requisition* request, Requisition& result
) {
    for (long i = 0; i < count_; ++i) {
	layout_[i]->request(count, request, result);
    }
}

void Superpose::allocate(
    const Allocation& given,
    GlyphIndex count, const Requisition* requisition, Allocation* result
) {
    for (long i = 0; i < count_; ++i) {
	layout_[i]->allocate(given, count, requisition, result);
    }
}
