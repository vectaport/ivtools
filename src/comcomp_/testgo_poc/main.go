// Command testgo_poc proves a ComValue can round-trip through a live
// github.com/vectaport/flowgraph graph and back into ComTerp, typed as
// Go's `any` the whole way through the Go side (no stringifying it for
// the hop between flowgraph hubs the way the bridge itself stringifies
// for the hop between languages).
//
// It: pulls an int value out of a real ComTerp instance via the same
// bridge comcomp_'s other POCs use, pushes it through a two-hub
// flowgraph (Array -> identity -> Sink, each carrying the value as
// `any`), then writes the flowgraph's result directly into the backing
// memory of a ring FIFO (feed(string(n AnyType))) built in that same
// ComTerp instance -- no feed() call, no text crossing the bridge for
// that leg -- and reads it back out with next() (through the bridge) to
// confirm ComTerp's own view of the ring agrees with the direct write.
package main

/*
#cgo CXXFLAGS: -std=gnu++17 -I${SRCDIR}/../.. -I${SRCDIR}/../../include -I${SRCDIR}/../../include/ivstd -I${SRCDIR}/../../include/ACE-lite
#cgo LDFLAGS: -L${SRCDIR}/../../ComTerp/LINUX -lComTerp -L${SRCDIR}/../../ComUtil/LINUX -lComUtil -L${SRCDIR}/../../Attribute/LINUX -lAttribute -L${SRCDIR}/../../TopoFace/LINUX -lTopoFace -L${SRCDIR}/../../Time/LINUX -lTime -L${SRCDIR}/../../Unidraw-common/LINUX -lUnidraw-common -L${SRCDIR}/../../IV-common/LINUX -lIV-common -L${SRCDIR}/../../ACE-lite/LINUX -lACE-lite -Wl,-rpath,${SRCDIR}/../../ComTerp/LINUX -Wl,-rpath,${SRCDIR}/../../ComUtil/LINUX -Wl,-rpath,${SRCDIR}/../../Attribute/LINUX -Wl,-rpath,${SRCDIR}/../../TopoFace/LINUX -Wl,-rpath,${SRCDIR}/../../Time/LINUX -Wl,-rpath,${SRCDIR}/../../Unidraw-common/LINUX -Wl,-rpath,${SRCDIR}/../../IV-common/LINUX -Wl,-rpath,${SRCDIR}/../../ACE-lite/LINUX -lstdc++
#include "shim.h"
#include <stdlib.h>
#include <string.h>
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

// ringInfo looks up name as a top-level ComTerp variable and returns its
// ring layout -- the live head/tail/count/wrap ints and the backing
// buffer, not copies (see shim.h).
func ringInfo(h C.comterp_handle, name string) (C.comterp_ring_info, bool) {
	cname := C.CString(name)
	defer C.free(unsafe.Pointer(cname))
	var info C.comterp_ring_info
	ok := C.comterp_bridge_ring_info(h, cname, &info)
	return info, ok != 0
}

// directPushInt writes val straight into the ring's backing memory at its
// current tail slot and advances tail/count -- the same fullness check
// and wraparound ring_push_elt() (strmfunc.c) applies, just done from Go
// against the live ints shim.h's accessor handed back, with no feed()
// call and no text crossing the bridge. Returns false if the ring is full.
func directPushInt(info C.comterp_ring_info, val int64) bool {
	capSlots := int(info.cap)
	tail := int(*info.tail)
	count := int(*info.count)
	wrap := int(*info.wrap)

	if capSlots <= 0 || count >= capSlots || tail >= capSlots {
		return false
	}

	chunk := make([]byte, int(info.elemsz))
	C.comterp_bridge_encode_int(C.long(val), (*C.char)(unsafe.Pointer(&chunk[0])))

	dst := unsafe.Pointer(uintptr(unsafe.Pointer(info.buf)) + uintptr(tail*int(info.elemsz)))
	C.memcpy(dst, unsafe.Pointer(&chunk[0]), C.size_t(info.elemsz))

	newtail := tail + 1
	if newtail >= capSlots {
		if wrap != 0 {
			newtail = 0
		} else {
			newtail = capSlots
		}
	}
	*info.tail = C.int(newtail)
	*info.count = C.int(count + 1)
	return true
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

	// Push the flowgraph's result straight into ring memory -- no feed()
	// call, no text crossing the bridge for the data itself -- then read
	// it back with next() (still through the bridge) as an independent
	// check that ComTerp's own view of the ring agrees with what the
	// direct write did.
	ring, ok := ringInfo(h, "r")
	if !ok {
		fmt.Fprintln(os.Stderr, "testgo_poc: ring_info lookup for \"r\" failed")
		os.Exit(1)
	}
	outInt, ok := out.(int64)
	if !ok {
		fmt.Fprintf(os.Stderr, "testgo_poc: flowgraph result %v isn't an int64 (%T)\n", out, out)
		os.Exit(1)
	}
	if !directPushInt(ring, outInt) {
		fmt.Fprintln(os.Stderr, "testgo_poc: direct push refused, ring full")
		os.Exit(1)
	}
	landed, err := bridgeEval(h, "next(r)")
	if err != nil {
		fmt.Fprintf(os.Stderr, "testgo_poc: next(r): %s\n", err)
		os.Exit(1)
	}

	fmt.Printf("expr:                    %s\n", expr)
	fmt.Printf("ComTerp value:           %s\n", original)
	fmt.Printf("flowgraph result (any):  %v (%T)\n", out, out)
	fmt.Printf("landed in ring (direct write, bridge read): %s\n", landed)
	if landed != original {
		fmt.Fprintf(os.Stderr, "testgo_poc: MISMATCH: %q != %q\n", landed, original)
		os.Exit(1)
	}
	fmt.Println("round trip OK")
}
