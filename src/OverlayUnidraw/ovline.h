/*
 * Copyright (c) 1994 Vectaport Inc.
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
 * OvLine and OvMultiLine component declarations.
 */

#ifndef overlay_line_h
#define overlay_line_h

#include <OverlayUnidraw/ovvertices.h>
#include <OverlayUnidraw/scriptview.h>

#include <IV-2_6/_enter.h>

class Line;
class ParamList;
class SF_MultiLine;

class LineOvComp : public OverlayComp {
public:
    LineOvComp(Line* = nil, OverlayComp* parent = nil);
    LineOvComp(istream&, OverlayComp* parent = nil);

    virtual void Interpret(Command*);
    Line* GetLine();

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual boolean operator == (OverlayComp&);
protected:
     ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovline_params;

};

class LineOvView : public OverlayView {
public:
    LineOvView(LineOvComp* = nil);

    virtual void Interpret(Command*);
    virtual void Update();

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual Command* InterpretManipulator(Manipulator*);

    virtual void GetEndpoints(Coord&, Coord&, Coord&, Coord&);
    LineOvComp* GetLineOvComp();
    virtual Graphic* GetGraphic();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void CreateHandles();
};

class LinePS : public OverlayPS {
public:
    LinePS(OverlayComp* = nil);

    virtual boolean Definition(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class LineScript : public OverlayScript {
public:
    LineScript(LineOvComp* = nil);

    virtual boolean Definition(ostream&);
    static int ReadOriginal(istream&, void*, void*, void*, void*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class MultiLineOvComp : public VerticesOvComp {
public:
    MultiLineOvComp(SF_MultiLine* = nil, OverlayComp* parent = nil);
    MultiLineOvComp(istream&, OverlayComp* parent = nil);

    SF_MultiLine* GetOvMultiLine();

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _ovmultiline_params;

};

class MultiLineOvView : public VerticesOvView {
public:
    MultiLineOvView(MultiLineOvComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual Command* InterpretManipulator(Manipulator*);
    MultiLineOvComp* GetMultiLineOvComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean VertexChanged();
};

class MultiLinePS : public VerticesPS {
public:
    MultiLinePS(OverlayComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual const char* Name();
};

class MultiLineScript : public VerticesScript {
public:
    MultiLineScript(MultiLineOvComp* = nil);

    virtual boolean Definition(ostream&);
    static int ReadPoints(istream&, void*, void*, void*, void*);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    virtual const char* Name();
};

#include <IV-2_6/_leave.h>

#endif
