/*
 * Copyright (c) 2000 IET Inc.
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
 * dot func for componentviews
 */

#if !defined(_grdotfunc_h)
#define _grdotfunc_h

#include <ComTerp/dotfunc.h>

//: . (dot) operator, for compound variables, and access to ComponentView AttributeList's.
class GrDotFunc : public DotFunc {
public:
    GrDotFunc(ComTerp*);

    virtual void execute();
    /* declared explicitly, not inherited, to keep execute()/post_eval() in
       step; inheriting would leave arg 0 an unfired CommandType token. */
    virtual boolean post_eval() { return true; }
    virtual const char* docstring() {
      return "%s(.) makes compound variables, and gives access to ComponentView AttributeList's."; }

};

//: attrlist command, for returning the attribute list of a component.
// attrlist([compview]) -- return attribute list of component.
class GrAttrListFunc : public ComFunc {
public:
    GrAttrListFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "%s([compview]) -- return attribute list of component, or a bare attrlist built from keyword args if compview is omitted."; }

};

#endif /* !defined(_grdotfunc_h) */

