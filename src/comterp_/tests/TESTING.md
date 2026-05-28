# ComTerp Test Suite — Coverage Strategy

## Overview

Each `.comt` test script covers a defined set of ComTerp functions.
Coverage is measured by how many testable slots have at least one test.
The goal is not exhaustive combinatorics but systematic slot coverage —
one good test per slot, not ten redundant ones.

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

    coverage = covered_slots / total_slots

Partial credit (marked `~`) is given when a slot is implicitly exercised
as a side effect of another test but never explicitly asserted. Partials
count as 0 in the score — they are noted to flag where a small addition
would close a real gap.

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

| script        | funcs                                          | covered | total |  %  |
|---------------|------------------------------------------------|---------|-------|-----|
| return.comt   | return func if for while run                   |      27 |    42 | 64% |
| stream.comt   | $$ $ ,, next each size                         |      24 |    33 | 72% |
| string.comt   | index substr split join eq size print(+:str) + |      25 |    66 | 37% |
