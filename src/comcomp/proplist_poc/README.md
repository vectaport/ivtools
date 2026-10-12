# proplist_poc

Proof of concept for a Go type (`CppPropList`) backed by a live C++
`AttributeList`, reached through a flat `extern "C"` shim
(`shim.h`/`shim.cc`) the same way `comcomp/bridge_poc` reaches
`ComTerpServ`. `CppPropList` implements the
[`flowgraph.PropList`](https://github.com/vectaport/flowgraph) interface
directly (`GetField`/`SetField` stubbed as always-not-found/no-op
pending a decision on the per-entry-field design), so
`DupCppPropList` clones a source `PropList` the same way
`flowgraph.DupPropList` does -- switching Go-backed for C++-backed
storage is then just calling a different constructor, for a
string-valued `PropList` (the underlying `AttributeList` is
string-only, so `Set` stringifies anything else via `fmt.Sprint`); see
the demo's `goClone`/`cppClone` pair in `main.go`. Unlike the Go-backed
clone, a C++-backed one owns a cgo handle: the caller must type-assert
it back to `*CppPropList` and `Close()` it, as the demo does with
`defer cppClone.(*CppPropList).Close()`.

## Building

Requires a full `./configure && make` of the ivtools tree first (this
PoC links against the `Attribute`/`ComUtil`/`Unidraw-common`/`IV-common`
libraries that build produces; it is not wired into the imake build
yet). Then:

    cd src/comcomp/proplist_poc
    go build -o proplist_poc_bin .
    ./proplist_poc_bin

### macOS

The `#cgo LDFLAGS` already pick the `DARWIN` build dirs on this
platform (vs. `LINUX` elsewhere), so the build step above links fine.
Running the binary needs one more thing: these libraries are built as
`.dylib`s whose `install_name` is an absolute path baked in at their
*own* build time (`site.def.DARWIN`'s `SharedLibraryCmdDarwin`), not
`@rpath`-relative, so the `-Wl,-rpath` flags here don't help dyld find
an uninstalled tree's libraries the way `-rpath` does for an uninstalled
`.so` on Linux. Point `DYLD_LIBRARY_PATH` at the build dirs before
running:

    export DYLD_LIBRARY_PATH="$PWD/../../Attribute/DARWIN:$PWD/../../ComUtil/DARWIN:$PWD/../../Unidraw-common/DARWIN:$PWD/../../IV-common/DARWIN"
    ./proplist_poc_bin

(Or `make install` those four libraries first, so they land at the
path already baked into each `.dylib`'s `install_name`.)
