/*
 * Copyright (c) 1994-1999 Vectaport Inc.
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
extern "C" {
#include <ComUtil/comutil.ci>
}

//: define methods for a class name and class symbol id.
// adds ::class_name() and ::class_symid() based on 'name' to any 
// class definition.  For use in servers built on ComTerp for generating a
// unique id for a given type of component.
#define classid(name) \
public: \
  static const char* class_name() {return name;}\
  static int class_symid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
protected: \
  static int _symid;

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

//: derived GraphicComp
// derived GraphicComp  with extensions for property lists of arbitrary 
// AttributeValue objects, event propogation inherited from Observer and 
// Observable, and new persistence mechanisms (serialization) derived from 
// ExternView (OverlayScript).
class OverlayComp : public GraphicComp, public Observer, public Observable {
public:
    OverlayComp(Graphic* g = nil, OverlayComp* parent = nil);
    // optional graphic and parent component used to initialize component.
    OverlayComp(istream& in);
    // component initialized by reading from 'in' (de-serialization).
    virtual ClassId GetClassId();
    // original Unidraw class id system, based on #define's in *classes.h file.
    virtual boolean IsA(ClassId);
    // original Unidraw class id system, based on #define's in *classes.h file.
    virtual ~OverlayComp();

    virtual void Interpret(Command*);
    // pass command to component for interpretation.
    virtual void Uninterpret(Command*);
    // pass command to component for reverse interpretation.

    boolean valid();
    // for checking if istream constructor was successfull (if de-serialization
    // was successful).							   
    const char* GetAnnotation();
    // return pointer to annotation string for this component.
    void SetAnnotation(const char*);
    // set the annotation string for this component.
    OverlayView* FindView(Viewer*);
    // return the view for this compoent (for this subject) in a specific viewer.

    virtual void SetPathName(const char*);
    // set pathname associated with this component.
    virtual const char* GetPathName();
    // get pathname associated with this component.
    virtual void SetByPathnameFlag(boolean);
    // set flag that determines whether component will be serialized (converted to
    // external persistent storage) by just the pathname or by the internal contents.
    virtual boolean GetByPathnameFlag();
    // return by-pathname flag
    virtual const char* GetBaseDir();
    // set base directory used for generating pathnames for this component
    // and all of its children.
    virtual void AdjustBaseDir(const char* oldpath, const char* newpath);
    // adjust base directory used for generating pathnames for this component,
    // done when a document is saved to a new location.

    virtual Graphic* GetIndexedGS(int);
    // return graphic state (gs) from table, a use of IndexedGsMixin.
    virtual MultiLineObj* GetIndexedPts(int);
    // return point-list (pts) from table, a use of IndexedPtsMixin.
    virtual OverlaysComp* GetIndexedPic(int);
    // return compound-graphic (pic) from table, a use of IndexedPicMixin.

    virtual Component* GetParent();
    // get pointer to parent component.
    virtual void SetParent(Component* child, Component* parent);
    // set pointer to 'parent' component on 'child' component.

    static boolean GraphicEquals(Graphic* a, Graphic* b);
    // compare two graphics, 'a' and 'b', for equality.
    virtual boolean operator == (OverlayComp&);
    // compare another component, including its graphic, for equality to this one.
    virtual boolean operator != (OverlayComp&);
    // compare another component, including its graphic, for non-equality to this one.

    AttributeList* GetAttributeList(); 
    // return pointer to associated AttributeList (property list made up of
    // arbitrary AttributeValue objects), creating if necessary.
    void SetAttributeList(AttributeList*);
    // set new property list for component, de-referencing the old one, and
    // increment the reference-count of the new one.
    AttributeList* attrlist() { return _attrlist; }
    // return pointer to property list, without allocating a new one if it is nil.
    virtual AttributeValue* FindValue
      (const char* name, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);
    // search component tree for specified attribute value by 'name'.  Only
    // default argument mode implemented so far -- return first occurence found
    // with a downward depth-first search.
    virtual AttributeValue* FindValue
      (int symid, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);
    // search component tree for specified attribute value by 'symid'.  Only
    // default argument mode implemented so far -- return first occurence found
    // with a downward depth-first search.

    virtual void Configure(Editor*);
    // to be filled in by derived class.  Useful for initializing a component
    // after the Editor has been constructed.								    
    OverlayComp* TopComp();
    // returns pointer to top-level component in the component tree.

    virtual void update(Observable*);  
    // update method for an Observer/Observable design pattern.
    virtual void notify() {Notify();} 
    // notify method for an  Observer/Observable design pattern.
    virtual void Notify(); 
    // method specialized from Component that incorporates the Observer/Observable
    // notification with the original Unidraw notification.
protected:
    ParamList* GetParamList();
    // return ParamList of required/optional/keyword arguments to be read
    // from an external (serialized) representation in the istream constructor
    // of a component.  Includes pointers to static methods that read portions of
    // an istream.  keyword arguments (or fields) not found in the ParamList are 
    // used to construct a new entry in the components property list instead of
    // being handled by one of the static methods.
    void GrowParamList(ParamList*);
    // construct the ParamList for this class if one does not exist, optionally
    // using similar base class methods to assist.

    static ParamList* _overlay_comp_params;
    // static holder for this classes ParamList.

    boolean _valid;
    char* _anno;
    OverlayComp* _parent;
    AttributeList* _attrlist;

friend OverlayScript;
friend OverlaysScript;
};

//: composite component, clone of GraphicComps derived from OverlayComp
// composite component that manages a list of sub-components.
class OverlaysComp : public OverlayComp {
public:
    OverlaysComp(OverlayComp* parent = nil);
    OverlaysComp(Graphic*, OverlayComp* parent = nil);
    OverlaysComp(istream&, OverlayComp* parent = nil);
    virtual ~OverlaysComp();

    virtual void Interpret(Command*);
    // interpret command, possibly across sub-components.
    virtual void Uninterpret(Command*);
    // uninterpret command, possibly across sub-components.

    virtual void First(Iterator&);
    // set Iterator to first element in the list.
    virtual void Last(Iterator&);
    // set Iterator to last element in the list.
    virtual void Next(Iterator&);
    // move Iterator to the next element in the list.
    virtual void Prev(Iterator&);
    // move Iterator to the previous element in the list.
    virtual boolean Done(Iterator);
    // test Iterator to see if off the end of the list.

    virtual GraphicComp* GetComp(Iterator);
    // return pointer to component within the current element of the list
    // as pointed to by the Iterator.
    virtual void SetComp(GraphicComp*, Iterator&);
    // set Iterator to point to the element of the list that contains the
    // given component.
    virtual void Bequeath();
    // pass graphic state of this component's graphic to children.

    virtual void Append(GraphicComp*);
    // append component to the end of the list, which is foremost when rendered.
    virtual void Prepend(GraphicComp*);
    // append component to beginning of the list, which is rearmost when rendered.
    virtual void InsertBefore(Iterator, GraphicComp*);
    // insert component before current position indicated by the Iterator,
    // which means to the rear in screen ordering.
    virtual void InsertAfter(Iterator, GraphicComp*);
    // insert component after current position indicated by the Iterator,
    // which means to the fore in screen ordering.
    virtual void Remove(GraphicComp*);
    // search for and remove component from list.
    virtual void Remove(Iterator&);
    // remove component indicated by Iterator.

    virtual void SetMobility(Mobility);
    // unused mechanism to restrain motion of component.

    virtual Component* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void GrowIndexedGS(Graphic*);
    // grow table of graphic states (gs) managed by IndexedGsMixin.
    virtual void GrowIndexedPts(MultiLineObj*);
    // grow table of point lists (pts) managed by IndexedPtsMixin.
    virtual void GrowIndexedPic(OverlaysComp*);
    // grow table of composite graphics (pic) managed by IndexedPicMixin.

    virtual void ResetIndexedGS();
    // reset table of graphic states (gs) managed by IndexedGsMixin.
    virtual void ResetIndexedPts();
    // reset table of point lists (pts) managed by IndexedPtsMixin.
    virtual void ResetIndexedPic();
    // reset table of composite graphics (pic) managed by IndexedPicMixin.

    virtual boolean SamePicture(OverlaysComp*);
    // compares two composite components, to see if sub-components are identical.
    virtual boolean operator == (OverlayComp&);
    // checks for equality of this component and sub-components.

    virtual void AdjustBaseDir(const char* oldpath, const char* newpath);
    // adjust base directory used for generating pathnames when saving
    // components to disk (serialization).

    virtual AttributeValue* FindValue
      (const char* name, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);
    // search component tree for specified attribute value by 'name'.  Only
    // default argument mode implemented so far -- return first occurence found
    // with a downward depth-first search.
    virtual AttributeValue* FindValue
      (int symid, boolean last = false, boolean breadth = false, 
       boolean down = true, boolean up = false);
    // search component tree for specified attribute value by 'symid'.  Only
    // default argument mode implemented so far -- return first occurence found
    // with a downward depth-first search.

protected:
    OverlayComp* Comp(UList*);
    UList* Elem(Iterator);
    // get at individual element of composite component list mechanism.

    void SelectViewsOf(OverlayComp*, Editor*);
    // add views of this component to selection list in this editor.
    void SelectClipboard(Clipboard*, Editor*);
    // add views of components in clipboard to selection list in this editor.

    void StorePosition(OverlayComp*, Command*);  
    // store position of this component in composite list for later reference
    // by a command.
    void RestorePosition(OverlayComp*, Command*); 
    // restore position of this component in composite list from command.

    void Group(Clipboard*, OverlayComp*, Command*);
    // group everything in the clipboard into a new OverlaysComp
    void Ungroup(OverlayComp*, Clipboard*, Command*);
    // ungroup everything in an OverlaysComp into the clipboard.

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);

    static ParamList* _overlay_comps_params;

protected:
    UList* _comps;

friend OverlaysScript;
};

#include <OverlayUnidraw/indexmixins.h>

//: top-level OverlaysComp, the root of a component tree or document.
class OverlayIdrawComp : public OverlaysComp, 
			 public IndexedGsMixin,
			 public IndexedPtsMixin,
			 public IndexedPicMixin
{
public:
    OverlayIdrawComp(const char* pathname = nil, OverlayComp* parent = nil);
    // construct component tree from 'pathname', with optional 'parent'
    // to graft onto.
    OverlayIdrawComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);
    // de-serialize component tree from istream, remembering optional 'pathname'
    // if available, with optional 'parent' to graft onto.
    virtual ~OverlayIdrawComp();

    virtual Component* Copy();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void SetPathName(const char*);
    // set pathname associated with this component tree.  Affects saving out.
    virtual const char* GetPathName();
    // return pathname associated with this component tree.

    virtual void GrowIndexedGS(Graphic* gr) { grow_indexed_gs(gr); }
    // grow table of graphic states (gs) managed by IndexedGsMixin.
    virtual void GrowIndexedPts(MultiLineObj* ml) { grow_indexed_pts(ml); }
    // grow table of point lists (pts) managed by IndexedPtsMixin.
    virtual void GrowIndexedPic(OverlaysComp* comp) { grow_indexed_pic(comp); }
    // grow table of composite graphics (pic) managed by IndexedPicMixin.

    virtual void ResetIndexedGS() { reset_indexed_gs(); }
    // reset table of graphic states (gs) managed by IndexedGsMixin.
    virtual void ResetIndexedPts() { reset_indexed_pts(); }
    // reset table of point lists (pts) managed by IndexedPtsMixin.
    virtual void ResetIndexedPic() { reset_indexed_pic(); }
    // reset table of composite graphics (pic) managed by IndexedPicMixin.

    virtual Graphic* GetIndexedGS(int i) { return get_indexed_gs(i); }
    // return graphic state (gs) from table, a use of IndexedGsMixin.
    virtual MultiLineObj* GetIndexedPts(int i) { return get_indexed_pts(i); }
    // return point-list (pts) from table, a use of IndexedPtsMixin.
    virtual OverlaysComp* GetIndexedPic(int i) { return get_indexed_pic(i); }
    // return compound-graphic (pic) from table, a use of IndexedPicMixin.

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    virtual const char* GetBaseDir();

    static ParamList* _overlay_idraw_params;

protected:
    float _xincr; // x grid dimensions
    float _yincr; // y grid dimensions
    char* _pathname;
    char* _basedir;

friend OverlayCatalog;
};

inline boolean OverlayComp::valid() { return _valid; }


#include <Unidraw/Graphic/graphic.h>

//: for the purpose of sliding in a derived Painter
class OverlayGraphic : public Graphic {
public:
  static void new_painter();
  // create an OverlayPainter to be used a the default Painter by the Graphic class.
};

#endif



