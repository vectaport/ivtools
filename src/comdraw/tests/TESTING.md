# comdraw Test Suite

See `src/comterp_/tests/TESTING.md` for the full coverage taxonomy,
scoring methodology, and scripting conventions (label format, `ok=ok&&(...)`
accumulation, the five LLM-authoring rules, intentional-error handling).
This document covers only what is different or additional at the comdraw
layer.

## What is Different Here

comdraw is comterp plus graphics in one process: no sockets, no separate
server to launch. That makes these closer to unit tests than the `drawmo`
integration suite in `src/drawserv_/tests/` -- but unlike plain `comterp_`
scripts, they exercise commands that touch a real GUI canvas (`select()`,
`group()`, raster paint, editor style state), so they need a mapped X11
window, not just an interpreter.

comdraw's startup seed `update()` (`src/comdraw/main.c`) realizes the
canvas before any `-runfile`/`-runexpr` script or interactive input runs,
so the window is already mapped by the time a test's first `select()` or
`group()` call needs it -- no explicit wait is required.

## Running Tests

```bash
comdraw -stdin_off -runfile src/comdraw/tests/run_all.comt   # scripted/CI
comdraw -runfile src/comdraw/tests/run_all.comt               # interactive
comdraw -runexpr 'run("src/comdraw/tests/run_all.comt");exit'
run("src/comdraw/tests/run_all.comt")                          # from the prompt/socket
```

Use `-stdin_off` for non-interactive runs: with stdin closed
(piped/backgrounded) comdraw quits on the stdin EOF instead of blocking on
a prompt that will never get input. CI (`.github/workflows/ci.yml`) runs
the `-runexpr ... exit` form headless under `xvfb-run -a`, with a
"substituting fixed font" guard for missing X core fonts on the runner.

Each script accumulates into `ok` and returns it; `run_all.comt`
aggregates all scripts' results into its own `ok` and prints a final
pass/FAIL summary line. There is no separate orchestrator here (unlike
`drawmo` for DrawServ) -- `run_all.comt` itself is the entry point.

## Test Inventory

| script          | funcs                                                                 | covers |
|-----------------|------------------------------------------------------------------------|--------|
| pixelfunc.comt  | raster() peek() poke() pokeline()                                     | packed-int and r,g,b/r,g,b,a tuple forms for pixel read/write, round-tripped through peek() |
| gsexim.comt     | select() at() export() import()                                        | graphic-state export/import round-trip in-process: export(:string :percomp)'s string is the command that recreates the graphic |
| group.comt      | rect() select() group() ungroup() size()                              | group() collapses a selection into one group graphic; ungroup() dissolves it back |
| gsget.comt      | brush() pattern() patternmask() colors() colorsrgb() font() fontbyname() | bare (no-arg) getters return the literal that reproduces the editor's current default; set-then-get round trips |
| dotattr.comt    | dot-assign dot-read setattr() frame()                                 | `comp.key=val` as sugar for `setattr(comp :key val)`, coexisting with explicit setattr() calls |
| lastkey.comt    | lastkey() keyname_test() help() index()                               | registration, `:reset`/`:shiftcapture` state, the XF86-keysym/SUPER_FLAG regression, a naming-vocabulary smoke test, and keyname_test()'s hidden-from-help() status; see its own header for why `lastkey()`'s actual keypress path can't be scripted headlessly |

`run_all.comt` runs these in the order above, each wrapped in
`if(run("./script.comt") :then ... :else ...;ok=false)`.

## What Belongs Here vs comterp_/tests vs drawserv_/tests

- Purely interpreter-level behavior (no graphic on screen, no editor state)
  belongs in `src/comterp_/tests/`, not here -- it doesn't need a window and
  runs under plain `comterp`.
- Graphical scripting that needs a live comdraw canvas but never leaves the
  process -- create/select/style/export a graphic, dot-assign attributes,
  read pixels -- belongs here.
- Anything that requires two live processes talking over a socket (`remote()`,
  distributed drawlink/select-lock propagation) belongs in
  `src/drawserv_/tests/`, driven by `drawmo`. A single comdraw process has no
  peer to talk to.
- Commands that need a real X11 KeyPress/ButtonPress event (not just a mapped
  window) -- `lastkey()`'s own return value, for instance -- can't be
  scripted in any of the three suites: there's no XTest under CI's `xvfb`.
  The pattern for covering these as much as possible without one: add a
  TEST-ONLY command that calls the same production logic directly with
  synthetic input, bypassing the event source (see `keyname_test()`,
  `src/ComUnidraw/unifunc.h`/`.c`, and `ComEditor::keyname()` in
  `src/ComUnidraw/comeditor.c`), and mark it `hidden` (see
  `ComFunc::hidden()`/`ComTerp::add_command()`'s `hidden` argument) so it
  doesn't clutter `help()` for ordinary driving-script authors. Pair it with
  a `*_keytest.comt` example under `src/comdraw/examples/` (manual,
  human-at-a-keyboard, deliberately excluded from every automated suite) for
  the coverage no script can reach.

## Adding a New Test

1. Write `funcname.comt` following the header/`ok`/label conventions in
   `src/comterp_/tests/TESTING.md`.
2. If the script needs the GUI (`select()`, `group()`, raster paint), no
   extra setup is needed -- comdraw's startup `update()` has already mapped
   the window by the time the script runs.
3. Register it in `run_all.comt`:
   ```
   if(run("./funcname.comt")
     :then print("pass: funcname\n")
     :else print("FAIL: funcname\n");ok=false)
   ```
4. Add a row to the Test Inventory table above.
