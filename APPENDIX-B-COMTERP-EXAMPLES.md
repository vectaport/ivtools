# Appendix B: ComTerp in Action

These examples show what ComTerp can do across a range of contexts —
from simple expressions to distributed drawing sessions. They are not
a tutorial. They are a sampler. Read them to get a feel for what is
possible, then go explore.

---

## The language as the REPL

```
// arithmetic is just commands
1+2*3                     // 7 -- precedence applies
(1+2)*3                   // 9 -- parens override

// variables
x=10; y=20; x+y           // 30

// strings
s="hello " + "world"      // "hello world"
size(s)                   // 11

// lists
lst=1,2,3,4,5             // {1,2,3,4,5}
at(lst 2)                 // 3
size(lst)                 // 5

// attrlists
al=(:x 10 :y 20)
al.x + al.y               // 30
al.x=99
al                        // (:x 99 :y 20)
```

---

## Streams doing what loops can do

```
// sum 1 through 100
total=0
each(1..100 * 1)          // traverse stream -- but better:
list(1..100)              // materialize it
size(list(1..100))        // 100

// each() traverses entire stream, returns count
each(1..10 * 2)           // 10 -- side effects only, returns count

// element-wise operations, no loop
list(1..5 * 2)            // {2,4,6,8,10}
list(1..5 * 1..5)         // {1,4,9,16,25} -- squares
list((1..5) + (10..14))   // {11,13,15,17,19} -- zip

// repeat
list(0**5)                // {0,0,0,0,0}
list("x"**3)              // {"x","x","x"}

// concatenate streams
list(1..3 ,, 10..12)      // {1,2,3,10,11,12}

// copy a stream and consume both independently
s=$$(1,2,3)
t=$$s                     // t is a copy at current position
next(s); next(s)          // advance s
list(s)                   // {3} -- s consumed two already
list(t)                   // {1,2,3} -- t starts from beginning

// empty() -- empty statement, returns blank type object; test with ==empty()
e=list()
at(e 0)==empty()          // true -- out-of-bounds returns BlankType
s=(0 nil 2)
v=next(s); v==empty()     // false -- 0 is a real value
v=next(s); v==empty()     // true -- nil terminates stream, next() returns BlankType
```

---

## Functions and scope

```
// define a function -- no formal parameter list
square=func(x*x)
square(:x 7)              // 49

// keyword args initialize locals before body runs
clamp=func(
  if(v<lo :then lo :else if(v>hi :then hi :else v)))
clamp(:v 15 :lo 0 :hi 10) // 10
clamp(:v 5  :lo 0 :hi 10) // 5

// func can read outer scope
scale=3
f=func(x*scale)
f(:x 7)                   // 21 -- reads outer scale
scale=10
f(:x 7)                   // 70 -- picks up new value

// func writes stay local
g=func(scale=99)
g()
scale                     // still 10

// recursive fibonacci
fib=func(
  if(n<=1 :then n :else fib(:n n-1)+fib(:n n-2)))
fib(:n 10)                // 55
```

---

## Building data structures

```
// list of attrlists -- a table
records=list()
for(i=0 i<5 i++
  records,attrlist(:id i :val i*i))
size(records)             // 5
at(records 3).val         // 9

// find max val
maxval=0
for(i=0 i<size(records) i++
  if(at(records i).val > maxval :then maxval=at(records i).val))
maxval                    // 16

// nested attrlists
node=attrlist(:name "root"
              :left attrlist(:name "left" :val 1)
              :right attrlist(:name "right" :val 2))
node.left.val             // 1
node.right.name           // "right"
```

---

## Introspecting the language itself

```
// the operator table is a live data structure
tbl=optable(:table)
size(tbl)                 // number of registered operators

// find the + operator entry
for(i=0 i<size(tbl) i++
  if(at(tbl i).opr=="+" :then print("found: %v\n" at(tbl i))))
// found: (:opr "+" :cmd "add" :pri 60 :rtol false :type BINARY)

// what does postfix produce?
postfix(a+b*c)            // "a b c mpy[2|0|1] add[2|0|1]"
postfix(if(x>0 :then 1))  // see the post_eval * marker

// help on anything
help(for)
help("$$")
help(stream)
```

---

## Symbol constants, symbol values, and indirect assignment

ComTerp distinguishes between:

* a symbol constant
* the value associated with a symbol
* a reference suitable for assignment through a symbol

Backquote creates a symbol constant:

```
// myvar currently contains 42
myvar=42

dynvar=`myvar
```

Now `dynvar` contains the symbol `myvar` itself, not the value `42`.

The same symbol can then be interpreted in two different ways.

`symval()` follows the symbol to obtain its current value:

```
symval(dynvar)      // 42
```

`symvar()` follows the symbol to obtain an assignable reference:

```
symvar(dynvar)=99
```

After the assignment:

```
myvar              // 99
```

This allows indirect access to variables through symbols stored in other variables.

A useful way to think about the three operations is:

```
`myvar     // create a symbol constant

symval(s)  // interpret symbol as a value
symvar(s)  // interpret symbol as a variable reference
```

The distinction is subtle but important. A backquoted symbol is not merely a symbol variable that has been protected from lookup. It is a literal symbol constant. `symval()` and `symvar()` then provide two different interpretations of that symbol: one for reading and one for assignment.

This capability makes it possible to construct variable names dynamically and then either read from them or assign through them without requiring a separate pointer or reference type.

---

## Execution model and parser semantics (ComTerp as a live system)

ComTerp is not built on a fixed execution substrate. The parser, operator table,
precedence rules, and evaluation strategy are all runtime-modifiable components.
This is a core architectural property: the language is a live system, not a frozen
syntax mapped onto a runtime.

The REPL is the wire protocol. There is no distinction between:

- interactive execution
- programmatic invocation
- IPC or networked execution

All of them reduce to the same mechanism:

    send expression → evaluate → return expression

This collapses the traditional separation between program, shell, runtime, and IPC.
The language itself is the protocol.

---

### Parser as a mutable system

The parser is not fixed. It can be reconfigured at runtime by changing:

- symbol bindings
- operator precedence
- associativity flags (including right-to-left mode)
- parsing modes (e.g. switching into instruction-level grammars via `ipl()`)

This allows a single runtime to host multiple syntaxes, including:

- instruction-level flowgraph assembly languages
- embedded domain-specific grammars
- experimental or temporary syntactic overlays

The parser is therefore part of the running program state, not a static compiler phase.

---

### Stream overloading as structural expansion

ComTerp supports stream overloading, where a single expression may expand into
multiple execution steps. This is conceptually similar to Verilog `generate`, but
it occurs at runtime rather than compile time.

This enables:

- one expression → multiple instructions
- implicit replication of computation graphs
- symbolic expansion of execution structure

---

### Visual and textual verification

ComTerp emphasizes observability of execution. Both textual and visual inspection
are first-class debugging tools, with visual confirmation often primary in systems
like DrawServ.

The `postfix()` command exposes the parsed representation of any expression,
allowing deterministic verification of parser behavior independent of execution.

The system can always be understood as:

    source → postfix → execution

---

### Runtime-modifiable components

The following system components may be changed at runtime without restarting:

- operator precedence table
- associativity rules (`rtol`)
- symbol table entries
- parser mode switches
- command dispatch bindings

These changes propagate immediately into all subsequent evaluation contexts.

---

### Semantic rules: whitespace, parentheses, and streams

Whitespace has no semantic meaning except at top level, where newline
terminates an expression.

Parentheses do not define scope or blocks. They define token coherence
and interpretation context.

Key rules:

- `(a b)` is a stream literal when evaluated in stream context
- `(a b)` is a grouped expression when embedded as a subexpression
- `(a b)` inside a command argument list is a literal argument group
- `cmd(a b)` is a command invocation with two arguments

Thus:

    cmd1(a b)

is a single invocation with two arguments.

But:

    (a b)

is context-dependent and may represent a stream literal.

---

### Structural principle

ComTerp treats program text, data, and control flow as a single unified token system.
The distinction between them is not syntactic, but contextual.

All structures reduce to:

    tokens → postfix stream → stack evaluation

This unification is what allows the REPL, IPC system, and language runtime to be identical.

---

## Driving a drawing session from ComTerp

DrawServ registers additional commands that operate on the live drawing.
These commands invoke Unidraw Commands — the structured action objects
developed by John Vlissides and Mark Linton at Stanford that carry
history, undo, and network propagation. These examples assume a running
comdraw or drawserv session:

```
// create graphics
r=rect(50 50 200 150)
e=ellipse(300 200 60 40)
t=text(100 100 "ivtools")

// manipulate selection
select(:all)
scale(1.5 1.5)
select(:clear)
select(r)
move(20 10)
rotate(15)

// attach data to a graphic
setattr(r :label "my_rect" :value 42)
attrlist(r)               // (:label "my_rect" :value 42)

// query drawing structure
frame()                   // the root component
size(frame())             // number of top-level graphics
at(frame() 0)             // first graphic

// coordinate conversion
stod(320 240)             // screen center in drawing coordinates
```

---

## Distributed session inspection

In a DrawServ session, the entire network topology is queryable:

```
// session identifiers for all open drawings
sid(:table)

// peer connections
drawlink(:table)

// combined: who is connected to what
tbl=drawlink(:table)
for(i=0 i<size(tbl) i++
  print("link %v: %v\n" i at(tbl i)))
```

The `drawmo` test orchestrator drives this remotely — it launches
DrawServ instances, connects to them via `socket()`, and sends
ComTerp expressions via `remote()` to verify state:

```
sock=socket("localhost" 20002)
remote(sock "sid(:table)")
remote(sock "drawlink(:table)")
remote(sock "rect(10 10 100 100)")
remote(sock "select(:all); scale(2.0 2.0)")
```

The same expressions a human types interactively are the ones the
test suite sends over TCP. There is no separate API for distributed
operation — the wire protocol is ComTerp, and ComTerp is what you
already know.

---

## What the streaming algebra points toward

Stream literals make it possible to describe graph topology inline.
Here is a sketch:

```
// build a series of named pipes using stream operations
nodes=(0 1 2 3)
while((n=next(nodes))!=nil
  run("pipe(node"+print(n :str)+" node"+print(n+1 :str)+")"))

// fan-out from one source to many destinations
src=0
result=list()
s=(1 2 3 4)
while((n=next(s))!=nil
  result,("fanout("+print(src :str)+" "+print(n :str)+")"))
result     // {"fanout(0 1)","fanout(0 2)","fanout(0 3)","fanout(0 4)"}
```

The stream literal is the wiring diagram. `next()` is the act of
reading it. No visible loop counter, no index arithmetic, no
fence-post errors. The data flow is the program.
