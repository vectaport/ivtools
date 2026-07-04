# ComTerp Test Suite — Coverage Strategy

## Overview

Each `.comt` test script covers a defined set of ComTerp functions.
Coverage is measured against the semantically valid combinatorial space
for each function — arities, keyword combinations, value classes, and
compositional pairings. Numbers are intentionally humbling; the goal is
honest tracking, not bragging. One good test per slot, not ten redundant ones.

## Coverage Taxonomy

For each function under test, the testable slots are:

### 1. Existence
The function is registered and callable without error.

### 2. Fixed argument arities
For a function with N fixed args where some are optional, there is one
slot per valid arity. Example: for type(val [ ...]) there is a 1-arity,
or a 2-arity, or n-arity.

### 3. Keyword absent (baseline)
One slot per keyword: a test that exercises the function *without* the
keyword, showing the default behavior.

### 4. Keyword present
One slot per keyword: a test that exercises the function *with* the
keyword, showing it does the thing the keyword suggests.

### 5. Keyword with value
For keywords that take a value (`:keyword value`), one additional slot:
a test supplying the value and verifying it is used as expected.

### 6. Return type
At least one test asserting the correct type of the return value.

### 7. Return value
At least one test asserting the correct value for known inputs.

### 8. Round-trips
Any pair (or triple) of functions that are semantic inverses of each
other gets one round-trip slot. Examples: `split`/`join`,
`export`/`import`, `print(:str)`/`run(:str)`. Round-trips are
identified by semantic analysis of help strings, not mechanically.

### 9. Compositional pairings
For each ordered pair of functions A and B in the script's scope, if
the return type of A is compatible with an input type of B, there is
one slot for A→B. The test plugs the return value of A directly into B
as an argument, verifying the composition works as expected.

Only semantically meaningful pairings are counted — type-compatible
but contrived combinations are excluded with a note. Round-trips
(slot 8) are a special case of A→B→A and are not double-counted here.

For N functions in a script there are at most N*(N-1) directional
pairings, filtered to those where the output type of A is compatible
with an input type of B.

## Scoring

### Combinatorial Space

The theoretical worst-case test space for a function is the product of
all valid argument arities, keyword presence/absence states, keyword
values, and argument type combinations. For a function with N keywords
this grows as 2^N before any value or type axes are added.

In practice most of this space is semantically invalid — combinations
that either can never occur (mutually exclusive keywords), produce
trivially identical behavior (`:reverse` on a single-element result),
or require inputs that don't exist (`:keep` without a delimiter in
char mode of split()). These are excluded from the denominator and
documented with a `!` prefix in the `missing:` header line.

A combination is considered **semantically valid** if there exists at
least one input for which both (or all) of the combined options
independently affect the output. Any combination that fails this test
is excluded with a `!` annotation.

After semantic filtering the combinatorial space is still large — a
function with 4 independent keywords has up to 16 valid combinations
in slot 10 alone, before arities or value axes are considered. Coverage
numbers are intentionally humbling. The goal is not 100% but honest
tracking of progress through a space that can never be fully exhausted.

### Coverage Formula

    coverage = covered_slots / total_valid_slots

Where `total_valid_slots` is the count of semantically valid slot
instances after `!` exclusions. Partial credit (marked `~`) is given
when a slot is implicitly exercised as a side effect of another test
but never explicitly asserted. Partials count as 0 in the score.

### Slot Priority

Slots are numbered in priority order. A script that covers slots 1-7
for all its functions is more useful than one that covers slot 10 for
one function and nothing else. The coverage number reflects breadth
across all slots, not depth in any one slot.

### 10. Keyword compatibility groups
For each function, test combinations of keywords that can be used
together. One slot per valid combination. Mutually exclusive keywords
are excluded with a `!` annotation.

### 11. Argument value stress
For each fixed argument and keyword value, exercise boundary value
classes: absent, default, zero, neg, pos, bool-t, bool-f, nil,
wrong-type, fake-key. One slot per value class per argument position.

### 12. Random sampling
Run the function with randomly sampled argument combinations. Uses a
random seed printed at the start of the run so failures are
reproducible — if a test fails, rerun with `srand(seed)` to reproduce.
Random tests assert no crash and known return type, not specific values.
Random tests contribute to coverage when they exercise combinations not
covered by slots 1-11.

## Coverage Header Format

Each test script (except `run_all.comt` and sub-scripts) carries a
three-item coverage summary in its header:

    // coverage: N/T (P%)
    // funcs: func1 func2 func3 ...
    // missing: func(slot) func(slot) ...

Where:
- `N` = covered slots, `T` = total slots, `P` = percentage
- `funcs` lists every function under test in this script
- `missing` lists uncovered slots in `func(slot)` notation

Partials are listed in `missing` with a `~` prefix, e.g. `~index(:last-absent)`.

Pairing slots use arrow notation in missing, e.g. `join->index`.

## Scripting Conventions

These conventions apply to all `.comt` test scripts:

- Use `print()` not `printf()` — comterp/comdraw scripts use `print()` exclusively
- Fixed arguments always come before keyword arguments:
  `print("fmt" val :str)` — correct
  `print(:str "fmt" val)` — wrong
- Single-char delimiters use single quotes: `split("a;b;c" :tokstr ';')`
- Char literals use single quotes: `'a'`, format with `%c`
- Each test accumulates into `ok`: `ok=ok&&(result==expected)`
- Deferred/broken tests are commented out with `/* */` and a note referencing the issue number
- Scripts return `ok` as the last expression for use by `run_all.comt`
- Sub-scripts (e.g. `return_sub1.comt`) are not subject to coverage measurement

### Test label convention

The print label for each test should show the actual ComTerp expression
being tested, not just a prose description. This makes the test output
self-documenting — reading the log is another way to learn the language:

```
// good -- shows the expression
print("14 list((2..4)*5) (expect {10,15,20}): %v\n" result)

// avoid -- prose description only
print("14 scaled ramp multiply (expect {10,15,20}): %v\n" result)
```

For multi-line expressions, show the key expression and summarize the
setup in the comment above:

```
// s=$$(1,2,3,4,5)
print("10 sum via while/next($$(1..5)) (expect 15): %v\n" total)
```

### Rules for LLM-assisted authoring

When an LLM generates or edits a `.comt` test script, it must follow
these four rules without exception. They are mechanical checklist items,
not stylistic suggestions — violating any one of them produces a broken
or misleading test file:

1. **Update the test number in the version header and coverage comment.**
   Every new test increments the count in `// coverage: N tests of ...`
   and in `print("scriptname.comt version N\n")` if the version bumps.
   Do not leave a stale count.

2. **Move the final summary line to the very end.**
   The `print("scriptname: %v\n" ok)` / `ok` footer must always be the
   last two lines of the file. When appending new tests, remove the old
   footer before appending, then re-add it after the new tests.

3. **Update the coverage stats in the header.**
   The `// coverage:`, `// funcs:`, and `// missing:` header lines must
   reflect the new test count and any new functions exercised. If a
   previously-missing slot is now covered, remove it from `// missing:`.

4. **Embed the original ComTerp expression in the print label.**
   The `print()` call for each test must show the actual expression being
   tested as the first element of the label string, not just prose.
   Followed by descriptive text if needed. This makes the test log
   self-documenting. See the **Test label convention** section above for
   examples.

## Adding Coverage

To improve coverage on an existing script:

1. Open the script and read the `// missing:` line
2. Find the relevant slot in this document's taxonomy
3. Add a numbered test following the existing style
4. Update the `// coverage:` and `// missing:` header lines

To add a new test script:

1. Identify the functions to cover
2. Run `help(funcname)` inside comterp to see the signature
3. Enumerate slots from the taxonomy above
4. Write tests, add the coverage header, add the script to `run_all.comt`

## Current Coverage Summary

| script        | funcs                                                        | slots   | covered | total |  %  |
|---------------|--------------------------------------------------------------|---------|---------|-------|-----|
| hello.comt    | (smoke test only)                                            | —       |       — |     — |  —  |
| return.comt   | return func if for while run                                 | 1-10,11 |      34 |    50 | 68% |
| stream.comt   | $$ $ ,, next each size .. **                                 | 1-10    |      46 |    50 | 92% |
| string.comt   | index substr split join eq size print(+:str) +               | 1-10,11 |      78 |    97 | 80% |
| global.comt   | global                                                       | 1-10,11 |      13 |    14 | 93% |
| assignops.comt| mod_assign mpy_assign add_assign sub_assign div_assign incr incr_after decr decr_after | 1-9,11 | 33 | 53 | 62% |
| attrlist.comt | dot list(:attr) attrlist at size attrname attrval + -        | 1-10    |      42 |    55 | 76% |
| print.comt    | print                                                        | 1-9     |      22 |    35 | 63% |
| parser.comt   | attrlist(:literal) errmsg postfix class type                 | 1-9     |      28 |    38 | 74% |
| symbol.comt   | ` symadd symid symbol symstr symval symvar strref eq(:sym) lt gt switch cond | 1-9 |  36 |    42 | 86% |
| help.comt     | help() help(:posteval) help(:top) help("op") optable(:table) optable(:bypri :byopr :bycom) | 1-14 | 14 | 14 |100% |
| random.comt   | (slot 12 stress: all funcs from return/stream/string/global) | 12      |      24 |    24 |100% |

### Planned coverage (ivtools-2.2)

| script        | funcs                                                                         | notes                                                      |
|---------------|-------------------------------------------------------------------------------|------------------------------------------------------------|
| parser.comt   | attrlist(:literal) parse error lock-in via errmsg()                           | add errmsg() assertions for bare-value-after-keyword cases |
| return.comt   | func() empty parens, func() positional varargs, arg(`sym) keyword lookup      | see issue #94                                              |

### Planned coverage (ivtools-3.0)

| script      | funcs                                                                                                          | notes         |
|-------------|----------------------------------------------------------------------------------------------------------------|---------------|
| stream.comt | stream literal `(0 1 2 3)`, mixed `(0 :flag 1 :color red)`, keyword element detection via class()/attrname()/attrval() | see issue #94 |

## The Self-Hosted Test Suite

The ComTerp test suite is written in ComTerp. This is not just a
convenience — it is the same bootstrap insight that underlies a C
compiler compiling itself, or yacc processing its own grammar. You
cannot use the test harness to test the test harness until the test
harness works well enough to run. The scaffolding (`testlib.comt`,
`run_all.comt`, `ok=ok&&(...)`, `check_fail()`) had to be bootstrapped
from a working-enough ComTerp before it could test ComTerp.

The payoff is that the test suite is also the most honest documentation
of what the language actually does. Any discrepancy between prose docs
and tests, the tests win — they run. And because the tests are written
in ComTerp, reading them teaches ComTerp in a way no external test
framework could. The test suite is also the tutorial.

Most languages never achieve this. Their test suites are written in
some other language, which means there is always a translation layer
between "what the tests say" and "what the language means." ComTerp
tests mean exactly what they say, in the language they are testing.
