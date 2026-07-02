# Spatial Applications on ivtools

ivtools exists to build custom drawing editors and spatial-data servers,
but "spatial application" names a space of recurring *shapes* an app can
take, not just a stack of libraries.  This document catalogs those
shapes: each category names a genre, gives its anatomy in terms of the
ivtools layers, points at a reference implementation in the tree, and
records the design rules learned building it.

The categories share one premise, inherited from the whole system: **the
command language is the application surface.**  There is no app layer
sitting above comterp that a user graduates *from* — the words they type
to use the application are the language they will extend it in.  Every
genre below is a different way of cashing that premise in.

---

## Category 1: The Askable Map

**Motto:** *the drawing is the datastore.*
**Reference implementation:** `src/comdraw/examples/zoomap.comt`
**Runs on:** comdraw (single canvas); drawserv unchanged (shared canvas).

An askable map is a drawing you can question.  Every graphic on the
canvas carries facts about itself; small typed words walk the drawing,
read the facts, and answer *on the canvas* — the matches get circled
in red marker, the answer is a place, not a table.

```
(comt) who("swims")
Found 1! Look for the red circles!     // a ring blinks around Fin in the pond
(comt) haslegs(4)
Found 2! Look for the red circles!     // rings blink around Ellie and Leo
(comt) countlegs()
All the legs in the zoo add up to: 12
```

Historically, the example is a deliberate graft: an Adventure-era TUI
(typed magic words, prose answers, a parser that scolds you) hooked to
a MacDraw-era GUI (a direct-manipulation canvas with tools, menus, and
selection).  Those two interface eras barely overlapped in the wild,
and each is one-eyed alone — the text adventure can't show you *where*,
the drawing editor can't tell you *what it knows*.  Here they share one
interpreter and one component tree, so the graft takes: the typed words
and the mouse gestures operate on the same graphics, and each half
answers the question the other half can't ask.

### Anatomy: three layers on one canvas

1. **The drawing** — ordinary graphics (`rect`, `ellipse`, splines,
   `text`) styled with `colorsrgb`/`pattern`/`brush`.  The picture is
   built by script, in the `petals.comt` tradition: the script is the
   picture.

2. **The data** — `setattr()` sticks an attribute list directly onto
   each graphic:

   ```
   ellie=ellipse(300,260, 55,35)
   setattr(ellie :name "Ellie" :kind "animal" :legs 4 :eats "plants" :moves "stomps")
   ```

   This is the load-bearing move.  The spatial database lives *on* the
   graphics, not beside them — no parallel table, no glue on the canvas.
   Save the drawing and the database goes with it; select a graphic and
   you are holding its record.

3. **The questions** — one-screen funcs that walk the component tree
   (`frame()`, `size()`, `at()`), read facts back with dot-access
   (`g.eats=="bugs"`), collect matches, and answer with display acts:
   each match gets a temporary red marker ring — an unfilled
   thick-brush ellipse sized from its `mbr()`, blinked with
   `hide()`/`show()`, then deleted, so the animals underneath never
   change:

   ```
   who=func(
     target=arg(0);
     matches=list();
     n=size(frame());
     for(qi=0 qi<n qi++
       g=at(frame() qi);
       if((g.name==target)||(g.moves==target)||(g.eats==target) :then matches,g));
     if(size(matches)>0 :then flash(:things matches));
     size(matches))
   ```

### Why this genre teaches the language

The askable map is a comterp tutorial that doesn't announce itself:

- **User words and system words are the same kind of word.**  A func
  bound to a symbol invokes bare, just like a built-in — `dance`,
  `zoohelp`, `countlegs` all work without parens, exactly as `update`
  and `zoomin` do.  The user's vocabulary and the shipped vocabulary
  are indistinguishable in use.

- **Extension is indistinguishable from use.**  The help text ends with
  a "your turn" recipe: create a shape, `setattr()` its facts, and the
  existing queries find it immediately.  The moment a kid adds Penny
  the pig and `who("Penny")` flashes her, they have crossed from user
  to programmer without noticing a boundary.

- **Answers are spatial.**  Query results are display acts — the map
  flashes *where* — with prints reserved for counts and sums.  The
  feedback loop runs through the canvas, which is what makes it a
  spatial application rather than a database with a picture attached.

### Error pedagogy: a wrong answer that looks right is the worst outcome

Playtesting with malformed input found the genre's characteristic trap.
An unquoted word — `who(swims)` — hands the func an unbound symbol,
which evaluates to nil; and since `nil==nil` is true, a nil target
"matches" every graphic that *lacks* the compared attribute.  In the
zoo that meant all six name tags, sitting right on top of the animals:
`who(swims)` confidently flashed what looked like everybody.

The rule that falls out: **guard every argument, and make the guard
teach the next thing to type.**

```
if(target==nil :then print("Oops! Put quotes around your word, like this:  who(\"swims\")\n"));
if(target==nil :then return(0));
```

Four tiers of miss, in decreasing order of the applet's control:

- **Guarded miss** — nil/malformed args caught by the func, answered
  with a corrected example to retype.
- **Honest miss** — well-formed query, no matches (`who("Swims")`,
  case-sensitive): a friendly "nobody matches, try zoohelp()".
- **Syntax miss** — a stray operator or unbalanced paren falls through
  to the interpreter's parser, which prints its raw errsys message.
  The applet cannot intercept that moment, but it can reach the error
  *after the fact*: `errmsg(:last)` returns the saved message — it
  survives any good commands typed in between, and reading it clears
  it.  zoomap wraps this in a recovery word:

  ```
  (comt) 1+*2
  comterp:  (1) Unexpected operator (*)
  (comt) oops()
  The map got confused. It said:
      "comterp:  (1) Unexpected operator (*)"
  Check for: quotes around words -- who("swims") -- and matching ( ) pairs.
  ```

  The interpreter's words are replayed, then followed with a
  kid-readable checklist.  Only the raw wording itself remains an
  errsys (`comterp.err`) question.

- **Silent miss** — a misspelled word (`zoohlep()`) is an unknown
  symbol: it returns nil with *no error saved*, by the same design
  that silently ignores unknown keywords.  This is the one tier no
  recovery word can see; the applet's defense is magic words short
  and pronounceable enough not to misspell.

### The distribution path

Because the REPL is the wire protocol, the askable map is already a
distributed application.  Run the same script under drawserv and the
queries travel unchanged:

```
sock=socket("localhost" 20002)
remote(sock "who(\"swims\")")     // Fin flashes on the shared canvas
```

Two kids, two canvases, one zoo — and nothing about the applet needed
to change.  The genre scales from a REPL toy to a spatial-data server
by re-running the file.

### Design rules for building another one

1. Facts live on the graphics (`setattr`) — never in a parallel
   structure the canvas can drift away from.
2. Answers happen on the canvas, as drawn marks — zoomap circles
   matches with temporary ring graphics rather than leaning on
   selection tic marks; print only what can't be shown (counts, sums).
3. Queries are one-screen funcs with names you can say out loud
   (`who`, `haslegs`, `countlegs`, `dance`).
4. Help ends with a your-turn recipe that extends the app using only
   words already taught.
5. Guard every argument; prefer a corrected example to an error
   message.  Remember the nil==nil trap.
6. Style commands (`colorsrgb`, `pattern`, `brush`) paint the current
   selection — `select(:clear)` before restyling for a new batch, or
   the last-created graphic silently changes color.
7. Append to match-lists with bare `lst,x` — parenthesized `(lst,x)`
   silently drops appends.  Creation coordinates are comma-tuples
   (`rect(0,0, 50,50)`), not space-separated.  `delete()` takes
   graphics one at a time — a list variable doesn't unpack.
8. Give the app a recovery word.  `errmsg(:last)` makes even parser
   errors part of the applet's pedagogy — an `oops()` that replays
   the saved message with a checklist costs six lines.

The same skeleton re-skins freely: a treasure map (`dig("palm tree")`),
a neighborhood map (`who("sells ice cream")`), a solar system
(`hasmoons(2)`), a garden planner (`blooms("May")`).  Swap the facts
and the questions; the three layers and the rules stay.

### Future work: the mouse half of the dialogue

So far the conversation runs one way — words in, canvas out.  But the
MacDraw half of the graft is an *input* device too, and the genre's
next moves put the mouse into the dialogue:

- **Select-then-ask.**  `select()` with no arguments returns the
  current selection — including one made by clicking.  A `tellme()`
  that walks the selection and speaks each graphic's facts turns any
  mouse click into a query: point at the thing instead of naming it.
  Scriptable today.

- **Pens and misplacement.**  Draw enclosures that carry facts of
  their own (`setattr(cage :kind "pen" :holds "flies")`).  Now the kid
  can *drag* an animal somewhere wrong — Zig in the bird cage — and a
  `wrongpen()` query catches it, because containment falls out of
  comparing `center()` points against pen `mbr()` boxes.  This is the
  genre's deepest question shape: a mismatch between where things
  *are* (geometry) and where their facts say they *belong*
  (attributes).  The mouse creates the puzzle; the words find it.
  Scriptable today.

- **The map that notices.**  True direct manipulation closes the loop
  without typing: the drop itself makes the map speak — "Zig doesn't
  fly!"  That needs an editor-side hook, a manipulator/Command
  callback that hands drag-and-drop events to interpreter funcs the
  way `errmsg(:last)` hands over parser errors.  The natural C++ next
  step for the genre.

---

## Future categories

Named but not yet written up; each has at least a seed in the tree.

- **The Animated Tableau** — the script is the picture, then the
  picture moves: group/rotate choreography paced by `update(usec)`
  (`src/comdraw/examples/petals.comt`).
- **The Shared Canvas** — two or more drawserv instances linked with
  `drawlink`, one drawing among them; play and pedagogy in the
  presence of another hand (`src/drawserv_/tests/` has the plumbing).
- **The Live Feed Display** — a canvas fed by an external process via
  `import(:popen)` or sockets; the map as instrument panel.
- **The Network Diagram** — nodes and edges as first-class comps
  (GraphUnidraw/TopoFace); topology you can query and traverse.
