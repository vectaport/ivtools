// Command bridge_poc proves a Go program can drive a real ComTerp
// instance through a flat C shim (shim.h/shim.cc), with no C++ linked
// directly into Go -- cgo only understands C, so the shim is the whole
// C++/Go boundary.
package main

/*
#cgo CXXFLAGS: -std=gnu++17 -I${SRCDIR}/../.. -I${SRCDIR}/../../include -I${SRCDIR}/../../include/ivstd -I${SRCDIR}/../../include/ACE-lite
#cgo LDFLAGS: -L${SRCDIR}/../../ComTerp/LINUX -lComTerp -L${SRCDIR}/../../ComUtil/LINUX -lComUtil -L${SRCDIR}/../../Attribute/LINUX -lAttribute -L${SRCDIR}/../../TopoFace/LINUX -lTopoFace -L${SRCDIR}/../../Time/LINUX -lTime -L${SRCDIR}/../../Unidraw-common/LINUX -lUnidraw-common -L${SRCDIR}/../../IV-common/LINUX -lIV-common -L${SRCDIR}/../../ACE-lite/LINUX -lACE-lite -Wl,-rpath,${SRCDIR}/../../ComTerp/LINUX -Wl,-rpath,${SRCDIR}/../../ComUtil/LINUX -Wl,-rpath,${SRCDIR}/../../Attribute/LINUX -Wl,-rpath,${SRCDIR}/../../TopoFace/LINUX -Wl,-rpath,${SRCDIR}/../../Time/LINUX -Wl,-rpath,${SRCDIR}/../../Unidraw-common/LINUX -Wl,-rpath,${SRCDIR}/../../IV-common/LINUX -Wl,-rpath,${SRCDIR}/../../ACE-lite/LINUX -lstdc++
#include "shim.h"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"os"
	"unsafe"
)

func main() {
	expr := "1+2*3"
	if len(os.Args) > 1 {
		expr = os.Args[1]
	}

	h := C.comterp_bridge_new()
	defer C.comterp_bridge_free(h)

	cexpr := C.CString(expr)
	defer C.free(unsafe.Pointer(cexpr))

	result := C.comterp_bridge_eval(h, cexpr)
	if result == nil {
		fmt.Fprintf(os.Stderr, "concomp bridge: %s\n", C.GoString(C.comterp_bridge_errmsg(h)))
		os.Exit(1)
	}
	fmt.Printf("%s => %s\n", expr, C.GoString(result))
}
