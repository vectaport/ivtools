/*
 * Copyright (c) 1995 Vectaport Inc.
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
 * Text from File component declarations.
 */

#ifndef textfile_h
#define textfile_h

#include <OverlayUnidraw/ovtext.h>

#include <IV-2_6/_enter.h>

class TextFileScript;

class TextFileComp : public TextOvComp {
public:
    TextFileComp(const char* pathname, const char* begstr,
		 const char* endstr, int linewidth = -1, Graphic* gs = nil,
		 OverlayComp* parent = nil);
    TextFileComp(istream&, OverlayComp* parent = nil);
    virtual ~TextFileComp();

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    const char* GetPathname() { return _pathname; }
    const char* GetBegstr() { return _begstr; }
    const char* GetEndstr() { return _endstr; }
    int GetLineWidth() { return _linewidth; }

    void Init();
    virtual boolean operator == (OverlayComp&);
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _textfile_params;

    char* _pathname;
    char* _begstr;
    char* _endstr;
    int _linewidth;

friend TextFileScript;
};

class TextFileView : public TextOvView {
public:
    TextFileView(TextFileComp* = nil);

    virtual Manipulator* CreateManipulator(Viewer*,Event&,Transformer*,Tool*);
    virtual Command* InterpretManipulator(Manipulator*);

    TextFileComp* GetTextFileComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class TextFileScript : public TextScript {
public:
    TextFileScript(TextFileComp* = nil);

    virtual boolean Definition(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadTextFile(istream&, void*, void*, void*, void*);
};

#include <IV-2_6/_leave.h>

#endif
