# translate

The Go side of `../comcomp`: reads a ComTerp `postfix(:tree)` value as
its own printed text (`{func,...}`) on stdin, and writes the
`funccompile.go`-compiled Go function body to stdout.

No cgo and no bridge -- `../comcomp` already runs inside a real
ComTerp, so it fetches the tree text itself (`postfix(expr :tree)`
natively) and pipes the text to this binary; this binary never talks
to ComTerp directly.

Same spike coverage as `../builtin_poc/funccompile.go`, which this
copies from: a single-array-param func made of
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
