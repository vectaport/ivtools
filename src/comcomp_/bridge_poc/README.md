# bridge_poc

Proof of concept for a Go-to-C++ calling path into ComTerp, for the
planned `comcomp` tool (a `.comt`-to-Go generator, sibling to `comlint`).

cgo only understands C, never C++, so `shim.h`/`shim.cc` is a flat
`extern "C"` layer in front of `ComTerpServ`; `main.go` calls through it
via cgo. `go build` compiles `shim.cc` with the C++ compiler automatically
because it's a `.cc` file in a cgo package directory -- no separate build
step or Makefile needed.

## Building

Requires a full `./configure && make` of the ivtools tree first (this POC
links against the `.so` files that build produces; it is not wired into
the imake build yet). Then:

    cd src/comcomp_/bridge_poc
    go build -o bridge_poc .
    ./bridge_poc '1+2*3'        # => 1+2*3 => 7
    ./bridge_poc '(1,2,3)'      # => (1,2,3) => {1,2,3}

The `#cgo LDFLAGS` paths assume the in-tree LINUX build layout
(`src/*/LINUX/lib*.so`); another platform's object-dir name would need
the same substitution comlint's own `program` dispatch does.

## Known issue found along the way

`ComValue::String()` (`src/ComTerp/comvalue.c:526`) streams `this`
instead of `*this`, so it prints the object's address instead of its
value -- reproduced with a standalone C++ program outside Go/cgo
entirely, so it's not a bridge artifact. `shim.cc` works around it by
streaming the `ComValue` through `operator<<` directly rather than
calling `String()`. Not fixed here pending a decision on the real fix.
