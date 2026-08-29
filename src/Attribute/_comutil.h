#ifndef __comutil_h
#define __comutil_h
// extern "C" {
#include <ComUtil/comutil.h>
// }

//: one node per class that uses CLASS_SYMID/CLASS_SYMID2.
// The registrars below are the nodes -- each links itself in from its own
// constructor, so the list costs no allocation and has no capacity to run out
// of.  'iscomp' marks the CLASS_SYMID2 classes, the ones carrying a Unidraw
// ClassId.  List order is the order the dynamic initializers happened to run,
// which is unspecified across translation units: sort before reporting.
//
// The node holds the class NAME, not its symbol id, so that linking a class
// in costs nothing but pointer stores.  Interning here instead would put a
// call to symbol_add() in every translation unit that includes one of these
// headers -- including libraries that never link ComUtil (AttrGlyph is one),
// which then fail to link.  Whoever reads the list interns the names; symbol
// _add() is idempotent, so the ids match what class_symid() later returns.
struct ClassSymid {
  const char* classname;   /* not 'name' -- that is the macro's own parameter */
  int iscomp;
  ClassSymid* next;
};

//: head of the class registry, construct-on-first-use.
// A registrar running before main() finds it already initialized -- a
// file-scope object would instead be racing the other initializers.
inline ClassSymid*& class_symid_list() { static ClassSymid* head = 0; return head; }

//: define methods for a class name and class symbol id.
// adds ::class_name() and ::class_symid() based on 'name' to any 
// class definition.  For use in servers built on ComTerp for generating a
// unique id for a given type of component.  The class enrolls itself in the
// registry above before main(), so class(:all) can report it without anything
// of that class ever being constructed.
#define CLASS_SYMID(name) \
public: \
  static const char* class_name() {return name;}\
  static int class_symid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual int classid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual const char* GetClassName()\
  { return symbol_pntr(classid()); }	\
protected: \
  static int _symid; \
  struct _symid_reg_t : ClassSymid { \
    _symid_reg_t(int comp) \
      { classname = class_name(); iscomp = comp; \
        next = class_symid_list(); class_symid_list() = this; } }; \
  static inline _symid_reg_t _symid_reg{0};

//: define methods for a class name and class symbol id.
// adds ::class_name() and ::class_symid() based on 'name' to any 
// class definition.  For use in servers built on ComTerp for generating a
// unique id for a given type of component.
// Will eventually tie it to the Unidraw class inheritance system of ::IsA 
#define CLASS_SYMID2(name, uclass_id)  \
public: \
  static const char* class_name() {return name;}\
  static int class_symid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual int classid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual const char* GetClassName()\
  { return symbol_pntr(classid()); }		\
protected: \
  static int _symid; \
  struct _symid_reg_t : ClassSymid { \
    _symid_reg_t(int comp) \
      { classname = class_name(); iscomp = comp; \
        next = class_symid_list(); class_symid_list() = this; } }; \
  static inline _symid_reg_t _symid_reg{1};

//: define class name and class symbol id WITHOUT enrolling in the registry.
// For a class whose symbol is machinery -- read by is_object() to recognize an
// internal marker value -- rather than something a script can be handed.  It
// gets class_symid()/classid() like any other, but plants no registrar, so
// class(:all) does not offer the reader a name they can never hold a value of.
#define CLASS_SYMID_HIDDEN(name)  \
public: \
  static const char* class_name() {return name;}\
  static int class_symid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual int classid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual const char* GetClassName()\
  { return symbol_pntr(classid()); }	\
protected: \
  static int _symid;

#endif /* !defined(__comutil.h) */

