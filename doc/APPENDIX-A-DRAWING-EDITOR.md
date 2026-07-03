# Appendix A: The Drawing Editor

## What it is

ivtools is built on InterViews, a C++ UI framework developed at
Stanford in the late 1980s. The drawing editor layer — comdraw and
DrawServ — descends from idraw, one of the first serious vector
drawing editors available on Unix workstations. idraw ran on X11,
produced PostScript, and was fast and direct in a way that most
drawing tools of the era were not.

ivtools extended idraw in three directions: a command interpreter,
distributed sessions, and a component/attribute model that lets
drawings carry data as well as graphics.

## What the drawing editor can do

**Drawing primitives** — rectangles, lines, ellipses, polygons,
splines, text, multilines, rasters. Everything can be created
programmatically:

```
r=rect(100 100 300 200)
t=text(150 150 "hello")
e=ellipse(200 200 80 40)
```

**Graphic state** — font, brush, pattern, color, all selectable from
menus or set by command. Colors can be specified by menu index or by
RGB name in standard hex notation (`#RRGGBB`). Pattern masks can be
set from a 16-bit integer or a list of 16 integers for a 16x16 mask.

**Direct manipulation** — select, move, scale, rotate, flip, group,
ungroup, front, back, delete. All available as commands:

```
select(:all)
scale(2.0 2.0)
rotate(45)
```

**Component attributes** — every graphic component can carry an
attrlist of arbitrary key-value data. `setattr()` attaches data,
`attrlist()` retrieves it. This makes the drawing a database as well
as a picture. Scripts can query and modify component attributes
without touching the visual representation.

**Import and export** — drawings can be imported from files or URLs,
exported to remote drawtool instances over TCP, saved as idraw format
or PostScript. `import()` accepts a `:popen` flag to read from a
command pipeline, making it straightforward to pull data from external
sources directly into the drawing.

**Viewer control** — zoom, pan, coordinate conversion between screen,
drawing, and graphic coordinate systems. `stod()`/`dtos()` convert
screen to drawing coordinates and back. `gtod()`/`dtog()` convert
between a component's local graphic coordinate system and drawing
coordinates.

**Structural queries** — `frame()` returns the composite component for
a drawing frame. `depth()` does a depth-first walk of the component
tree. `parent()`, `size()`, `at()` navigate and index into the
component hierarchy. A drawing is a tree and the tree is queryable.

## Where it came from

idraw was written by Mark Linton and his graduate students at Stanford
in the late 80s.  Unidraw was an evolution of idraw by John Vlissides
that made available an extendible framework that incorporated most of
idraw.  ivtools-0.3 was so numbered because it was the third attempt to
librarify idraw, and this one was built on Unidraw. InterViews was
one of the first serious C++ UI frameworks, and idraw was one of the
first structured drawing editors available on Unix workstations — fast,
PostScript-native, and direct enough to be useful for real work.

Unidraw evolved idraw into exposing the application framework for:
Components, Views, Commands, and Tools as objects that
could be subclassed to build new kinds of graphical editors. ivtools
is built on Unidraw. The ComTerp interpreter hooks into Unidraw's
Command layer — every drawing operation is a Command object that
can be replayed, undone, and propagated across a network using
comterp commands.

Various contracts and employment have fueled the ongoing extensions:
DrawServ, the UUID-based session model, the ACE reactor integration for
TCP communication that makes the REPL the wire format. The requirement
was collaborative editing of annotated imagery across a network — the
kind of thing that would not be straightforward in any drawing tool
until decades later.

The result is a drawing editor that is also a programmable distributed
computing environment. The graphics are the visible surface; the
ComTerp layer is the substrate that makes everything else possible.
And because it comes from the past when the internet was more a
trusting place it by design leaves trust a human problem.

## DrawServ: distributed drawing sessions

DrawServ extends comdraw with peer-to-peer distributed state. Multiple
instances connect over TCP and share drawing sessions. Every command
that modifies drawing state — brush or color change, selection, paste, delete —
is a ComTerp expression that propagates to all connected peers and
executes there.

Each drawing in a session has a unique identifier (`sid`). Each
distributed link between peers is tracked in a `drawlink` table. A
fresh DrawServ instance starts with one sid (the empty drawing) and an
empty drawlink table:

```
sid(:table)           // list of session identifiers
drawlink(:table)      // list of peer connections
```
++
These are queryable ComTerp commands — the entire topology of a live
distributed session is inspectable without looking at the screen. The
test suite in `src/drawserv_/tests/` exploits this: the `drawmo`
orchestrator launches DrawServ instances, connects to them over TCP,
and verifies session state using nothing but ComTerp expressions sent
via `remote()`.

The same wire protocol that the test suite uses is the same protocol
that human users interact with. There is no separate testing API.

## See Also

- `INTRODUCTION.md` — project overview and history
- `APPENDIX-B-COMTERP-EXAMPLES.md` -- how to learn comterp command language
- `APPENDIX-C-IVTOOLS-PROGRAMMING.md` -- how to extend both commands and drawing editors

- `LANGUAGE.md` -- the comterp language
- `src/comterp_/tests/TESTING.md` -- comterp self regression tests
- `src/drawserv_/tests/TESTING.md` -- drawmo testing utility for drawserv
- `src/DrawServ/HACKING.md` -- drawserv programming

