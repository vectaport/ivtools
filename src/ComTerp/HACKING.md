# Hacking on ComTerp

This document covers what you need to know to write, patch, and debug
C++ code in the ComTerp/ComUtil layer of ivtools. It complements
`ARCHITECTURE.md` (which explains the evaluation model) and
`LANGUAGE.md` (which covers the scripting language from the user side).

## Adding a New Command

A command is a subclass of `ComFunc` with an `execute()` method. The
pattern is the same for every command:

```cpp
void MyFunc::execute() {
    // 1. Capture all args and keywords BEFORE reset_stack()
    ComValue arg0(stack_arg(0));
    ComValue arg1(stack_arg(1));
    static int foo_symid = symbol_add("foo");
    ComValue foov(stack_key(foo_symid));
    boolean fooflag = foov.is_true();

    // 2. reset_stack() always before doing any work
    reset_stack();

    // 3. Do work, push exactly one return value
    ComValue retval(42, ComValue::IntType);
    push_stack(retval);
}
```

**Critical:** `stack_arg()`, `nargs()`, and `stack_key()` are invalid
after `reset_stack()`. Capture everything by value first.

**Return value:** always push exactly one value. Use
`push_stack(ComValue::nullval())` if there is nothing meaningful to
return.

### Header boilerplate

```cpp
class MyFunc : public ComFunc {
public:
    MyFunc(ComTerp*);
    virtual void execute();
    virtual const char* docstring() {
        return "retval=%s(arg [:foo]) -- one-line description"; }
    virtual const char** dockeys() {
        static const char* keys[] = {
            ":foo       what :foo does",
            nil
        };
        return keys;
    }
};
```

The `%s` in `docstring()` is replaced with the command name by
`help()`. Square brackets denote optional fixed arguments. Keyword
arguments (`:keyword`) are never wrapped in brackets — they are always
optional by nature and the brackets would be redundant and misleading.
Only fixed positional arguments need brackets when optional, e.g.
`func(arg [optarg] :keyword)`.

### Registration

Register in `ComTerp::add_defaults()` in `comterp.c`, at the bottom
of the existing block, following the established order:

```cpp
add_command("mycommand", new MyFunc(this));
```

Commands registered here are available in all interpreters — comterp,
comdraw, drawserv. Commands that require a graphical editor context
belong in `ComUnidraw` or higher layers instead.

### Docstring extension for higher layers

If a lower-layer command needs DrawServ-specific keyword documentation,
use the alternate docstring form rather than modifying the lower layer:

```cpp
comterp->add_command("select", func, nil, docstring2);
```

This replaces the dockeys in `help()` output without a layer violation.

## Keyword Arguments

Keywords are looked up by symbol id, cached as a `static int`:

```cpp
static int keep_symid = symbol_add("keep");
ComValue keepv(stack_key(keep_symid));
boolean keepflag = keepv.is_true();
```

`:keyword` (flag form) — `keepv.is_true()` returns true if present.
`:keyword value` — use `keepv.int_val()`, `keepv.string_ptr()`, etc.

Unknown keywords are silently ignored by `reset_stack()` — this is
intentional and enables layered keyword extension up the library
hierarchy. Do not add error checking for unknown keywords.

## Error State: _errbuf and _errbuf2

`ComTerp` maintains two error buffers:

- `_errbuf` — the current error string, filled by `this->err_str()`
  and cleared immediately after use
- `_errbuf2` — the last reported error, saved automatically whenever
  `_errbuf` is non-empty; persists until explicitly cleared

**Always use the instance method wrappers**, not the raw ComUtil C
functions, inside `ComTerp` member functions:

```cpp
this->err_str(_errbuf, BUFSIZ, "comterp");   // fills _errbuf, saves to _errbuf2
this->err_print(stderr, "comterp");           // saves to _errbuf2, prints, clears
```

Calling `::err_str()` or `::err_print()` directly bypasses the
`_errbuf2` save and breaks `errmsg(:last)`.

### Why _errbuf2 exists

In batch script execution (`comterp run script.comt`), parse and eval
errors are detected, printed, and cleared by the `run()`/`runfile()`
loop before any subsequent expression can call `errmsg()`. `_errbuf2`
is the persistence layer that makes `errmsg(:last)` work in that
context.

### errmsg() from scripts

```
e=errmsg()         -- current error (cleared after retrieval)
e=errmsg(:last)    -- last saved error from _errbuf2 (cleared after retrieval)
e=errmsg(:keep)    -- current error, not cleared
e=errmsg(:last :keep) -- last saved error, not cleared
n=errmsg(:cnt)     -- number of errors on stack
n=errmsg(:num)     -- error number
```

## Adding a New Error Code

Two files must be updated together:

**`src/ComUtil/comterp.err`** — add the `#define` and comment:

```c
#define ERR_MY_NEW_ERROR    1317
/* "(%d) descriptive message for this error" */
```

**`src/ComUtil/errsys.c`** — add to the `default_errmsgs` table in
numeric order:

```c
{1317, "(%d) descriptive message for this error"},
```

The `(%d)` receives the line number via `COMERR_SET1(errcode, linenum)`.
Use `COMERR_SET2` for errors that also embed a string token.

## extern Declarations for ComUtil Functions

`debugfunc.c` and other files that call ComUtil C functions directly
need explicit `extern` declarations since there is no C++ header that
covers them all:

```cpp
extern void err_str(char*, int, const char*);
extern void err_clear();
extern int err_cnt();
extern int comerr_get();
```

These go near the top of the `.c` file, after the `#include` block.
Do not add them to headers — they are implementation details of the
files that need them.

## Layer Hierarchy and Violations

The library layers from bottom to top are:

```
ComUtil → ComTerp → OverlayUnidraw → ComUnidraw → FrameUnidraw|GraphUnidraw → DrawServ
```

A lower layer must never `#include` a header from a higher layer.
When lower-layer code needs behavior that only exists in a higher layer,
use a virtual no-op method in the lower layer overridden in the higher:

```cpp
// In ComTerp (lower):
virtual void on_something() {}   // no-op

// In DrawServ (higher):
virtual void on_something() { /* real implementation */ }
```

The same rule applies to `dynamic_cast` — it is not used anywhere in
ivtools. Use virtual dispatch instead.

## Naming Conventions

- `comterp` (lowercase) — the interpreter: the binary, the `comterp_`
  directory, `.comt` scripts, `comterp_listen`, the REPL. Use this in
  user-facing docs, issue descriptions, and script comments.
- `ComTerp` (PascalCase) — the C++ library: `src/ComTerp/`, class names
  (`ComFunc`, `ComValue`, `ComTerp`), HACKING.md, ARCHITECTURE.md.
- PascalCase for classes inherited from Unidraw/InterViews heritage
  (`ComValue`, `ComFunc`, `DrawServCmd`)
- snake_case for newer additions (`dist_script()`, `make_brush_cmd()`,
  `last_errmsg()`)
- New methods and members go at the **bottom** of the header and source
  file, not interspersed with existing code
- Test scripts use `.comt` extension; see `TESTING.md` for conventions

## Patch Workflow

`git apply` requires exact context matches. The safest way to generate
a patch for this codebase is with Python `difflib` against fresh copies
of the files:

```python
import difflib
with open('original.c') as f: orig = f.readlines()
# ... make changes to a copy ...
diff = difflib.unified_diff(orig, new,
    fromfile='a/src/ComTerp/foo.c',
    tofile='b/src/ComTerp/foo.c')
with open('my.patch', 'w') as f: f.writelines(diff)
```

Then apply with:

```bash
git apply ~/Downloads/my.patch
```

For multi-line commit messages, use `git commit -F -` with a heredoc
to avoid shell quoting issues:

```bash
git commit -F - << 'EOF'
subject line

Body paragraph one.

Body paragraph two.
EOF
```

Never hand-write unified diff hunk headers (`@@ -n,m +n,m @@`) —
they are computed from the actual file contents and will be wrong if
typed manually.

## Dot Operator Rhs — Symbol vs Command Dispatch

On the rhs of `.`, the parser distinguishes bare identifiers from
command calls with explicit parens:

- **Bare identifier** (`a.type`, `a.exit`) — emits `TOK_COMMAND`
  with nids set to -1  used as a key lookup against the attrlist.
  Never dispatches as a command, even if the name is a registered
  zero-arg command.
- **Command with parens** (`a.type()`, `a.print("k_%v" n :sym)`) —
  emits `TOK_COMMAND`, dispatches normally, result used as key.

This is implemented in `_parser.c` at the bare identifier emission
point (line ~1028): when the top of the operator stack is the `dot`
operator and no left paren follows, emit `TOK_COMMAND` with nids
set to -1 instead of `TOK_COMMAND`. A `dot_symid` static
(initialized lazily) identifies the dot operator by its command
symid via `opr_tbl_commid()`.

The chain form `a.type.class.exit` works automatically — each bare
identifier after `.` hits the same emission point with dot on top of
the operator stack.

**Assignment to commands** is separately blocked in the evaluator:
`false=7` produces a warning and returns nil. This is independent of
the parser fix.

## Empty Delimiter Literals

`{}` and `()` as standalone expressions produce empty containers:

- `{}` → empty `AttributeValueList` (`ListType`), equivalent to `list()`
- `()` → empty `AttributeList` (`ObjectType`), equivalent to `attrlist()`
- `[]` and `<>` → only the *empty* pairs are left undefined; their meaning is held in reserve for a possible future flowtran flowgraph layer (see LANGUAGE.md, *ipl, and a future flowtran layer*). The bracket tokens emit `TOK_BLANK`, so non-empty `[5]` passes through as ordinary grouping (just `5`, the same as `(5)` and `{5}`) — it is specifically the empty pair that has no defined meaning.

This is implemented in `_parser.c` in the empty delimiter `else` branch — when
a closing delimiter is seen with `narg==0` and no `comm_id`, the parser emits
`COMMAND(list) narg 0` for `}` and `COMMAND(attrlist) narg 0` for `)` rather
than `TOK_BLANK`.

Prior to this fix, `{}` produced a `TOK_BLANK` which caused `offlimit` warnings
in `skip_arg` and failed to assign or round-trip. If you see `offlimit hit by
ComTerp::skip_arg` followed by `stack empty, blank returned`, an empty delimiter
pair reaching evaluation without this fix is the likely cause.

## remote() Return Values and Wire Protocol

`RemoteFunc::execute()` sends a command string over a socket, reads back one
line terminated by `\n`, then calls `comterpserv()->run(buf, true)` to evaluate
the response string locally. This means **the return value must be something the
local ComTerp parser can parse back**.

Types that round-trip cleanly:
- Integers, floats, booleans, strings — print and re-parse correctly
- `{}` (empty list) — works after the empty delimiter fix in `_parser.c`
- `{1,2,3}` (non-empty list) — works
- `nil` — works

Types that do **not** round-trip:
- `ObjectType` wrapping an `AttributeList` row — prints as `:key val :key val`
  without delimiters, which the remote parser sees as keyword arguments to
  nothing and fails. If you need to return a table of attrlists, wrap them in
  an `AttributeValueList` (`ArrayType`) — the outer `{}` gives the parser
  something to hang the rows on.
- `BlankType` — prints as nothing, `run("")` returns blank, caller sees
  `stack empty, blank returned`.

When `drawlink(:table)` returns an empty list, the drawserv writes `{}\n`.
The receiving `remote()` calls `run("{}", true)` — which only works because
of the `_parser.c` empty delimiter fix. Without it, `{}` parsed to `TOK_BLANK`
and the result was lost.

## Resource ref/unref and AttributeValue Constructors

`AttributeValue` manages refcounting automatically for two `ObjectType` classes:

```cpp
// Constructor calls Resource::ref(ptr) automatically:
AttributeValue(AttributeList::class_symid(), (void*)row)   // ref'd
AttributeValue(Attribute::class_symid(), (void*)attr)       // ref'd

// Destructor calls Resource::unref via unref_as_needed():
// -- fires for AttributeList and Attribute ObjectType only,
//    and only when RESOURCE_COMPVIEW is defined
```

`ComValue(AttributeValueList* avl)` uses the `ArrayType` constructor — also
ref-managed automatically.

**Do not** add explicit `Resource::ref()`/`Resource::unref()` at the call site
for these types — the `AttributeValue` constructor and destructor handle it.
Doing so double-refs and leaks.

For any other `ObjectType` (custom class, DrawLink, etc.) the caller is
responsible for refcounting — `AttributeValue` will not do it.

## Eager vs Post-Eval Commands

Most commands are **eager** — arguments are evaluated before `execute()` fires,
and `stack_arg(n)` retrieves the nth already-evaluated argument. This is the
default and covers the vast majority of cases.

Use `post_eval() { return true; }` and `stack_arg_post_eval(n)` only when:
- The command conditionally evaluates an argument (`if`, `while`, `for`)
- The command captures an unevaluated body for later execution (`func`)
- The command needs to evaluate an argument zero or more than once (`while` body)
- The command needs to inspect the argument's token structure, not its value

`ListFunc` uses `stack_arg_post_eval` not because it needs lazy evaluation but
because it accepts an optional stream argument and needs to handle the case
where no argument is provided without triggering stream consumption. When in
doubt, use eager — post_eval adds complexity and the `pedepth` / argoffval
machinery has subtle edge cases (see `offlimit` warnings).

The quick test: if your command would work identically with pre-evaluated
arguments in all cases, use eager `stack_arg`.

## nargspost() vs nargsfixed()

Inside a `post_eval` command's `execute()`:

- `nargsfixed()` — number of fixed positional arguments actually passed by
  the caller. Use this to check whether an optional argument was supplied.
- `nargspost()` — total number of post-eval argument slots, including the
  argoffval bookmark. **Do not use this to count caller-supplied arguments.**
  It counts tokens in the postfix buffer scope, not what the caller passed.
- `nargs()` — for eager commands, the number of evaluated arguments on the
  stack. For post_eval commands, unreliable before `reset_stack()`.
- `nkeys()` — number of keyword arguments passed by the caller, for both
  eager and post_eval commands.

The common mistake is using `nargspost()` where `nargsfixed()` is intended,
which gives wrong counts when optional arguments are omitted. If you see a
post_eval command behaving as if an optional argument was always supplied,
check which of these is being used to test for its presence.

## ComTerpServ vs ComTerp

`ComTerpServ` subclasses `ComTerp` and adds server-mode execution with
its own `runfile()`. When updating error handling or execution loop
behavior in `ComTerp::run()` or `ComTerp::runfile()`, check whether
`ComTerpServ::runfile()` in `comterpserv.c` also needs the same
change. The two `runfile()` implementations are parallel but not
identical — `ComTerpServ::runfile()` handles line-by-line buffering
and has its own `err_str` call sites.

## Deferred symbol_add in _parser.c

Static symbol IDs in `_parser.c` that are needed during parsing must
not be initialized at file scope with `symbol_add()` — the symbol table
may not be ready yet at static init time. Instead initialize to `-1`
and call `symbol_add()` lazily on first use:

```c
static int dot_symid = -1;
// ...
if (dot_symid == -1) dot_symid = symbol_add("dot");
```

This is the established pattern for all parser symbol IDs in that file:
`attrlist_symid`, `list_symid`, `dot_symid`, and the delimiter symids.

## String Return Values from Commands

To return a string from a `ComFunc`, use:

```cpp
ComValue retval(mystr, ComValue::StringType);
push_stack(retval);
```

`postfix()` is the canonical example — it captures its output in a
`strstreambuf`, then pushes the result as `StringType` rather than
writing to stdout. This makes the return value testable and composable
with other string commands.

## Commands Without Parentheses — Design Implications

Some commands fire without parentheses: `exit`, `beep`, `true`,
`false`, `nil`. This is intentional. The implications reach further
than they might seem:

**Zero-arg commands are symbolic constants.** `true`, `false`, `nil`
are not keywords — they are registered commands that return values.
Any zero-arg command can serve as a symbolic constant in an expression.

**The dot operator creates a clean namespace boundary.** Because `.`
suppresses command dispatch on its rhs for bare identifiers, attrlist
keys can freely use any name — including command names like `type`,
`class`, `exit`, `true`, `false`. The dot operator is a namespace
escape hatch that makes attrlists safe to use as general-purpose
key/value stores without collision.

**Runtime operator table mutation makes this composable.** Because
the operator table is mutable, new zero-arg commands could be registered
as operators, extending the "symbolic constant" set in a domain-specific
way. A boot script could define a vocabulary of named constants that
look and behave like language keywords.

**The lhs-of-assignment block is the safety valve.** The warning on
`false=7` prevents accidental rebinding of constants. Combined with the
dot-rhs symbol emission, the language has three well-defined contexts
for bare identifiers with consistent, predictable behavior.

## Contributor Note: `list()` vs `,` for List Growth

**Do not use `list(lst x)` to append to a list.** `list()` constructs
a new list from its arguments verbatim — if `lst` is already a list,
`list(lst x)` produces a 2-element list `{lst, x}` (a list-of-lists),
not `lst` with `x` appended.

Use the `,` (tuple) operator to append:

```
lst,x          // appends x to lst in place -- correct
list(lst x)    // wraps lst and x as two elements -- almost certainly wrong
```

This distinction has been considered for change and rejected (see
issue on list append semantics). The `,` operator is the correct and
efficient idiom — it mutates the existing `AttributeValueList` in place
via `TupleFunc::execute()` without allocation. `list()` always
allocates a new list.

The same applies inside loops and func bodies:

```
// correct:
for(i=0 i<n i++ lst,i)

// wrong -- builds nested list structure:
for(i=0 i<n i++ lst=list(lst i))
```

## Commit Message Convention

Commit messages are one-liners, as long as needed. Call out every
significant change — bug fixes, new files, and doc sections — separated
by semicolons:

```
Fix <bug summary>; add <new file/feature>; add <doc section> to <file>
```

Example from the `comterp-lookup-symval-scope-fix-and-docs` branch:

```
Fix lookup_symval func-scope order (++ infinite loop); add deeptest.comt stress suite; add Delimiter Semantics/func scoping/list idioms to ARCHITECTURE.md, LANGUAGE.md, HACKING.md
```

The PR description carries the full narrative — what broke, why, how it
was fixed, and what else changed. The commit message is the scannable
one-line summary that shows up in `git log`.

## C++ vs ComTerp naming convention

When referring to implementation internals, use C++ qualified syntax to
distinguish from ComTerp commands:

- **C++ methods**: `ComTerp::runfile()`, `ComTerpServ::run()`,
  `EvalFunc::execute()` — class-qualified
- **ComTerp commands**: `errmsg()`, `run()`, `postfix()`, `for()` —
  unqualified, in `code` formatting in prose

Never write `run()` when you mean `ComTerpServ::run()` in the same
sentence as the ComTerp `run()` command — they are different things at
different levels.

## Testing Parse Errors with errmsg()

Parse errors from expressions evaluated inline in a `.comt` script file
are consumed by `ComTerp::runfile()` before the ComTerp command
`errmsg()` can retrieve them. By the time the next line of the script
executes, the error has already been reported to stderr and cleared.

To reliably test that an expression produces a parse error, wrap it in
the ComTerp `run()` command with `:str` to force an isolated nested
evaluation via `ComTerpServ::run()`:

```
errmsg(:clear)
run("(0 :flag 1)" :str)      // ComTerp run() calls ComTerpServ::run() -- isolated context
e=errmsg(:last)              // error is now retrievable
ok=ok&&(e!=nil)
```

In interactive mode `errmsg()` works directly because each expression
is a complete round-trip. In a script file, `run(:str)` is the only
reliable way to isolate a parse error for assertion.

`errmsg(:clear)` before the test ensures no prior error bleeds in.
`errmsg(:last)` retrieves the most recent error without clearing it.
`errmsg(:clear)` again after cleans up for the next test.
