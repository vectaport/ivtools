/*
 * Copyright (c) 2026 Vectaport Inc.
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

#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/setattrcmd.h>

#include <Attribute/attrlist.h>

#include <Unidraw/editor.h>

/*****************************************************************************/

AttrData::AttrData(AttributeList* al) {
    _attrlist = new AttributeList(al);
    Resource::ref(_attrlist);
}

AttrData::~AttrData() {
    Resource::unref(_attrlist);
}

/*****************************************************************************/

ClassId SetAttrCmd::GetClassId () { return SETATTR_CMD; }

boolean SetAttrCmd::IsA (ClassId id) {
    return SETATTR_CMD == id || Command::IsA(id);
}

SetAttrCmd::SetAttrCmd (ControlInfo* c, AttributeList* al) : Command(c) {
    _attrlist = al;
    Resource::ref(_attrlist);
    _replace = false;
}

SetAttrCmd::SetAttrCmd (Editor* ed, AttributeList* al) : Command(ed) {
    _attrlist = al;
    Resource::ref(_attrlist);
    _replace = false;
}

SetAttrCmd::~SetAttrCmd () {
    Resource::unref(_attrlist);
}

Command* SetAttrCmd::Copy () {
    /* the copy gets its own list: a logged command outlives the caller's. */
    SetAttrCmd* copy = new SetAttrCmd(CopyControlInfo(), new AttributeList(_attrlist));
    copy->replace() = _replace;
    InitCopy(copy);
    return copy;
}
