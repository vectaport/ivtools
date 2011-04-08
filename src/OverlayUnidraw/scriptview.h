/*
 * Copyright (c) 1994-1997,1999 Vectaport Inc.
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

//: serialized view of OverlayComp.
// the OverlayScript class hierarchy is a tree of ExternView classes
// that are derived from OverlayPS and PostScriptView for convenience,
// to inherit a complete mechanism for writing and reading graphical data.
// <p>
// This capability is similar to the serialization mechanism of JavaBeans,
// but there is no need for a versioning system because the format used
// for the ASCII serialization supports arbitrary extension of an object's
// format by adding defaulted keyword-prefixed arguments.  
// <p>
// An older program can read newer formats, because the deserialization 
// is set up to convert unknown keyword arguments into attributes on 
// in a property list.  A newer program can read older formats by 
// using default values for missing keyword-prefixed arguments.
class OverlayScript : public OverlayPS { 
public: 
    OverlayScript(OverlayComp* = nil); 
    virtual ~OverlayScript();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    int Indent(ostream&, int extra = 0);
    // method to indent nested object output.

    virtual void MinGS(ostream&);
    // output a minimal description of a graphic state.
    virtual void FullGS(ostream&);
    // output a full description of a graphic state.
    virtual void TextGS(ostream&);
    // output a description of a graphic state adequate for a text graphic (TextGraphic).
    virtual void StencilGS(ostream&);
    // output a description of a graphic state adequate for a stencil graphic (UStencil).

    boolean DefaultGS();
    // return true if this is default gs, with no brush, colors, font, or pattern
    // specified.
    int MatchedGS(Clipboard*);
    // return index of where the graphic state of this component
    // matches the graphic state of a component in the clipboard.
    Iterator MatchedGS(Clipboard*, int&);
    // return iterator that points to where the graphic state of this component
    // matches the graphic state of a component in the clipboard.
    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    // output a decription of this component's graphic state, and append a
    // copy to a clipboard used to avoid outputting it twice.
    virtual Clipboard* GetGSList();
    // return pointer to clipboard of components with unique graphic states.

    int MatchedPts(Clipboard*);
    // return index of where the point list of this component
    // matches the point list of a component in the clipboard.
    Iterator MatchedPts(Clipboard*, int&);
    // return iterator that points to where the point list of this component
    // matches the point list of a component in the clipboard.
    virtual boolean EmitPts(ostream&, Clipboard*, boolean);
    // output a decription of this component's point list, and append a 
    // copy to a clipboard used to avoid outputting it twice.
    virtual Clipboard* GetPtsList();
    // return pointer to clipboard of components with unique point lists.
    
    int MatchedPic(Clipboard*);
    // return index of where the compound graphic of this component
    // matches the compound graphic of a component in the clipboard.
    Iterator MatchedPic(Clipboard*, int&);
    // return iterator that points to where the compound graphic of this component
    // matches the compound graphic of a component in the clipboard.
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    // output a decription of this component's compound graphic, and append a 
    // copy to a clipboard used to avoid outputting it twice.
    virtual Clipboard* GetPicList();
    // return pointer to clipboard of components with unique compound graphics.
    
    virtual boolean GetByPathnameFlag();
    // return flag that indicates whether to serialize component
    // by only a pathname or by the raw data.

    static int ReadGS(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // a description of graphic state.
    static int ReadFillBg(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // the fill-background flag of a graphic state.
    
    static int ReadNoneBr(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // the none-brush description.
    static int ReadBrush(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // all brush descriptions except for a none-brush.
    static int ReadFgColor(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // a foreground-color description.
    static int ReadBgColor(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // a background-color description.
    static int ReadFont(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // a font description.
    static int ReadNonePat(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // the none-pattern description.
    static int ReadPattern(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // all brush descriptions except for a none-pattern and gray-pattern.
    static int ReadGrayPat(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // the gray-pattern description.
    static int ReadTransform(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // a 6-parameter affine transform (a 2x3 matrix).

    static int ReadAnnotation(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // an annotation.
    static int ReadOther(istream&, void*, void*, void*, void*);
    // method used by OverlayComp istream constructor to deserialize 
    // any keyword-prefixed argument with unknown keyword symbol, by
    // adding the value to a components property list (an AttributeList).

    static boolean skip_comp(istream& in);
    // skip the text for the current component while de-serializing.

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

//: composite version of OverlayScript.
// a serialized view of OverlaysComp.
class OverlaysScript : public OverlayScript {
public:
    OverlaysScript(OverlaysComp* = nil);
    virtual ~OverlaysScript();

    virtual void Update();
    // construct all sub-views.

    OverlaysComp* GetOverlaysComp();
    // return pointer to associated component.

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);
    // set flags to indicate what should be compacted when serialized,
    // graphic state ('gs'), point lists ('pts'), or composite graphics ('pic').

    virtual boolean EmitGS(ostream&, Clipboard*, boolean);
    // iterate over sub-views, outputting decriptions of each unique graphic state.
    virtual boolean EmitPts(ostream&, Clipboard*, boolean);
    // iterate over sub-views, outputting decriptions of each unique point list.
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    // iterate over sub-views, outputting decriptions of each unique 
    // composite graphic.  This is not yet fully generalized, because there are
    // questions as to what that would mean.
    virtual boolean Definition(ostream&);
    // output variable-length ASCII record that defines the component.

    static int read_name(istream&, char* buf, int bufsiz);
    // static method to read the name of an object when de-serializing.
    static int read_gsptspic(const char* name, istream&, OverlaysComp* comps);
    // static method to read either a graphic state, a point list, or a
    // compound graphic when de-serializing.
    static OverlayComp* read_obj(const char* name, istream&, OverlaysComp* comps);
    // static method to read an arbitrary object when de-serializing.
    static int ReadChildren(istream&, void*, void*, void*, void*);
    // static method to read a list of arbitrary objects when de-serializing.
    static int ReadPic(istream&, void*, void*, void*, void*);
    // static method to read a composite graphic id (a pic), and substitute
    // the corresponding component from a list.

    virtual ExternView* GetView(Iterator);
    // get sub-view pointed to by the Iterator.
    virtual void SetView(ExternView*, Iterator&);
    // set sub-view pointed to by the Iterator.

    virtual OverlayScript* GetScript(Iterator);
    // get sub-view pointed to by the Iterator.
    virtual void SetScript(OverlayScript*, Iterator&);
    // set sub-view pointed to by the Iterator.

    virtual void First(Iterator&);
    // set Iterator to point to first sub-view.
    virtual void Last(Iterator&);
    // set Iterator to point to last sub-view.
    virtual void Next(Iterator&);
    // set Iterator to point to next sub-view.
    virtual void Prev(Iterator&);
    // set Iterator to point to prev sub-view.
    virtual boolean Done(Iterator);
    // return true if Iterator points off the front or end of list of sub-views.

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

protected:
    UList* Elem(Iterator);
    void DeleteView(Iterator&);
    void DeleteViews();

    UList* _views;
};

//: serialized view of top-level OverlayIdrawComp.
class OverlayIdrawScript : public OverlaysScript {
public:
    OverlayIdrawScript(OverlayIdrawComp* = nil);
    virtual ~OverlayIdrawScript();

    virtual void SetCompactions
    (boolean gs = false, boolean pts = false, boolean pic=false);
    // set flags to indicate what should be compacted when serialized,
    // graphic state ('gs'), point lists ('pts'), or composite graphics ('pic').
    virtual void SetByPathnameFlag(boolean);
    // set flag that indicates whether to serialize component
    // by only a pathname or by the raw data.
    virtual boolean GetByPathnameFlag();
    // return flag that indicates whether to serialize component
    // by only a pathname or by the raw data.

    virtual boolean Emit(ostream&);
    // serialize entire document to ostream.
    virtual boolean EmitPic(ostream&, Clipboard*, Clipboard*, boolean);
    // serialize a composite graphic.
    virtual Clipboard* GetGSList();
    // return pointer to list of graphic states that are stored in components.
    virtual Clipboard* GetPtsList();
    // return pointer to list of point lists that are stored in components.
    virtual Clipboard* GetPicList();
    // return pointer to list of composite graphics that are stored in components.

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
