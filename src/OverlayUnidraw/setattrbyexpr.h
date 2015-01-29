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
 * SetAttrByExprCmd - a command for setting attributes by expression
 */

#ifndef set_attr_by_expr_h
#define set_attr_by_expr_h

#include <Unidraw/Commands/command.h>
#include <OverlayUnidraw/slctbyattr.h>

class Clipboard;
class AttrDialog;

//: command to set attributes on a component by evaluating an expression.
// This command checks for a non-empty selection in the current editor,
// then pops up a dialog box for entering attribute expressions to evaluate.
// symbols on the right hand side of an assigment operator are used to
// look up and return values from the property list (AttributeList) of
// a component.  symbols on the left-hand side of an assignment operator are
// used to create or modify an entry in the same component property list.
class SetAttrByExprCmd : public Command {
public:
    SetAttrByExprCmd(Editor* = nil, AttrDialog* = nil);
    SetAttrByExprCmd(ControlInfo* = nil, AttrDialog* = nil);
    void Init(AttrDialog*);

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void Execute();
    virtual boolean Reversible();
    virtual Clipboard* PostDialog();

    Selection* selection();
    Clipboard* clipboard();
protected:
    AttrDialog* calculator_;
    OverlaysComp* comps_;
    Clipboard* clipboard_;
};

//: interpreter command for plugging together AttrDialog and SetAttrByExprCmd.
// interpreter command used to iterate over all the components in the current 
// selection.
class NextInSelectionFunc : public AttrListFunc {
public:
    NextInSelectionFunc(ComTerp*, AttrDialog*, Selection* sel, Iterator* i);
    virtual void execute();
};


//: interpreter command for plugging together AttrDialog and SetAttrByExprCmd.
// interpreter command used to do nothing whether the attribute expression
// evalutes to true or false, because the assignment of values by interpreting
// assignment commands has already occurred.
class BothSetAttrFunc : public AttrListFunc {
public:
    BothSetAttrFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb);
    virtual void execute();

};

//: interpreter command for plugging together AttrDialog and SetAttrByExprCmd.
// interpreter command used to indicate when done iterating over all the components 
// in the current selection.
class DoneSetAttrFunc : public AttrListFunc {
public:
    DoneSetAttrFunc(ComTerp*, AttrDialog*, OverlaysComp* comps, Iterator* i, Clipboard* cb, Viewer* v);
    virtual void execute();

protected:
    Viewer* viewer_;
};

#endif
