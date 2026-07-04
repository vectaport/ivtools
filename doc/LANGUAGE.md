# ComTerp Language Guide

ComTerp is the embedded scripting layer of the ivtools toolkit — the
scriptable nervous system of its vector-graphics and
distributed-drawing tools (comdraw, drawtool, DrawServ, and the drawmo
orchestrator).  In character it is, roughly, a Lisp for C: it keeps
the interpreted, expression-all-the-way-down, read-eval-print quality
that makes an interpreter pleasant to drive a tool with, but it is
built in C and stays in close, transparent contact with the machine —
a powerful orchestrator of sequences of instructions, not a
self-describing model.  The aim throughout is precise yet elegant
interaction with the instruction set: enough of the interpreter's
allure to be scriptable, none of the apparatus a tool's scripting
layer does not need.

ComTerp is a scripting language where the syntax is also the wire
protocol. Every value — integer, string, list, attrlist, boolean —
serializes back to valid ComTerp syntax that can be parsed and
evaluated again. This means a terminal session on stdin/stdout and
a programmatic session over a TCP socket are the same thing: send
an expression, get back a value that is itself an expression.

This property makes distributed computation natural. DrawServ uses
it to propagate drawing commands between peers — a brush change on
one node is just a ComTerp expression sent to all connected nodes,
evaluated in place. The drawmo test orchestrator drives drawserv
instances the same way a human would from a terminal.

ComTerp achieves this with a compiler pipeline (scanner → parser →
code_conversion → interpreter) that converts each top-level expression
to a flat postfix token stream, then evaluates it iteratively. Nesting
depth in the original expression does not affect the interpreter's
call stack — everything is a token stream and an operand stack.

### The REPL is the wire protocol

"The syntax is the wire protocol" has a concrete meaning that goes
beyond serialization. In most distributed systems there are two
distinct layers: an expression language for local computation, and a
separate serialization format (JSON, protobuf, XML) for sending values
over the wire. ComTerp collapses these into one. The value returned by
any expression is itself a valid ComTerp expression — so sending a
value and sending an expression are the same act.

This is demonstrated completely by `remote()`:

```
remote(hoststr|sockobj [portnum] cmdstr :nowait :str)
```

`remote()` sends `cmdstr` to another ComTerp instance over a TCP
socket and — by default — waits for the response, evaluates it
locally, and pushes the result onto the operand stack as a live
ComTerp value:

```
// ask a remote node for its drawlink connection table, use it locally
t=remote("peer" 9988 "drawlink(:table)")
// t is a live list of attrlists -- dot-access works immediately
if(size(t)>0 :then at(t).host)
```

That last step — evaluating the returned string and pushing it on the
stack — is what makes the protocol self-consistent. The remote node
doesn't return a serialized blob that needs to be deserialized by a
separate layer. It returns a ComTerp expression that the local
interpreter evaluates directly, because every ComTerp value round-trips
through its own syntax.

The `:str` keyword skips the local evaluation and returns the raw
response string instead — useful when you want to inspect the wire
traffic or defer evaluation:

```
raw=remote("peer" 9988 "drawlink(:table)" :str)
// raw is something like "(:nlinks 2 :host0 \"peer2\" :port0 9988 ...)"
// a ComTerp expression string -- parse or log it, or run() it later
run(raw :str)   // evaluate it now -- same result as without :str
```

`:nowait` fires and forgets — sends the expression without waiting for
a response. Used for one-way commands like brush propagation in
DrawServ where the sender doesn't need the return value.

The same property makes the `drawmo` test orchestrator work — it drives
drawserv instances over stdin pipes using the same expressions a human
would type at the REPL, and the responses come back as ComTerp values
that the orchestrator can inspect, compare, and branch on. There is no
separate test protocol, no mock layer, no serialization adapter. The
REPL session and the wire session are the same thing.

## Background: how ComTerp got its streams

ComTerp's design did not start where it ended. It began with a dream,
set the dream aside to build something tractable, and only years later
discovered that the tractable thing had quietly become the foundation
the dream required. The order matters, because it explains why the
implementation looks the way it does — much of the machinery that powers
streams was built for other purposes first.

**The original dream: stream-overdriven operators.** The starting vision
was dataflow all the way down — operators like `*` and `+` as inherently
stream operators, operands flowing through them. This traces to the
NCL/dataflow lineage (Karl Fant, Honeywell) and the command-interpreter
work at Honeywell IRL and Triple Vision. But an operator cannot flow
operands through itself until there is a defined notion of what an
operand *is* — its type, its evaluation, its timing. The dream was set
aside, not abandoned. It was waiting for a substrate.

**Layer 1 — the expression interpreter.** What got built instead was a
sober, C-like expression evaluator on a Fischer-LeBlanc scanner/parser
pipeline: postfix, arity-counted, evaluated left to right. The governing
rule — *everything is a C-like expression; there are no declarations* —
is this layer. Unglamorous next to the dataflow dream, but it is bedrock:
a stream element turns out to be nothing more than a *deferred
expression*, and you cannot defer an expression you cannot first
evaluate definitely.

**Layer 2 — post-eval control commands.** Control flow (`if`, `for`,
`while`, conditional and lazy evaluation — the post_eval mechanism
synthesized from the lazy-vs-eager evaluation literature) required
commands that receive their operands *unevaluated* and drive evaluation
themselves. That deferral machinery — hold the operand's tokens,
evaluate them when and how it chooses — is exactly what a lazy stream
needs. A stream literal's `next()` *is* a post-eval: it holds an
element's token span and evaluates one element on demand. The post-eval
apparatus (the argoff bookmark, the backward span-finding walk) was built
for control flow and turned out to be stream infrastructure under another
name.

**Layer 3 — the duck-typed value model.** A single `ComValue` type
carrying any value (int, float, string, symbol, list, attrlist, object,
command, stream) via a `void*`/`interface{}`-style union with runtime
`isa`/`geta` dispatch. Streams are heterogeneous and polymorphic — an
element may be an int, a list, an attrlist, a FuncObj, or another stream.
Without a uniform any-value type, "a stream of whatever" is not
expressible, and `next()` could not return a value without the iteration
machinery knowing its type.

`StreamType` is the *last* first-class type added to this union. After it,
the extension mechanism changed: rather than keep growing the closed set
of built-in union types — each with its own slot and per-type handling —
a single open-ended `ObjectType` was added as the one extension point.
Its contract is "give me an object tagged by class symbol and I will
reflect it back if I recognize it" (the `class_symid()` plus the
`_as_needed` recognition in the printer and elsewhere). So the union
being *full* is not an accident or a limit bumped into; it is a deliberate
boundary. `StreamType` marks the end of one extension era (more union
types) and the start of another (`ObjectType`, bounded reflection). This
is why `ObjectType` is the sole remaining extension point, and why
payloads carried through it must honor a uniform contract (see the
Resource discussion in `HACKING.md`). The name `StreamType` — rather than
the older printed name `StreamObj` — reflects that it is, correctly, the
last of the union *types*.

**Layer 4 — builtin primitives and user functions.** `func` is a command
that returns a `FuncObj` you bind with `=` (there is no function
*declaration* — see the function section). The `FuncObj` carries its own
copy of the postfix token buffer. This is the last brick, and it is the
one that finally made streams first-class: a stream that outlives the
expression that created it needs an owned, reference-counted, lifetime-
managed copy of its tokens — which is precisely what `FuncObj` already
was, built for user functions. A first-class stream literal *is* a
`FuncObj` wrapping a token buffer plus a consumption cursor.

**Why it took so long to make streams first-class.** A first-class stream
is the intersection of all four layers at once: a `FuncObj` (layer 4)
holding deferred token spans (layer 2) of arbitrary expressions (layer 1)
that yield any-typed values (layer 3), flowed through ordinary operators
(the original dream). Four of the five things a stream is made of did not
exist when the dream was first conceived. The foundation got built by
solving four other problems that each looked unrelated at the time, and
the streams could only become first-class once all four were in place.

The dream was not compromised — it was *deferred*, fittingly the same
move as post-eval: the stream-operator idea was held unevaluated until a
substrate that could evaluate it existed. Today, `((1,2,3),)*((4,),(5,),(6,))`
performing a matrix multiply through the ordinary `*` operator is that
original dream finally running, on top of the four layers it needed.

For how the resulting overdrive compares to APL, Lucid, MATLAB, and
Haskell — and what is genuinely without prior art — see *Design
Provenance and Prior Art* under Streams below.

## Expressions and Sequencing

The basic unit of execution is an expression. Multiple expressions are
sequenced with `;` (the sequence operator):

```
x=1; y=2; x+y
```

The value of a sequence is the value of its last expression.

A script file (`.comt`) is a sequence of top-level expressions consumed
one at a time. The return value of the script is the value of the last
expression evaluated. By convention test scripts return `ok` (a boolean).

The body argument to a control command (`for`, `while`, `if`, `switch`, `func`)
is a **single expression**. The canonical way to express a
multi-statement body is semicolons with no enclosing delimiters —
the least punctuation needed:

```
for(i=0 i<10 i++ lst,i; total=total+i)   // canonical two-statement body
for(i=0 i<10 i++ lst,i)                  // one statement, no ; needed
```

**Parens have no special body-grouping role.** Their purposes in
ComTerp are:

1. **Argument lists** — enclose arguments to a command: `f(a b c)`
2. **Attrlist literals** — `(:key val)` when first token is a keyword
3. **Stream literals** — `(val val ...)` when first token is a value *(ivtools-3.0)*
4. **Precedence override** — `(a+b)*c` to override operator priority

That's it. A body does not need parens. `(lst,i; total=total+i)` is
a single `;`-sequence expression that happens to be wrapped in parens,
but the parens add nothing — the semicolons do all the work:

```
for(i=0 i<10 i++ (lst,i; total=total+i))  // works but parens unnecessary
```

**Warning:** a space between two expressions inside parens — without a
semicolon — is not a two-statement body. It is currently an error and
will become a stream literal in ivtools-3.0:

```
for(i=0 i<10 i++ (lst,i total=total+i))   // error now, stream literal in 3.0
for(i=0 i<10 i++ lst,i total=total+i)     // error: for loop with two bodies
```

**Warning (ivtools-3.0):** Once stream literals land, `(ding beep)`
will be parsed as a stream literal of two values, not a grouped
two-statement body. Any code using space-separated statements inside
parens without semicolons must be fixed before 3.0. See issue #103.

## Types

| Type | Example | Notes |
|------|---------|-------|
| int | `42` | |
| float | `float(3.14)` | 32-bit, via float() conversion |
| double | `3.14` | 64-bit, default for numeric literals |
| string | `"hello"` | double-quoted |
| symbol | `` `foo `` | backquote suppresses lookup |
| char | `'a'` | single-quoted, format with `%c` |
| boolean | `true` `false` | |
| nil | `nil` | no value |
| blank | `BlankType` | return of `return()` with no arg |
| list | `1,2,3` or `(1,2,3)` or $1,2,3| comma operator |
| stream | `$$(1,2,3)` or `(1 2 3)` *(ivtools-3.0)* | sequence of values produced and consumed one at a time |
| attrlist | `(:x 1)` or `attrlist(:x 1)` | key/value store |
| compview | returned by drawing commands | graphic component handle |

Use `type(val)` to inspect the type of any value. Use `class(val)` for
object types.

## Variables

Variables are local to the current ComTerp instance by default:

```
x=42
s="hello"
```

Variables can be reassigned to any type at any time. A symbol bound to a registered command cannot be reassigned.
See `ARCHITECTURE.md` for how commands are registered.

### Global variables

Declare a variable global with `global()` to share it across all
ComTerp instances in the process:

```
global(counter)
counter=0
```

Global variables persist across script runs and are visible to all
interpreters (e.g. UI and network interpreters in drawserv).

## Arguments: Fixed Before Keywords — Always

Every ComTerp command accepts fixed positional arguments followed by
keyword arguments. This ordering is enforced and consistent across all
commands — unlike Unix shell commands:

```
print("value=%v" 42 :str)     // correct
print(:str "value=%v" 42)     // wrong
```

Keywords come in two forms:
- `:keyword` — flag (presence is meaningful, value is `true`)
- `:keyword value` — keyword with an associated value

Unknown keywords are silently ignored, which enables layered keyword
extension across the library hierarchy. See `ARCHITECTURE.md` for how
this works internally.

The same ordering applies inside a **stream literal**: positional values
first, keywords after. A stream literal may end with keywords —
`(a b :key v)` — but keywords belong after the positional content, never
before or among it. Keep to positionals-then-keywords here as everywhere;
a keyword in the position that decides a construct's kind (the second
element, which is what tips grouping into a stream) is the one case the
rule protects against, and following the rule keeps it out of reach.

## Operators

Standard arithmetic, comparison, and logical operators work as expected.
String concatenation uses `+`.

### Precedence Table

Operators are listed highest to lowest priority. RtoL means right-to-left
associativity. Run `optable()` inside comterp to see the live table.

| Priority | Operator | Command       | Assoc | Type            |
|----------|----------|---------------|-------|-----------------|
| 130      | `.`      | dot           | LtoR  | BINARY          |
| 125      | `` ` ``  | bquote        | RtoL  | UNARY PREFIX    |
| 110      | `~`      | bit_not       | RtoL  | UNARY PREFIX    |
| 110      | `--`     | decr_after    | RtoL  | UNARY POSTFIX   |
| 110      | `--`     | decr          | RtoL  | UNARY PREFIX    |
| 110      | `-`      | minus         | RtoL  | UNARY PREFIX    |
| 110      | `++`     | incr_after    | RtoL  | UNARY POSTFIX   |
| 110      | `++`     | incr          | RtoL  | UNARY PREFIX    |
| 110      | `!`      | negate        | RtoL  | UNARY PREFIX    |
| 100      | `$$`     | stream        | RtoL  | UNARY PREFIX    |
| 90       | `..`     | iterate       | LtoR  | BINARY          |
| 80       | `**`     | repeat        | LtoR  | BINARY          |
| 75       | `,,`     | concat        | LtoR  | BINARY          |
| 70       | `/`      | div           | LtoR  | BINARY          |
| 70       | `*`      | mpy           | LtoR  | BINARY          |
| 70       | `%`      | mod           | LtoR  | BINARY          |
| 60       | `-`      | sub           | LtoR  | BINARY          |
| 60       | `+`      | add           | LtoR  | BINARY          |
| 55       | `>>`     | rshift        | LtoR  | BINARY          |
| 55       | `<<`     | lshift        | LtoR  | BINARY          |
| 50       | `>=`     | gt_or_eq      | LtoR  | BINARY          |
| 50       | `>`      | gt            | LtoR  | BINARY          |
| 50       | `<=`     | lt_or_eq      | LtoR  | BINARY          |
| 50       | `<`      | lt            | LtoR  | BINARY          |
| 45       | `==`     | eq            | LtoR  | BINARY          |
| 45       | `!=`     | not_eq        | LtoR  | BINARY          |
| 44       | `&`      | bit_and       | LtoR  | BINARY          |
| 43       | `^`      | bit_xor       | LtoR  | BINARY          |
| 42       | `\|`     | bit_or        | LtoR  | BINARY          |
| 41       | `&&`     | and           | LtoR  | BINARY          |
| 40       | `\|\|`   | or            | LtoR  | BINARY          |
| 35       | `,`      | tuple         | LtoR  | BINARY          |
| 32       | `$`      | list          | RtoL  | UNARY PREFIX    |
| 32       | `~~`     | spread        | RtoL  | UNARY PREFIX    |
| 30       | `=`      | assign        | RtoL  | BINARY          |
| 30       | `/=`     | div_assign    | RtoL  | BINARY          |
| 30       | `-=`     | sub_assign    | RtoL  | BINARY          |
| 30       | `+=`     | add_assign    | RtoL  | BINARY          |
| 30       | `*=`     | mpy_assign    | RtoL  | BINARY          |
| 30       | `%=`     | mod_assign    | RtoL  | BINARY          |
| 10       | `;`      | seq           | LtoR  | BINARY          |

A few things worth noting:

- `.` binds tightest — `f(:x 5).x` works without parens
- `..` and `**` bind above arithmetic — `(2..4)*5` needs parens around the range
- `,` binds below all arithmetic and comparison — `1+2,3+4` is `(1+2),(3+4)`
- `=` is right-associative and below `,` — `a=b=1` chains correctly
- `;` binds lowest of all — everything to its left and right is a complete expression
- `$$` and `$` are unary prefix RtoL so `$$lst` and `$strm` parse without parens

### Streaming operators

| Operator | Description |
|----------|-------------|
| `,` | tuple / list construction |
| `$$` | create stream from list |
| `$` | collect stream into list |
| `,,` | stream concatenation |
| `..` | iterate / range |
| `**` | repeat |
| `~~` | spread a collection into a call's arguments (see *The spread operator*) |

### Dot operator

`.` accesses attributes on a compound variable or attrlist:

```
foo.bar=42
foo.bar          // returns 42
```

The dot namespace rooted at a symbol is scoped with that symbol — see
**Attribute Lists** below.

### Backquote

`` ` `` (backquote) returns a symbol without looking it up:

```
`foo             // the symbol foo, not its value
type(val)==`IntType
```

`StreamObj` was temporarily exported as the literal for a `StreamType`,
and a warning will be printed if a script makes use of it as a symbol (by
prefixing it with a back-quote). Use `` `StreamType `` instead.

## Control Flow

Control flow commands use `post_eval` — they receive an offset into
the read-only postfix buffer for their body expressions and choose
when to evaluate them.  This is what makes `if`, `for`, `while` and
`switch` work as language constructs rather than ordinary functions.

The postfix buffer at this stage of the Fischer/Leblanc pipelines
is made of values ready to be pushed on the comterp stack and
interpreted, which adds to its efficiency along with the only
storing an offset to switch from lazy to eager interpretation and
back again.

### if

```
if(testexpr :then trueexpr :else falseexpr)
```

`:else` is optional. `if` returns the value of the branch taken.

### for

```
for(i=0 i<10 i++
  print("%v\n" i))
```

Positional args: init, while-test, next, body. `:body expr` is an
explicit keyword form for the body.

### while

```
while(i<10
  print("%v\n" i); i++)
```

Keywords: `:nilchk` (test for nil instead of false), `:until` (test
after body), `:body expr` (explicit body keyword).

### return, break, continue

```
return([retval])   // return from func or script, optional value
break([retval])    // break out of for/while
continue           // skip to next iteration
```

`return()` with no argument returns `BlankType`. The `_returnflag`
propagates through `SeqFunc`, `ForFunc`, `WhileFunc`, and `runfile()`.

### switch and cond

`switch` dispatches on string, symbol, integer, or char value:

```
switch("red" :red "stop" :green "go" :blue "sky")  // "stop"
switch(2 :case1 "one" :case2 "two" :case3 "three") // "two"
switch(`unknown :red "stop" :default "unknown")     // "unknown"
```

`cond` is an inline ternary — eagerly evaluated unlike `if`:

```
cond(x>0 "positive" "non-positive")
cond(nil "yes" "no")   // "no" -- nil is false
```

Use `if(:then :else)` when the branches should be lazily evaluated.

## print()

```
print("fmt" val [val...])          // print to stdout
print("fmt" val [val...] :str)     // print to string and return it
print("fmt" val [val...] :err)     // print to stderr
```

Format verbs: `%v` (any value), `%d` `%i` (decimal int), `%u` (unsigned int),
`%o` (octal int), `%x` `%X` (hex int lower/upper), `%f` (decimal float),
`%e` `%E` (scientific float), `%g` `%G` (shorter of `%e`/`%f`),
`%s` (string), `%c` (char). `%v` is a ComTerp extension; all others are
standard C `printf` verbs passed through to the underlying C library.
Use `\%` for a literal percent sign — `%%` is not supported.

## help()

```
help(funcname)       // help for one command
help(:all)           // help for every registered command
help(:top)           // help for top-level commands in this program
help(:posteval)      // help for post_eval commands
```

`help()` is the primary reference for command signatures. The docstring
format is: `retval=name(arg [optarg] :keyword :keyword value) -- description`.
Square brackets indicate optional fixed args.

## Functions

Define a function with `func()`:

```
f=func(body)
```

The body is the first positional argument. There is no formal parameter
list — any symbol used in the body is a local variable. Call with
keyword args to initialize locals before the body runs:

```
f=func(if(x>5 :then return(x*2)))
v=f(:x 6)          // x is set to 6 before body runs, returns 12
v=f(:x 3)          // returns nil (no early return taken)
v=f()              // x is nil, condition is false, returns nil (ivtools-2.2 or >)
```

### Scoping rules

Variable lookup follows a three-level priority chain:

- **func scope** — variables local to this invocation, including any
  `:key val` args set before the body runs.  The frame is an attrlist,
  created for the call and discarded at return — the same structure
  that carries keyword args and `setattr()` properties everywhere else
- **local scope** — the interpreter's flat top-level symbol table,
  where prompt and `run()` assignments live; `local(x)` reads and
  writes it explicitly (see *Escaping the func scope* below)
- **global scope** — symbols declared with `global()`; `global(x)`
  reads and writes it explicitly

A variable can be **read** from any level — func scope wins over local,
local wins over global. A variable **written** inside a func always goes
to func scope only, never propagating outward. The only exception is
`global()` which explicitly reaches the global scope.

Func scopes do **not** chain: a func called from inside another func
reads its own frame and then the top-level table — never the calling
func's frame:

```
inner=func(v)
outer=func(v=5; inner())
outer()            // nil -- inner cannot see outer's v
v=99
outer()            // 99  -- inner falls through to the top level
```

Writing through a reference passed in as a keyword arg is not an
exception to this rule — the symbol is local, but the attrlist object
it points to lives outside the func and is mutated via that reference:

```
al=attrlist(:x 0)
f=func(al.x=99)    // al is local symbol pointing to outer object
f(:al al)
al.x               // 99 -- the object was mutated, not the scope
```

A symbol not supplied by the caller and not yet written in the body
reads as nil:

```
f=func(x*2)
f()                // nil*2 -- x is nil, result is nil
f(:x 5)            // 10
```

But that nil is the func-scope **miss** falling through to local/global —
the very close-over described next — so "reads as nil" holds **only when
no outer variable of that name exists**. If the surrounding scope already
holds an `x`, an unsupplied `x` reads *that value*, not nil. The slot is
not zero-initialized; treat it as an uninitialized variable in the C/C++
sense.

The optional-parameter-with-default idiom rests on exactly this nil:

```
f=func(if(x==nil :then 99 :else x))   // default 99 when x not supplied
f()                // 99   -- only if no outer x is in scope
f(:x 5)            // 5
```

So when an outer `x` might be present — e.g. several scripts sharing one
flat ComTerp through `run()` — **initialize it first** (`x=nil` before the
call) or the default silently won't fire. Rule of thumb: *if you test a
name for nil, set it nil.*

This means a func can close over outer variables for reading without
declaring them, but any write stays local:

```
scale=3
f=func(x*scale)    // reads outer 'scale'
f(:x 7)            // 21 -- scale read from local/global scope
scale=10
f(:x 7)            // 70 -- picks up new value of scale

g=func(scale=99)   // writes to func-local 'scale'
g()
scale              // still 10 -- write did not escape
```

```
f=func(if(x>5 :then return(x*2)))
v=f(:x 6)          // returns 12
v=f(:x 3)          // returns nil (no early return taken)
```

Function body assignments are local — variables assigned inside a func
do not escape to the caller's scope. This includes dot-notation
attributes: a dot namespace rooted at a local symbol is local to the
call.

### Escaping the func scope: local() and global()

When a func genuinely needs to write outside its own frame, two
commands name the outer scopes explicitly, as both lvalue and rvalue:

- **`local(x)`** — the interpreter's default symbol table: the scope
  bare assignment already uses *outside* a func, named so it can be
  reached from *inside* one.  As an lvalue, `local(x)=val` writes that
  table, skipping the frame; as an rvalue, `local(x)` reads that table
  and only that table — no func-frame shadow, no global fallback.
- **`global(x)`** — the interpreter-shared table (one per process,
  shared by every interpreter instance — every connection of a comterp
  or drawserv server).  `global(x)=val` writes it from anywhere;
  `global(x)` reads exactly it.

```
count=0
bump=func(local(count)=count+1)   // reads outer count, writes it back
bump(); bump()
count                  // 2 -- the func published through local()

f=func(count=99; count,local(count))
f()                    // {99,2} -- frame shadow vs explicit outer read
```

In a single-interpreter session `local()` and `global()` differ only in
which table they touch; in a multi-session server they are session
scope versus process scope — publish per-connection state with
`local()`, cross-connection state with `global()`.

Note that the backquote is **not** a scope escape: `` `x `` is the
symbol-quote (the symbol itself, lookup suppressed), and assigning
through it stays in the current scope like any other write.  Use
`local()`/`global()` to escape; use `return` (possibly of an attrlist)
to hand results to the caller.

### Multi-value returns

A func returns a single value — the result of its last expression. Two
clean patterns exist for returning or receiving multiple values:

**Pull — dot on return value.** Return an attrlist, caller uses `.` to
extract just the field it needs. Good for functional style where the
caller picks what it wants:

```
f=func((:x x*2 :y x+1))
result=f(:x 5)
result.x               // 10
result.y               // 6
f(:x 5).x              // 10 -- extract inline, no intermediate variable
```

**Push — pass in an attrlist to update.** Caller passes an existing
attrlist as a keyword arg; func writes into it via the reference. Good
for updating a running accumulator or shared context, or when setting
one field in a larger attrlist without disturbing the rest:

```
al=attrlist(:x 0 :y 0)
f=func(al.x=x*2; al.y=x+1)
f(:al al :x 5)
al.x                   // 10
al.y                   // 6
```

The push pattern is also the idiomatic way to set a single field in an
existing attrlist — "set a needle in a haystack" — without constructing
a new one. The pull pattern with `.` is the corresponding "get a needle
in a haystack" from a func that returns a rich result.

## Streams

Streams are lazy — values are produced and consumed one at a time,
rather than all at once. Unlike a list which holds all its values in
memory, a stream yields the next value only when asked. This makes
streams suitable for processing large or unbounded sequences without
materializing the whole thing.

### The stream contract

A stream is a **monotonic, nil-terminated, forkable** sequence. These are
not three separate features; they are one shape stated three ways, and
together they define what it means to be a stream in ComTerp.

- **Monotonic** — consumption only moves forward. `next()` advances the
  position; there is no rewind or seek in the contract. (A backing source
  may happen to be seekable, but the stream abstraction does not expose
  it.) To replay, reassign the stream.

- **Nil-terminated** — the forward motion has a defined end. An exhausted
  stream yields `nil` from `next()` and stays exhausted. `nil` is the
  universal terminator; it is what makes a stream finite *in contract*
  even when the underlying source is unbounded.

- **Forkable** — at any position a stream can be split (`$$s` /
  `stream(s)`) into independent continuations, each of which is itself a
  monotonic, nil-terminated, forkable stream. Forkability is part of the
  definition, not an added feature: a sequence that cannot be forked is
  not a stream.

The contract is **closed**: a fork of a stream is a stream, and a fork of
a fork is a stream, all the way down.

**The mechanism that delivers forkability** is a shared growing-shrinking
buffer of elements. A fork copies the stream's *current* position (see
*Stream copy forks at the current position*), and the buffer retains
exactly those elements that some forks have consumed but not yet all —
freeing each element once every fork has passed it. This lets any
streamable source satisfy the fork contract regardless of its nature:

- a **stream literal**'s buffer is born full and only shrinks as it is
  consumed (the degenerate case — all production happened up front);
- a **file or pipe** stream's buffer both grows at the front as elements
  are produced and shrinks at the back as the slowest fork advances,
  bounded by the spread between the fastest and slowest fork.

The literal mechanism and the file/pipe mechanism are therefore the same
structure at different settings: a shared ordered buffer with per-fork
positions and front-reclamation by the slowest fork. (File/pipe stream
forking is specified in a separate issue and may not yet be implemented;
the contract above is what any such implementation must satisfy.)

### The Streaming Algebra

The streaming algebra is the set of operations that construct, compose,
and consume streams. Understanding the algebra — what operations exist,
how they compose, and what the laws are — is the core of ComTerp's
stream model.

**Construction** — creating a stream from a source:

```
s=$$(1,2,3,4,5)    // stream from list (materialized source)
s=1..5              // range stream: 1,2,3,4,5 (iterate)
s=3**5              // repeat stream: 3,3,3,3,3 (repeat)
s=(0 1 2 3)         // stream literal (lazy source)
```

Stream literals can include keyword elements. Positional values come
through as-is; each keyword-value pair becomes a singleton attrlist
element; a bare keyword (flag) becomes `(:flag true)`:

```
s=(0 1 :key 99 :flag)
// s is a stream that produces: 0, 1, (:key 99), (:flag true)
next(s)   // 0
next(s)   // 1
next(s)   // (:key 99)
next(s)   // (:flag true)
next(s)   // nil -- end of stream
```

If there are no positional values before the first keyword, it is
an attrlist literal, not a stream literal:

```
(:key 99 :flag)   // attrlist -- first token is a keyword
(0 :key 99)       // stream literal -- first token is a value
```

**Element spans.** Each element of a stream literal is an expression of
arbitrary depth, and its size is the number of postfix tokens that
expression occupies — not a fixed stride. The same counting applies
whether the element is a positional or a keyword's value:

```
(3 ...)              // element 3        -- 1 token
(x,y ...)            // element x,y      -- 3 tokens (x y ,; comma is a binary op)
(1+2+3+5 ...)        // element 1+2+3+5  -- 7 tokens (1 2 + 3 + 5 +)
(0 :key 1+2+3+5)     // the keyword's value is the same 7-token span
(0 :key 3)           // the keyword's value is 1 token
(0 :standalonekey)   // a bare keyword has no value span (0 tokens)
```

A keyword value and the identical expression as a standalone positional
have the same span — a value expression has one postfix length regardless
of how it arrives. A bare keyword contributes no value span. (This is why
keyword elements belong after positionals: see *Arguments: Fixed Before
Keywords*. A bare keyword in the element that decides stream-ness — the
second element — is the one shape to avoid.)

**Consumption** — pulling values out:

```
next(s)             // pull next value, nil when exhausted
l=$s                // collect stream into list
each(s)             // traverse stream, return count
```

`each()` is the idiomatic way to consume an entire stream when you
want the count or just the side effects:

```
each(1..10 * 2)     // 10 -- traverses stream, returns count
```

`empty()` is an empty statement that returns a BlankType object —
used when something other than nil is needed to indicate nothing was
returned. Test for it with `==empty()`:

```
s=(0 nil 2)
v=next(s); v==empty()      // false -- 0 is a real value
v=next(s); v==empty()      // true -- nil terminates stream early,
                           //         next() returns BlankType
e=list()
at(e 0)==empty()           // true -- out-of-bounds returns BlankType
```

**Composition** — combining streams:

```
s1,,s2              // concatenate: s1 elements then s2 elements
(1..3)+(10..12)     // zip: element-wise binary op → {11,13,15}
$$s                 // copy stream at current position (checkpoint)
```

**Scalar overdrive** — vectorizing scalar operators:

```
(1..5)*2            // {2,4,6,8,10} -- scalar op over stream
"node"+(1..4)       // char codes, not strings -- see str() note
```

**Nil termination** — streams end naturally:

```
next(exhausted)     // nil -- end of stream signal
```

For unknown-length streams (stream literals, file/pipe streams), nil
is not an error — it is the natural end-of-stream. A stream literal
element that evaluates to nil terminates the stream early, leaving
remaining elements unevaluated in the token buffer.

**Two stream kinds:**

- *Known-length* — `$$list`, `..`, `**`: length fixed at construction
- *Unknown-length* — stream literals `(...)`, file/pipe streams:
  length unknown, terminates on nil

Round-trip: `$($$(1,2,3))` returns `(1,2,3)`.

The streaming algebra is still being formalized. The stream literal
syntax (ivtools-3.0) completes the source end of the algebra; ongoing
work continues to clarify the composition laws, particularly around nil
propagation through composed operations and zip semantics between lazy
and materialized sources.

### Scalar overdrive

A stream on either side of a scalar operator vectorizes it — the
operator is applied once per element, producing a stream of results.
This is the core design intent: streams overdrive scalar operations
without the scalar operator knowing anything about streams.

```
list((2..4)*5)         // {10,15,20}  -- scaled ramp
list(100-(100..0))     // {0,1,...,100} -- inverted ramp
list((1..5)+10)        // {11,12,13,14,15}
list((1..5)*10)        // {10,20,30,40,50}
list(0**5+1)           // {1,1,1,1,1}
```

Parameterized ramp — the `setbuf` pattern:

```
a=0; b=10; c=1000
ss=(a..b)+c            // lazy -- not yet consumed
list(ss)               // {1000,1001,...,1010}
```

### Files and pipes as streams

`open()` returns a `fileobj` or `pipeobj` (`help(open)`:
`fileobj|pipeobj=open([filename [modestr]] :pipe :in :out :err)`).
`stream()`/`$$` accepts either as a source (`help(stream)`:
`strm=stream(strm|list|attrlist|val|fileobj|pipeobj) -- copy stream or
convert list`), converting it to a lazy stream of lines:

```
ff=open("/tmp/diffs")
list($$ff)              // {"line one","line two",...}

pp=open("ls -l" :pipe)
list($$pp)               // lines of command output
```

Each `next()` on `$$ff` (or `$$pp`) reads one line lazily -- the same
lazy, single-pass model as any other stream, just backed by file or pipe
I/O instead of a token buffer. This is the same `$$` used to convert a
list to a stream (`$$(1,2,3)`); here the source is a `fileobj`/`pipeobj`
instead of a list.

### Overdrive rules: post-eval vs non-post-eval commands

Whether a command can be overdriven by one of its own arguments depends
on whether it is post-eval. This is inspectable directly: `postfix(help)`
shows a trailing `*` on post-eval commands, e.g. `"each[0|0|1]*"`,
`"stream[2|0|1]*"`. Commands without the trailing `*` are non-post-eval.

**Non-post-eval commands** are overdrivable by any stream argument --
internal overdrive. This covers every scalar operator (`*`, `+`, etc.)
and any other non-post-eval command, since the only requirement is "be a
command with arguments." The operator/command itself is oblivious; the
evaluator peels one stream element at a time and calls the command once
per element, assembling the results back into a stream.

**Post-eval commands** cannot be overdriven by their own arguments --
they receive arguments as unevaluated token regions (`argoff`) and decide
for themselves what to do with any stream found there. But a post-eval
command is not exempt from streams entirely:

- it can be **externally overdriven**: combined via a non-post-eval
  scalar operator with a stream operand, that operator drives repeated
  calls to the post-eval command as a whole
- it can **return a stream**, becoming the overdrive source for whatever
  it is combined with downstream

In short: streams overdrive *into* non-post-eval commands through their
arguments, and *around* post-eval commands through external combination
or return values -- never *through* a post-eval command's own arguments.

### The spread operator `~~`: apply instead of map

A bare stream argument overdrives a command -- the command is lifted
into a lazy stream and runs once per element (map).  A `~~`-tagged
stream does the opposite: the collection expands into the arguments of
ONE call (apply):

```
list(print("A %v B\n" (10 20 30)))   // overdrive: A 10 B / A 20 B / A 30 B
print("%v %v %v\n" ~~(10 20 30))     // spread: 10 20 30 -- one call
```

`~~` is unary prefix RtoL at priority 32 -- below the comma at 35,
beside `$` -- so `~~lst` spreads a comma-built list without parens: a
whole-collection transformer binds looser than the operator that builds
the collection.  (`$$` sits at 100 because its operand is usually a
single source.)

What spreads, and what each element becomes:

- a **list**: plain elements become positionals in order; an
  attrlist-singleton element (below) becomes a real `:key value` keyword
- a **stream**: same dispatch per element; a stream literal can carry
  raw keywords directly -- `("%v %v\n" 10 20 :str)`
- an **attrlist**: every attribute becomes a keyword

Keywords delivered by `~~` are *live* -- indistinguishable from keywords
typed at the call site.  print's `:str` (return the formatted string
instead of writing it) makes that observable:

```
strm=("[%d %d %d]" 10 20 30 :str)
print(~~strm)                        // "[10 20 30]" -- the whole call,
                                     // format and mode included, traveled
                                     // in one variable
lstv="[%d %d %d]",10,20,30,(:str)
print(~~lstv)                        // identical: the carriers are
                                     // interchangeable
```

#### Carrying keywords: the attrlist singleton

A keyword cannot circulate as a bare value (the evaluator's frame scan
treats any KeywordType value as a live keyword marker), so a keyword
*carried as data* travels as a one-entry attrlist.  Keyword-first parens
are reader syntax for exactly this:

```
(:str)              // the singleton (:str true) -- a bare flag defaults true
(:key 7)            // (:key 7)
al=(:a 7); al.a     // 7 -- dot-accessible like any attrlist
```

Value-first parens remain stream literals -- the first token decides.
`list()` reifies a stream literal's raw keywords into singletons, so the
two carrier forms interconvert:

```
list((10 20 30 :key 55))    // {10,20,30,(:key 55)}
```

The distinction to hold onto is *carrying vs spending*: a singleton is a
keyword carried -- printable, storable, inert in any argument frame --
and `~~` spends it, at which point it stops being a value and becomes
behavior.  A spent keyword leaves no printable residue: spreading
`(:key)` into print leaves an unmatched `%v`, because print ignores
unknown keywords like every other command.

#### echo and the round-trip law

`echo` is the inverse of `~~`: it returns its evaluated arguments in
`~~`-passable form -- positionals in a list with keywords as trailing
attrlist singletons, or bare when there is only one piece.  Together
they form a fixed point:

```
echo(~~echo(10 20 30 :key 55))   // {10,20,30,(:key 55)} -- and stacking
                                 // more rounds changes nothing: emit and
                                 // apply are inverses
```

That is the governing round-trip rule: anything a command emits as its
representation must be passable back through `~~` to reconstruct the
same call.  The fine print is "the same call, not a better one" -- `~~`
faithfully reproduces whatever the direct call would have done,
including its surprises.

#### Limits

- Put `~~` before any parse-time keywords in the call; a spread in
  post-keyword position lands in the keyword-counting zone and
  misbehaves.
- Streams are single-drain, so a spread consumes its stream; lists are
  copied and reusable.
- Post-eval commands (`if`, `while`, `func`, ...) read token-spans, not
  stack values, so `~~` does not reach their arguments -- the same
  boundary as overdrive, from the other side.

Python needs two operators (`*args`, `**kwargs`) and Tcl's `{*}` splats
untyped words; `~~` is one operator over one container because the
element type carries the binding mode -- a plain value is a positional,
an attrlist singleton is a keyword, and one list holds both.

### Stream-scalar broadcast via replay

When a non-post-eval binary operator (`+`, `*`, `,`, `**`, etc.) finds
that exactly one of its two operands `is_stream()` and the other is a
plain scalar, the runtime constructs a per-element stream from the
scalar operand and hands two streams to the operator's existing
stream-zip path -- the same path that already handles `(10**4)*(1..4)`
correctly, producing `10,20,30,40`.

The scalar operand's *first* value is whatever was already computed by
normal evaluation (no extra cost -- "the first draw is done"). For each
subsequent element, the scalar operand's postfix token-slice is replayed
via the same mechanism stream literals already use for lazy per-element
evaluation (`StreamLiteralNextFunc`'s `comterpserv()->run(tokbuf+offset,
cnt)`, proven by `(rand rand rand)` giving three distinct values).

```
s=10**4*rand()
list(s)     // four independently-drawn random values, each *10
```

The operator (`*`) never sees anything but two streams -- it is
unchanged, oblivious, and identical to the `(10**4)*(1..4)` case.

**The stream is what makes the sibling operand post-eval.** `for` and
`while` are post-eval commands that replay their body argument's
token-slice once per iteration -- the loop construct itself decides to
replay. Here, the *same replay mechanism* applies to the scalar operand,
but the *trigger* is different: not the construct's own post-eval-ness,
but the presence of a stream sibling. A stream operand effectively
extends post-eval-style replay to whatever it's combined with -- the
operator stays oblivious and non-post-eval throughout; only the sibling
operand's evaluation pattern changes, from "once" to "replayed per
element," exactly as a loop body goes from "once" to "replayed per
iteration." Same mechanism (token-slice replay via the static postfix
buffer -- see *Why the Pipeline Is So Clean: Fischer-LeBlanc and argoff*
in APPENDIX-C), different trigger.

### Writable (bidirectional) streams *(planned, ivtools-3.x)*

The `[]` empty bracket form — currently reserved — is proposed as the
syntax for an empty writable stream:

```comterp
s=[]          // empty writable stream (bidirectional channel)
push(s 10)    // write a value
push(s 20)
push(s 30)
next(s)       // 10
next(s)       // 20
```

`push(strm val)` / `next(strm)` form a producer/consumer pair
analogous to `print` / `eval` on the wire protocol.  The stream
accumulates values until `close(strm)` seals it, after which
subsequent `next()` returns nil.

#### next(:nowait) — non-exhausting peek

A plain `next()` on an empty-but-not-closed writable stream returns nil
and marks the stream exhausted immediately -- no blocking anywhere.
`next(:nowait)` is the "peek without exhausting" primitive using a
three-way distinction:

- value ready → returns the value
- empty (not yet closed) → returns `blank`
- closed and exhausted → returns `nil`

This preserves `nil` as the exclusive exhaustion sentinel and avoids the
race between "stream is done" and "value hasn't arrived yet."  The
consumer pattern in reactor mode:

```comterp
while(1
  v=next(strm :nowait);
  if(v==nil :then break());
  if(type(v)!=`BlankType :then process(v));
  update())     // yield to ACE reactor
```

`update()` yields to the ACE event loop between polls, so `push()` from
a callback or remote connection can arrive without blocking the REPL.
This is the same pattern used by `remote(:nowait)` and the `drawmo`
orchestrator.

Bidirectional streams work naturally in `comterp listen` / ACE reactor
mode.  In the plain REPL, `:nowait` with `update()` polling is the
correct approach (same as `remote(:nowait)` already works).


### Design Provenance and Prior Art

The automatic scalar overdrive mechanism — where any scalar operator
applies element-wise over a stream without the operator knowing anything
about streams — is an original invention of Scott Johnston. The
conceptual lineage traces to a SPIE paper in 1988 and the command
interpreter work at Honeywell and Triple Vision that preceded ComTerp.
The working `$$` stream mechanism with lazy evaluation and automatic
scalar overdrive was developed and refined well into the 2000s and
continues to be extended in the ivtools-2.x series.

**What existed before:**

- **APL/J/K** — automatic broadcasting over arrays, but eager and fully
  materialized. The spirit is the same but the execution model is
  opposite: arrays are computed all at once, not lazily on demand.
- **MATLAB** — closest in surface syntax (`a*2` broadcasts over a
  vector), but eager, and `*` vs `.*` means the programmer must
  explicitly signal element-wise intent. Not automatic.
- **Lucid (language, Wadge and Ashcroft, 1985)** — a dataflow language
  where every variable is implicitly a stream and operators apply
  element-wise. A whole-language commitment, not an embeddable
  mechanism, and never became practical. Intellectually in the same
  lineage as Karl Fant's NCL work which also influenced ComTerp.
- **Haskell/Clojure** — lazy sequences, but explicit lifting required.
  The programmer must write `fmap (*2) list` or `(map #(* % 2) coll)` —
  the operator does not overdrive automatically.

**What is distinctive about ComTerp's overdrive:**

- **Automatic** — `(1..5)*2` just works. No `fmap`, no `.*`, no lifting.
  The scalar operator `*` is oblivious to streams entirely.
- **Lazy** — streams are single-pass and evaluate on demand. No full
  materialization required.
- **Post-eval flag** — a clean architectural separation between operators
  that are overdriven by streams (scalar operators, non-post-eval
  commands) and consumers that receive the stream intact (post-eval
  commands like `list()`, `sum()`, `each()`). The flag is per-command
  and controls the overdrive boundary.
- **Three-level hierarchy** — scalar (fully overdriven), post-eval
  (receives stream intact, consumes one level), deep (receives full
  nested structure, traverses itself). This is a runtime dispatch
  mechanism, not a type system feature.
- **Embeddable** — ComTerp is a scripting layer on top of a C++
  application framework. The stream algebra is available wherever
  ComTerp expressions are evaluated, including over TCP sockets.
- **nd by composition** — higher-dimensional array operations emerge
  from composing zip (`,` overdriven by n streams) and cross product
  (`for`/`while` overdriven by a stream in the body). No special nd
  array type required.

The combination of laziness, automatic overdrive without explicit
lifting, the post-eval/deep distinction as an explicit architectural
flag, and nd structure emerging from stream composition rather than
being declared upfront — this specific architecture has no known prior
art.

### Two-stream binary ops zip element-wise

When both operands are streams, the operator is applied pairwise —
not a cross-product:

```
list((1..3)+(10..12))  // {11,13,15}  -- zipped add
list((1..3)*(1..3))    // {1,4,9}     -- element-wise multiply (squares)
```

### Streams are single-pass

A stream is exhausted after consumption. `next()` returns `nil` on an
exhausted stream. Reassign to replay:

```
ss=(1..5)*2
list(ss)               // {2,4,6,8,10} -- consumed
next(ss)               // nil -- exhausted
ss=(1..5)*2            // reassign to replay
```

### Stream copy forks at the current position

`$$s` (equivalently `stream(s)`) copies a stream **at its current state of
consumption** — not from the beginning. The copy is an independent
continuation from wherever `s` has been consumed to, and the two streams
thereafter advance without affecting each other.

```
s=(10 20 30)
next(s)                // 10  -- s is now at {20,30}
t=$$s                  // t forks here: t is {20,30}, NOT {10,20,30}
next(s)                // 20  -- s advances
next(t)                // 20  -- t advances independently
next(s)                // 30
next(t)                // 30
next(s)                // nil -- s exhausted; t was unaffected by s
```

This is the property cross-products rely on: copy the inner stream at the
point the outer loop has reached, drain the copy, leave the original at
its position for the next outer step.

Mechanically, a stream's unconsumed remainder *is* its state — consumption
diminishes the stream's internal directory in place, so "copy at current
position" is simply a deep copy of whatever remains. The copy duplicates
that remaining directory but **shares the underlying token buffer by
reference** (it is immutable and reference-counted), so forking is cheap
and never duplicates the elements' code — only the small record of which
elements are left. A fresh full copy is therefore just a copy taken before
any `next()`:

```
s=(10 20 30)
t=$$s                  // copy before consuming -- t is a full {10,20,30}
```

This is the consume-once contract (above) made forkable: single-pass means
each stream is consumed once and is gone as it flows by; copy-at-position
means you may fork the *unconsumed* part into an independent stream at any
point, and each fork is then itself single-pass.

### List construction and growth with `,`

The canonical way to build a list is with the `,` (tuple) operator.
`list()` with no arguments creates an empty list to start from; `,`
appends to it **in place**:

```
lst=list()             // empty list -- use list() only for this
lst,1                  // {1}
lst,2                  // {1,2}
lst,"hello"            // {1,2,"hello"}
```

Since `,` mutates and returns the same object, reassignment is optional:

```
lst=list()
for(i=0 i<10 i++ lst,i)
// lst == {0,1,2,3,4,5,6,7,8,9}
```

A literal list without a prior `list()`:

```
lst=1,2,3,nil,5        // {1,nil,3,nil,5} -- nil is a valid list element
```

The `,` operator binds below all arithmetic and comparison operators
(precedence 35), so expressions on either side are fully evaluated first:

```
lst,x*2+1              // appends (x*2+1), not x*(2+1)
```

**`list()` vs `,`**: `list()` with no args creates an empty list.
`list()` with space-separated args treats them like a stream — it stops
at the first `nil` argument:

```
list(1 nil 3 nil 5)    // {1,} -- stops at first nil!
1,nil,3,nil,5          // {1,nil,3,nil,5} -- correct, use , instead
```

`list(lst x)` does not append — it constructs a new 2-element list
containing `lst` and `x`. Use `,` to append:

```
lst=list(1 2 3)        // {1,2,3} -- new list from args (no nils here)
lst,4                  // {1,2,3,4} -- append via ,
list(lst 4)            // {{1,2,3},4} -- wraps lst, does not append
```

Out-of-bounds `at()` access returns `BlankType`, not `nil`. Test with
`empty()`:

```
e=list()
at(e 0)==empty()       // true -- BlankType, not nil
```

### String concatenation with streams

`+` between a string and an integer stream produces char codes, not
digit strings. Use `str()` to convert:

```
list("node"+(1..3))        // {"node\001","node\002","node\003"} -- char codes
list("node"+str(1..3))     // observe -- str() over a stream
```

### next() as escape hatch

When stream algebra coordination is too complex to model at parse or
runtime, `next()`/`while` always works:

```
total=0
s=$$(1,2,3,4,5)
while((v=next(s))!=nil
  total=total+v)            // total==15
```

This is the reliable fallback when operator-level stream driving doesn't
coordinate as expected.

## Strings

String literals use double-quotes. Escape sequences: `\"` for a literal
double-quote, `\n` for newline, `\t` for tab, `\\` for a literal
backslash before `n`, `r`, or `t`.

Key string commands:

```
index(str fragment)           // 0-based position of fragment, nil if not found
index(str fragment :last)     // last occurrence
index(str fragment :all)      // list of all positions
index(lst val :substr)        // strstr match on list elements
substr(str match :after)      // string after match
substr(str match :nonil)      // return input string if no match (instead of nil)
split("a;b;c" :tokstr ';')    // split by single-char delimiter (single-quoted)
split("foo bar" :tokstr)      // split by whitespace
split("abc")                  // list of char codes
join(lst)                     // join list of chars back to string
eq(str1 str2 :n len)          // partial string comparison
size("hello")                 // 5
"foo"+"bar"                   // "foobar"
print("val=%v" 42 :str)       // returns formatted string
```

Note: `:substr` is only needed when the first arg to `index` is a list.
When both args are strings, substring search is the default behavior.

Single-quoted literals are chars, not strings: `'a'`, printed with `%c`.

## Symbols

A symbol is an interned string — a unique integer id associated with a
name, stored once in a global symbol table. Symbols are the basis of
variable names, command names, keywords, and type names in ComTerp.

### Creating and converting symbols

```
`foo                   // backquote returns symbol without lookup
symadd("foo")          // create symbol from string, return without lookup
                       // symadd is idempotent -- safe to call on existing symbol
symstr(`foo)           // symbol -> string: "foo"
symid(`foo)            // symbol -> integer id
symbol(id)             // integer id -> symbol
```

Round-trip: `symstr(symbol(symid(`foo)))` → `"foo"`.

`symadd` is idempotent — calling it on an already-existing symbol returns
the existing id rather than creating a duplicate. There is no need to
check whether a symbol exists before calling `symadd`.

To unbind a symbol from its value, assign nil:

```
myvar=42
myvar=nil              // unbind
```

### Symbol variables

```
myvar=42
symval(myvar)          // 42 -- value of symbol variable, NO backquote
symvar(`dynvar)=99     // assign to symbol variable by name, WITH backquote
dynvar                 // 99
```

Note the asymmetry: `symval` takes the variable directly (no backquote),
while `symvar` takes a backquoted symbol. This is because `symval` receives
the variable's value — which is already a symbol — while `symvar` needs the
symbol identity to avoid looking up the variable first.

The `symvar`+`symadd` combination enables runtime variable creation:

```
name=symadd("runtime_var")
symvar(name)=123
runtime_var            // 123
```

### Symbol table introspection

```
symid(:cnt)            // current number of symbols in table
symid(:max)            // maximum capacity of symbol table
strref("hello")        // reference count for a string
```

### Symbol comparison

Use `==` for symbol equality — it works reliably for all symbol values:

```
`foo==`foo             // true
`foo==`bar             // false
symbol(symid(`foo))==`foo  // true
```

Lexicographic symbol ordering:

```
lt(`aaa `bbb :sym)     // true
gt(`bbb `aaa :sym)     // true
lt_or_eq(`aaa `aaa :sym) // true
```

### Symbols as type and class names

`type()` and `class()` return symbols, which can be compared with backquote:

```
type(42)==`IntType        // true
type(3.14)==`DoubleType   // true  (3.14 is double, not float)
type("hello")==`StringType // true
type(true)==`BooleanType  // true
type(`foo)==`SymbolType   // true
class(attrlist())==`AttributeList // true

float(3.14)            // explicit conversion to FloatType
double(3)              // explicit conversion to DoubleType
```

### print(:sym) — materializing symbols from formatted strings

`print(:sym)` returns its output as a symbol rather than printing it.
This enables dynamic symbol construction from formatted strings:

```
s=print("val=%v" 42 :sym)  // symbol whose name is "val=42"
symstr(s)                  // "val=42"
```

Combined with `symvar`, this enables fully dynamic dispatch:

```
key=print("handler_%v" type(x) :sym)
symvar(key)=func(...)      // register handler for type
```

## Attribute Lists

An attrlist is a key/value store. Create one with `attrlist()` or
`list(:attr)`, or with the **attrlist literal** syntax *(ivtools-2.2)* — parentheses
whose first token is a keyword:

```
al=(:a 1 :b 2)         // literal, same postfix as attrlist(:a 1 :b 2)
al=(:flag)             // keyword-only sets value to true
al=attrlist(:a 1 :b 2) // equivalent command form
```

The parser distinguishes an attrlist literal from a grouping expression
by the presence of a leading keyword. Plain grouping `(1+2)*3` is
unaffected. A value before the first keyword is an error:
`(4 :x 7)` → parse error "attribute literal must start with :key".

```
al=attrlist(:foo 42 :bar "hello" :flag)
al.foo             // 42
al.bar             // "hello"
al.flag            // true (keyword-only sets true)
al.missing         // nil
```

Dot notation on any symbol creates a compound variable backed by an
attrlist on first use:

```
point.x=10
point.y=20
```

### Enumerating an attrlist

Use `size()`, `at()`, `attrname()`, and `attrval()` to enumerate:

```
al=attrlist(:x 1 :y 2 :z 3)
for(i=0 i<size(al) i++
  print("%v=%v
" attrname(at(al i)) attrval(at(al i))))
```

`at(attrlist n)` is the escape mechanism into the raw `Attribute` layer.
`attrname()` and `attrval()` are the only commands that receive it before
auto-dereference — any other command gets the dereferenced value instead.
Note that `type(at(al n))` returns the value's type, not an attribute type,
and enumeration order may not match insertion order.

`Attribute` objects can live on the stack and be passed to any command.
Whether the key is preserved depends on whether the receiving command
explicitly checks for `AttributeType` before dereferencing — `attrname()`
and `attrval()` do this; all other current built-in commands dereference
immediately via `stack_arg()`, losing the key. A custom `ComFunc` could
preserve the key by inspecting the `ComValue` type before calling
`stack_arg()`. In practice, for the built-in scripting layer, `attrname()`
and `attrval()` are the only commands that see the key.

### Stream enumeration of an attrlist

`attrname()` and `attrval()` also accept a stream of attributes
directly, returning a stream of keys or values respectively. An attrlist
literal used as a stream source yields its entries as `Attribute` objects:

```
$list(attrname($$(:a 4 :b 7)))   // {"b","a"}
$list(attrval($$(:a 4 :b 7)))    // {7,4}
```

The two streams are consistent with each other — the nth name corresponds
to the nth value — so they can be zipped or processed in parallel.
Note that the order is reverse insertion order (last key first), which
reflects the underlying attrlist storage. If you need both key and value
together, use the `for`/`at()`/`size()` loop form above instead.

### Merging and subtracting attrlists

`+` merges two attrlists into a new one — the second operand wins on key collision.
`-` removes from the first attrlist any keys present in the second. Both operands
are unchanged; a new attrlist is returned:

```
al1=attrlist(:a 1 :b 2)
al2=attrlist(:b 99 :c 3)
merged=al1+al2       // :a 1 :b 99 :c 3  (al2's :b wins)
diff=al1-al2         // :a 1              (:b removed)
```

### Portable key/value pairs

A single-element attrlist is the idiomatic portable key/value pair —
it passes anywhere as a first-class value:

```
pair=attrlist(:foo 42)
attrname(at(pair))   // "foo"
attrval(at(pair))    // 42
```

Scope rules: the dot namespace is scoped with its root symbol. Inside a
`func()` body, dot attributes on a local symbol are local to that call.
Use `global()` or pass an attrlist via keyword arg to share state.

### Lists of attrlists

A list of attrlists uses the tuple `,` operator between attrlist literals:

```
lst=(:a 1),(:b 2)      // 2-element list, size(lst)==2
at(lst 0).a            // 1
at(lst 1).b            // 2
```

Note: `[]` is reserved for flowtran flowgraph syntax and is not a
subscript operator. Use `at(lst n)` to index into a list.

For a **singleton list** (one attrlist), a trailing `,` inside `{}` is
required to force the parser to produce a list rather than a bare attrlist:

```
lst={(:a 4),}          // 1-element list, size(lst)==1
at(lst 0).a            // 4
```

Without the trailing comma, `{(:a 4)}` passes the attrlist through
unwrapped — the `{}` adds nothing for a single non-list value.

The serializer (`print()`) emits the trailing comma automatically
for singleton lists, so `print()`/`run()` round-trips correctly:

```
s=print(lst :str)      // produces "{(:a 4),}"
lst2=run(s :str)       // recovers the 1-element list
at(lst2 0).a           // 4
```

## Conventions for .comt Scripts

- Use `print()` not `printf()`
- Fixed args always before keyword args
- Single-quoted char literals: `'a'`, `';'`
- Double-quoted string literals: `"hello"`
- Test accumulator: `ok=ok&&(result==expected)`
- Deferred/broken tests: comment out with `/* */`, reference the issue number
- Return `ok` as the last expression for use by `run_all.comt`
- Sub-scripts are not subject to coverage measurement

---

## Symbols are rare strings

The symbol/string distinction is not a type dichotomy; it is a **rarity gradient**,
with interning as the mechanism. A symbol and a string can spell the same
characters — what differs is rarity:

* **Strings are common/open.** Arbitrary character sequences, mostly unique, seen
  once: a filename, a message, a UUID fragment. You expect new ones constantly.
* **Symbols are rare/closed.** A small interned vocabulary, reused, meaningful by
  identity: `Red`, `White`, the month names, command names. You expect the same
  ones to recur.

Interning is rarity made mechanical: you intern the things that recur, because
compare-by-id and store-once pay off only under reuse. Interning a value seen
exactly once buys nothing. So the same characters land on different sides of the
gradient by *how often they recur*, and the surface syntax follows:

* `` `Red `` — backquote: a rare symbol, a vocabulary member, worth interning.
* `"a4b78d83"` — double-quote: a common string, a unique opaque id, nothing to
  intern.

This is why a UUID-like build key reads correctly as a quoted string and wrongly
as a backquoted symbol: the eye is trained to expect quotes around opaque
identifier *values* and backquotes around *names that mean something in the
language* — and that training reflects the real mechanism, not just convention. A
build key never gets dispatched on; it is displayed and compared; it is
string-nature. A color name gets dispatched on by id; it is symbol-nature.

(Implementation: the symbol *is* the interned string; the intern table holds both
forms. The split is one of rarity and use, recovered at the surface by which
quote the author reaches for.)

## See Also

- `INTRODUCTION.md` — project overview and history
- `APPENDIX-B-COMTERP-EXAMPLES.md` -- how to learn comterp command language

- `src/comterp_/tests/TESTING.md` -- comterp self regression tests
- `src/ComTerp/ARCHITECTURE.md` -- ComTerp C++ design
- `src/ComTerp/HACKING.md` -- comterp programming
