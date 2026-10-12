# proplist_poc

Proof of concept for a Go type (`CppPropList`) backed by a live C++
`AttributeList`, reached through a flat `extern "C"` shim
(`shim.h`/`shim.cc`) the same way `comcomp/bridge_poc` reaches
`ComTerpServ`. Method shapes mirror the Go-only `flowgraph.PropList`
interface (`Get`/`Set`/`Delete`/`Has`/`Len`/`Keys`/`Touch`/`At`).

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
