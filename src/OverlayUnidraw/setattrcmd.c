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
#include <Attribute/attribute.h>
#include <Attribute/attrvalue.h>

#include <Unidraw/editor.h>

#include <string.h>

/* paired with the delete [] in every destructor below; strdup's malloc is
   not. */
static char* anno_copy(const char* anno) {
    if (!anno) return nil;
    char* copy = new char[strlen(anno)+1];
    strcpy(copy, anno);
    return copy;
}

/*****************************************************************************/

/* AttributeList's copy constructor shares each value's array or stream with
   the original, which is right for an ordinary copy and wrong for a snapshot:
   a later write through the shared container would rewrite history.  Arrays
   and streams are duplicated here; dup_as_needed() goes all the way down,
   since AttributeValueList's own copy constructor takes each element by
   pointer and so duplicates in turn.

   Object values are deliberately left shared.  Undo restores a handle to the
   same component, and dup_as_needed() on a ComponentView would Duplicate()
   it and Attach() the copy to the subject. */
static void snapshot_values(AttributeList* al) {
    ALIterator i;
    for (al->First(i); !al->Done(i); al->Next(i)) {
        AttributeValue* av = al->GetAttr(i)->Value();
	if (av->is_array() || av->is_stream())
	    av->dup_as_needed();
    }
}

AttrData::AttrData(AttributeList* al) {
    _attrlist = new AttributeList(al);
    Resource::ref(_attrlist);
    snapshot_values(_attrlist);
}

AttrData::~AttrData() {
    Resource::unref(_attrlist);
}

/*****************************************************************************/

ClassId SetAttrCmd::GetClassId () { return SETATTR_CMD; }

boolean SetAttrCmd::IsA (ClassId id) {
    return SETATTR_CMD == id || Command::IsA(id);
}

/* the caller hands over its reference, so the values are snapshotted in place
   rather than into a copy: the history replays this list on every redo, and a
   write through an array shared with the caller would change what redo
   applies. */
SetAttrCmd::SetAttrCmd (ControlInfo* c, AttributeList* al) : Command(c) {
    _attrlist = al;
    Resource::ref(_attrlist);
    if (_attrlist) snapshot_values(_attrlist);
    _replace = false;
}

SetAttrCmd::SetAttrCmd (Editor* ed, AttributeList* al) : Command(ed) {
    _attrlist = al;
    Resource::ref(_attrlist);
    if (_attrlist) snapshot_values(_attrlist);
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

/*****************************************************************************/

AnnoData::AnnoData(const char* anno) {
    _anno = anno_copy(anno);
}

AnnoData::~AnnoData() {
    delete [] _anno;
}

/*****************************************************************************/

ClassId SetAnnoCmd::GetClassId () { return SETANNO_CMD; }

boolean SetAnnoCmd::IsA (ClassId id) {
    return SETANNO_CMD == id || Command::IsA(id);
}

SetAnnoCmd::SetAnnoCmd (ControlInfo* c, const char* anno) : Command(c) {
    _anno = anno_copy(anno);
}

SetAnnoCmd::SetAnnoCmd (Editor* ed, const char* anno) : Command(ed) {
    _anno = anno_copy(anno);
}

SetAnnoCmd::~SetAnnoCmd () {
    delete [] _anno;
}

Command* SetAnnoCmd::Copy () {
    SetAnnoCmd* copy = new SetAnnoCmd(CopyControlInfo(), _anno);
    InitCopy(copy);
    return copy;
}
