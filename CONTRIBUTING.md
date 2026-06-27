# Contributing to ivtools

ivtools is a mature, layered C++ codebase — a backward-compatible InterViews 3.1
+ Unidraw, plus the ComTerp command interpreter and the DrawServ distributed
drawing server. It is unusually well documented at the source level, which is
what makes it approachable. This is the short version of how to land a change.

## Orient first

Read **`CLAUDE.md`**. It is the fast orientation to the repository for humans and
AI assistants alike: the layer hierarchy, where things live, how to build, how to
test, and the coding and commit conventions. If you work with an AI assistant,
point it at `CLAUDE.md` first — the per-layer `ARCHITECTURE.md` / `HACKING.md`
docs go deeper, and the tests are the spec.

## Build

```sh
autoreconf -i      # only if ./configure is missing (needs autoconf >= 2.70)
./configure
make
```

See **`INSTALL`** for platform notes and dependencies (Debian/Ubuntu base:
`libx11-dev xutils-dev libxext-dev`; ACE is required only for the network /
DrawServ features).

## Test

There is no `make test`. The suites are written in ComTerp itself and run from a
built binary:

- **ComTerp unit tests** — `comterp run src/comterp_/tests/run_all.comt`
  (or `run("run_all.comt")` from inside `comterp`/`comdraw`/`drawserv`).
  See `src/comterp_/tests/TESTING.md`.
- **comdraw graphical-scripting tests** — `src/comdraw/tests/run_all.comt`
  (needs an X display; under CI, via `xvfb`).
- **DrawServ integration tests** — `src/drawserv_/tests/drawmo` (needs X11, free
  TCP ports in the 20000+ range, and the network binaries on `PATH`).
  See `src/drawserv_/tests/TESTING.md`.

Read the relevant `TESTING.md` before adding or editing a `.comt` test — there
are mandatory rules for LLM-authored test scripts.

## Open a pull request

- Work on a **feature branch**, never directly on `master`.
- Follow the commit convention in `CLAUDE.md`: a scannable one-line summary
  calling out each significant change; the PR body carries the narrative.
- Open the PR against `master`. **CI must be green to land** — the Linux build
  and the `comterp` suite gate every pull request. (The `comdraw` and `drawserv`
  GUI suites currently run but are *non-blocking*, pending the known issue below.)

## Finding work

There is no curated starter list. The open issues, the open pull requests, and
the CI's own red and non-blocking marks are the live to-do — browse them and pick
what interests you. There is no wrong place to start.

Welcome aboard.
