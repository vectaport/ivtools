## Layout as a Command: unifying structured documents and structured graphics

A structured document and a structured drawing look like two different things to
build. A drawing is marks with *given* positions; a document is text that flows,
breaks, aligns, justifies — geometry nobody placed by hand. The long-standing
assumption was that documents therefore need their own substrate: a constraint
engine, a layout engine, something running underneath that owns the positions.

This appendix records a smaller claim, which Unidraw turns out to have been
demonstrating all along: **a structured document is just a structured drawing
whose geometry is *derived* rather than *given*** — and the thing that derives it
is an ordinary `Command`.

### Given geometry and derived geometry

In direct manipulation a mark's position is *given*: it is where your hand
dragged it. In a document a position is *derived*: it is what a rule computed —
"left-aligned with that one," "broken to this measure," "centered in the box."
These are not different *kinds* of structure. They are the same structure with
the position arrived at two different ways.

Unidraw already has a name for "arrive at positions by rule, as an undoable,
scriptable step": a `Command`. `AlignCmd` is the proof in miniature. The instant
a selected object's position stops being *where you left it* and becomes *what
the alignment computed*, that object has crossed the entire document/graphics
boundary — by rule instead of by hand — inside the graphics model, undoable like
any other edit. Alignment is the atom of layout, and it was a graphics command
the whole time.

### A worked demonstration: Square Wire

`AlignCmd` instantiates the idea but is too simple to *reveal* it — "tidy up the
selection" is a sufficient reading, and the sufficient reading wins. The idea
becomes visible only at a command rich enough to resist that reading. **Square
Wire** (the `SQUARE_CMD` of `ForkComp` in
[vectaport/ipl-1.1](https://github.com/vectaport/ipl-1.1)) is one such command:
it orthogonalizes a flowgraph by moving a node until each edge it can claim runs
cleanly along an axis.

For each incident edge it compares the run in *x* to the run in *y*; the dominant
axis is zeroed and the node is translated along the other until the two endpoints
share a coordinate — a mostly-horizontal edge snapped flat by a vertical move, a
mostly-vertical edge snapped upright by a horizontal one. It iterates, because
moving the node changes the geometry and the far endpoint can move too; and it
runs incoming edges before outgoing ones, with a `horiz`/`vert` flag so the
second pass *yields* to a straightening the first already made rather than
fighting it. The net translation is folded into a single `MoveData`, so one undo
returns the picture to exactly what your hand made.

Every property that makes Square Wire *work* is a property of the general idea:

- **It is a pass, not a structure.** The orthogonality is never stored in the
  drawing; it is an instruction a command consumes to emit concrete coordinates.
- **"Everything it can."** A node pulled toward both axes cannot satisfy both, so
  it squares what it can and leaves the rest — a best-effort solver that knows it
  is not sovereign.
- **Turn-taking, not takeover.** Because the move is one undoable `MoveData`, the
  optimizer takes its turn when invoked and hands authorship straight back.
  Ownership of the marks is *temporal*, traded across the command boundary.

### Why a command, and not an engine

The historical attempts at this unification all reached for the *heavy* version —
make the layout continuous, total, always on. Unidraw's own connector model
(Vlissides's pins, slots, and pads, with the constraint solver behind them) kept
relationships alive in the document, a solver running to maintain them. The
flyweight-glyph composition of InterViews 3.1 (Calder and Linton) pursued layout
that would simply *emerge* from the tree, declaratively, TeX-quality. Both are
the "engine continuously owns the marks" shape — powerful, and in tension with
the hand, because the moment a user places one mark by fiat the continuous solve
is broken.

The command does less, and that is why it closes. Isolating an optimizer under a
command does not cage it: inside the pass it can be as global and ruthless as
Knuth's line-breaker. What the boundary bounds is not the optimizer's *power* but
its *tenure* — it owns the marks for the duration of the pass, not forever. Full
power, bounded tenure. The hand and the solver share the document because neither
holds it continuously.

### In ivtools: the command is the protocol

This is why the idea is at home here specifically. In ivtools a `Command` is, at
the upper layers, a ComTerp expression — and ComTerp's defining property is that
the command language is also the wire protocol. So a layout pass is not a private
mechanism bolted into the editor; it is a first-class, scriptable, transmissible
command. The document becomes the script that lays it out: graphics plus a stream
of layout commands, where the stream is the file is the message.

The capability arrives in two rungs. `drawtool` (OverlayUnidraw) already composes
with an external renderer the Unix way — export to a shell command, piping
PostScript through Ghostscript — *using* a command without yet having a command
language of its own. `comdraw` (ComUnidraw) and up wire in ComTerp, and only
there do "render," "align," "square the wires" become the same kind of thing:
native commands in a language that is also the protocol. `drawtool` learned to
*use* a command; `comdraw` learned to be *made of* them.

### The unwritten menu

`AlignCmd` and Square Wire are two members of a family whose other members are
not yet written: distribute, flow, break, justify, resolve-glue. Glue itself
belongs here and not in the drawing — it was never graphical *structure* you
could point at in the finished page, but a *parameter to a pass*: "when the
layout command runs, the slack goes here." Same category as an orthogonality
constraint, same category as a line-break penalty. Structure is always the
concrete result; glue, constraints, and typeset-specs are the declarative inputs
to the commands that produce it.

So the "document layer" a drawing editor is missing was never an engine. It is
two modest things: a way to *write the spec down*, and a *command* that consumes
it — of the shape Unidraw already had, and in ivtools, of the shape ComTerp
already speaks.
