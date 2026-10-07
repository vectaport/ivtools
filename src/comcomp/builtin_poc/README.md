# comcomp built-in spike: func -> compiled, linked-in Go command

Proves the direction the compcomp-go-bridge thread's bridge POC didn't cover:
not Go calling into ComTerp, but a ComTerp `func` compiled to Go, linked
into a copy of `comterp` as a native built-in command, and called from
ComTerp source like any other command.

## Pipeline

1. `isort_baseline.comt` -- the func under test (`isort`, a textbook
   insertion sort over a plain ComTerp list via `@`), run interpreted, timed
   with `time(:mono :ns)`. Baseline: **n=2000, 31.99s**, correct.
2. `tree.go` (copied from `../compile_poc`) parses the func's own real
   `postfix(:tree)` output -- fetched live through the bridge shim, not
   hand-transcribed.
3. `funccompile.go` walks that tree and emits a Go function body: one
   native Go statement per ComTerp primitive this spike covers (`assign`,
   `for`, `while`, `@` read/write, `size`, arithmetic, comparisons, `&&`/`||`).
   `gencheck.go` drives this end to end and is the thing to run to see each
   stage's output.
4. `archive.go` writes the compiled body into `gosort/sort.go` and builds it
   with `go build -buildmode=c-archive`, producing `libgosort.a`/`.h` --
   the ComTerp->Go direction of the boundary (cgo itself only ever covers
   Go calling C/C++; a `//export`ed c-archive is how the reverse direction
   gets a C ABI).
5. `host/gosortfunc.h`/`.c` is a `ComFunc` (`GoSortFunc`) wrapping the
   exported `SortIntsCSV`. Per Scott's call: the boundary passes **one
   serialized CSV string each way**, not one ComValue round-trip per
   element -- Go's own timing shouldn't pay for ComTerp-side marshaling.
6. `host/host.c` is a copy of `comterp`'s own `ComTerpServ` setup
   (`add_defaults()` + one extra `add_command("gosort", ...)`) -- "a copy
   of comterp with it added as a built-in function," statically linked;
   dynamic loading is future work. It lives in its own `host/`
   subdirectory, away from the cgo-driven `.go` files at this directory's
   top level: cgo auto-compiles every `.c`/`.cc` file next to the package
   it's building, so a C++ file meant to be built separately with plain
   `g++` has to sit outside that directory instead.

## Build

`host/` is a normal imake program directory, like any other binary in
this tree (`host/Imakefile` -> `ComplexProgramTarget(gosort_host)`), so
it builds with the project's own macros instead of hand-typed `g++`
commands guessing at install paths:

```
cd src/comcomp/builtin_poc
go build -o gencheck_bin .                 # runs stages 2-4, writes gosort/libgosort.a
cd host && make Makefiles && make          # imake-generated build; builds libgosort.a
                                            # first via the Imakefile's go-build rule if
                                            # gencheck_bin hasn't already
```

The generated `host/$(CPU)/` object directory holds the result as
`a.out`, same as every other program directory in this tree (`make
install` is what copies it to `gosort_host`); run it in place or
install it. `make Makefiles` needs `./configure && make` to have run at
the tree's top level first (same as any other program here), which is
also what makes `$(LIBCOMTERP)` etc. resolve to this source tree's own
build output rather than an installed prefix.

## Result

```
./host/LINUX/a.out gosort_bench.comt
gosort(n=2000) elapsed_ns=1771412L ok=true first5=0L,2L,7L,12L,14L
```

Same input, same output (`first5` matches the interpreted baseline
exactly) -- **31.99s interpreted vs. ~1-2ms compiled-and-linked-in, ~20,000x**.
That gap is mostly the interpreter's own per-op dispatch cost on an O(n^2)
algorithm, not something a faster bridge call would close -- the win here
is real compiled-and-linked-in native code, not a faster remote call.

## Scope and what's not here

`funccompile.go` hand-covers exactly the primitives `isort` uses -- it is
not `compile_poc`'s native/fallback composition (no bridge fallback exists
inside a built-in command; there's no running ComTerp to call back into).
A primitive this spike doesn't recognize is a compile-time error, not a
silent runtime fallback. Extending primitive coverage, and dynamic loading
in place of the static link, are the natural next spikes.
