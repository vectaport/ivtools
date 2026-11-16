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

#ifndef setattrcmd_h
#define setattrcmd_h

#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/data.h>

class AttributeList;

//: one component's prior property list, for SetAttrCmd undo.
class AttrData : public Data {
public:
    AttrData(AttributeList*);
    // copies the list; the original goes on being edited.  Array and stream
    // values are duplicated rather than shared, so a later write through one
    // of them cannot rewrite what undo restores.
    virtual ~AttrData();

    AttributeList* attrlist() { return _attrlist; }
protected:
    AttributeList* _attrlist;
};

//: command to merge attributes into each target component's property list.
// Targets are the command's clipboard, or the editor's selection when it has
// none, so the one command serves a scripted setattr() naming a component and
// an interactive edit of whatever is selected.
class SetAttrCmd : public Command {
public:
    SetAttrCmd(ControlInfo*, AttributeList* = nil);
    SetAttrCmd(Editor* = nil, AttributeList* = nil);
    virtual ~SetAttrCmd();

    AttributeList* attrlist() { return _attrlist; }
    // the attributes to merge into each target.  The constructor takes over
    // the caller's reference and duplicates any array or stream value in
    // place, since the history reapplies this list on every redo.
    boolean& replace() { return _replace; }
    // when set, each target's property list is emptied before the merge, so
    // the command carries the list the target ends up with rather than an
    // addition to it -- what the attribute dialog hands back, where an entry
    // the user removed has to go away.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    AttributeList* _attrlist;
    boolean _replace;
};

//: one component's prior annotation, for SetAnnoCmd undo.
class AnnoData : public Data {
public:
    AnnoData(const char*);
    // copies the string, which may be nil for a component with none.
    virtual ~AnnoData();

    const char* annotation() { return _anno; }
protected:
    char* _anno;
};

//: command to set each target component's annotation.
// The annotation is a component's own string rather than a property-list
// entry, so it needs its own command; targeting works as SetAttrCmd's does.
class SetAnnoCmd : public Command {
public:
    SetAnnoCmd(ControlInfo*, const char* = nil);
    SetAnnoCmd(Editor* = nil, const char* = nil);
    virtual ~SetAnnoCmd();

    const char* annotation() { return _anno; }
    // the annotation to set on each target; nil clears it.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    char* _anno;
};

#endif
