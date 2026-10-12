// Command proplist_poc proves that a Go type can hold and mutate a live
// C++ AttributeList via a cgo handle (shim.h/shim.cc), rather than a
// translated copy. CppPropList implements flowgraph.PropList directly,
// so DupCppPropList clones a source PropList the same way
// flowgraph.DupPropList does -- switching Go-backed for C++-backed
// storage is then just calling a different constructor. GetField/
// SetField are stubbed as always-not-found/no-op pending a decision on
// the per-entry-field design (plain fields vs. a nested attrlist).
package main

/*
#cgo CXXFLAGS: -std=gnu++17 -I${SRCDIR}/../.. -I${SRCDIR}/../../include -I${SRCDIR}/../../include/ivstd -I${SRCDIR}/../../include/ACE-lite
#cgo linux LDFLAGS: -L${SRCDIR}/../../Attribute/LINUX -lAttribute -L${SRCDIR}/../../ComUtil/LINUX -lComUtil -L${SRCDIR}/../../Unidraw-common/LINUX -lUnidraw-common -L${SRCDIR}/../../IV-common/LINUX -lIV-common -Wl,-rpath,${SRCDIR}/../../Attribute/LINUX -Wl,-rpath,${SRCDIR}/../../ComUtil/LINUX -Wl,-rpath,${SRCDIR}/../../Unidraw-common/LINUX -Wl,-rpath,${SRCDIR}/../../IV-common/LINUX -lstdc++
#cgo darwin LDFLAGS: -L${SRCDIR}/../../Attribute/DARWIN -lAttribute -L${SRCDIR}/../../ComUtil/DARWIN -lComUtil -L${SRCDIR}/../../Unidraw-common/DARWIN -lUnidraw-common -L${SRCDIR}/../../IV-common/DARWIN -lIV-common -Wl,-rpath,${SRCDIR}/../../Attribute/DARWIN -Wl,-rpath,${SRCDIR}/../../ComUtil/DARWIN -Wl,-rpath,${SRCDIR}/../../Unidraw-common/DARWIN -Wl,-rpath,${SRCDIR}/../../IV-common/DARWIN
#include "shim.h"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"unsafe"

	"github.com/vectaport/flowgraph"
)

var _ flowgraph.PropList = (*CppPropList)(nil)

// CppPropList is a flowgraph.PropList-shaped wrapper around a live
// AttributeList held entirely on the C++ side; every method is a cgo
// call into shim.cc, never a Go-side copy of the data.
type CppPropList struct {
	h C.proplist_handle
}

func NewCppPropList() *CppPropList {
	return &CppPropList{h: C.proplist_new()}
}

func (p *CppPropList) Close() {
	C.proplist_free(p.h)
}

func (p *CppPropList) Get(key string) (any, bool) {
	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))
	var found C.int
	v := C.proplist_get(p.h, ckey, &found)
	if found == 0 {
		return nil, false
	}
	return C.GoString(v), true
}

func (p *CppPropList) Set(key string, v any) flowgraph.PropList {
	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))
	cval := C.CString(fmt.Sprint(v))
	defer C.free(unsafe.Pointer(cval))
	C.proplist_set(p.h, ckey, cval)
	return p
}

func (p *CppPropList) Delete(key string) (any, bool) {
	v, ok := p.Get(key)
	if !ok {
		return nil, false
	}
	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))
	C.proplist_delete(p.h, ckey)
	return v, true
}

func (p *CppPropList) Has(key string) bool {
	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))
	return C.proplist_has(p.h, ckey) != 0
}

func (p *CppPropList) Len() int {
	return int(C.proplist_len(p.h))
}

func (p *CppPropList) Keys() []string {
	n := p.Len()
	out := make([]string, n)
	for i := 0; i < n; i++ {
		out[i] = C.GoString(C.proplist_key_at(p.h, C.int(i)))
	}
	return out
}

func (p *CppPropList) Touch(key string) bool {
	ckey := C.CString(key)
	defer C.free(unsafe.Pointer(ckey))
	return C.proplist_touch(p.h, ckey) != 0
}

func (p *CppPropList) At(i int) (key string, v any, ok bool) {
	if i < 0 || i >= p.Len() {
		return "", nil, false
	}
	k := C.GoString(C.proplist_key_at(p.h, C.int(i)))
	val, _ := p.Get(k)
	return k, val, true
}

// Print renders through AttributeList::serialize(), the same code path
// ComTerp itself uses -- not a Go-side reconstruction of the output.
func (p *CppPropList) Print() string {
	return C.GoString(C.proplist_print(p.h))
}

// GetField, SetField: deferred -- see the AttributeList-nested-field
// design question this PoC doesn't settle. Always report not-found/
// no-op rather than panic, so CppPropList still satisfies PropList.
func (p *CppPropList) GetField(key, field string) (any, bool) { return nil, false }
func (p *CppPropList) SetField(key, field string, v any) bool { return false }

func (p *CppPropList) Empty() bool { return p == nil }
func (p *CppPropList) Base() any   { return p }

// DupCppPropList clones src via the PropList interface alone (the
// same At()-based loop as flowgraph.DupPropList), proving a caller
// can swap Go-backed for C++-backed storage by changing only the
// constructor it calls.
func DupCppPropList(src flowgraph.PropList) flowgraph.PropList {
	p := NewCppPropList()
	for i := 0; i < src.Len(); i++ {
		if k, v, ok := src.At(i); ok {
			p.Set(k, v)
		}
	}
	return p
}

func main() {
	p := NewCppPropList()
	defer p.Close()

	p.Set("a", "1").Set("b", "2").Set("c", "3")
	fmt.Printf("Keys() = %v\n", p.Keys())

	if !p.Touch("a") {
		panic("Touch(a) = false, want true")
	}
	fmt.Printf("Keys() after Touch(a) = %v\n", p.Keys())

	if k, v, ok := p.At(0); !ok || k != "b" || v != "2" {
		panic(fmt.Sprintf("At(0) = %v,%v,%v, want b,2,true", k, v, ok))
	}

	if v, ok := p.Delete("c"); !ok || v != "3" {
		panic(fmt.Sprintf("Delete(c) = %v,%v, want 3,true", v, ok))
	}
	fmt.Printf("Len() after Delete(c) = %d\n", p.Len())

	// Same live AttributeList, rendered by ComTerp's own serialize() --
	// proves this is the boundary-crossing case, not a translated copy.
	fmt.Printf("AttributeList::serialize() = %s\n", p.Print())

	// One-line swap: identical clone logic, Go-backed or C++-backed,
	// chosen only by which constructor is called.
	src := flowgraph.NewPropList()
	src.Set("x", "10").Set("y", "20")

	goClone := flowgraph.DupPropList(src)
	cppClone := DupCppPropList(src)
	defer cppClone.(*CppPropList).Close()

	fmt.Printf("goClone.Keys() = %v\n", goClone.Keys())
	fmt.Printf("cppClone.Keys() = %v\n", cppClone.Keys())
}
