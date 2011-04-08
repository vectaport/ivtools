/*
 * Copyright (c) 1994-1997 Vectaport Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  The copyright holders make 
 * no representation about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * OverlayScript - command-oriented external representation of component.
 */

#ifndef script_view_h
#define script_view_h

#include <OverlayUnidraw/ovpsview.h>

class Clipboard;
class OverlayComp;
class OverlaysComp;
class OverlayIdrawComp;
class istream;
class ostream;

class OverlayScript : public OverlayPS {
public:
    OverlayScript(OverlayComp* = nil);
    virtual ~OverlayScript();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    int Indent(ostream&, int extra = 0);

    virtual void MinGS(ostream&);
    virtual void FullGS(ostream&);
    virtual void TextGS(ostream&);
    virtual void StencilGS(ostream&);

    boolean DefaultGS();
    int MatchedGS(Clipboard*);
    Iterator MatchedGS(Clipboard*, int&);
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    virtual Clipboard* GetGSList();

    int MatchedPts(Clipboard*);
    Iterator MatchedPts(Clipboard*, int&);
    virtual boolean EmitPts(ostream&, Clipboard*, boolean);
    virtual Clipboard* GetPtsList();
    
    int MatchedPic(Clipboard*);
    Iterator MatchedPic(Clipboard*, int&);
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    virtual Clipboard* GetPicList();
    
    virtual boolean GetByPathnameFlag();

    static int ReadGS(istream&, void*, void*, void*, void*);
    static int ReadFillBg(istream&, void*, void*, void*, void*);
    
    static int ReadNoneBr(istream&, void*, void*, void*, void*);
    static int ReadBrush(istream&, void*, void*, void*, void*);
    static int ReadFgColor(istream&, void*, void*, void*, void*);
    static int ReadBgColor(istream&, void*, void*, void*, void*);
    static int ReadFont(istream&, void*, void*, void*, void*);
    static int ReadNonePat(istream&, void*, void*, void*, void*);
    static int ReadPattern(istream&, void*, void*, void*, void*);
    static int ReadGrayPat(istream&, void*, void*, void*, void*);
    static int ReadTransform(istream&, void*, void*, void*, void*);

    static int ReadAnnotation(istream&, void*, void*, void*, void*);

    static int ReadOther(istream&, void*, void*, void*, void*);

protected:
    virtual void FillBg(ostream& out);
    virtual void Brush(ostream& out);
    virtual void FgColor(ostream& out);
    virtual void BgColor(ostream& out);
    virtual void Font(ostream& out);
    virtual void Pattern(ostream& out);
    virtual void Transformation(ostream& out);
    virtual void Transformation(ostream& out, char* keyword, Graphic* gr = nil);
    virtual void Annotation(ostream& out);

    void Attributes(ostream& out);

    OverlayScript* CreateOverlayScript(OverlayComp*);

    virtual ComponentView* GetParent();
    virtual void SetParent(ComponentView* child, ComponentView* parent);

    OverlayScript* _parent;
};

class OverlaysScript : public OverlayScript {
public:
    OverlaysScript(OverlaysComp* = nil);
    virtual ~OverlaysScript();

    virtual void Update();
    OverlaysComp* GetOverlaysComp();

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);

    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    virtual boolean EmitPts(ostream&, Clipboard*, boolean);
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    virtual boolean Definition(ostream&);

    static int read_name(istream&, char* buf, int bufsiz);
    static int read_gsptspic(const char* name, istream&, OverlaysComp* comps);
    static OverlayComp* read_obj(const char* name, istream&, OverlaysComp* comps);
    static int ReadChildren(istream&, void*, void*, void*, void*);
    static int ReadPic(istream&, void*, void*, void*, void*);

    virtual ExternView* GetView(Iterator);
    virtual void SetView(ExternView*, Iterator&);

    virtual OverlayScript* GetScript(Iterator);
    virtual void SetScript(OverlayScript*, Iterator&);

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual boolean Done(Iterator);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    OverlayComp* Ellipse(istream&, OverlayComp* parent = nil);

protected:
    UList* Elem(Iterator);
    void DeleteView(Iterator&);
    void DeleteViews();

    UList* _views;
};

class OverlayIdrawScript : public OverlaysScript {
public:
    OverlayIdrawScript(OverlayIdrawComp* = nil);
    virtual ~OverlayIdrawScript();

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);
    virtual void SetByPathnameFlag(boolean);
    virtual boolean GetByPathnameFlag();

    virtual boolean Emit(ostream&);
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    virtual Clipboard* GetGSList();
    virtual Clipboard* GetPtsList();
    virtual Clipboard* GetPicList();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    Clipboard* _gslist;
    Clipboard* _ptslist;
    Clipboard* _piclist1;
    Clipboard* _piclist2;
    boolean _gs_compacted;
    boolean _pts_compacted;
    boolean _pic_compacted;
    boolean _by_pathname;
};

#endif
