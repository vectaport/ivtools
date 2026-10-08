// Command gencheck drives the funccompile.go spike standalone: fetch the
// real postfix(:tree) for the insertion-sort func through the bridge (the
// same shim bridge_poc/compile_poc already proved), compile it to Go, build
// and run the result, and compare it against the interpreted func's own
// output on the same input -- before any of this is wired into a ComTerp
// built-in command.
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
	"path/filepath"
	"unsafe"
)

const isortSrc = `func(lst;n=size(lst);for(i=1 i<n i=i+1 key=lst@i;j=i-1;while(j>=0&&lst@j>key lst@(j+1)=lst@j;j=j-1);lst@(j+1)=key);lst)`

func bridgeEval(h C.comterp_handle, expr string) (string, error) {
	cexpr := C.CString(expr)
	defer C.free(unsafe.Pointer(cexpr))
	result := C.comterp_bridge_eval(h, cexpr)
	if result == nil {
		return "", fmt.Errorf("%s", C.GoString(C.comterp_bridge_errmsg(h)))
	}
	return C.GoString(result), nil
}

func main() {
	h := C.comterp_bridge_new()
	defer C.comterp_bridge_free(h)

	treeText, err := bridgeEval(h, "postfix("+isortSrc+" :tree)")
	if err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: getting tree: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("tree: %s\n", treeText)

	tree, err := parseTree(treeText)
	if err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: parsing tree: %s\n", err)
		os.Exit(1)
	}

	body, err := compileSortFunc(tree)
	if err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: compiling: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("generated Go body:\n%s\n", body)

	if err := os.WriteFile("generated_body.txt", []byte(body), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: %s\n", err)
		os.Exit(1)
	}

	genDir, err := filepath.Abs("generated")
	if err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: %s\n", err)
		os.Exit(1)
	}
	if err := writeAndRunSortProgram(genDir, body, 2000); err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: generated sort program failed: %s\n", err)
		os.Exit(1)
	}

	gosortDir, err := filepath.Abs("gosort")
	if err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: %s\n", err)
		os.Exit(1)
	}
	if err := writeGoArchivePkg(gosortDir, body); err != nil {
		fmt.Fprintf(os.Stderr, "gencheck: building c-archive: %s\n", err)
		os.Exit(1)
	}
	fmt.Println("libgosort.a + libgosort.h built from the compiled func body")
}
