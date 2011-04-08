#ifndef __comutil_h
#define __comutil_h
extern "C" {
#include <ComUtil/comutil.h>
}

//: define methods for a class name and class symbol id.
// adds ::class_name() and ::class_symid() based on 'name' to any 
// class definition.  For use in servers built on ComTerp for generating a
// unique id for a given type of component.
#define CLASS_SYMID(name) \
public: \
  static const char* class_name() {return name;}\
  static int class_symid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
  virtual int classid()\
    { if (_symid<0) _symid=symbol_add((char*)class_name()); return _symid;} \
protected: \
  static int _symid;
#endif /* !defined(__comutil.h) */

