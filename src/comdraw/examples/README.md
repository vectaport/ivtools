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
  carries facts attached with `setattr()` (`:name :legs :eats :moves`)
  or one at a time by dot-assignment (`pig.legs=4`);
  typed queries walk the component tree (`frame()`, `at()`, dot-access)
  and flash the matches (`select()` + `update()`).  Try `zoohelp()`,
  `who("swims")`, `haslegs(4)`, `countlegs()`, `dance()`.  Written to be
  buildable-along by a kid: the drawing is the database, the queries are
  one-screen funcs.  *The drawing is the datastore.*

- **spirograph.comt** — the toy with the gear-ring and the pen-hole
  wheel, driven from the prompt: `spiro(R r d)` takes the same teeth
  counts as the real thing (`:epi` rolls outside; try `gallery()`,
  `surprise()`, `spin()`, `spirohelp()`).  Each curve is ONE
  `closedspline()` from a computed point list — no interpolation solve
  needed, because a spirograph is a sum of two circular harmonics and
  sampled sinusoids are eigenvectors of the B-spline's (1,4,1)/6 knot
  filter: divide each radius by its eigenvalue `(2+cos wh)/3` and the
  spline lands exactly on the true curve at every knot, ~10 control
  points per harmonic cycle.  Every curve remembers its parameters via
  dot-assignment (`curve.R=96` — `setattr()` spelled as plain
  assignment), and `recipes()` prints back the re-typeable `spiro()`
  command for each.  `orbit()` then turns the curves into planets:
  spring-gravity toward the shared center of gravity, short-range
  repulsion when they bump, each spinning on its own axis.
  *The prompt is the control panel.*

- **etchasketch.comt** — two knobs, one pen, no lifting.  `h(n)` and
  `v(n)` turn the knobs (the knob graphics visibly rotate — each is an
  ellipse grouped with its tick mark so the tick orbits the axle, the
  petals.comt trick), each turn etches one line segment, the pen clamps
  at the glass edges, and `shake()` wiggles the toy and erases only the
  graphics tagged `kind="etch"`.  Pen state escapes the knob funcs with
  `local(penx)=` — the session-scope escape from LANGUAGE.md's scoping
  rules — and a knob turn is just the expression `h(3)`, so anything
  that can send an expression can turn a knob: a second player over
  `remote()`, or a trackball daemon split into two axes piping deltas.
  *The knobs are the wire.*

- **lastkey_keytest.comt** — not a toy, a diagnostic: an interactive,
  manual test of `lastkey()`'s full key vocabulary (arrows, Home/End,
  PgUp/PgDn, Insert, F1-F12, Esc/Tab/Backspace/Enter/Space as their own
  C character literals, `:shiftarrow` capture's `"S-"` prefix). Requires
  a human pressing real keys — there's no way to script X11 keyboard
  events reliably enough to automate it — so it stays here rather than
  in `run_all.comt`/CI. Re-run it by hand whenever
  `ComEditor::keyname()` changes. `keytest()` for plain keys,
  `keytest_capture()` for the `:shiftarrow`-captured versions.

zoomap.comt is the reference implementation of the "Askable Map"
pattern; the genre write-up (anatomy, error pedagogy, design rules,
the drawserv distribution path) is `doc/SPATIAL-APPLICATIONS.md`.
