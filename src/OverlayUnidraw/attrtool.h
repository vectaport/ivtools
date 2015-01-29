/*
 * Copyright (c) 1996-1999 Vectaport Inc.
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

#ifndef attrtool_h
#define attrtool_h

#include <Unidraw/Tools/tool.h>

class Selection;

//: tool for editing attributes of a component (property list editing).
class AttributeTool : public Tool {
public:
    AttributeTool(ControlInfo* =nil);

    virtual Manipulator* CreateManipulator(Viewer*, Event&, Transformer* =nil);
    // select component under event, and bring up dialog for editing attributes.
    virtual Command* InterpretManipulator(Manipulator*);
    // empty method.

    virtual Tool* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void Localize(Selection*, Viewer*);
};

#endif
