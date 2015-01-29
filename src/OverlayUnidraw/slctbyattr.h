/*
 * Copyright (c) 1997,1999 Vectaport Inc.
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
 * SlctByAttrCmd - a command for selecting by attribute expression
 */

#ifndef slct_by_attr_h
#define slct_by_attr_h

#include <Unidraw/Commands/command.h>

class Clipboard;
class AttrDialog;

//: command to select components by evaluating an expression.
// This command pops up a dialog box for entering attribute expressions
// to evaluate on all the components in an editor.  unknown
// symbols are replaced with the corresponding AttributeValue on the
// AttributeList associated with the component.  expressions that evaluate
// to non-zero cause that component to be added to the current selection
// of the drawing editor.
class SlctByAttrCmd : public Command {
public:
    SlctByAttrCmd(Editor* = nil, AttrDialog* = nil);
    SlctByAttrCmd(ControlInfo* = nil, AttrDialog* = nil);
    void Init(AttrDialog*);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void Execute();
    virtual boolean Reversible();
    virtual Clipboard* PostDialog();

    Clipboard* clipboard();
protected:
    AttrDialog* calculator_;
    Clipboard* clipboard_;
    OverlaysComp* comps_;
};

#include <ComTerp/comfunc.h>

class AttrDialogImpl;

class AttrListFunc : public ComFunc {
public:
    AttrListFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb, Selection* sel=nil);
    virtual ~AttrListFunc();

protected:
    AttrDialog* attrdialog_;
    OverlaysComp* comps_;
    Iterator* compiter_;
    Clipboard* clipboard_;
    Selection* selection_;
};

class OverlaysComp;

//: interpreter command for plugging together AttrDialog and SlctByAttrCmd.
// interpreter command used to iterate over all the components in an editor.
// selection.
class NextAttrListFunc : public AttrListFunc {
public:
    NextAttrListFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb);
    virtual void execute();
};

//: interpreter command for plugging together AttrDialog and SlctByAttrCmd.
// interpreter command for adding component to current selection when 
// attribute expression evaluates to true.
class TrueAttrListFunc : public AttrListFunc {
public:
    TrueAttrListFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb);
    virtual void execute();

};

//: interpreter command for plugging together AttrDialog and SlctByAttrCmd.
// interpreter command used to not add a component to current selection when 
// attribute expression evaluates to false.
class FalseAttrListFunc : public AttrListFunc {
public:
    FalseAttrListFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb);
    virtual void execute();
};

//: interpreter command for plugging together AttrDialog and SlctByAttrCmd.
// interpreter command used to indicate when done iterating over components.
class DoneAttrListFunc : public AttrListFunc {
public:
    DoneAttrListFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb, Viewer* v);
    virtual void execute();

protected:
    Viewer* viewer_;
};

#endif
