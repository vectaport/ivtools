// Command testgo_poc proves a ComValue can round-trip through a live
// github.com/vectaport/flowgraph graph and back into ComTerp, typed as
// Go's `any` the whole way through the Go side (no stringifying it for
// the hop between flowgraph hubs the way the bridge itself stringifies
// for the hop between languages).
//
// It: pulls an int value out of a real ComTerp instance via the same
// bridge comcomp_'s other POCs use, pushes it through a two-hub
// flowgraph (Array -> Pass -> Sink, each carrying the value as `any`),
// and feeds the result the flowgraph produced back into a ComTerp ring
// (feed(string(n AnyType))) built in that same ComTerp instance -- then
// reads it back out with next() to confirm it landed.
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
	"strconv"
	"unsafe"

	"github.com/vectaport/flowgraph"
)

func bridgeEval(h C.comterp_handle, expr string) (string, error) {
	cexpr := C.CString(expr)
	defer C.free(unsafe.Pointer(cexpr))
	result := C.comterp_bridge_eval(h, cexpr)
	if result == nil {
		return "", fmt.Errorf("%s", C.GoString(C.comterp_bridge_errmsg(h)))
	}
	return C.GoString(result), nil
}

// capture is the flowgraph Sinker that pulls the round-tripped value back
// out of the graph; Run() has returned by the time anything reads it.
type capture struct {
	value any
	got   bool
}

func (c *capture) Sink(source []any) {
	c.value = source[0]
	c.got = true
}

// identity is a one-source, one-result Transformer that hands its input
// straight through -- the HubCode.Pass shortcut can't be used here since
// its generic passthrough (flowgraph.go's nil-FireFunc fallback) never
// recognizes EOS, so the hub it builds never terminates; OneOf's fire
// func (flowgraph.go's oneOfFire) does that recognition itself and only
// asks the Transformer for the identity part.
type identity struct{}

func (identity) Transform(h flowgraph.Hub, source []any) (result []any, err error) {
	return source, nil
}

// roundtrip pushes val through a single-value Array -> (identity hub) ->
// Sink flowgraph, carrying it as `any` across every hub and pipe, and
// returns what the sink received.
func roundtrip(val any) any {
	fg := flowgraph.New("testgo")

	src := fg.NewHub("val", flowgraph.Array, []any{val}).
		SetResultNames("X")

	pass := fg.NewHub("pass", flowgraph.OneOf, identity{}).
		SetSourceNames("A").
		SetResultNames("X")

	c := &capture{}
	sink := fg.NewHub("sink", flowgraph.Sink, c).
		SetSourceNames("A")

	fg.Connect(src, "X", pass, "A")
	fg.Connect(pass, "X", sink, "A")

	fg.Run()

	if !c.got {
		fmt.Fprintln(os.Stderr, "testgo_poc: flowgraph sink never received a value")
		os.Exit(1)
	}
	return c.value
}

func main() {
	expr := "42"
	if len(os.Args) > 1 {
		expr = os.Args[1]
	}

	h := C.comterp_bridge_new()
	defer C.comterp_bridge_free(h)

	// A ring built over an AnyType buffer boxes a value whole rather
	// than converting it, so it can hold whatever comes back from the
	// flowgraph unchanged -- see src/comterp_/tests/feedring.comt test 30.
	if _, err := bridgeEval(h, "r=feed(string(4 AnyType))"); err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: building ring: %s\n", err)
		os.Exit(1)
	}

	original, err := bridgeEval(h, expr)
	if err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: evaluating %q: %s\n", expr, err)
		os.Exit(1)
	}
	n, err := strconv.ParseInt(original, 10, 64)
	if err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: %q isn't an integer ComValue (only int is wired up so far): %s\n", original, err)
		os.Exit(1)
	}

	out := roundtrip(n)

	pushExpr := fmt.Sprintf("feed(r %v)", out)
	if _, err := bridgeEval(h, pushExpr); err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: %s: %s\n", pushExpr, err)
		os.Exit(1)
	}
	landed, err := bridgeEval(h, "next(r)")
	if err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: next(r): %s\n", err)
		os.Exit(1)
	}

	fmt.Printf("expr:                  %s\n", expr)
	fmt.Printf("ComTerp value:         %s\n", original)
	fmt.Printf("flowgraph result (any): %v (%T)\n", out, out)
	fmt.Printf("landed in ring:        %s\n", landed)
	if landed != original {
		fmt.Fprintf(os.Stderr, "testgo_poc: MISMATCH: %q != %q\n", landed, original)
		os.Exit(1)
	}
	fmt.Println("round trip OK")
}
