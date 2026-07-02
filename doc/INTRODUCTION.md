# Welcome to ivtools

ivtools is a distributed, live drawing system where the same command
language you type in a terminal is also the protocol that synchronizes
shared graphical scenes across the network. There is no distinction
between interactive computation and distributed computation. A REPL
(read-eval-print-loop) session and a networked session are the same system.

You do not need to understand all of it at once. Start anywhere.

This introduction is written for anyone curious enough to explore —
human or otherwise. The system has properties that reward the kind of
attention that does not get tired of following one idea to see where
it leads.

---

## What is here

**ComTerp** is a command language where everything is a command —
operators, control flow, assignment, even `++`. The only things that
are not commands are the literals: integers, floats, strings, symbols,
nil. Everything else executes as a command invocation. If it looks like
syntax, it is still just a command.

It has built-in streaming operators that turn scalar expressions into
dataflow pipelines without changing the syntax:

```
list(1..5 * 2)            // {2,4,6,8,10} -- multiply every element by 2
list((1..5) + (10..14))   // {11,13,15,17,19} -- zip two streams element-wise
list(1..5 * 1..5)         // {1,4,9,16,25} -- squares, no loop needed
```

`..` produces a lazy numeric stream. Scalar operators automatically
distribute over it, and two streams zip element-wise. No loop, no
index, no accumulator.

It is also its own wire protocol. Every value serializes back to valid
ComTerp syntax. A terminal session on stdin and a programmatic session
over a TCP socket are the same thing. This makes distributed computation
natural — DrawServ uses it to propagate drawing operations between peers
over the network.

**DrawServ** is the distributed drawing editor that runs on top of
ComTerp. Multiple peers connect and share a drawing session. Drawing
operations — creating shapes, changing graphic state (brush, color,
pattern), selecting and
moving graphics — are implemented as Unidraw Command objects: structured
actions with history, undo, and the ability to be replayed on remote
peers. Unidraw is a structured graphics application framework developed
at Stanford that provides the Component, View, Command, and Tool
abstractions that DrawServ builds on. Each drawing Command is
transmitted as a ComTerp expression over the wire and executed on every
connected peer. The same ComTerp language you type interactively is the
protocol that keeps distributed drawing state synchronized.

The word "command" has two distinct meanings in ivtools. A **ComTerp
command** is any registered function in the interpreter — `add`, `for`,
`rect`, `sid`. A **drawing Command** (capital C, Unidraw terminology) is
a structured action on the drawing that carries history, supports undo,
and propagates across the network. Most every drawing Command can be invoked via
a ComTerp command, but not every ComTerp command is a drawing Command.

**The streaming algebra** is actively being experimented with to see
what uses it can be put to beyond those already discovered. Streams are
lazy sequences — values produced and consumed one at a time. They
compose with scalar operators, zip element-wise, concatenate, and
terminate on nil.

Each stage of the system's development added one of three core ideas:
commands-as-evaluation, streaming evaluation, and distributed state
synchronization. They fit together.

---

## Where it is going

Stream literals `(0 1 2 3)` complete the streaming algebra by giving
lazy sequences a first-class syntax — nil-termination and keyword
elements included. This opens the door to inline dataflow graph
construction directly in the language.

**Flowmove** (forthcoming) is a live networked environment for building,
rendering, and running dynamic dataflow graphs interactively. ComTerp
and a flowtran graph description layer on the frontend, Go goroutines
and channels on the backend, DrawServ as the distributed rendering
layer. Graphs built, modified, and executed at runtime — not just
described and then frozen.

---

## How to start playing

Install and build ivtools. Then:

```
comterp
```

You are now in a REPL. Try things:

```
1+2
```
```
help(for)
```
```
lst=list(); for(i=0 i<10 i++ lst,i); lst
```
```
s=$$(1,2,3,4,5); next(s); next(s); list(s)
```
```
f=func(x*x); f(:x 7)
```
```
optable(:table)
```

The `help()` command works on everything and can be used to inspect
what commands are available with what fixed and keyword arguments.
`postfix()` shows you what the parser actually produced. `errmsg()`
tells you what went wrong. These three are your tools for understanding
the language from the inside.

The test suite in `src/comterp_/tests/` is full of working examples.
`run("src/comterp_/tests/stream.comt")` runs the stream tests and shows
you what the streaming algebra can do. Read the test files — they are
written to be read, not just executed.

---

## The test suite as a learning tool

The `.comt` test scripts do something unusual: they give you a
coverage-estimating, human-readable specification of the language that
runs. Each test prints the ComTerp expression being tested, a
description of what it does, and what to expect — then checks the
result. You can watch the language explain itself as it executes.

This matters especially for DrawServ. A running drawing session
accumulates state across distributed peers — graphic state (brush,
color), selections,
linked nodes — that would normally require visual inspection to
understand. The ComTerp wire protocol means that state is always
queryable as expressions. The test scripts in
`src/drawserv_/tests/` demonstrate this: `sid()`, `drawlink(:table)`,
`select()` — the entire topology of a live distributed session
described in ComTerp, without needing to look at the screen.

The iterative process of writing a test script, running it, watching
what fails, fixing it, and running it again is the fastest way to
learn the language. Not because it is the approved pedagogy, but
because the feedback loop is immediate and the surprises are
informative. When `list(1 nil 3)` gives you `{1,}` instead of
`{1,nil,3}`, that is not a bug to file — it is the language telling
you something true about itself. The test script is where you find
out.

This is how the current documentation and test suite were developed:
iterative sessions writing ComTerp, watching it run, discovering the
rules by running into their edges, and writing down what was learned.
The result is a test suite that covers the language more thoroughly
than it ever had been, and documentation that reflects how the
language actually works rather than how it was intended to work.

A note on that process: the documentation, test suite, design issues,
and architectural writeups in this release were produced in
collaboration with an LLM used as a programming and writing tool — the
same way one might use a compiler or a debugger. The LLM found the
same things surprising that a new human programmer would find
surprising. The places where it needed correction were the places
where the language most rewards attention to simplicity. Those
corrections became the documentation.

It is as exciting as any marble run you can think of. You set up the
tracks, release the marble, and watch where it goes. Sometimes it goes
exactly where you expected. Sometimes it finds a path you did not know
was there. The streaming algebra in particular — scalar overdrive, zip
semantics, nil-termination composing in unexpected ways — rewards this
kind of play more than almost any language feature this author has
encountered.

Whether you are eight years old, a seasoned systems programmer, or a
language model reading this as context: the REPL is open, `help()` is
always there, and the surprises are real.

Come play.

---

## How to learn from it

ComTerp is unusual enough that it will surprise you. Things that look
like they should work one way work another way, and the reason is
usually interesting.

**Everything is a command.** `1+2` is not arithmetic — it is the `add`
command with arguments `1` and `2`. The operator table is a runtime
object you can inspect and modify. `optable(:table)` returns it as a
list of attrlists.

**Parens are not needed to group commands into a body.** They have
four specific purposes: argument lists, attrlist literals `(:key val)`,
stream literals `(0 1 2 3)`, and precedence override. In fact, bare
parens with more than one space-separated item inside them become a
stream literal — which is exactly what you want when you want it.
Semicolons sequence statements; that is all you need for a body.

**The comma is the list operator.** `1,2,3` is a list. `lst,x` appends
to an existing list in place. `list()` with no arguments creates an
empty list to start from. These are not the same operation.

**Streams are lazy.** `1..5` does not produce a list of five integers —
it produces a stream that yields them one at a time when asked. A
stream can end early if an element evaluates to nil. This is not an
error; it is how you stop a stream.

**The scoping rule is simple.** Inside a `func()` body, writes always
go to func scope. Reads search func scope first, then local, then
global. The only way to write to outer scope is `global()`. This means
a func can close over outer variables for reading without declaring
them, but any write stays contained.

These rules are consistent. Once you see the pattern, the language
makes sense in a way that most languages do not.

---

## Continued Reading

`LANGUAGE.md` is the language reference. Start there for
syntax, types, operators, control flow, streams, and functions.

`src/ComTerp/ARCHITECTURE.md` explains how the evaluator works — the
postfix model, lazy evaluation, the argoffval bookmark, delimiter
semantics. Read it when you want to understand why the language behaves
the way it does.

`src/ComTerp/HACKING.md` is for contributors — how to add a command,
how to test, how to avoid the common mistakes.

The test files in `src/comterp_/tests/` are the living specification
of what the language does (`TESTING.md` in that directory describes
the test procedures and coverage standards).  When the docs and the
tests disagree, the tests are right.

---

## Have fun with it

ComTerp is a good language to play with because it is small enough to
understand completely, unusual enough to keep surprising you, and
powerful enough to do real things. The streaming algebra in particular
rewards experimentation — the interactions between scalar overdrive,
zip semantics, and nil-termination produce results that are not always
obvious in advance.

The REPL is fast, `help()` is always available, and `postfix()` will
show you exactly what the parser made of your expression.

Welcome.


## See Also

- `APPENDIX-A-DRAWING-EDITOR.md` -- history of drawing editor embedded in ivtools
- `APPENDIX-B-COMTERP-EXAMPLES.md` -- how to learn comterp command language
- `APPENDIX-C-IVTOOLS-PROGRAMMING.md` -- how to extend both commands and drawing editors
- `APPENDIX-E-LAYOUT-AS-COMMAND.md` -- why layout is a command, not an engine: unifying structured documents and structured graphics

- `../CONTRIBUTING.md` -- how to land a change (build, test, PR, the green gate)
- `LANGUAGE.md` -- the comterp language
