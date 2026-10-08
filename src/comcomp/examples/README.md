# examples

ComTerp func sources `comcomp` has been proven to compile, kept here as
living documentation of its coverage. Only the `.comt` sources are
checked in -- their Go output is generated, not committed.

- `isort.comt` -- insertion sort over an array, in place (the
  file+funcname path: `comcomp isort.comt isort`).
- `double.comt` -- doubles every element in place, a bare `func()`
  literal with no name (the bare-file path: `comcomp double.comt`).

## Generating the Go output

`comcomp` must already be installed/on PATH first (`make install` in
`src/comcomp`). `generate` is its own target, deliberately not hooked
to `all`, since a normal top-level build doesn't have `comcomp`
available yet:

```
cd src/comcomp/examples
make generate
```

This writes `isort.go` and `double.go` here (git-ignored). `make clean`
removes them.
