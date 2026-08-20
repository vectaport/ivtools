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
  self-bound dispatch: real per-object mutation (a `:calls` counter that
  persists across calls), positional args via `arg(n)`, and a keyword arg
  (`:times`) that reverts after the call because nothing inside the method
  writes to it. Also shows chaining straight off a list index
  (`at(barnyard i).speak()`, no intermediate variable) and the
  `(stream).field` lift over `$$barnyard`, reading one field across every
  object in the list at once.

- **testpatterns.comt** -- classic signal/video test pattern generators
  (ramp, sawtooth, square wave, staircase, triangle wave, sine wave),
  each a single overdriven stream expression: `..` for the range, `%%`
  to replay a whole sequence N times, `**` to repeat each element N
  times, `,,` to concat an ascending and descending ramp into one
  triangle cycle, `sin()`/`degtorad()` overdriving a range for the sine
  wave. No loop written anywhere -- what used to be a hand-written
  FORTRAN `setbuf` routine per pattern is one line each.

- **postevalcombinators.comt** -- control-flow combinators written in
  plain comterp using `func(:posteval)`, the lazy-argument keyword: an
  argument's expression only runs if the body actually reads it, and
  reading it again re-runs it rather than reusing a cached value. `steer`
  takes `:primary`/`:fallback` as keyword-carried *code*, not values, and
  only fires `fallback` if `primary` actually returns `nil` -- a
  retry/fallback combinator with no special syntax beyond the keyword
  declaration. `retry` instead reads a positional `arg(0)` inside a
  `while` loop, showing off the other half of `:posteval`: every read
  re-fires the caller's expression for real, so `retry(flaky())` makes
  up to three genuine calls to `flaky()`, not one call retried against a
  cached failure.

- **gcd.comt** -- Euclid's algorithm two ways. A single scalar
  `gcd=func(a=arg(0); b=arg(1); while(...); a)` is called first with plain
  numbers, then handed two streams of `(m n)` pairs: the streams drive the
  invocation, so the body fires once per pair with `arg(0)`/`arg(1)` bound
  to scalars and the `while` inside is the same scalar loop, run four
  times -- the func itself is unchanged between the two uses, and `$$`
  hands out copies so the same `m`/`n` can drive it again. The second half
  rewrites the same algorithm branch-free, hoisting the loop out of the
  func so every pair reduces in lockstep: `b + (b==0)` guards the modulo
  so a finished lane parks at zero instead of dividing by it, and
  `(b!=0)*b + (b==0)*a` masks the update so that lane keeps its answer
  while the others keep going. Worth reading against the first version --
  it is what vectorizing by hand costs, and it is also the shape that maps
  onto data-parallel hardware, where per-lane control flow is the
  expensive part.

- **chunking.comt** -- `chunk(strm n)` re-grains a stream into a stream of
  n-element indexed lists, so a script loop runs `total/n` times instead of
  `total` times. Sums 60,000 elements three ways and times each: per element
  (0.73s), chunked with `sum()` folding each block in C (0.11s), and chunked
  but indexed per element from script (0.89s -- *slower* than not chunking,
  since the same interpreted operations remain and the chunking is added on
  top). That third one is the trap, and it is what you write if you think of
  `chunk` as buffering for speed. The afterword states the rule: it is about
  the consumer, not the stream -- `chunk` pays when C can swallow a block
  whole, and does nothing when every element read must be emitted again.
