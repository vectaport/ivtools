# ComTerp Language Guide

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

The body is the first positional argument. Call with keyword args:

```
f=func(if(x>5 :then return(x*2)))
v=f(:x 6)          // returns 12
v=f(:x 3)          // returns nil (no early return taken)
```

Function body assignments are local — variables assigned inside a func
do not escape to the caller's scope. This includes dot-notation
attributes: a dot namespace rooted at a local symbol is local to the
call.

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

The streaming algebra:

```
s=$$(1,2,3,4,5)    // create stream from list
l=$s                // collect stream into list
s1,,s2              // concatenate two streams
next(s)             // pull next value, nil when exhausted
each(s)             // traverse stream, return count
1..5                // range stream: 1,2,3,4,5
3**5                // repeat stream: 3,3,3,3,3
```

Round-trip: `$($$(1,2,3))` returns `(1,2,3)`.

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

The `:sym` keyword on comparison operators (`eq`, `lt`, `gt`, etc.) is
intended for symbol comparison but has a known bug: `eq(:sym)` returns
false for symbols returned by `symbol()` even when `==` returns true.
Use `==` instead until this is resolved.

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

## Flowtran: Flowgraph Composition Language *(reserved, ivtools-4.0)*

ComTerp is designed to evolve into a flowgraph composition language —
**flowtran** — that can drive the
[ipl simulator](https://ipl.sourceforge.net/ipl/IPL_Home.html) directly
(when linked) and export executable
[vectaport/flowgraph](https://github.com/vectaport/flowgraph) goroutines.

The flowtran syntax is reserved in the current parser. This section
documents the intended design.

### Concepts

A flowgraph is made up of **hubs** and **pipes**. A hub is a unit of
computation; pipes are the typed channels connecting hubs. This maps
directly to the `vectaport/flowgraph` Hub and Stream model and to the
ipl (pipe|invo),connector model.  What was called first pipe, then
invocation in ipl, became node and edge in vectaport/fgbase then Hub
and Stream in vectaport/flowgraph.  In ipl it was realized that the
nodes between edges were the place of buffering, so they were the
pipe holding value.  You would think the edges/Stream should be the
pipe but buffering is done at the node/Hub and edge/Stream is just
the instantaneous interconnect, the wire.

ComTerp streams (`..`, `**`, `$$`, `,,`) are a separate concept —
they are evaluation-time value sequences within ComTerp expressions.
Flowtran pipes are persistent communication channels between hubs in
a running flowgraph. The two levels interact: ComTerp stream algebra
is used to *construct* flowgraphs (replicating hubs and wiring pipes),
while the flowgraph itself executes asynchronously once assembled.

### Hub definition: `def()`

```
def(hubname[srclist][dstlist](hubbody))
```

Defines a hub type. `srclist` is the list of input pipes; `dstlist` is
the list of output pipes. `hubbody` is a ComTerp expression that runs
when the hub fires. A hub with no destinations has an empty `dstlist`.

By convention, input pipes are named `a`, `b`, `c` and output pipes
`x`, `y`, `z` in hub definitions — the Steve Johnson compiler tradition.
At invocation sites, pipes are given names that make sense from both
ends of the connection.

Def's with no body must link to a language primitive and are just there to
declare the API.   Otherwise def's can be a comterp expression that is a
FuncObj to be fired when AllOf or OneOf is satisfied on inputs arriving,
the result getting marshalled out on the destination list.  Or the body can
be an ongoing nested declaration of a flowgraph connected up to the streams
in the srclist and dstlist.  The primitives are flowtran's bottoming out,
the comterp expressions are the comterp programmer's way of bottoming out,
everything above that is a flowgraph.

Any side effects from an embedded comterp fragment would not fit the model 
so they will all be pre-evaluated down to the code fragment waiting for stream
input.  For example:

```
def(area[r][a](a=pi()*r*r))
```

will get evaluated at "compile" time, and symbols not in the stream lists will
be evaluated and stored in an internalized representation, i.e.:

```
area[r][a](a=3.141519*r*r)
```

A less useful but more illustrative example might be a hub that returned the
date it was compiled forever:

```
datecompiled[][d](d=print("%v" date :str))
```

which becomes:

```
datecompiled[][d](d=" 6-Jun-2026")
```

The two canonical Invocation Language primitives illustrate the full syntax:

```
def(ARBIT[a | b][x](x=a||b))        // OneOf input, single output
def(STRVAL[a b][x | y](if(a :then x=b :else y=b)))  // AllOf input, OneOf output
```

The `|` operator separates **firing groups** within a srclist or
dstlist. Space separates pipes within a group (AllOf — all must be
present). `|` separates groups (OneOf — any one group fires the hub).
The rule "no bare values after keywords begin" applies within each
group, as elsewhere in ComTerp.

```
def(sink[a][](print(a)))              // AllOf-1 input, no output
def(double[a][x](x=a*2))             // AllOf-1 in, AllOf-1 out
def(add[a b][x](x=a+b))              // AllOf-2 in, AllOf-1 out
def(merge[a | b][x](x=a||b))         // OneOf-2 in (either fires), AllOf-1 out
def(route[a b][x | y](if(a :then x=b :else y=b)))  // AllOf-2 in, OneOf-2 out
```

`|` between two sublists means the hub fires on whichever group
arrives — useful for multiplexing normal data flow with a heartbeat
or control path:

```
def(watchdog[data | heartbeat][x](x=data||heartbeat))
// fires on normal data OR on the watchdog pulse, whichever arrives
```

Pipes in `srclist` and `dstlist` can include keywords for configuration
values that are set at wiring time rather than flowing per-token:

```
def(scale[a :factor][x](x=a*factor))
```

### Hub invocation: `hubname[srclist][dstlist]`

```
hubname[srclist][dstlist]
```

Instantiates a hub and wires it into the flowgraph. The hub name leads
— you read the topology as *what* fires, then what flows in, then what
flows out. A stream symbol is created the first time it appears in any
srclist or dstlist and updated as both ends become connected.

The two NCL primitives invoked in a small flowgraph:

```
ARBIT[raw_a | raw_b][arbitrated]
STRVAL[direction arbitrated][to_x | to_y]
```

`arbitrated` is the stream connecting `ARBIT`'s output to `STRVAL`'s
input — named from the perspective of what it carries. `direction` is
a control input; `to_x` and `to_y` are the two steered outputs.
Reading top to bottom: arbitrate between two raw inputs, then steer
the result based on a direction signal.

A simple pipeline:

```
double[raw][scaled]
sink[scaled][]
```

`scaled` is created as `double`'s output and simultaneously wired as
`sink`'s input. Pipes are fully connected when both ends are bound.

### Range replication: `<<` and `>>`

`<<` and `>>` are added to the operator table as flowtran range
operators, used to replicate hub names and pipe names across a range.
Combined with ComTerp's `..` stream, they expand to indexed variants:

```
sink<<0..9>>=sink[input<<0..9>>][]
```

Expands to `sink0` through `sink9`, each wired to `input0` through
`input9`. The ten input pipes are left unconnected on their source
end — other hubs will wire into them. This is the `setbuf` pattern:
declare the pipes where they are consumed, let the producer side wire
to them later.

`<<` and `>>` apply to any identifier in a hub or pipe position:

```
add<<0..3>>[a<<0..3>> b<<0..3>>][sum<<0..3>>]
```

Creates four `add` hubs, each with its own `a`, `b`, and `sum` pipes.

### Relationship to existing reserved syntax

The following are already reserved in the current parser and will
be activated for flowtran:

- `[]` — hub srclist and dstlist delimiters
- `<>` — reserved (angle bracket template fill-in, complementing `<<`/`>>`)
- `<<` / `>>` — added to the operator table as flowtran range operators

It is hoped that not using `[` and ` ]` and `<<` and `>>` before now
in the public use of comterp will make any conflicts between comterp and
flowtran resolvable.

### Target backends

- **ipl simulator** — when ComTerp is linked with the ipl library,
  `def()` and hub invocation compile to IPL node/arc creation calls
  directly.
- **vectaport/flowgraph** — hub definitions export to Go source
  implementing the `flowgraph.Hub` interface, with pipes as
  `flowgraph.Edge` channels. The exported package is runnable as
  a standalone Go program.
