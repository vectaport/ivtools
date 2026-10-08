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

    cd src/comcomp/compile_poc
    go build -o compile_poc .
    ./compile_poc '1+2*3'
    ./compile_poc 'size("ab")+size("xyz")*2'

The first is fully native (0 runtime bridge calls); the second falls
back only for the two `size()` leaves, composing their results into
native Go `+`/`*` -- proving the fallback granularity is per-primitive,
not per-expression.

## unparse() as a decompiler, not just a fallback-arg builder

`unparse()` turns a tree node back into real ComTerp source -- originally
just to build the fallback calls above, but it's the same thing as func
reflection: rendering a `FuncObj`'s compiled postfix body back as
something a person can read (the kind of thing a new `info()` field could
expose, generated only when `info()` asks for it, alongside its existing
func dispatch -- `:ntoks`/`:nspans`/`:posteval`, PR #635). This prototype
is in Go against the printed tree *text*; a real `info()` field would
read a `FuncObj`'s postfix tokens directly in C++ instead.

`-roundtrip` and `-roundtrip-file <path>` test it: get a real tree,
unparse it back to source, re-parse that regenerated source, and check
the resulting tree is identical to the original (the regenerated syntax
is allowed to differ -- `x=1` vs `assign(x 1)` -- as long as it parses
back to the same tree).

    ./compile_poc -roundtrip            # 6 cases from postfixtree.comt, all pass
    ./compile_poc -roundtrip-file somefile.comt

Passes: plain expressions, `;`-joined statements, keyword args (including
the bare-flag `(:b {})` case), `global()` lvalues, `func(params;body)`
definitions, and whole multi-statement files via `parse(fileobj :tree)`.
`unparse()` always writes a trailing newline, since ComTerp's file scanner
silently drops the last statement without one (found via this very
round-trip test, not documented anywhere).

**`func(params;body)` needed special handling.** `func(a b;a+b*2)`'s tree
is `{func,a,{seq,b,{add,a,{mpy,b,2}}}}` -- a call whose own argument list
mixes plain space-separated positionals (`a`) with a `;`-joined tail,
folds that tail into one trailing `seq` item rather than keeping every
piece flat (confirmed by comparing against `func(a b a+b*2)`, no
semicolon at all, whose tree is the fully flat `{func,a,b,{add,...}}}` --
and `func(a;b;a+b*2)`, semicolons throughout, which nests *everything*
into `{func,{seq,{seq,a,b},{add,...}}}}`). Reconstructing the first tree's
trailing seq as a parenthesized argument (`func(a (b;a+b*2))`) parses back
to a *different* tree -- the paren group merges with its preceding
space-separated sibling into one combined value instead of staying two
positional args, the documented `(`-with-spaces-builds-a-stream-literal
trap (AGENTS.md's ComTerp scripting gotchas, issue #488) biting the
decompiler itself. Fixed by splicing the trailing seq's own pieces back
into the outer space-separated argument list, with a semicolon only
before the very last piece, instead of wrapping it in its own parens.

## What else this doesn't cover yet

- Only one primitive family (integer arithmetic) has native rules.
- No variables/assignment in the *compiler* (separate from the decompiler
  above) -- see the thread discussion on inferring a variable's static
  type at its first assignment (mirroring how a typed string commits to
  its element type at construction) as the next refinement.
- `unparse()` handles what's been tested against real cases above; still
  not a fully general ComTerp pretty-printer (e.g. no infix sugar for
  arithmetic, only for `assign`/`seq`). A fully semicolon-joined call
  (`func(a;b;a+b*2)`, nested seq-of-seq) isn't handled -- only the one
  real shape (`func(a b;a+b*2)`) that actually came up.
