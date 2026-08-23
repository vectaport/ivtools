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
3. **Stream literals** — `(val val ...)` when first token is a value
4. **Precedence override** — `(a+b)*c` to override operator priority

That's it. A body does not need parens. `(lst,i; total=total+i)` is
a single `;`-sequence expression that happens to be wrapped in parens,
but the parens add nothing — the semicolons do all the work:

```
for(i=0 i<10 i++ (lst,i; total=total+i))  // works but parens unnecessary
```

**Warning:** a space between two expressions inside parens — without a
semicolon — is not a two-statement body. It is a stream literal:

```
for(i=0 i<10 i++ (lst,i total=total+i))   // body is a 2-element stream literal, not two statements -- loop no-ops
for(i=0 i<10 i++ lst,i total=total+i)     // error: for loop with more than one body -- missing semicolon between statements
```

`(ding beep)` parses as a stream literal of two values, not a grouped
two-statement body. Code relying on space-separated statements inside
parens without a semicolon needs a semicolon added between them.

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
| stream | `$$(1,2,3)` or `(1 2 3)` | sequence of values produced and consumed one at a time |
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

### Symbol-then-parens is always a call attempt

`SYMBOL (args)` on one line is always parsed as a call attempt on
`SYMBOL` — whitespace never matters, only a newline between the symbol
and the `(` escapes it:

```
undefinedsym (print("side effect\n"))   // nothing prints -- glued, drained unevaluated
undefinedsym
print("side effect\n")                  // "side effect" prints -- newline
                                         //   makes these two unrelated statements
```

This is deliberate, minimal-verbiage syntax (`(args)` immediately after
any identifier means "call it"), not something to work around. What
happens next depends on what `SYMBOL` resolves to:

- **A registered command** — dispatches normally.
- **A variable holding a `FuncObj`** (`f=func(...)`) — invoked properly,
  args and all. This is the *dynamic* case: the call actually runs code.
- **Anything else -- except for an undefined symbol** — the arglist is
  evaluated (for side effects) and discarded, and the call's result is
  whatever `SYMBOL` itself resolves to. A variable holding a plain value
  behaves like a *static* `FuncObj`: it "runs" (its arglist still gets
  evaluated), but always returns the same thing regardless of what it
  was given — the same idiom `true()`/`false()`/`pi()` already use
  deliberately ("wrap any expression, override its result"), just
  generalized to any bound value:

  ```
  x=5
  x (print("side effect runs\n"))   // prints, then evaluates to 5
  ```

  An **undefined** symbol is the one case where the arglist is never
  evaluated at all (not even for side effects) — internally it's
  substituted with the built-in, argument-draining `nil` command before
  anything runs. This is a genuinely useful idiom in its own right: a
  `.comt` script can gate a whole feature on whether an optional
  command or func happens to be defined before the script runs —
  `hook (doWork())` silently no-ops if `hook` was never defined, and
  calls it for real the moment it is:

  ```
  // script.comt -- runs doWork() only if the caller predefined `hook`
  hook (doWork())
  ```

  ```
  run("./script.comt")            // hook undefined -- silent no-op
  hook=func(print("hooked\n"))
  run("./script.comt")            // hook now defined -- runs for real
  ```

The same unconditional glue rule applies inside a **stream literal**,
and it is decided once, at parse time, before anything is known about
what the symbol will resolve to — so a stream's *length* can depend on
it in a way that's worth knowing about rather than discovering by
surprise:

```
x=5
y=20
(x (3) y)       // a 2-element stream: {x(3), y} -- always, regardless
                //   of what x is, since the glue to (3) is decided
                //   before x is ever looked up
postfix(x (3) y)  // "3 x{1|0} y" -- confirms it: only one arg got
                  //   attached, to x, and y is untouched
```

If `x` had been a real command or a `FuncObj`, this would be the
*correct* reading (a call followed by a second stream element). Since
`x` is a plain value here, the second element is "surprising" only
until you know the rule: the parser can't see three elements, because
it never considers *not* gluing an identifier to an immediately
following same-line paren group.

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
| 79       | `%%`     | replay        | LtoR  | BINARY          |
| 77       | `@`      | at            | LtoR  | BINARY          |
| 75       | `,,`     | concat        | LtoR  | BINARY          |
| 71       | `*`      | next          | RtoL  | UNARY PREFIX    |
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
- `@` sits well below `.`, not tied to it — `lst@i+1` reads as `(lst@i)+1`
  (arithmetic still binds looser than `@`), but `lst@0..2` and `lst@0**3`
  read as `lst@(0..2)`/`lst@(0**3)` (the numeric stream operators `..`/`**`/
  `%%` all bind looser than `@`, so a range/repeat/replay expression
  indexes directly, no parens needed) — see *At operator* below
- `..` and `**` bind above arithmetic — `(2..4)*5` needs parens around the range
- `,` binds below all arithmetic and comparison — `1+2,3+4` is `(1+2),(3+4)`
- `=` is right-associative and below `,` — `a=b=1` chains correctly
- `;` binds lowest of all — everything to its left and right is a complete expression
- `$$` and `$` are unary prefix RtoL so `$$lst` and `$strm` parse without parens
- `*` plays two roles at once: binary `*` (`mpy`, LtoR, 70) and unary prefix
  `*` (`next`, RtoL, 71) are two separate table entries sharing one operator
  string — the same double-duty pattern `-` already uses for `minus`/`sub`.
  `*s` means `next(s)`, and it composes with binary `*` even without parens
  (`2 * *s` doubles the next value pulled off `s`) — but the two roles need
  a real priority gap, not just different associativity, to resolve cleanly:
  at an exact tie the parser can't settle which role a bare `*` token plays
  before both are on the stack, and silently mis-emits the binary op before
  its right operand is even read. One point of separation (71 vs 70) is
  enough. The one remaining caveat is lexical, not a priority matter: `**`
  is already the stream-repeat operator, so an unspaced `2**s` reads as
  `2 ** s` (repeat) before it ever reaches `*s` — a space or parens avoid it

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
| `*` (unary prefix) | shorthand for `next()` — advance a stream one element |

Unary prefix `*` is plain sugar for `next()`:

```
s=$$(10 20 30)
*s               // 10 -- same as next(s)
*s               // 20
*s               // 30
*s               // nil -- exhausted, same as next() always reports
```

It composes with binary `*` (multiplication) even without parens — `2 * *s`
doubles the next value pulled off `s`, same as `2*(*s)`. The one thing to
watch is lexical, not grammatical: `**` is already the stream-repeat
operator at a higher priority (80 vs. 71), so an *unspaced* `2**s` reads as
`2 ** s` (repeat), never as `2 * (*s)` — a space or parens separate them.

This operator adds almost no new C++ machinery: it is one line in
`ComUtil/optable.c`'s `DefaultOperatorTable[]` mapping the string `*`,
unary-prefix, RtoL, to the existing `next` command — nothing `next()`
didn't already do. Its priority (71) sits one above binary `*`'s (70)
rather than matching it exactly: at an exact tie the parser can't decide
which role a bare `*` token plays before both are on the operator stack,
and silently mis-emits the binary op before its right operand is even
parsed. One point of separation is enough for the unary role to declare
itself. `optable()` is a live view onto that same table at runtime, and
`optable(:insert)`/`optable(:delete)` can add or remove operators exactly
this way from a running script — see `src/comterp_/tests/starnext.comt`,
which round-trips this very operator through `optable(:delete)`/
`optable(:insert)` to prove it isn't special
cased. Any script can bind its own symbol to any command this same way.

### Dot operator

`.` accesses attributes on a compound variable or attrlist:

```
foo.bar=42
foo.bar          // returns 42
```

The dot's lhs may also be a **comp** (a graphic component, in comdraw
and above): the dot resolves into the comp's attribute list, so
`comp.key=val` is `setattr(comp :key val)` spelled as plain assignment,
and `comp.key` reads the fact back:

```
pig=ellipse(200,220, 25,15)
pig.legs=4               // setattr(pig :legs 4)
pig.legs                 // 4 -- and who()/frame() queries see it too
```

Use `setattr()` to stamp many facts in one call; the dot for one at a
time.  The mutation rides the comp reference, so it works from inside a
func even though the symbol binding is frame-local (see *Scoping rules*).

The dot namespace rooted at a symbol is scoped with that symbol — see
**Attribute Lists** below.

### At operator

`@` is binary sugar for `at()`: `lst@n` reads the nth item of a list, and
`lst@n=val` writes it in place:

```
lst=10,20,30,40,50
lst@0            // 10
lst@2=999
lst               // {10,20,999,40,50}
```

A **nil index means the last item**, reading or writing, on a list, an
attrlist or a string alike — so `lst@nil` is the end of the list without
having to say `lst@(size(lst)-1)`:

```
lst=10,20,30
lst@nil          // 30
lst@nil=99
lst              // {10,20,99}

s="abc"
s@nil            // 'c'
s@nil='C'
s                // "abC"
```

That has been `at()`'s behavior since 2015 and was simply never written
down; the list `:set` path was the one place that read a nil index as 0
instead of the last, which is now consistent with the rest.

A string index writes through the same way a list index does:

```
s="teststring"
s@0='x'
s                // "xeststring"
```

A **symbol** is not writable this way — its text is its identity, shared
by everything holding that symid — so `sym@n=c` is declined and leaves the
symbol as it was, the same refusal `at(sym n :set c)` gives.

It chains left-to-right, the same as `.`:

```
outer=999,20,30
outer@0=999,999,777
outer@0@1@2      // 777
```

**Why a separate operator from `.`, not `lst.0`:** an earlier design tried
exactly that — numeric indices after `.` — and ran into a lexer-level wall:
once `0.1.2` is scanned, whether it started life as `0`, `.`, `1`, `.`, `2`
(three chained indices) or `0.1`, `.`, `2` (a float followed by one index)
is genuinely indistinguishable after the fact, since both parse to the
identical token stream a decimal literal already produces. `@` is never
part of any number's own syntax, so `lst@0@1@2` can't collide with a float
literal no matter how it chains. `.` keeps its narrower, simpler job
(attribute/comp access, see *Dot operator* above); `@` owns list/attrlist
indexing exclusively.

On an attrlist, `al@n` returns a **detached, single-entry attrlist** —
not a live handle into `al`:

```
al=(:x 10 :y 20 :z 30)
al@1             // (:y 20) -- a real, independent one-entry attrlist
attrname(al@1)   // "y"
attrval(al@1)    // 20
al@1=99          // no effect -- al@1 has no live connection back to al
```

`attrname()`/`attrval()` accept this shape directly, using its one entry
— the same functions also still accept the older dotted-pair `Attribute*`
shape `.` produces (see *Dot operator* above), so either form works. A
bare positional read handing back a live handle would make `al@n=val`
unenforceable as a no-op (`.`'s own dotted-pair *does* write through, via
`foo.bar=42`'s general lvalue mechanism) — returning a plain, detached
attrlist sidesteps that automatically: a plain `AttributeList` isn't a
recognized assignment target at all, so `al@n=val` just falls through to
the same warning any other non-writable lvalue gets, with no
attrlist-specific rejection code needed.

`@`'s priority (77) is deliberately *not* tied to `.`'s (130) — see the
Precedence Table note above for the tradeoff (`lst@i+1` vs. `lst@0..2`).
Like any other binary operator, `@` overdrives when its rhs is a stream:
`lst@(0..2)` or `lst@s` (for a stream variable `s`) both vectorize into a
stream of results — nothing `@`-specific was needed for that either, it's
the same scalar-overdrive mechanism described under *Scalar overdrive*
below. A **list** of indices is not a stream and does not fan out: it has
no position in it, so `lst@idx` answers nil rather than reading as some
particular index.

```
lst=10,20,30,40
idx=0,2
lst@$$idx        // {10,30} -- a stream of indices fans out
lst@idx          // nil     -- a list of them does not
```

Unlike unary prefix `*`, which is a single `optable.c` line mapping
straight onto the existing `next()` command with no other change, `@`
needed two small, targeted additions to `at()` itself: on the write side
(`lst@n=val` on a plain list), `at()` recognizes when it's being fired as
an assignment's before-part and hands back a `[list, index]` pair instead
of a value, so the assignment can complete the write through `at()`'s own
tested `:set` path rather than a second, independent mutation
implementation; on the read side for an attrlist, `at()`'s existing
per-position loop builds a detached one-entry attrlist to return instead
of the old live `Attribute*` (its `:set`/`:ins` mutation logic underneath
is unchanged). Chaining and stream overdrive needed nothing beyond
that — see `src/comterp_/tests/atop.comt` for the full behavior this
section describes, exercised end to end.

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
- **local scope** — the interpreter's flat top-level variable table,
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

### Closures — captured at declaration time

A `func()` value captures its free variables when it's *declared*, not
when it's called. Define one while a particular binding is live, let it
escape, mutate that binding, then call the escaped func — it sees the
value that existed at definition time, not whatever the binding has
become since:

```
y=1
outer=func(y=42; func(y))   // construction itself must be the last thing
                              // evaluated -- see "istype()/isclass()..." above
escaped=outer()
y=100
escaped()                    // 42 -- not 100
```

That's the standard test for closures in any language (define under a
binding, escape, mutate, call), and `func()` passes it.

**Which free variables get captured, and which don't.** A `FuncObj` is
still just a saved token buffer (`_toks`/`_ntoks`) — nothing about *how*
it runs changed, only *when* a free variable's value gets read:

- **Read-only or read-before-write** — a name the body reads without
  ever writing it first (`func(y)`), or reads and only *then* locally
  reassigns (`func(y=y+1)`) — is a genuine input, and is captured once,
  at declaration time, exactly as above.
- **Write-before-read** — a name the body assigns before it's ever read,
  even to `nil` (`func(y=nil; y)`) — is pure local scratch. It never
  touches the outer scope at all, capture or otherwise; this is the way
  to deliberately opt a name out of capture.
- **`local(y)`/`local(y)=...` and `global(y)`/`global(y)=...`** stay
  exactly as dynamic as they've always been — the explicit escape to the
  session-scope or process-scope table, unaffected by any of this (see
  *Escaping the func scope* above).

**An explicit keyword always overrides a capture.** Keywords still build
the body's own locals the same way they always have; if the caller
supplies one, it lands on top of whatever was captured, no different
from any other keyword-shadows-outer-binding case:

```
y=42
f=func(y)
y=999
f()                // 42 -- the capture
f(:y 7)             // 7 -- an explicit keyword always wins
```

**A different tool for a different job: `eval()`'s own `:alist` keyword.**
Declaration-time capture above snapshots a *value* — good for an
ordinary closure, but the snapshot is frozen the moment `func()` runs,
same as any other language's captured-by-value locals. When what's
wanted instead is a *live, shared, mutable* binding — several callers all
seeing each other's writes — reach for `eval(cmdstr|funcobj :alist
attrlist)`, which runs its argument with the func-local scope (`_alist`)
set to the given attrlist first, the same lookup tier a func's own
keyword args live in, just supplied explicitly instead of by the caller
passing keywords. Attrlists are mutable reference objects (`al.x=99`
already mutates the same object a caller holds — see *Writing through a
reference* above), so this supports genuine, *persisting, mutating* state
across calls, not just a frozen snapshot:

```
y=42
eval(func(y) :alist attrlist(:y y))   // 42 -- same capture as the bake-in-literal trick
```

The real payoff is combining it with dot access on an attrlist that holds
both data *and* a method, using the attrlist itself as `:alist` — real,
working `self`-bound method dispatch:

```
counter=(:n 0 :incr func(n=n+1; n))
eval(counter.incr :alist counter)     // 1
eval(counter.incr :alist counter)     // 2
eval(counter.incr :alist counter)     // 3
counter.n                              // 3 -- the object's own field, genuinely mutated
```

`counter` is a single attrlist holding one data field (`:n`) and one
method (`:incr`, a `FuncObj`). Passing `counter` itself as `:alist` means
`incr`'s free variable `n` resolves against `counter`'s own field — the
same attrlist is simultaneously "the method" and "self." This is a real
object system — data and behavior bundled together, with genuine identity
and mutation — just without privacy: `counter.n` is directly readable and
writable from outside (as above), and `incr` itself isn't bound to
`counter` specifically — it's a plain `FuncObj` value, callable against
any attrlist via `:alist`. No private/protected keywords, the same way
plenty of dynamic languages (Python, JavaScript, Lua) leave access control
to convention rather than enforcement. Built entirely from existing
pieces — attrlist literals, `func()`, dot access, `eval()`'s `:alist` —
no new mechanism required. A single-method object built this way is a
closure with the field-vs-snapshot tradeoff made explicit and visible:
the data field is a real, shared binding every caller can see and mutate,
where declaration-time capture above gives each `func()` its own private,
frozen copy instead.

One rule carried over from everywhere else in this doc: the method reference
(`counter.incr`, or a plain `func(...)`) must be constructed or
dot-accessed directly in the `eval()` call, never assigned to a plain
variable and referenced bare first — a bare reference to a `FuncObj` fires
immediately, same niladic-firing rule as always, with no exception for
being an argument to `eval()`.

**`obj.method(args)` — sugar for the same thing.** Writing out
`eval(obj.method :alist obj)` every time is more ceremony than the pattern
needs, so a dot access with parentheses attached fires the method directly,
self-bound the same way, with any arguments reaching `arg(n)` inside the
body:

```
counter=(:n 0 :incr func(n=n+1; n))
counter.incr()                        // 1
counter.incr()                        // 2
counter.n                              // 2 -- same real mutation as eval(:alist)

al=(:addto func(n=n+arg(0); n) :n 0)
al.addto(2)                            // 2
al.addto(5)                            // 7 -- arg(0) is the call's own positional
al.n                                    // 7
```

Bare access (`counter.incr`, no parens) is completely unaffected — it still
returns the raw `FuncObj`, unfired, exactly as it always has (that's what
lets `eval(counter.incr :alist counter)` work in the first place). Only a
dot access with a trailing arglist — empty parens included — fires. A call
on an attribute that isn't a `FuncObj`, or a method name the attrlist
doesn't have, warns and returns `nil` rather than erroring:

```
al=(:x 10)
al.x(1)                                // WARNING: "x" is not a func-valued attribute -- nil
al.nosuchmethod(1)                      // WARNING: "nosuchmethod" is not a func-valued attribute -- nil
```

Arguments are evaluated in the *caller's* own scope, before `self` is
switched in — so a variable reference in an argument resolves normally,
not against the object being called:

```
src=(:val func(cnt) :cnt 7)
dst=(:store func(cnt=arg(0)) :cnt 0)
dst.store(src.val())                   // 7 -- src.val() runs in the caller's scope
```

Nested attrlists self-bind independently — a method on an inner attrlist
sees only its own fields, never the outer one's:

```
outer=(:cnt 0 :bump func(cnt=cnt+1; cnt) :inner (:cnt 100 :bump func(cnt=cnt+1; cnt)))
outer.bump()                           // 1
outer.inner.bump()                      // 101 -- inner's own :cnt, untouched by outer's
```

**There is no `self` — just the enclosing attrlist's own name.** A method
calling a sibling method *with arguments* by bare name always misfires: a
symbol-with-arglist that isn't a registered global command never reaches
through `_alist` to a sibling, and there's no "myself" keyword to write
instead. But the object is nothing more than an ordinary attrlist bound to
an ordinary variable, and that variable is fully readable from inside one
of its own methods — nothing about entering a self-bound call hides outer
scope, `_alist` is only consulted *first*. So a method reaches a sibling
the same way any outside caller would, by writing the object's own name
and an explicit dot-call:

```
zoo=(:flash func(things) :report func(zoo.flash(:things "hello")))
zoo.report()                           // "hello" -- report reaches flash via zoo's own name
```

No special mechanism was added for this — `zoo` inside `report`'s body is
the exact same ordinary global lookup it would be anywhere else. That's
also the catch: nothing ties this lookup to *the object the method
happens to be running against*. It's the current value of whatever name
was written, resolved fresh at the point of use — so slipping a different
attrlist under the same name mid-flight redirects it, even for a call
still self-bound to the original object:

```
zoo=(:flash func("MINE") :report func(zoo.flash()))
saved=zoo
zoo=(:flash func("OTHER"))             // a different object, same global name
saved.report()                         // "OTHER" -- report is self-bound to `saved`, but
                                        // its body's "zoo" reads the CURRENT global, not saved
zoo=saved                              // restore
```

`saved.report()` self-binds `_alist` to `saved` (the original object) for
the call, exactly as always — but `report`'s own body never says `self`
or `saved`, it says `zoo`, and `zoo` is just a name in scope like any
other. This is the same tradeoff called out for `counter.n` above: no
privacy, no enforcement, convention rather than a language guarantee —
and it works as a "self" reference only for as long as nothing reassigns
the name out from under it, which is ordinarily true (a `zoo=(...)`
attrlist literal isn't usually reassigned mid-script) but never actually
enforced.

**Keyword arguments to a method call are ephemeral, unless the method
writes them.** `al.method(:key val)` writes `key` onto `al` before firing
— exactly as if you'd written `al.key=val` first — then compares it back
afterward: unchanged means nothing inside the call touched it, so it
reverts (removed entirely if `al` never had that field before this call);
different means the method's own body assigned a new value there,
self-bound, and that assignment persists same as any other write would.
Reading a keyword-supplied value never makes it stick — only writing to it
does:

```
c=(:incr func(cnt++) :cnt 0)
c.incr(:cnt 10)                        // 10 -- cnt++ reads 10, writes 11
c.cnt                                   // 11 -- the write's own result persists

t=(:tell func(cnt) :cnt 0)
t.tell(:cnt 99)                        // 99 -- reads the keyword value
t.cnt                                   // 0 -- never written, reverts to what it was

u=(:show func(nope))
u.show(:nope 5)                        // 5 -- nope never existed on u at all
u.nope                                  // nil -- read-only, so never added for real
```

A keyword naming a field the object never had works the same way as one
overriding an existing field — it's added before the call and, if nothing
writes to it, removed again afterward, not left behind as a stray
attribute.

**Making a keyword arg actually stick, on purpose.** `c.incr(:cnt 10)`
sticks *because* `cnt++` writes `cnt` as a side effect of what it was
already going to do, not because anyone asked for the keyword itself to be
kept. A `cnt=cnt` re-assignment written specifically to try to keep it
doesn't work, and can't be made to: after that line runs, `al` looks
identical whether the assignment happened or not — the value's the
same either way, so there is nothing left to distinguish "genuinely
written" from "never touched" by inspecting the object afterward. Two
ways to actually get a persisting set, both already just working today:

- **Set it directly**, no method call needed: `al.cnt=10` is a plain
  attribute write, no keyword-ephemerality involved at all.
- **Use a differently-named setter.** Give the keyword parameter and the
  field it sets different names, and the ambiguity disappears — the
  parameter is genuinely read-only (reverts, correctly) and the field is a
  genuine write to a *different* name (persists, correctly, the same
  self-bound mutation that makes `cnt++` work):

```
c=(:cnt 0 :setcnt func(cnt=val))
c.setcnt(:val 8)                       // 8
c.cnt                                   // 8 -- a real write, to a name the keyword never used
c.val                                   // nil -- the keyword's own name, untouched, reverts
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
the same fallback described above, not a captured reference to anything —
so "reads as nil" holds **only when no outer variable of that name
exists**. If the surrounding scope already
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

### Introspecting a func's IO contract: `help(f)`

Comterp already communicates several contracts textually rather than
leaving them implicit: `postfix(help)` appends a trailing `*` to a
post-eval command's name, and `help()` renders a registered command's
docstring and keyword list on request. `help(f)`, where `f` is a bare,
unfired FuncObj, extends the same idea to a func you wrote yourself —
rendering its positional and keyword contract as one line of text,
without running the body:

```
f=func(arg(0)+arg(1))
help(f)              // "(arg0 arg1)"
```

`help()` already reads its arguments symbol-preserving (it's post_eval),
so `f` reaches it completely unfired — the same non-firing read
`isclass(x :sym)` relies on elsewhere. Positionals render as `arg0`,
`arg1`, ... (from the highest literal `arg(n)` index referenced), or
`...` when the count can't be pinned down statically (`narg()` usage, or
a computed index). Keywords show only the ones a caller can meaningfully
supply — read-only and read-before-write free variables — since
write-before-read is local scratch a keyword would just be clobbering,
not a genuine input. Escaping (`local()`/`global()`) variables are
reported in a trailing annotation instead, since they're not part of the
func's own frame:

```
f=func(local(w)=1)
help(f)              // "()  -- escapes: w->local"
```

**Defaults are shown too, in both of the senses that turn out to
matter.** A func using the `if(x==nil :then DEFAULT :else x)`
optional-keyword idiom has its coded `DEFAULT` rendered inline, when
`DEFAULT` is a single literal:

```
ini=func(if(x==nil :then 99 :else x))
help(ini)             // "(:x [99])"
```

But the coded default is only the *written* fallback — closures
(above) mean it isn't necessarily the *effective* one. If `x` already
had a real value in scope the moment `func()` ran, declaration-time
capture grabbed that value, not `nil` — the body's `x==nil` check will
never be true for as long as that capture stands, so `99` is currently
dead code:

```
x=7
ini=func(if(x==nil :then 99 :else x))
help(ini)             // "(:x [99, 7])"  -- coded default, then what got captured
ini()                 // 7, not 99
```

The same capture applies even without a coded default idiom at all —
declaration-time capture doesn't care whether the func author wrote
optional-parameter logic for a variable or just read it plainly. From a
caller's side, both look the same: call the func bare, get whatever was
captured; supply the keyword explicitly, get that instead:

```
w=42
f=func(w+1)
help(f)               // "(:w [42])"
```

This isn't a bug or a special case worth working around — it's the same
fact the earlier closures section already establishes (a func's
free-variable reads resolve to whatever was true *when `func()` ran*,
not whatever's true when it's called), surfaced in text instead of
staying implicit. A captured value is simple, stable, and constant once
the func exists, which is exactly what makes it worth `help()` showing
rather than leaving to be discovered by firing the func and being
surprised.

None of this is fully perfected — return-kind isn't derived yet, output
is one line rather than column-aligned, and there's no way yet to feed a
`help(f)` string back in to reconstruct the func the way `~~` round-trips
a stream's own emitted representation. But the direction is the same one
the rest of the language already leans on: make a contract legible as
text first, and let round-tripping follow once the shape earns it.

### Lazy arguments: `:posteval`

An ordinary `func()` call is eager: every positional and keyword argument
is fully evaluated once, before the body runs, whether or not the body
ever reads it (see `arg()`/`narg()` above). `func(body :posteval)` makes
the call lazy instead: none of its arguments are evaluated at call time.
They stay unevaluated — the same "wait until pulled" contract any
post_eval command's own pending args already have — resolved only the
moment something inside the body actually asks for them: `arg(n)` on its
first read, a keyword on its own first read-before-write. An argument the
body never reads is never evaluated at all, side effects included:

```
f=func(c=arg(0); if(c :then arg(1) :else -1) :posteval)
f(false print("never runs\n"))   // -1 -- arg(1)'s expression is never touched
f(true print("runs\n"))          // prints "runs", then true
```

Compare the same body without `:posteval` — the caller's argument is
evaluated up front regardless of what the body's own `if` ever reads:

```
g=func(c=arg(0); if(c :then arg(1) :else -1))
g(false print("runs anyway\n"))   // prints "runs anyway" first, then -1
```

**Every access re-fires — `arg(n)` and a keyword alike.** An unwritten
`:posteval` argument is a live tap, not a constant memoized on first
read: each access re-runs the caller's expression fresh. This is the
behavior a user would expect if the argument expression were literally
inlined at each point of use, and it's what lets a `while` loop inside
the body see a live, current value each iteration, the same way any
post_eval command's own operand (`while`'s condition, for instance) is
genuinely re-evaluated every pass:

```
hits=list()
counter=func(hits,1; size(hits))
loopf=func(n=0; while(arg(0)<4 n=n+1) n :posteval)
loopf(counter())   // 3 -- 4 condition checks (hits reaches 1,2,3,4), 3 loop bodies
size(hits)         // 4

side=list()
bump=func(side,1; size(side))
h=func(y+y :posteval)
h(:y bump())   // 3 -- y is read twice, evals twice: bump()=1, then bump()=2, 1+2=3
size(side)     // 2
```

`arg(n)` has no lvalue form — there's no `arg(0)=...` — so there's
nothing to protect by caching it. A keyword *can* be written (`y=5`),
and any write (plain or compound) freezes it into an ordinary owned
local from that point on — the same write-freezes convention #310's
capture classifier already uses for a free variable — but a keyword
that's only ever read stays a live tap for as long as it's read.

Assigning to a keyword before ever reading it, or never reading it at
all, means its argument expression never runs at all — the same
write-before-read rule captured above, applied to a keyword argument
instead of a free variable. The idiom for pinning one draw of a
repeatedly-read `arg(n)` or keyword is the same one non-func code
already uses for any post_eval command's operand: assign it to a local
once (`ycopy=y`), then read that local from then on.

**Composes with any existing control command, no special-casing needed.**
`if`/`while`/`switch` already selectively evaluate their own operands via
the same on-demand mechanism (resolving one token-span bookmark at a
time) that `arg()`/a keyword read now use to reach back into the
*caller's* still-pending arguments. So a control command inside a
`:posteval` body that skips a branch transparently skips whatever
caller-side expression that branch's `arg(n)` would have pulled — the
laziness of the control command and the laziness of the call compose for
free, all the way back through a chain of `:posteval` calls, without
either side needing to know the other exists.

**Steering: a keyword's own defining code runs only if it's actually
needed.** That composition isn't just an optimization — it's a
higher-level control construct in its own right. `:posteval` lets a
keyword carry *behavior to try*, not just a value, with the decision of
whether to run it left entirely to the body:

```
steer=func(
  r=primary();
  if(r==nil :then fallback() :else r)
  :posteval)

hits=list()
cheap=func(hits,"cheap"; 42)
expensive=func(hits,"expensive"; 99)
steer(:primary cheap :fallback expensive)   // 42 -- hits=["cheap"], fallback's own code never ran

hits2=list()
failing=func(hits2,"failing"; nil)
backup=func(hits2,"backup"; 7)
steer(:primary failing :fallback backup)    // 7 -- hits2=["failing","backup"], fallback ran only because primary did fail
```

`primary`/`fallback` are ordinary bare names — they fire when read, same
as anywhere else in the language — but *when* they're read is entirely up
to `steer`'s own control flow, deferred by `:posteval` until the `if`
actually needs one. A retry/fallback/circuit-breaker combinator falls out
for free, without `steer` needing any special "don't run this yet" syntax
beyond the keyword declaration itself.

**The gate is dynamic within the limits of one static classification
pass.** A whole `;`-joined statement chain is tokenized and classified
once, before any of it runs — the same fact issue #328 is built around.
Two cases correctly stay dynamic across that: an *undefined* name (the
original #328 forward-reference shape) and a name that's *already*
`:posteval` at that single classification pass — both get pedepth-marked
so the real dispatch-time gate re-checks the *current* value right when
the call fires, honoring a reassignment that happened earlier in the same
chain (or on an earlier line entirely — the ordinary case: define on one
line, call on a later one). What does *not* stay dynamic: a name that
resolves to an already-*eager* `FuncObj` at that same classification
pass never gets pedepth-marked at all, so its arguments are evaluated
immediately by the ordinary eager path — before a same-chain reassignment
to `:posteval` earlier in that chain has even run. By the time the
reassignment takes effect, the arguments are already gone; there's no
token span left for any dispatch-time recheck to defer. Closing this gap
fully would mean pedepth-deferring every call-shaped symbol reference
unconditionally, not just undefined or already-`:posteval` ones, so the
dynamic gate is consulted for literally every func call in the language —
a much larger change than this feature makes on its own.

**Observation: `:posteval` turns keywords into a redirection/distribution
mechanism, not just a delay.** A few things fall out of the mechanics
above that are worth noticing on their own, not just as consequences of
how the pull works:

- *Independent re-draws, not a cached square.* `h=func(y*y :posteval)`
  reading `y` twice does not compute a square — each read re-fires
  whatever expression `y` was bound to. `h(:y int(rand(1,10)))` can
  return the product of two *different* random draws, not one draw
  squared:
  ```
  hits=list()
  draw=func(v=int(rand(1,10)); hits,v; v)
  h=func(y*y :posteval)
  r=h(:y draw())
  // y*y reads y twice, two independent draws: hits={1,2} -> r=2, not 1 or 4
  ```
  The idiom for pinning one draw instead — `ycopy=y before ycopy*ycopy`
  — is the same "assign it to a local once" pattern any repeatedly-read
  `:posteval` value uses (see `posteval.comt` test 6's comment).
- *Sibling keywords.* Because a marker pull does not swap `_alist`, one
  `:posteval` keyword's deferred expression can reference *another*
  keyword of the same call by name — `f(:x y :y 5 :posteval)`'s `x`
  resolves against `f`'s own `y`, not whatever `y` means in the caller's
  scope. That reads like keywords renaming or redistributing each other
  on the fly, entirely as a side effect of the pull mechanism, not
  anything deliberately built for it.
- *Testable before firing, via `isclass(:sym)`, not via parens.* Passing
  a bare `func(...)` in by keyword hands the callee an actual `FuncObj`
  value — behavior, not just a result — and `isclass(name :sym)` reads
  its type without firing it, the same symbol-preserving convention used
  anywhere else in the language:
  ```
  f=func(print("isclass a %v b %v\n" isclass(a :sym) isclass(b :sym));
         a==b :posteval)
  f(:a func(1) :b func(1))
  // isclass a FuncObj b FuncObj
  // true
  ```
  `a`/`b` report as `FuncObj` under `isclass(:sym)` — inspectable without
  firing — and only fire when actually used bare, in `a==b`. So a
  `:posteval` body gets exactly the "pass code in, test what it is,
  decide whether to run it" pattern the `steer` example above leans on,
  without needing any bare-vs-parens distinction at all: `:sym` is the
  look-without-firing escape hatch, bare use is the fire.
- *Streams are pinned, not re-fired.* "Every access re-fires" (above) has
  exactly one exception: if the pulled result is a **stream**, it's
  pinned in place after that first resolution instead of being re-fired
  on later reads. Without this, a caller expression that *constructs* a
  stream (a range literal, `$$list`, ...) would hand back a brand-new,
  never-exhausted stream on every single read — a `while` loop pulling
  from it could never see it end:
  ```
  // BEFORE this pin existed: infinite loop. Every *s re-fires 1..10 from
  // scratch, and a freshly-built range's first element is always non-nil.
  f=func(while(*s print("%v\n" x)) :posteval)
  f(:s 1..10 :x int(rand(0,10)))   // never terminates
  ```
  The fix: the first read that resolves to a stream writes that object
  back in place of the pending marker — the same mechanism an explicit
  write already uses to fix a keyword's value — so every later read, for
  `arg(n)` and keywords alike, finds the one real, same-identity stream
  object directly:
  ```
  g=func(a=*s; b=*s; c=*s; a,b,c :posteval)
  g(:s 1..3)   // 1,2,3 -- correctly advances, not 1,1,1
  ```
  This costs nothing for the case that was already safe: re-firing a
  caller expression that's just a bare reference to an already-built
  stream variable (`g(:s mystream)`) was always harmless, since
  re-resolving a symbol just re-fetches the same object — the pin is a
  no-op there, not a rescue. It only changes behavior for the genuinely
  dangerous case: a caller expression that mints a fresh stream on every
  evaluation.

None of this was purpose-built — it's what falls out of "a keyword is a
deferred pull against the callee's own in-progress scope, resolved as an
ordinary read on demand."

**A stream's truthiness is not an exhaustion signal — watch for this even
outside `:posteval`.** `while(arg(0) ...)` (bare, no `*`/`next()`) looks
like it should loop until the stream runs out, but a raw stream's
truthiness reflects a static mode flag, not remaining-element count — so
that condition never goes false no matter how exhausted the stream gets,
`:posteval` or not. The correct idiom always pulls explicitly and checks
against `nil`: `while((v=next(arg(0)))!=nil ...)`. This matters even more
once you're pulling elements out one at a time, because a *pulled*
element is an ordinary value with its own, unrelated truthiness — `0`,
`false`, or `""` are all perfectly valid stream contents, and confusing
"the value I just pulled happens to be falsy" with "the stream is
exhausted" silently ends a loop early on legitimate data:
```
f=func(while(v=*arg(0) print("%v\n" v)))
f(0..1)   // prints nothing -- the first element, 0, is falsy, so the
          // loop exits immediately; it never reaches "exhausted"
f(1..2)   // 1  2  -- works, because neither element happens to be falsy
```
`nil` is the only signal `next()`/`*` actually use for exhaustion; nothing
else pulled from a stream should ever be treated as one, no matter how
falsy it looks.

Follow the `isclass(:sym)` case one step further and a pattern falls
out: a bare variable, or a `:posteval`-pulled keyword, both treat *any*
read as a request to fire — the only place a `FuncObj` sits as pure,
inert data, inspectable without an implicit fire, is an attrlist. Dot
access into one still needs the explicit `()` to run it (same contract
as everywhere else in the language). So passing a `FuncObj` around as an
actual *object* — data now, behavior later, on request — routes through
the same attrlist substrate comterp already uses for objects generally,
not through some func-specific mechanism.

### Escaping the func scope: local() and global()

When a func genuinely needs to write outside its own frame, two
commands name the outer scopes explicitly, as both lvalue and rvalue:

- **`local(x)`** — the interpreter's default variable table: the scope
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
bump=func(local(count)=local(count)+1)   // reads outer count, writes it back
bump(); bump()
count                  // 2 -- the func published through local()

f=func(count=99; count,local(count))
f()                    // {99,2} -- frame shadow vs explicit outer read
```

Note the RHS is `local(count)`, not bare `count` — `local()` on the lvalue
side alone does not exempt a bare rvalue mention of the same name from
declaration-time capture (see *Closures* above): a bare, never-locally-
written `count` here would still count as a genuine free-variable read,
captured once when `bump` is declared, so every call would recompute
`0+1` instead of reading the live outer value. Wrapping both sides in
`local()` keeps the read genuinely live on every call.

In a single-interpreter session `local()` and `global()` differ only in
which table they touch; in a multi-session server they are session
scope versus process scope — publish per-connection state with
`local()`, cross-connection state with `global()`.

Note that the backquote is **not** a scope escape: `` `x `` is the
symbol-quote (the symbol itself, lookup suppressed), and assigning
through it stays in the current scope like any other write.  Use
`local()`/`global()` to escape; use `return` (possibly of an attrlist)
to hand results to the caller.

`local(x)`/`global(x)`'s deeper job, as an lvalue, is to designate `x`
as a symbol instance scoped specifically to that table — internally
they hand back a raw, backquoted symbol reference, not a value:
evaluation stops at the symbol rather than collapsing it, exactly so
`x=val`'s assignment machinery has an identity to write through rather
than a value to overwrite (a symbol is a symbol until something
actually asks for its value). This only matters if `x`'s name collides
with a registered command: a *bare* command reference self-invokes
during ordinary argument evaluation before `local()`/`global()` ever
see it — harmless for a niladic constant, not so for one with side
effects — so both refuse outright, bare or backquoted, the same
"assignment to command ... not allowed" way bare `x=val` already does.
There is no way to shadow a command through `local()`/`global()`; a
colliding name simply isn't usable as a variable there, matching the
rule everywhere else in the language.

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
syntax completes the source end of the algebra; ongoing work continues
to clarify the composition laws, particularly around nil propagation
through composed operations and zip semantics between lazy and
materialized sources.

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

**Overdrive results are themselves lazy** — the "producing a stream of
results" above is the operative word: nothing about *any* of the N
repeated calls happens at construction time, only when that result's
position in the resulting stream is actually pulled. For a pure operator
like `*`/`+` this is invisible (no side effect to notice either way), but
it's directly observable for a command with a side effect, like `print()`:

```
r=print("%v " 0..2)   // nothing prints yet -- r is a lazy 3-element stream
next(r)                // *now* "0 " prints -- the first call fires here
next(r)                // "1 " prints
next(r)                // "2 " prints
```

`print()` is itself non-post-eval (`postfix(help(print))` shows
`print[0|0|1]`, no trailing `*` — see *Overdrive rules* below), so a
stream argument overdrives it internally exactly like `*`/`+` do; the
laziness is a property of overdrive in general, `print()` just happens to
be the case where it's visible. This matters directly for *Auto-draining
an orphaned result* below: draining such a stream isn't free of side
effects the way draining a plain data stream is.

### Auto-draining an orphaned result

The final result of a stand-alone expression -- never assigned to
anything, never streamed further -- used to just vanish if it happened to
be a stream: printed as an uninformative `[]` (a lazy, unconsumed stream
shown as if it were empty), or in a multi-line script, silently discarded
without even that. Every one of those instances had already done real,
possibly side-effecting work (*Overdrive results are themselves lazy*,
above) that then went nowhere.

```
0..100                  // used to just disappear -- built and dropped
$(1,2,3)                 // same
```

Now, whenever a stream is about to be discarded and nothing else
references it, it's drained instead and its element count shown:

```
0..100                  // 101
$(1,2,3)                 // 3
```

This applies everywhere a value can be discarded, not just the last line
of a script: every freestanding statement in a multi-line `.comt` script
(each line is its own read-eval step), every `;`-joined statement, and
the interactive prompt. A stream still bound to a variable is never
touched -- draining checks whether anything else still references the
same underlying stream buffer before doing anything, so `x=$(1,2,3)` at a
prompt (or as a non-final script statement) leaves `x` fully intact for
later use, whether or not the surrounding expression that produced it is
itself discarded.

The same guard covers a bare *reference* to an already-bound stream, not
just the assignment that creates it. This is deliberately unlike a bound
`func`, where a bare reference always fires it -- looking up a
func-valued symbol *is* how you invoke it in comterp, and it fires
anywhere the symbol resolves, not just at the top level (`f=func(42);
x=f+1` gives `43`). A stream can't work that way: resolving a symbol has
to stay lazy everywhere -- including a bare top-level reference -- or the
streaming discipline above (never mid-expression) would be violated by
the plainest possible case, just naming the variable. (Dot-bound access
is a third case again: `al.m` retrieves the `FuncObj` value without
calling it -- only `al.m()`, with explicit call syntax, invokes a
method.)

```
s=run("some-script-with-a-freestanding-stream.comt")   // []
s                                                        // []  -- not drained; s is still bound
each(s)                                                  // 101 -- explicit consumption still works
```

`s` alone still prints `[]` rather than a count -- the value on top of
the stack there is the *same* stream object the variable is bound to, not
an independent copy, so draining it would silently exhaust `s` the
moment you typed its name to look at it. `each()`/`next()`/an overdrive
op (`s**2`, and so on) still consume it explicitly, and `$$s` makes an
independent copy to drain, leaving `s` itself untouched:

```
s=$$(1,2,3)
$$s                     // 3  -- a fresh, orphaned copy: auto-drains
next(s)                 // 1  -- s was never touched
```

**Why this took decades to build.** ivtools' streams have held one strict
discipline since they were first designed: never let streaming happen
prematurely, i.e. never mid-expression -- a stream stays lazy and
uninitiated until something at the top genuinely needs it. That
discipline is exactly why this feature waited: it only became safe once
there was a place to drain a stream that is unambiguously *not*
mid-expression -- the literal top of the stack, fully resolved, with
nothing left to do but discard it or show it. Before that boundary
existed, the only correct choice was to leave an orphaned stream alone
and accept the waste; the alternative would have meant streaming
somewhere inside evaluation, which the discipline never allowed.

**Getting the order right mattered.** Because overdrive results are lazy
(above), draining one isn't a side-effect-free operation the way draining
a plain data stream is -- for a stream built by overdriving `print()`,
draining fires the deferred `print()` calls right then. That makes the
*order* auto-draining happens in observable, not just an implementation
detail: each freestanding statement's leftover result has to be drained
before the *next* statement runs, not after, or a deferred side effect
from one statement shows up interleaved into the following statement's
own output.

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

**FuncObj invocations** follow the non-post-eval rule. A func called with
a stream argument is overdriven the same way a command is: the stream
drives the *invocation*, firing the body once per element with `arg(n)`
bound to a scalar. The body is then ordinary scalar code -- a `while` or
`if` inside it sees scalars, never the stream -- so the same func serves
both uses unchanged:

```
gcd=func(a=arg(0); b=arg(1); while(b!=0 t=b; b=a%b; a=t); a)

gcd(48 18)                                   // 6
list(gcd((48 1071 17 270) (18 462 5 192)))   // {6,21,1,6} -- one firing per pair
```

`func(:posteval)` is the exception, and deliberately so: its contract is
the opposite one. Its arguments stay unevaluated, and the body itself is
the drain, pulling elements with `*arg(n)`. A `:posteval` func is never
overdriven -- it fires once, and sees the stream rather than an element
of it.

In short: streams overdrive *into* non-post-eval commands and ordinary
func invocations through their arguments, and *around* post-eval commands
through external combination or return values -- never *through* a
post-eval command's own arguments, and never into a `:posteval` func,
which drains its stream from the inside instead.

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

### Growable FIFO streams: `feed()`

`feed()` is the write-end complement to `next()` — where every other
stream in ComTerp is built once (as a literal, a range, a file handle)
and only ever drained, `feed()` builds a stream that can keep being
written to after it exists.

```comterp
fifo=feed()          // empty FIFO
feed(fifo 1)          // append
feed(fifo 2 3)         // append more, in one call
next(fifo)             // 1
next(fifo)             // 2
```

`feed(val ...)` alone builds a fresh FIFO from its arguments (`feed(1 2
3)` is equivalent to `feed(); feed(fifo 1 2 3)`); `feed(fifo val ...)`
appends to an existing one, recognized by checking whether the first
argument is already a `feed()`-built FIFO. Values drain in the order
they were fed — first in, first out — regardless of how many separate
`feed()` calls contributed them.

`nil` comes up twice below, in two unrelated senses that behave
oppositely — worth naming both up front so neither reads as a
contradiction of the other:

- an **input nil** — a literal `nil` given to `feed()` as one of the
  values to store, sitting in the queue like any other element until
  `next()` reaches it.
- an **output nil** — what `next()` returns when there is nothing
  queued to hand back. Nothing was stored; it's just the "empty right
  now" answer.

An input nil is destructive (next). An output nil is not (*A FIFO's
output nil is not permanent*, further down).

#### An input nil is destructive

A literal `nil` fed in as data is destructive: once `next()` reaches it,
everything queued *after* it in that FIFO is discarded.

```comterp
f=feed(1 2 3 nil 4 5 6)
list(f)      // {1,2,3} -- 4,5,6 are gone, not just skipped
```

This is inherited, not a bug specific to `feed()`: when `next()` reaches
a stored input nil, it hands that nil back to the caller exactly like
any other stored value — and an output nil is ComTerp's one universal
"stream exhausted" signal (see *The stream contract* above), which every
stream-consuming command already treats as authoritative. `feed()`
cannot let an input nil surface as a non-authoritative output nil
without breaking that uniformity for everything downstream of it — so it
discards the rest of the queue instead of leaving it silently
unreachable.

#### A FIFO's output nil is not permanent

This is the other nil: nothing was fed in here, `next()` is simply
reporting that the FIFO is currently empty. The stream contract above
says an exhausted stream "yields `nil` from `next()` and stays
exhausted." A `feed()`-built FIFO does not honor that literally:
`next()` on an empty FIFO returns an output nil, but the FIFO is not
thereby *done* — `feed()` can add more to it afterward, and the next
`next()` call returns real data again.

```comterp
f=feed()
next(f)      // nil -- nothing queued yet (an output nil, not stored data)
feed(f 42)
next(f)      // 42 -- the earlier nil was not the last word
```

This is deliberate. A FIFO's output nil means "nothing available *right
now*," not "nothing will ever be available again" — those are different
facts, and a single `next()` call cannot distinguish them. The
alternative — a "closed" flag that `next()` itself consults before
deciding what an output nil means — was considered and set aside: it
would make `next()`'s meaning depend on which kind of stream it's called
on, and every existing consumer (`each()`, `list()`, `for`) would need
to know the difference. Leaving the output nil uniformly non-committal
keeps the contract simple everywhere else; telling a FIFO's "empty for
now" apart from "permanently done" is left to a still-open design
question (see *Still planned* below), not folded into `next()`'s return
value.

#### `` `EOS `` — a delimiter *within* a FIFO

Marking a boundary *inside* an ongoing FIFO — "this batch is over, more
may follow" — can't be built from either nil above: an output nil isn't
a stored value, so there's nothing to place at a specific spot in the
queue; and deliberately storing an input nil as that marker would
trigger the destructive behavior just described, discarding everything
meant to follow it. `feed()` streams use a separate convention instead:
a bquoted symbol, `` `EOS ``, checked by *identity*, not by resolving it
as a variable.

```comterp
f=feed(1 2 3 `EOS 4 5 6 `EOS)
list(f)     // {1,2,3}
list(f)     // {4,5,6}
```

`each()` and `list()` recognize `` `EOS `` and stop a single drain there,
consuming the delimiter but leaving the rest of the FIFO intact for the
next drain — unlike `nil`-as-data, nothing after the delimiter is lost.
`next()` itself does not honor the convention at all; it walks straight
through `` `EOS `` as an ordinary queued value, all the way to genuine
exhaustion. The delimiter has meaning to a *draining* command, not to
the FIFO's own read primitive:

```comterp
f=feed(1 `EOS 2)
next(f)          // 1
v=next(f)         // the `EOS symbol itself -- next() does not stop here
v!=nil            // true -- it survived intact, not resolved away
next(f)           // 2
```

(`v` here genuinely holds the `` `EOS `` symbol, not `nil` — but printing
it directly with `print("%v" v)` shows `nil` regardless, a display quirk
in how a plain argument fetch resolves bquoted symbols, unrelated to
`next()` or to FIFOs. The `!=nil` comparison above is unaffected and
shows the real value.)

The identity check matters: `` `EOS `` is recognized because it is a
bquoted symbol whose symbol id matches a reserved name, not because
resolving it as a variable happens to fail — an ordinary, unbquoted
symbol that resolves to nothing would otherwise look like a stopping
point too. The delimiter is deliberately independent of whatever `EOS`
may or may not be bound to.

### Lazy ingestion of nested streams

A stream-valued argument to `feed()` is drained lazily, one value per
`next()`, rather than stored as a single opaque, undrained element:

```comterp
fifo=feed(0..2)
list(fifo)      // {0,1,2} -- not a single StreamType element
```

This reuses the nested-stream mechanism that already exists for results
like `(1 2)*(3 4)`'s overdrive — a stream whose drained value is itself a
stream gets driven down to flat values rather than handed back as-is.
`feed()` tags a stream-valued argument with that same mechanism instead
of storing it plain.

Feeding one FIFO into another therefore concatenates them, lazily, with
no special case needed:

```comterp
a=feed(1 2 3)
b=feed(7 8 9)
feed(a b)
list(a)     // {1,2,3,7,8,9}
```

Because `feed()`-built FIFOs share their underlying storage the way any
stream does, draining `a` after `feed(a b)` also drains `b` as a side
effect — `b`'s contents are handed over to `a`, not copied. A separate
reference to `b` held elsewhere would find it emptied out from under it.
This matches how every other stream command already behaves (`next()`,
`list()`, and `each()` are all destructive); feeding a stream's contents
into a FIFO is not an exception.

Nesting stops at one level: a stream nested *inside* a fed-in stream is
left alone.

```comterp
fifo=feed(((1 2)(3 4)))
list(fifo)      // two raw StreamType elements, not {1,2,3,4}
```

This matches an existing, general ComTerp preference: deep, arbitrary
auto-flattening has only ever proven right for graphics data structures,
never for signal/image/video-style streams, where one level of
unwrapping is exactly what's wanted and further levels are requested
deliberately rather than happening automatically.

#### `feed(f f)` — feeding a FIFO into itself

Sharing storage (above) has one hazard: a FIFO cannot lazily ingest
*itself* the way it ingests another FIFO. Doing so would make its own
backing list hold an element that points back to that same list — a
cycle that recurses without end the instant anything walks it (draining
it, but just as easily an unrelated future copy or print), crashing the
interpreter rather than looping in ComTerp. `feed()` detects a fed-in
stream whose backing list *is* the destination FIFO's own list and, for
that argument only, appends a snapshot of the FIFO's current contents
instead of the live, shared list — the same copy `$$`/`stream()` makes
(*Forking a FIFO*, below). Self-feeding therefore duplicates what is
queued *right now*, rather than crashing or growing forever:

```comterp
f=feed(1 2 3)
feed(f f)
list(f)     // {1,2,3,1,2,3} -- a snapshot of {1,2,3}, appended once
list(f)     // {} -- drained normally, no cycle left behind
```

This guard only catches the direct case — a FIFO fed into itself, in
either argument position. A longer cycle built up across several `feed()`
calls between two or more FIFOs (`feed(a b); feed(b a)`) is not
detected and remains a hazard; only the immediate self-reference that a
single `feed()` call can construct is guarded against.

### Forking a FIFO

`$$fifo` / `stream(fifo)` produce an independent copy, as for any
stream — but the copy is a snapshot of the FIFO's contents *at the
moment of the fork*, not a shared, position-tracked view of an ongoing
buffer the way file and pipe streams are documented above. Values fed to
the original after the fork are not visible through the fork, and values
already in the fork are unaffected by further draining of the original.

```comterp
a=feed(1 2 3)
b=$$a
feed(a 4)
list(a)     // {1,2,3,4}
list(b)     // {1,2,3} -- the 4 fed after the fork isn't here
```

### Still planned

A FIFO has no way today to be told "no more will ever be fed to me" —
the output nil `next()` returns stays ambiguous between "empty for now"
and "actually done" for the FIFO's whole lifetime (see above). The planned resolution
is a `:state` field on every stream, not just FIFOs, living in a
reserved slot rather than overloading `next()`'s return value:
`` `Opening ``, `` `Open ``, `` `Closing ``, `` `Closed `` — the first and
third are optional, for stream types with a genuine transitional phase
to report (a socket mid-handshake, one draining a backlog after being
told to stop); a plain `feed()`-built FIFO only ever needs `` `Open ``/
`` `Closed ``. `info()` would surface it as a new field, the same way it
already reports a stream's backing function and element count — no
internal mechanism should ever need to construct an `info()` attrlist
just to make its own decisions, so the field lives somewhere directly
and cheaply readable in C++, and `info()` stays one way of looking at
it, not the source of truth.

The motivating use case is non-blocking socket I/O — a handler already
does non-blocking, byte-at-a-time reads for other purposes; the plan is
a mode that instead does `feed(targetfifo, line)`, letting a script
consume socket data through the same `next()`/FIFO vocabulary it already
uses for everything else, with `:state` distinguishing "no data yet"
from "peer disconnected," which a bare output nil cannot do. The
`next(:nowait)` peek and `update()`-driven reactor-polling idiom below
remain the intended consumption pattern once this lands.

#### next(:nowait) — non-exhausting peek

Plain `next()` already leaves a FIFO's output nil non-committal (see
above) -- `next(:nowait)` is the planned way to make that explicit at
the call site, using a three-way distinction:

- value ready → returns the value
- `` `Open ``/`` `Closing `` and empty → returns `blank`
- `` `Closed `` and exhausted → returns `nil`

This preserves the output nil as the exclusive *permanent* exhaustion
sentinel and avoids the race between "stream is done" and "value hasn't
arrived yet."
The consumer pattern in reactor mode:

```comterp
while(1
  v=next(strm :nowait);
  if(v==nil :then break());
  if(type(v)!=`BlankType :then process(v));
  update())     // yield to ACE reactor
```

`update()` yields to the ACE event loop between polls, so `feed()` calls
made from a callback or remote connection can arrive without blocking
the REPL. This is the same pattern used by `remote(:nowait)` and the
`drawmo` orchestrator.

FIFOs fed from a socket handler work naturally in `comterp listen` / ACE
reactor mode. In the plain REPL, `:nowait` with `update()` polling is the
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
name, stored once in a global symbol table (not to be confused with the
global variable table). Symbols are the basis of variable names, command
names, keywords, and type names in ComTerp.

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

`class(:all)` lists every class the running program linked, sorted by name;
`class(:comps)` narrows that to the component classes:

```
class(:all)            // in comterp: Attribute,AttributeList,DateObj,...,SocketObj
class(:comps)          // in comterp: empty -- it links no component classes
```

What enrolls is a class whose instances are *values* -- things that can sit on
the stack and be handed to a command.  A command is not one of those, so the
command classes do not enroll; nor does the marker the `:posteval` machinery
puts where an argument it has not evaluated yet will go, which is scaffolding
the interpreter substitutes rather than anything a script is given.  Both still
carry a class symbol where the C++ needs one to recognize them -- they simply
plant no registrar.

`Attribute` is listed even though `class(al.foo)` answers nil, and the two facts
do not contradict: the dotted pair really is a value that can be passed along
(`attrname(al.foo)` reads it), but `class()` is one of the many commands that
dereference through `stack_arg()` before looking, so the key is gone by the time
it answers.  See the dotted-pair discussion under attribute lists below.

Under comdraw the same two calls answer differently, because a different set
of classes got linked:

```
class(:comps)          // ArrowLineComp,ArrowMultiLineComp,...,TextComp,VerticesComp
size(class(:comps))    // 19
```

Nothing has to be drawn first. Each class enrolls itself before the program
starts, so the list is what this binary *can* work with, not what the session
has happened to touch -- which is what makes it usable for walking the
component types:

```
for(i=0 i<size(class(:comps)) i=i+1 print("%v\n" at(class(:comps) i)))
```

That is a weaker guarantee than `type(:all)`, and deliberately so: the type
symbols are a closed set the language defines, while a class only exists to be
listed if something linked it. `class(:all)` in drawserv includes
`DrawLinkComp`; in comterp it does not.

`type(:all)` returns the whole set of type symbols, in enum order:

```
type(:all)             // UnknownType,CharType,UCharType,ShortType,UShortType,
                       // IntType,UIntType,LongType,ULongType,FloatType,
                       // DoubleType,StringType,SymbolType,ListType,StreamType,
                       // CommandType,KeywordType,ObjectType,EofType,
                       // BooleanType,OperatorType,BlankType
size(type(:all))       // 22
at(type(:all) 5)==type(1)  // true -- the listing and the per-value answer agree
```

That list is complete: every value in the language carries one of those 22
types, and `ArrayType` is absent because it and `ListType` are one type under
two names, registered as `ListType`.

Asked with no value at all, both commands answer `blank` rather than `nil` --
`nil` is the answer *about* a value, so it needs a value to be about:

```
class(3)               // nil   -- a value was named; it has no class
class()                // blank -- no value was named at all
type()                 // blank -- likewise
type()==nil            // false -- the two stay distinguishable
type()==blank()        // true
```

### istype()/isclass()/iscomm()/isfunc() — inspecting a variable without firing it

`type()` and `class()`, above, both evaluate their argument the ordinary
way before looking at it — which means they cannot answer *"what is this
variable bound to"* for a command name or a `func()`-bound name, because
referencing either one fires it first:

```
pi                 // 3.14159 -- bare reference fires the command
myfunc=func(1+1)
myfunc              // 2 -- bare reference fires the func too
type(pi)            // DoubleType -- the type of the fired result, not of pi itself
```

This isn't just risky when the thing being checked has side effects — it's
uninformative even when it doesn't:

```
f=5
g=func(5)
type(f)             // IntType
type(g)             // IntType -- identical; g's func-ness is invisible here
```

`istype()`, `isclass()`, `iscomm()`, and `isfunc()` are `post_eval` — they
read their first argument as a raw, unevaluated token (the same mechanism
`help()` already uses to describe a command without running it) and peek
at what it resolves to with a single non-invoking table lookup, rather
than evaluating it:

```
iscomm(pi)                 // true  -- no firing
istype(pi CommandType)     // true
istype(pi `CommandType)    // true -- a redundant backquote on the type
                            //         name is harmless either way
```

`isfunc(var)` and `iscomm(var)` are fixed single-argument shortcuts for
`isclass(var FuncObj)` and `istype(var CommandType)` — the two questions
this whole feature exists to answer. With one argument, `istype()`/
`isclass()` partition every value into exactly one of two buckets:

```
x=99
istype(x)          // true  -- a plain/regular value type
isclass(x)          // false -- nothing to ask a class of

f=func(1+1)
istype(f)           // false -- f is an object (a FuncObj), not "plain"
isclass(f)           // true  -- and an object has a class
```

The motivating case — a func that returns another func — is exactly what
this makes checkable for the first time:

```
outer=func(y=42; func(y))
escaped=outer()      // outer() already ran; escaped holds a live FuncObj
isfunc(escaped)       // true -- confirmed without ever calling escaped
```

A standalone variable is a niladic call site everywhere — including inside
a func's own body. There is no separate case to reason about for "inside
vs. outside a func": a func-local reference to a `FuncObj`-bound name fires
exactly like a top-level one does. So the construction above only works
because `func(y)` — the construction itself — is the last thing outer's
body evaluates. Naming it first doesn't help:

```
outer=func(y=42; inner=func(y); inner)   // inner fires here, same as z=inner would
outer()                                   // NOT a FuncObj -- it's inner's own result
```

The only way to hand a `FuncObj` back intact is for its construction to be
the literal last expression evaluated — never a bare reference to a name
already holding one, anywhere, including a func's own return position.

**Caveat: this is a syntactic-shape peek, not a "what would this evaluate
to" test.** A compound expression's raw, unevaluated form is a command
call like any other — `4*3` is a call to `mpy`, and `4..7` is a call to
`iterate`, indistinguishable in kind at this level:

```
istype(4*3 CommandType)    // true
istype(4..7 CommandType)   // true  -- also just a command call
istype(4..7 StreamType)    // false -- the stream only exists in iterate's
                            //          *result*, once it actually runs
```

For a bare name, the peek tells you exactly what you want — is this
callable, without calling it. For a compound expression, it only tells
you the outermost command being invoked, never what that command would
produce.

**`:sym` — the symbol itself, not just a flag.** All four take a `:sym`
keyword: instead of a boolean, return the resolved type/class symbol
directly (`nil` if there isn't one) — the same non-firing peek, reported
the way `type()`/`class()` report it for a value that's already been
fired:

```
x=99
istype(x :sym)      // `IntType -- same symbol type(x) would give
isclass(x :sym)      // nil -- x isn't of ObjectType at all

f=func(1+1)
isfunc(f :sym)        // `FuncObj -- no firing, same guarantee as isfunc(f)
```

On `istype()`/`isclass()`, `:sym` wins over a two-arg comparison target
if both are given — it always reports what the variable actually is,
not whether it matches something else:

```
istype(pi IntType :sym)   // `CommandType -- pi's own type, IntType ignored
```

`iscomm()`/`isfunc()` have no comparison target to begin with (they're
fixed shortcuts), so `:sym` there is a symbol-or-`nil` version of the
same flag rather than new information — `` `CommandType``/`` `FuncObj``
on a match, `nil` otherwise.

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

`at(attrlist n)` (a bare read, no `:set`) returns the nth attribute as a
**detached, single-entry attrlist** — e.g. `(:y 2)` for `at(al 1)` above
— not a live handle into `al`. `attrname()` and `attrval()` accept this
shape directly, reading its one entry (they also still accept the older
"dotted pair" `Attribute` shape `.` produces for named lookup — see
below — either works as their argument). `at(al n :set val)` still
writes through unrestricted, exactly as before — only assigning directly
to a bare read's result is blocked: `al@n=val` (the `@` operator is pure
sugar for a bare `at()` call) can never write through to `al`, since
there's no live handle in a detached copy to write through in the first
place.

`type(at(al n))` is `ObjectType` and `class(at(al n))` is `AttributeList`
— not the enclosed value's own type/class, since what's returned is a
whole (if tiny) attrlist, not the value itself. Enumeration order matches
insertion order — the order keys were first written (in a literal) or
first added (via `al.key=val`) — for both construction paths.

A named lookup via `.` (`al.foo`) instead hands back the older "dotted
pair" `Attribute` object — a lower-level, internal representation with no
literal syntax of its own in the language (the same way a bare keyword
has none; both only ever exist as part of an attrlist). `Attribute`
objects can live on the stack and be passed to any command, but whether
the key survives depends on whether the receiving command explicitly
checks for it before dereferencing — `attrname()`/`attrval()` do; every
other built-in command dereferences immediately via `stack_arg()`, losing
the key. A custom `ComFunc` could preserve it by inspecting the `ComValue`
type before calling `stack_arg()`.

Assigning a dotted pair to a variable doesn't preserve its shape either —
`x=a.foo` stores the bare, already-dereferenced value in `x` (`AssignFunc`
unwraps any `Attribute`-shaped rhs at assignment time), so `attrname(x)`
afterward fails; call it inline instead (`attrname(a.foo)`). The
single-entry-attrlist shape `at()`/`@` return doesn't have this problem —
assignment doesn't touch it, `attrname()`/`attrval()` resolve a bound
variable before checking its shape, and it works either way:

```
x=al.foo        // x is 42 (bare value) -- attrname(x) fails
z=al@0          // z is (:foo 42) (still a real attrlist) -- attrname(z) works
```

### Stream enumeration of an attrlist

`attrname()` and `attrval()` also accept a stream of attributes
directly, returning a stream of keys or values respectively. An attrlist
used as a stream source yields its entries as `Attribute` objects — the
older dotted-pair shape (see above), not the single-entry-attrlist shape
`at()`/`@` return:

```
$list(attrname($$(:a 4 :b 7)))   // {"a","b"}
$list(attrval($$(:a 4 :b 7)))    // {4,7}
```

The two streams are consistent with each other — the nth name corresponds
to the nth value — so they can be zipped or processed in parallel. Order
matches insertion order, same as the `for`/`at()`/`size()` loop above. If
you need both key and value together, use that loop form instead.

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
it passes anywhere as a first-class value. It's also exactly what
`at(al n)`/`al@n` hand back for a multi-key attrlist (above), so the two
ideas are really the same shape at different scales: one built explicitly,
one produced automatically by positional access.

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

(Implementation: the symbol *is* the interned string; the intern table, aka symbol
table, holds both forms. The split is one of rarity and use, recovered at the
surface by which quote the author reaches for.)

## See Also

- `INTRODUCTION.md` — project overview and history
- `APPENDIX-B-COMTERP-EXAMPLES.md` -- how to learn comterp command language

- `src/comterp_/tests/TESTING.md` -- comterp self regression tests
- `src/ComTerp/ARCHITECTURE.md` -- ComTerp C++ design
- `src/ComTerp/HACKING.md` -- comterp programming
