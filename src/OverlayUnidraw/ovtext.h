/*
 * Copyright (c) 1994,1999 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
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
 * Overlay Text component declarations.
 */

#ifndef overlay_text_h
#define overlay_text_h

#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovpsview.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Graphic/ulabel.h>

#include <IV-2_6/_enter.h>

class TextGraphic;
class TextManip;
class istream;

//: clone of TextComp derived from OverlayComp.
class TextOvComp : public OverlayComp {
public:
    TextOvComp(TextGraphic* = nil, OverlayComp* parent = nil);
    TextOvComp(istream&, OverlayComp* parent = nil);

    virtual void Interpret(Command*);
    // interpret brush, pattern, and font commands, pass rest to base class.
    virtual void Uninterpret(Command*);
    // uninterpret brush, pattern, and font commands, pass rest to base class.
    
    TextGraphic* GetText();
    // return pointer to graphic.
    
    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovtext_params;

friend OverlaysScript;
};

//: graphical view of TextOvComp.
class TextOvView : public OverlayView {
public:
    TextOvView(TextOvComp* = nil);

    virtual void Interpret(Command*);
    // interpret align-to-grid command, pass rest to base class.
    virtual void Update();

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    // create manipulator for creating and reshaping a text component.
    virtual Command* InterpretManipulator(Manipulator*);
    // interpret manipulator for creating and reshaping a text component.

    TextOvComp* GetTextOvComp();
    // return pointer to associated component.
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean TextChanged();
};

//: "PostScript" view of TextOvComp.
class TextPS : public OverlayPS {
public:
    TextPS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);
    // output "PostScript" fragment for this component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    const char* Filter(const char*, int);
};

//: serialized view of TextOvComp.
class TextScript : public OverlayScript {
public:
    TextScript(TextOvComp* = nil);

    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadText(istream&, void*, void*, void*, void*);
    // read text string and construct a TextGraphic.
};

#include <IV-2_6/_leave.h>

#endif
