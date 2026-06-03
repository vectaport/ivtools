# ComTerp Language Guide

ComTerp is an embedded scripting language with a compiler pipeline
(scanner → parser → code_conversion → interpreter). Each top-level
expression is scanned, parsed, and converted to a flat postfix token
stream, then interpreted iteratively. Nesting depth in the original
expression does not affect the interpreter's call stack — everything
is a token stream and an operand stack.

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
| float | `3.14` | |
| double | `3.14` | |
| string | `"hello"` | double-quoted |
| symbol | `` `foo `` | backquote suppresses lookup |
| char | `'a'` | single-quoted, format with `%c` |
| boolean | `true` `false` | |
| nil | `nil` | no value |
| blank | `BlankType` | return of `return()` with no arg |
| list | `(1,2,3)` | comma operator |
| stream | `$$(1,2,3)` | sequence of values produced and consumed one at a time |
| attrlist | `attrlist(:x 1)` | key/value store |
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

Control flow commands use `post_eval` — they receive unevaluated token
streams for their body expressions and choose when to evaluate them.
This is what makes `if`, `for`, and `while` work as language constructs
rather than ordinary functions.

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

To work with an attrlist across a func boundary, pass it as a keyword
arg — it is a reference type and writes inside the func are visible
after the call:

```
al=attrlist(:x 0)
f=func(al.x=99)
f(:al al)
// al.x is now 99
```

## Attribute Lists

An attrlist is a key/value store. Create one with `attrlist()` or
`list(:attr)`:

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

## Streams

A stream is a sequence of values produced and consumed one at a time, rather than all at once. Unlike a list which holds all its values in memory, a stream yields the next value only when asked — via `next()` or `each()`. This makes streams suitable for processing large or unbounded sequences without materializing the whole thing.

The streaming algebra:

```
s=$$(1,2,3,4,5)    // create stream
l=$s                // collect stream into list
s1,,s2              // concatenate two streams
next(s)             // pull next value, nil when exhausted
each(s)             // traverse stream, return count
```

Round-trip: `$($$(1,2,3))` returns `(1,2,3)`.

Drive a loop with `next`:

```
total=0
s=$$(1,2,3,4,5)
while((v=next(s))!=nil
  total=total+v)
```

## Running Scripts

```
run("path/to/script.comt")         // run from file
run("path/to/script.comt" :str)    // run from string
```

**Path resolution:** when running interactively, relative paths in
`run()` resolve relative to the directory of the calling script. When
invoking `comterp run script.comt` from the command line, `./`-relative
paths resolve relative to the process working directory, not the script
directory. Run interactively with a full path to avoid this:

```
// from inside comterp:
run("src/comterp_/tests/run_all.comt")
```

## print()

```
print("fmt" val [val...])          // print to stdout
print("fmt" val [val...] :str)     // print to string and return it
print("fmt" val [val...] :err)     // print to stderr
```

Format verbs: `%v` (any value), `%d` (int), `%f` (float), `%s` (string),
`%c` (char). Fixed args always before `:str`, `:err`, `:file` keywords.

## Conventions for .comt Scripts

- Use `print()` not `printf()`
- Fixed args always before keyword args
- Single-quoted char literals: `'a'`, `';'`
- Double-quoted string literals: `"hello"`
- Test accumulator: `ok=ok&&(result==expected)`
- Deferred/broken tests: comment out with `/* */`, reference the issue number
- Return `ok` as the last expression for use by `run_all.comt`
- Sub-scripts are not subject to coverage measurement

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
