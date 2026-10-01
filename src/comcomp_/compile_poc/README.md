# compile_poc

Proof of concept for the comcomp compiler's core mechanism: a two-tier
native/fallback compile from a real ComTerp parse tree to Go, at
per-primitive granularity.

Given a `.comt` expression, this:

1. Gets its real parse tree via `postfix(expr :tree)` -- the same
   mechanism comlint already uses -- through the `bridge_poc` shim
   (copied in here so this POC builds standalone).
2. Parses that tree's printed form (`tree.go`) back into a small Go
   `Node` type.
3. Walks it (`compile.go`): a node compiles to native Go wherever its
   operator is hand-written (just integer `+ - * /` for this spike,
   `nativeBinOps`); otherwise it falls back to a bridge call evaluating
   *only that node's own reconstructed ComTerp source* (`unparse()`),
   composed into the surrounding native Go rather than widening the
   fallback to the whole expression.
4. Generates an actual Go program embodying the compiled expression,
   builds it with `go build`, and runs it -- a real compile, not a
   simulation -- comparing its result against evaluating the original
   expression directly for ground truth.

## Building and running

Same prerequisite as `bridge_poc` (a built ivtools tree). Then:

    cd src/comcomp_/compile_poc
    go build -o compile_poc .
    ./compile_poc '1+2*3'
    ./compile_poc 'size("ab")+size("xyz")*2'

The first is fully native (0 runtime bridge calls); the second falls
back only for the two `size()` leaves, composing their results into
native Go `+`/`*` -- proving the fallback granularity is per-primitive,
not per-expression.

## What this doesn't cover yet

- Only one primitive family (integer arithmetic) has native rules.
- No variables/assignment, no type inference across them -- see the
  thread discussion on inferring a variable's static type at its first
  assignment (mirroring how a typed string commits to its element type
  at construction) as the next refinement once this per-primitive
  mechanism is in place.
- `unparse()` only round-trips what this spike's fallback leaves need
  (int/string literals, bare symbols, call syntax); not a general
  ComTerp pretty-printer.
