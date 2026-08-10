# comterp examples

Runnable, plain-comterp scripts (no graphics, no editor, just the
interpreter) that demonstrate the language -- unlike `../tests/`, these
are not pass/fail tests (none return an `ok` boolean, none are
registered in `run_all.comt`); they are meant to be read and run to see
the language do something, the same spirit as
`src/comdraw/examples/` for the graphical side.

Run any of them from the repo root with:

```
comterp run src/comterp_/examples/<name>.comt
```

## The examples

- **ducktyping.comt** -- duck typing, comterp style. Three unrelated
  attrlists (`duck`, `cow`, `robot`) share no declared class, no
  interface, nothing but each happening to hold a `FuncObj` under
  `:speak` -- calling `.speak()` uniformly across a list of them works
  because comterp only ever asks "does this have the attribute I want,"
  never "what kind of thing is this." Exercises the `obj.method(args)`
  self-bound dispatch from issue #295: real per-object mutation
  (a `:calls` counter that persists across calls), positional args via
  `arg(n)`, and a keyword arg (`:times`) that reverts after the call
  because nothing inside the method writes to it. Also shows chaining
  straight off a list index (`at(barnyard i).speak()`, no intermediate
  variable) and previews the not-yet-built `(stream).field` lift over
  `$$barnyard` (issue #304) -- that line still warns today, on purpose,
  as a marker of where that feature will land.
