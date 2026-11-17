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
#include <ComUnidraw/unifunc.h>

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

protected:
    boolean write_pair(AttributeValue& compviewval, ComValue& after_part, int after_nids);
    /* In lhs context (comp.field=val), hand back the pair that routes the
       write through a command instead of the component's live property
       list -- see GrDotSetAttrFunc below.  Returns false, having touched
       nothing, for anything that isn't a plain named field, which then
       takes the ordinary read path. */

};

//: attrlist command, for returning the attribute list of a component.
// attrlist([compview]) -- return attribute list of component.
class GrAttrListFunc : public ComFunc {
public:
    GrAttrListFunc(ComTerp*);

    virtual void execute();
    virtual const char* docstring() {
      return "alst=%s([compview] [:<key> [val]] ... :bincnt) -- the component's own attribute list, not a copy, or a bare attrlist from keyword/value pairs if compview is omitted (:bincnt sizes/disables its index)."; }

};

//: hidden func the dot hands back to complete comp.field=val.
// A write through the dot has to become a command, like setattr()'s, but
// the dot can never see the value: AssignFunc does that half, in a layer
// that names no component type.  So for a component view in lhs context
// GrDotFunc hands back [this func, compview, field] and AssignFunc appends
// the value and calls it -- the same shape lst@N=val uses to re-drive at()
// with :set, and the reason one SetAttrCmd serves both entry points.
// Hidden: an internal completion, reached only through a dot expression.
class GrDotSetAttrFunc : public UnidrawFunc {
public:
    GrDotSetAttrFunc(ComTerp*, Editor*);

    virtual void execute();
    virtual const char* docstring() {
      return "val=%s(compview field val) -- internal: set one attribute of compview through the same command setattr() builds; see GrDotFunc."; }

};

#endif /* !defined(_grdotfunc_h) */

