# AGENTS.md

Guidance for AI assistants (and humans) working in the **ivtools** repository.
This file orients you fast; the authoritative deep-dives live in the per-layer
docs linked throughout. When this file and a linked doc disagree, the linked
doc wins — and when a doc and the tests disagree, **the tests win** (they run).

---

## What ivtools is

ivtools (release **2.1.1**) is a layered collection of C++ application
frameworks for building custom drawing editors and spatial-data servers. It
bundles a backward-compatible copy of the original **InterViews 3.1** and
**Unidraw** class libraries and adds new layers: a glyph toolkit, an extended
Unidraw/idraw framework, a graph/frame editor, and — most actively developed
today — an embedded command interpreter (**ComTerp**) and a distributed drawing
server (**DrawServ**).

The defining idea: **the command language is also the wire protocol.** Every
ComTerp value serializes back to valid ComTerp syntax, so a terminal REPL
session and a TCP session between drawing servers are the same act — send an
expression, get back a value that is itself an expression. See
`doc/INTRODUCTION.md` for the conceptual tour and `doc/LANGUAGE.md` for the
language from the user's side.

This is a mature, ~30-year-old codebase (ComUtil dates to 1989). Match the
surrounding code; do not modernize idioms wholesale.

---

## Repository layout

```
src/                  all libraries and example programs (48 subdirs)
config/               imake build configuration (per-platform site.def.*, *.mk)
CHANGES/              per-release changelogs (CHANGES-0.3 ... CHANGES-2.1)
INSTALL               build instructions (short + long form)
doc/                  narrative docs: INTRODUCTION, APPENDIX-A..E, LANGUAGE, WAVE-CONTRIBUTION
configure.ac          autoconf input -> ./configure
Imakefile             top-level imake target
VERSION               "Release 2.1.1"
```

### The library layer hierarchy (bottom → top)

```
ComUtil → ComTerp → OverlayUnidraw → ComUnidraw → FrameUnidraw|GraphUnidraw → DrawServ
```

A lower layer must **never** `#include` a higher-layer header. When a lower
layer needs higher-layer behavior, add a virtual no-op in the lower layer and
override it above. See `src/ComTerp/HACKING.md` → *Layer Hierarchy and
Violations*.

### Key `src/` directories

| Dir | Role |
|-----|------|
| `IV`, `IV-2_6`, `IV-X11`, `IV-common`, `InterViews`, `OS`, `Dispatch`, `TIFF` | InterViews 3.1 / 2.6 base libraries, borrowed whole |
| `ComUtil` | C-level interpreter engine: scanner, parser, postfix codegen, error system (1989 SBIR code) |
| `ComTerp` | C++ command-interpreter library (`ComFunc`, `ComValue`, `ComTerp`) |
| `comterp_` | the `comterp` / `comterp_listen` binaries + `tests/` (language ref now in `doc/LANGUAGE.md`) |
| `Attribute`, `AttrGlyph`, `ComGlyph`, `IVGlyph` | property lists, glyphs |
| `Unidraw`, `UniIdraw`, `OverlayUnidraw`, `ComUnidraw` | drawing-editor frameworks |
| `GraphUnidraw`, `FrameUnidraw`, `TopoFace` | graph/frame/spatial-network editors |
| `DrawServ`, `drawserv_` | distributed drawing server + its `tests/` |
| `comdraw`, `drawtool`, `idraw`, `graphdraw`, `flipbook` | editor binaries |
| `tests` | top-level / cross-cutting tests (e.g. y2k) |

Note the trailing-underscore convention: `comterp_` and `drawserv_` are the
**program** directories; `ComTerp` and `DrawServ` are the corresponding
**library** directories. See the naming convention below.

---

## Building

ivtools uses an **imake**-based build wrapped by autoconf. From the repo root:

```bash
autoreconf -i      # only if ./configure is missing; needs autoconf >= 2.70
./configure
make
sudo make install  # optional
```

On Debian/Ubuntu you first need: `libx11-dev xutils-dev libxext-dev`
(plus `autoconf`). Full details, platform notes, and the long-form
instructions are in **`INSTALL`**.

- `configure` writes `config/config.mk`. Its two usual failure points are CPU
  detection and `XCONFIGDIR`; both can be hand-edited into `config/config.mk`.
  `CPU` must match one of the `config/site.def.$CPU` files (LINUX, DARWIN,
  FREEBSD, …).
- Built binaries land in per-CPU object subdirectories under each program dir
  (e.g. `src/comterp_/LINUX/comterp`). These dirs (`DARWIN`, `Makefile`,
  `*.o`, `a.out`, …) are git-ignored.
- `ARCHBUILD.sh` is the canonical full-from-scratch build recipe (it runs
  `configure` + `make` + `make Makefiles` + `make` to settle imake deps).
- To build with AddressSanitizer for memory debugging, follow
  **`config/SANITIZE.md`** — note the imake-specific gotcha: use the `OTHER_*`
  flag hooks in Imakefiles, **not `EXTRA_*`** (which `config/params.def`
  overrides).

ACE (a C++ middleware toolkit) is required only for the network-server
capabilities (`comterp_listen`, DrawServ). See `INSTALL` §0.f.

---

## Testing

There is no `make test`. ivtools has **three self-hosted test suites**, all
written in ComTerp itself.

### ComTerp unit tests — `src/comterp_/tests/`

`.comt` scripts run from inside a built `comterp`/`comdraw`/`drawserv`:

```
run("src/comterp_/tests/run_all.comt")   # runs every script, prints pass/FAIL
```

Each script returns a boolean `ok`; `run_all.comt` aggregates them. The full
coverage taxonomy, scoring methodology, header format (`// coverage:`,
`// funcs:`, `// missing:`), and the **five mandatory rules for
LLM-authored test scripts** are in **`src/comterp_/tests/TESTING.md`**. Read it
before adding or editing any `.comt` test. Highlights:

- Use `print()`, never `printf()`. Fixed args before keyword args.
- Each test accumulates: `ok=ok&&(result==expected)`.
- The print label must embed the **actual ComTerp expression** under test, not
  a prose description — the log doubles as documentation.
- Keep the `print("scriptname: %v\n" ok)` / `ok` footer as the last two lines.
- Register new scripts in `run_all.comt`.
- Some tests feed the parser **deliberately malformed text** — the malformed
  text is the fixture; the `errmsg()` it raises is the behavior under test
  (e.g. `(4 :x 7 8)`: more than one positional after a keyword). Never edit it
  into valid syntax. These sites carry an
  `// intentional error: ... -- do not remove or make valid` comment naming
  the malformation and print an INTENTIONAL banner into the log just before
  the error fires (TESTING.md rule 5).

### comdraw graphical-scripting tests — `src/comdraw/tests/`

`.comt` scripts exercising comdraw-specific scripting (raster pixels,
graphic create/select/export, dot-assignment on comps, `lastkey()`) --
comdraw is comterp plus graphics in one process, so these need a mapped
window; comdraw's startup seed `update()` (`main.c`) realizes the canvas
before any script runs:

```bash
comdraw -stdin_off -runfile src/comdraw/tests/run_all.comt   # scripted/CI
comdraw -runexpr 'run("src/comdraw/tests/run_all.comt");exit'
```

Same conventions as the ComTerp unit tests above (accumulate `ok`,
register in `run_all.comt`, etc); what's different at this layer,
the test inventory, and the GUI-vs-headless boundary are in
**`src/comdraw/tests/TESTING.md`**. `lastkey()`'s own return value can't
be scripted headlessly -- it depends on a real X11 KeyPress, and there's
no XTest under CI's `xvfb` -- but `keyname_test()` calls the same
production naming logic (`ComEditor::keyname()`) directly with a
synthetic keysym, bypassing the keyboard entirely, and is registered
`hidden` (`ComFunc::hidden()`) so it stays out of `help()`'s listing;
see `src/comdraw/tests/lastkey.comt` and
`src/comdraw/examples/lastkey_keytest.comt` (the manual,
human-at-a-keyboard complement, deliberately excluded from every
automated suite).

### DrawServ integration tests — `src/drawserv_/tests/`

These are **integration** tests requiring a live X11 display, free TCP ports
(20000+), and the `drawserv`/`comterp_listen` binaries on `PATH`. They are
driven by the **`drawmo`** orchestrator (itself a `comterp_listen` script that
launches drawserv instances, talks to them via `remote()` over TCP, and shuts
them down):

```bash
./drawmo                         # run all; exit 0 = pass, 1 = any failure
./drawmo --tests updown          # one test
./drawmo --tests updown,updown1  # several
./drawmo --help
```

Conventions, the `updown` launch pattern, port stepping, and what belongs here
vs. in `comterp_/tests` are in **`src/drawserv_/tests/TESTING.md`**.

---

## Where the deep docs live

This codebase is unusually well-documented at the source level. Before changing
C++ in a layer, read its docs:

| Doc | Covers |
|-----|--------|
| `src/ComUtil/ARCHITECTURE.md` | the 1989 compiler chain: scanner, parser (shunting-yard), postfix codegen, error system |
| `src/ComTerp/ARCHITECTURE.md` | the evaluation model: postfix execution, the argoffval bookmark, eager vs. lazy (`post_eval`) commands, pedepth |
| `doc/POSTFIX-INDEXING.md` | ground-truth on postfix buffer layout & the two arity families (token counts vs. stack counts) |
| `src/ComTerp/HACKING.md` | **the practical C++ how-to**: adding commands, keywords, error codes, refcounting, the patch workflow, naming, commit conventions |
| `doc/LANGUAGE.md` | the ComTerp language from the user side |
| `src/DrawServ/HACKING.md` | adding distributed commands via the `DrawServCmd` mixin and `dist_script()`; the *REPL is the wire protocol* model |
| `doc/APPENDIX-A..E-*.md` | drawing-editor usage, comterp examples, ivtools programming, Flowtran, layout-as-command |
| `doc/ARG-LEVELS.md`, `doc/FUNC-AND-ARGS-DESIGN.md` | arg/keyword counting levels and the func-and-args design |

---

## Conventions you must follow

These come from `src/ComTerp/HACKING.md` — read it in full before non-trivial
C++ work. The essentials:

### C++ command authoring
- A command is a `ComFunc` subclass with an `execute()` method. **Capture all
  args/keywords by value before calling `reset_stack()`** — `stack_arg()`,
  `nargs()`, `stack_key()` are invalid afterward. Always push exactly one
  return value (`push_stack(ComValue::nullval())` if nothing meaningful).
- Default to **eager** commands. Use `post_eval()` only for conditional/lazy
  argument evaluation (`if`, `while`, `for`, `func`).
- Register globally-available commands in `ComTerp::add_defaults()` in
  `comterp.c`; editor-context commands belong in `ComUnidraw` or higher.
- Cache keyword symbol IDs as `static int x = symbol_add("x")`. Unknown
  keywords are silently ignored **by design** — do not add error checks.
- `docstring()` format: `"retval=%s(arg [optarg] :keyword) -- description"`.
  `[]` = optional positional; never bracket `:keyword` args.
- New error codes update **two files together**: `src/ComUtil/comterp.err`
  (the `#define`) and `src/ComUtil/errsys.c` (the `default_errmsgs` table).
- No `dynamic_cast` anywhere — use virtual dispatch.
- New methods/members go at the **bottom** of the header and source file.

### Code comments
- Keep comments embedded in source code fairly terse declaratives that add
  context the variable and function names don't already document. Code is
  the SPOT for what it does and should be read as code.
- Reserve paragraph-length explanation for the related `.md` file (this
  one, a layer's `HACKING.md`/`ARCHITECTURE.md`, or `doc/`) — a short
  in-code comment can point there instead of inlining the essay.
- Exception: when a locale genuinely needs the long version to head off a
  likely misreading by a future developer that would cause chaos, leave it
  in place.
- If you encounter an existing paragraph-length comment that doesn't meet
  that bar, shorten it and move the substantive content to the relevant
  `.md` file.

### Naming
- `comterp` (lowercase) = the interpreter / binary / `.comt` scripts / REPL.
- `ComTerp` (PascalCase) = the C++ library and its classes.
- PascalCase for classes of Unidraw/InterViews heritage (`ComValue`,
  `DrawServCmd`); snake_case for newer free functions/methods (`dist_script()`,
  `make_brush_cmd()`).
- When writing prose, qualify C++ internals (`ComTerp::runfile()`) to
  distinguish them from same-named ComTerp commands (`run()`).

### Patches & commits
- `git apply` needs exact context. Prefer generating patches with Python
  `difflib` against fresh file copies; never hand-write `@@` hunk headers.
- Use `git commit -F -` with a heredoc for multi-line messages.
- **Commit message convention**: a one-liner (as long as needed) calling out
  every significant change separated by semicolons —
  `Fix <bug>; add <file/feature>; add <doc section> to <file>`. The *PR
  description* carries the full narrative; the commit line is the scannable
  `git log` summary.
- **Code comments never reference GitHub issue/PR numbers** (`#223`, etc.).
  That context belongs in the commit message and PR description — external,
  mutable state has no business embedded in the source tree. Explain the
  *why* directly in the comment instead of pointing at a ticket.
- When bumping interpreter internals, the `PATCH_KEY` constant in the relevant
  `main.c` (e.g. `src/comterp_/main.c`) is bumped so the startup banner shows
  the change.
- **PR description must include a closing keyword** (`Closes #N`, `Fixes #N`,
  etc.) when the PR resolves a tracked issue, so the issue auto-closes on
  merge instead of being left open for manual cleanup.

### ComTerp scripting gotchas (bite C-trained authors)

Ask the interpreter rather than reasoning about it -- it answers questions
about itself, and a two-line probe settles in seconds what an argument about
precedence or evaluation order will not settle at all:

    postfix(expr)          how it actually parsed
    help(cmd)              the signature and its keywords
    istype(v :sym)         what you are holding, without consuming it
    info(strm).func        which stream implementation this is
    print(v :str)          the rendering, as a string you can compare

Every entry below was found that way, most of them after confidently
believing the opposite.

- **Everything is an expression**; there are no declarations. `func` is a
  *command* that returns a `FuncObj` — write `name=func(...)`, never
  `func name (...)`. A func that "returns nil" is usually this mistake.
- **Append with `,` (the tuple operator), not `list()`.** `lst,x` appends in
  place; `list(lst x)` builds a nested list-of-lists.
- **A one-element list needs the trailing comma** -- `('x',)` is a one-element
  list, `('x')` is just a parenthesized value. Comparing a one-element result
  against the second form silently fails.
- **There is no `%%` escape in `print()`.** A literal percent is just `%`.
  Writing `%%` before a verb letter leaves a stray `%` and a *live* verb --
  `"%%v"` prints `%` and then consumes an argument.
- **`symid()` takes its argument unevaluated; `symstr()` evaluates.** With
  ``f=`abc``, `symid(f)` answers about the name `f`, not about `abc`, while
  `symstr(f)` gives `"abc"`. To read a symbol *value* out of an attrlist field
  use `symstr(al.field)`, or compare it directly against a bquoted symbol.
- **Measuring a stream can consume it.** A stream argument overdrives an
  ordinary command, so `type(s)` reports once *per element* and leaves the
  stream exhausted. Use `istype(s StreamType)`, which inspects rather than
  being overdriven. The same overdrive is why a `%v` of a stream drains it.
- **A char is signed.** `int(char(160))` is `-96`; `:u` asks for the unsigned
  reading, `int(char(160 :u))` is `160`. The display goes by the unsigned byte
  either way, so what you see and what arithmetic sees can differ.
- **The space binds looser than `,`, and looser than everything else** -- which
  is why it separates arguments. Loosest first:
  `space < , < comparison/arithmetic < unary $$ $ *`. So `list(1,2,3)` is one
  argument and `list(1 2 3)` is three; a comma-built list needs no parens as an
  argument, but a space-form literal does (`list((1 2 3))`), and `f((:a 1))`
  passes an attrlist where `f(:a 1)` passes a keyword to `f`. Reaching for
  parens defensively is the wrong instinct -- `((1,2,3))` has a pair too many,
  `((1 2 3))` does not, and what is inside decides which. The trap this hides
  is unary: `$$1,2,3` is `stream(1)` with `2` and `3` glued on, not a stream
  over the list. `postfix(expr)` shows the parse when in doubt.
- **Never use a termination test that goes true on nil.** An unsupplied
  `arg(n)` reads nil, so every arg-based func has a "called with too few
  arguments" path landing straight in the body. `nil!=0` is `true`, so
  `while(b!=0 ...)` spins forever on a bare call; `nil>0` is nil, so
  `while(b>0 ...)` just ends. Prefer the comparison that propagates nil
  (`>`, `<`) over the one that answers it (`!=` manufactures a `true`;
  `==` at least lands on `false`). Priming the inputs before declaring the
  func removes the nil at the source instead.
- **Don't let a value pulled from a stream be the loop condition — test what
  you got instead.** `while(item=*fifo ...)` reads naturally and mis-terminates
  silently, because a pulled value's truthiness rarely matches the intent: a
  stream object is *true* (so `while(b!=0 ...)` where `b` is a whole stream
  never ends), a bquoted symbol is *false* (so a `` `EOS `` marker ends the
  loop the moment it arrives, having been delivered correctly), and a string
  is true where a `0` is false. Prime the variable, make the condition an
  explicit test, and re-read at the bottom of the body:

  ```
  item=*fifo;
  while(<test on item>
    ...body...;
    item=*fifo);
  ```

  The test names what you want to *continue* on, so its polarity follows what
  the fifo carries: `!istype(item SymbolType)` for ordinary data terminated by
  a `` `EOS `` marker (the marker is the one symbol, so the negation is the
  continuation), or `istype(item StreamType)||istype(item SymbolType)` for a
  ring carrying streams plus a marker the body also acts on. Taking either
  pair verbatim from the other case inverts the loop.

---

## Git / workflow

- Work on a dedicated feature branch, never directly on `master`. Develop,
  commit, and push to that branch (create it locally if missing); push with
  `git push -u origin <branch>`. Never push to another branch without explicit
  permission. (A task that assigns you a specific branch overrides this — use
  the branch it names.)
- Do **not** open a pull request unless explicitly asked.
- CI is a real gate: `.github/workflows/ci.yml` builds the whole tree with g++
  on Ubuntu (a different compiler from the macOS dev builds, so g++-only
  warnings surface there) on every push to `master` and every PR, then runs
  all three self-hosted suites above (`comterp_`, `comdraw`, `drawserv_`'s
  `drawmo`) headless under `xvfb`, plus a fast ACE-lite standalone-unit gate
  ahead of the full build. A failing test fails the check. Still verify
  locally first — the full run takes several minutes.
  `.github/workflows/send-merge-summary.yml` separately emails a changelog
  summary (from the PR body) when a PR merges to `master`; unrelated to the
  test gate.

---

## Quick orientation for common tasks

- **Add a ComTerp command** → `src/ComTerp/HACKING.md` (*Adding a New
  Command*), register in `comterp.c`, add a `.comt` test in
  `src/comterp_/tests/` per its `TESTING.md`.
- **Add a distributed (DrawServ) command** → `src/DrawServ/HACKING.md`
  (`DrawServCmd` mixin + `dist_script()`), add a `drawmo` test per
  `src/drawserv_/tests/TESTING.md`.
- **Understand evaluation/parsing behavior** → `src/ComTerp/ARCHITECTURE.md`
  and `doc/POSTFIX-INDEXING.md`; confirm against the tests.
- **Debug memory corruption** → `config/SANITIZE.md` (AddressSanitizer).
- **Learn the language to write scripts** → `doc/LANGUAGE.md`, then
  read the `.comt` files in `src/comterp_/tests/` (the suite is the tutorial).
