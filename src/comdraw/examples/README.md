# comdraw examples

Runnable comdraw scripts that demonstrate graphical scripting — unlike
`../tests/`, these are not pass/fail tests (none return an `ok` boolean,
none are registered in `run_all.comt`); they are meant to be watched and
played with.

Run any of them from the repo root with:

```
comdraw -runfile src/comdraw/examples/<name>.comt
```

The script draws its picture, then leaves you at the `(comt)` prompt with
the drawing (and any funcs the script defined) live in the session.

## The examples

- **petals.comt** — six closed-spline petals in a ring, then a spin
  animation: the whole picture is grouped (`group()`) so `rotate()`
  pivots about the shared center, paced by `update(usec)`.
  *The script is the picture.*

- **zoomap.comt** — a zoo map you can ask questions.  Every animal
  carries facts attached with `setattr()` (`:name :legs :eats :moves`);
  typed queries walk the component tree (`frame()`, `at()`, dot-access)
  and flash the matches (`select()` + `update()`).  Try `zoohelp()`,
  `who("swims")`, `haslegs(4)`, `countlegs()`, `dance()`.  Written to be
  buildable-along by a kid: the drawing is the database, the queries are
  one-screen funcs.  *The drawing is the datastore.*
