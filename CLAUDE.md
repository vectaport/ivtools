# CLAUDE.md

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
`INTRODUCTION.md` for the conceptual tour and `src/comterp_/LANGUAGE.md` for the
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
INTRODUCTION.md       project overview & philosophy
APPENDIX-A/B/C-*.md   drawing-editor, comterp-examples, programming guides
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
| `comterp_` | the `comterp` / `comterp_listen` binaries + `LANGUAGE.md` + `tests/` |
| `Attribute`, `AttrGlyph`, `ComGlyph`, `IVGlyph` | property lists, glyphs |
| `Unidraw`, `UniIdraw`, `OverlayUnidraw`, `ComUnidraw` | drawing-editor frameworks |
| `GraphUnidraw`, `FrameUnidraw`, `TopoFace` | graph/frame/spatial-network editors |
| `DrawServ`, `drawserv_` | distributed drawing server + its `tests/` |
| `comdraw`, `drawtool`, `idraw`, `graphdraw`, `flipbook` | editor binaries |
| `tests` | top-level / cross-cutting tests (e.g. y2k) |

Note the trailing-underscore convention: `comterp_`, `drawserv_`, `comterp_`
are the **program** directories; `ComTerp`, `DrawServ` are the **library**
directories. See the naming convention below.

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

There is no `make test`. ivtools has **two self-hosted test suites**, both
written in ComTerp itself.

### ComTerp unit tests — `src/comterp_/tests/`

`.comt` scripts run from inside a built `comterp`/`comdraw`/`drawserv`:

```
run("src/comterp/tests/run_all.comt")   # runs every script, prints pass/FAIL
```

Each script returns a boolean `ok`; `run_all.comt` aggregates them. The full
coverage taxonomy, scoring methodology, header format (`// coverage:`,
`// funcs:`, `// missing:`), and the **four mandatory rules for
LLM-authored test scripts** are in **`src/comterp_/tests/TESTING.md`**. Read it
before adding or editing any `.comt` test. Highlights:

- Use `print()`, never `printf()`. Fixed args before keyword args.
- Each test accumulates: `ok=ok&&(result==expected)`.
- The print label must embed the **actual ComTerp expression** under test, not
  a prose description — the log doubles as documentation.
- Keep the `print("scriptname: %v\n" ok)` / `ok` footer as the last two lines.
- Register new scripts in `run_all.comt`.

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
| `src/ComTerp/POSTFIX-INDEXING.md` | ground-truth on postfix buffer layout & the two arity families (token counts vs. stack counts) |
| `src/ComTerp/HACKING.md` | **the practical C++ how-to**: adding commands, keywords, error codes, refcounting, the patch workflow, naming, commit conventions |
| `src/comterp_/LANGUAGE.md` | the ComTerp language from the user side |
| `src/DrawServ/HACKING.md` | adding distributed commands via the `DrawServCmd` mixin and `dist_script()`; the *REPL is the wire protocol* model |
| `APPENDIX-A/B/C-*.md` | drawing-editor usage, comterp examples, ivtools programming |

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
- When bumping interpreter internals, the `PATCH_KEY` constant in the relevant
  `main.c` (e.g. `src/comterp_/main.c`) is bumped so the startup banner shows
  the change.

### ComTerp scripting gotchas (bite C-trained authors)
- **Everything is an expression**; there are no declarations. `func` is a
  *command* that returns a `FuncObj` — write `name=func(...)`, never
  `func name (...)`. A func that "returns nil" is usually this mistake.
- **Append with `,` (the tuple operator), not `list()`.** `lst,x` appends in
  place; `list(lst x)` builds a nested list-of-lists.

---

## Git / workflow

- Active development branch for this task: **`claude/claude-md-docs-nukptb`**.
  Develop, commit, and push there; create it locally if missing. Never push to
  another branch without explicit permission. Push with
  `git push -u origin <branch>`.
- Do **not** open a pull request unless explicitly asked.
- CI is light: `.github/workflows/send-merge-summary.yml` emails a summary when
  a PR is merged to `master` (using the PR body as the changelog). There is no
  build/test CI gate — verify locally.

---

## Quick orientation for common tasks

- **Add a ComTerp command** → `src/ComTerp/HACKING.md` (*Adding a New
  Command*), register in `comterp.c`, add a `.comt` test in
  `src/comterp_/tests/` per its `TESTING.md`.
- **Add a distributed (DrawServ) command** → `src/DrawServ/HACKING.md`
  (`DrawServCmd` mixin + `dist_script()`), add a `drawmo` test per
  `src/drawserv_/tests/TESTING.md`.
- **Understand evaluation/parsing behavior** → `src/ComTerp/ARCHITECTURE.md`
  and `POSTFIX-INDEXING.md`; confirm against the tests.
- **Debug memory corruption** → `config/SANITIZE.md` (AddressSanitizer).
- **Learn the language to write scripts** → `src/comterp_/LANGUAGE.md`, then
  read the `.comt` files in `src/comterp_/tests/` (the suite is the tutorial).
