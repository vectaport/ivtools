/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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
 * Overlay components
 */

#ifndef ovcomps_h
#define ovcomps_h

#include <UniIdraw/idcomp.h>
#include <InterViews/observe.h>

class AttributeList;
class AttributeValue;
class MultiLineObj;
class OverlayCatalog;
class OverlayView;
class PicturePS;
class ParamList;
class Observer;
class Observable;
class OverlaysComp;
class OverlayScript;
class OverlaysScript;
class Viewer;
class istream;

class OverlayComp : public GraphicComp, public Observer, public Observable {
public:
    OverlayComp(Graphic* g = nil, OverlayComp* parent = nil);
    OverlayComp(istream& in);
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    virtual ~OverlayComp();

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    boolean valid();
    const char* GetAnnotation();
    void SetAnnotation(const char*);
    OverlayView* FindView(Viewer*);
    virtual const char* GetBaseDir();

    virtual void SetPathName(const char*);
    virtual const char* GetPathName();
    virtual void SetByPathnameFlag(boolean);
    virtual boolean GetByPathnameFlag();
    virtual void AdjustBaseDir(const char* oldpath, const char* newpath);

    virtual Graphic* GetIndexedGS(int);
    virtual MultiLineObj* GetIndexedPts(int);
    virtual OverlaysComp* GetIndexedPic(int);

    virtual Component* GetParent();
    virtual void SetParent(Component* child, Component* parent);

    static boolean GraphicEquals(Graphic*, Graphic*);
    virtual boolean operator == (OverlayComp&);
    virtual boolean operator != (OverlayComp&);

    AttributeList* GetAttributeList(); // creates if necessary
    void SetAttributeList(AttributeList*);
    AttributeList* attrlist() { return _attrlist; }
    virtual AttributeValue* FindValue
      (const char* name, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);
    virtual AttributeValue* FindValue
      (int symid, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);

    virtual void Configure(Editor*);
    OverlayComp* TopComp();

    virtual void update(Observable*);  // adds in an extra
    virtual void Notify();             // subject-view mechanism
    virtual void notify() {Notify();}  // independent of comps and views					    
protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);

    static ParamList* _overlay_comp_params;
    boolean _valid;
    char* _anno;
    OverlayComp* _parent;
    AttributeList* _attrlist;

friend OverlayScript;
friend OverlaysScript;
};

class OverlaysComp : public OverlayComp {
public:
    OverlaysComp(OverlayComp* parent = nil);
    OverlaysComp(Graphic*, OverlayComp* parent = nil);
    OverlaysComp(istream&, OverlayComp* parent = nil);
    virtual ~OverlaysComp();

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual boolean Done(Iterator);

    virtual GraphicComp* GetComp(Iterator);
    virtual void SetComp(GraphicComp*, Iterator&);
    virtual void Bequeath();

    virtual void Append(GraphicComp*);
    virtual void Prepend(GraphicComp*);
    virtual void InsertBefore(Iterator, GraphicComp*);
    virtual void InsertAfter(Iterator, GraphicComp*);
    virtual void Remove(GraphicComp*);
    virtual void Remove(Iterator&);

    virtual void SetMobility(Mobility);

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void GrowIndexedGS(Graphic*);
    virtual void GrowIndexedPts(MultiLineObj*);
    virtual void GrowIndexedPic(OverlaysComp*);

    virtual void ResetIndexedGS();
    virtual void ResetIndexedPts();
    virtual void ResetIndexedPic();

    virtual boolean SamePicture(OverlaysComp*);
    virtual boolean operator == (OverlayComp&);

    virtual void AdjustBaseDir(const char* oldpath, const char* newpath);

    virtual AttributeValue* FindValue
      (int symid, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);

protected:
    OverlayComp* Comp(UList*);
    UList* Elem(Iterator);

    void SelectViewsOf(OverlayComp*, Editor*);
    void SelectClipboard(Clipboard*, Editor*);

    void StorePosition(OverlayComp*, Command*);
    void RestorePosition(OverlayComp*, Command*);

    void Group(Clipboard*, OverlayComp*, Command*);
    void Ungroup(OverlayComp*, Clipboard*, Command*);

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);

    static ParamList* _overlay_comps_params;

protected:
    UList* _comps;

friend OverlaysScript;
};

#include <OverlayUnidraw/indexmixins.h>

class OverlayIdrawComp : public OverlaysComp, 
			 public IndexedGsMixin,
			 public IndexedPtsMixin,
			 public IndexedPicMixin
{
public:
    OverlayIdrawComp(const char* pathname = nil, OverlayComp* parent = nil);
    OverlayIdrawComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);
    virtual ~OverlayIdrawComp();

    virtual Component* Copy();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void SetPathName(const char*);
    virtual const char* GetPathName();

    virtual void GrowIndexedGS(Graphic* gr) { grow_indexed_gs(gr); }
    virtual void GrowIndexedPts(MultiLineObj* ml) { grow_indexed_pts(ml); }
    virtual void GrowIndexedPic(OverlaysComp* comp) { grow_indexed_pic(comp); }

    virtual void ResetIndexedGS() { reset_indexed_gs(); }
    virtual void ResetIndexedPts() { reset_indexed_pts(); }
    virtual void ResetIndexedPic() { reset_indexed_pic(); }

    virtual Graphic* GetIndexedGS(int i) { return get_indexed_gs(i); }
    virtual MultiLineObj* GetIndexedPts(int i) { return get_indexed_pts(i); }
    virtual OverlaysComp* GetIndexedPic(int i) { return get_indexed_pic(i); }

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    virtual const char* GetBaseDir();

    static ParamList* _overlay_idraw_params;

protected:
    float _xincr, _yincr;    // anachronisms
    char* _pathname;
    char* _basedir;

friend OverlayCatalog;
};

inline boolean OverlayComp::valid() { return _valid; }

#endif



