# Troubleshooting: symptoms and their first moves

Things that have cost someone here an afternoon, indexed by what you actually
see rather than by what turns out to be wrong. Each entry is short on purpose —
symptom, what is going on, the first move, and where the real detail lives.

## A crash that has nothing to do with what you changed

Typically a SIGSEGV inside a copy or an assignment, in a file you never
touched.

Header dependencies live in `Makefile.depend`, which `make depend` writes as a
snapshot rather than maintaining. An `#include` added since the last run stays
unknown to the build, so later edits to that header rebuild nothing that needs
it. Objects built against an old copy of a header then link alongside newer
ones, and if the header changed the layout of a class the crash surfaces far
from the cause.

**First move:** `make clean` and rebuild before reading any more code. Rerun
`make depend` after adding includes. See `INSTALL`.

## `update()` came back immediately instead of waiting

`update(usec)` is `handle_events` — it returns on the first event, not after
the timeout. It is not a settle, and using it as one leaves whatever you were
waiting for racing whatever you expected to have finished.

**First move:** poll for the condition you actually care about, with `update()`
inside the loop rather than in place of it.

## A loop that never ends, or ends immediately

Comparisons against `nil` do not behave like comparisons against zero: `nil != 0`
is true, while `nil > 0` is `nil`. A termination test that a `nil` satisfies
never terminates.

**First move:** test for the value you expect rather than for its absence, and
check what the variable holds before the loop starts.
