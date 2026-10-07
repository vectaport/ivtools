# translate

`../comcomp` now compiles natively (`../funccompile.comt`, loaded into
its own scope via `run()`) and no longer calls this binary. This is
kept as a standalone, process-boundary form of the same compiler --
reads a ComTerp `postfix(:tree)` value as its own printed text
(`{func,...}`) on stdin, and writes the `funccompile.go`-compiled Go
function body to stdout -- for anything that needs the compiler from
outside a running ComTerp (no cgo, no bridge: it never talks to
ComTerp directly, only to printed tree text a caller already has).

Same spike coverage as `../builtin_poc/funccompile.go` (which this
copies from) and `../funccompile.comt` (the native port): a
single-array-param func made of
assign/for/while/@-read/@-write/size/arithmetic/comparisons/&&/||,
ending by returning that same array.

## Build

```
cd src/comcomp/translate
go build -o translate .
```

## Usage

Normally invoked by `../comcomp`, not run directly. To use standalone:

```
echo '{func,{seq,lst,{size,lst}}}' | ./translate
```
