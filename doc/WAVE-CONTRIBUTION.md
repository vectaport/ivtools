# WAVE-CONTRIBUTION.md

## ivtools improvements for Wave (2013–2014)

This note records a specific period of work on ivtools — thirteen patches
authored between **October 2013 and April 2014** by Scott Johnston
(`scott@wavesemi.com`) while at Wave — and the understanding of how that work
fit into the larger design. The patches are preserved verbatim in
`patches/ivtools-13*-scott-0*.txt` and `patches/ivtools-14*-scott-0*.txt`
(added to the repository in commit `fdb8690`), applied against ivtools-1.2.11.
From patch 006 onward each carries the subject line **"support for ipl
development."**

---

## What it was for

The work served the development of **IPL** — the Invocation Programming
Language, a concurrent / flowgraph language after Karl M. Fant's invocation
model — which is built on top of ivtools (the open LGPL line lives at
`github.com/vectaport/ipl-1.1`). None of these patches is IPL itself; they are
improvements to the **substrate** IPL stands on. The relationship was the usual
one for this codebase: the author used his own tools to build something
demanding, and the tools got better under that stress. Almost every change is
in the interpreter core — `ComUtil`, `ComTerp`, `ComUnidraw`, `Attribute` —
not in the drawing editor.

It helps to remember what comterp *is*. It did not begin as a drawing-editor
scripting language; it first came up (at Triple Vision) as the **command layer
over a real-time image-processing pipeline** — a Datacube box. That origin is
visible in its bones: terse, stream-oriented, every value serializing back into
valid comterp syntax ("the command language is the wire protocol"). The
2013–2014 work leans directly on those bones.

---

## The contributions

**Embedding and headless operation** — prerequisites for using comterp as a
batch / tooling engine rather than an interactive REPL:

- `comterp "expr"` — evaluate a single expression from the command line and
  exit (patch 001).
- Guards so component/graph code constructs cleanly with no GUI attached
  (`nodecomp.c`, patch 001).

**A stream/list processing model — and flowgraph construction.** The stream and
list machinery was extended and, importantly, put to work *generating flowgraph
structure*: emitting interconnect and even binary trees in the flowgraph
assembly language IPL used.

> **Note on two "streams."** comterp's `stream` — a lazy value sequence *inside*
> the expression language (`$` / `$$` / `next` / `each` / `depth` / `filter` /
> `,,`) — is a **distinct notion** from a flowgraph's data `Stream`, the
> handshake-driven dataflow that runs *along the edges between hubs*. They share
> the word only. Do not conflate them.

- depth-first traversal of a component tree as a lazy stream (`depth()`), with a
  `GetUp()` hook for the parent axis (patches 002, 003).
- `filter(comps classid)` — filter a stream by class (patch 002).
- `$` (stream) and `$$` (list) unary operators — reworked and re-prioritized to
  sit just above `,,` stream-concat, and extended to streamify/listify a plain
  scalar (patches 007, 008).

**String and numeric primitives** — the crunching a pipeline needs:
`index(... :substr)`; `search` offset/NULL fixes and `:nonil`; `substr :nonil`;
`splitstr` tokenization with delimiter/char and quote handling; `log2`
(patches 001, 006, 009).

**Introspection and debugging** — visibility into the interpreter's own state,
essential for building and debugging real pipelines: `print_stream` (recursive
stream-structure dump), `print_stack(ostream&)`, `print_attrlist`, and the
`class_name()` / `command_name()` / `type_name()` accessors, plus printing of
stream and object values (patches 006, 013).

**Core correctness and robustness**:

- A **shunting-yard parser fix** (`_parser.c`, patches 004 and 005) for command
  arguments that begin with a parenthesized expression — the case that arises
  when flowgraph interconnect is expressed as *multiple* parenthesized argument
  lists. The pending expression is now flushed off the operator stack before the
  paren is pushed (only when a binary operator is expected). This capability
  remains live infrastructure for later flowgraph work.
- `exec()` made virtual, for extensibility (patch 006).
- `popen2` pipe / `O_NONBLOCK` portability (patch 006).
- `break([retval])` — break out of a loop with a value; `continue` documented
  (patch 010).
- A run of null-safety guards in `ulabel`, `ovcomps`, `nodecomp`
  (patches 011, 012).

---

## The understanding

The reason these changes cluster where they do is a layering that took a long
time to arrive, and that IPL finally made load-bearing. Two towers had to exist
first, each hard in its own right:

1. **The combinatorial expression bottom** — `ComUtil` / `ComTerp`: a complete,
   compositional expression evaluator. This is what a flowgraph node
   *computes*.
2. **The node/edge model with graph editing** — `Unidraw` / `GraphUnidraw` over
   `TopoFace`: an *editable* network of nodes and edges. This is the flowgraph's
   topology and the means to build it.

**IPL is the seam where those two meet:** nodes carry expressions from the
bottom layer, wired into a network from the middle layer. An earlier attempt to
grow a flowgraph interpreter *inside the code-conversion pass* (the `xgraph`
struct in `ComUtil`) was the wrong layer — a linear scan → parse → codegen
pipeline is the wrong shape to birth a topology — and it came before either
tower was tall enough to build on.

A telling detail: **GraphUnidraw was reused extensively by IPL and never
modified during this period.** A layer that gets leaned on hard and needs no
edits is the signature of an abstraction that was already right — the quiet
layer that turned out to be the most load-bearing.

---

## Forward

The line continues in **Flowtran**, where comterp is embedded as the possible
*bottom of each hub* (a hub can resolve to a comterp expression — the
combinatorial-expression bottom slotted in as the flowgraph leaf). What runs at
a hub is a **stream-overdriven expression**: the flowgraph `Stream` drives it
per token, while the non-stream portions are **pre-evaluated at flowgraph
construction time** — a binding-time / partial-evaluation split, so the graph
that runs is already partly reduced and each hub does only the work that varies
with the data flowing through it. The parser fix from patches 004/005 is part
of the infrastructure Flowtran reuses — the durable-substrate payoff, again:
reused, not rewritten.

---

## Provenance

The durable record of this work is the git history and the preserved patch
files in `patches/` (commit `fdb8690`), together with the open LGPL line at
`github.com/vectaport/ipl-1.1`. The patch headers name the author and the
dates; the commits carry the rest.

*Compiled from the patches in commit `fdb8690` and the author's account of the
period. Author of the described work: Scott Johnston.*
