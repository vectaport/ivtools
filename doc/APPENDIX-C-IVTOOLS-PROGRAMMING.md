# Appendix C: ivtools Programming

This appendix is for programmers working inside the ivtools codebase —
adding ComTerp commands, extending DrawServ, building new ComFunc
subclasses, or navigating the C++ layer that underlies the scripting
system. It assumes familiarity with INTRODUCTION.md and
LANGUAGE.md.

## The Two Hierarchies

ivtools has two parallel C++ class hierarchies that never subclass each
other, but cooperate closely:

**Unidraw Commands** (`Command` subclasses) — structured action objects
with history, undo/redo support, and the ability to propagate across a
DrawServ network session. `MoveCmd`, `ScaleCmd`, `PasteCmd`. These live
in `src/Unidraw/` and `src/OverlayUnidraw/`.

**ComTerp Commands** (`ComFunc` subclasses) — ComTerp-registered
functions callable from scripts and the REPL: `move()`, `scale()`,
`paste()`. These live alongside the commands they expose in
`src/ComTerp/`, `src/DrawServ/`, etc.

`ComFunc::execute()` typically creates and calls `Execute()` on a
Unidraw Command internally. The relationship is composition, not
inheritance.

## Adding a New ComTerp Command

A ComTerp command is a `ComFunc` subclass with an `execute()` method:

```cpp
class MyFunc : public ComFunc {
public:
    MyFunc(ComTerp*);
    virtual void execute();
    virtual const char* docstring() {
      return "val=%s(arg1 arg2 :keyword) -- brief description"; }
    CLASS_SYMID("MyFunc");
};
```

The canonical `execute()` pattern:

```cpp
void MyFunc::execute() {
    // 1. Capture ALL args and keywords BEFORE reset_stack()
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

For post-eval commands (where arguments should be evaluated lazily,
as in `for`, `while`, `if`, `stream`):

```cpp
virtual boolean post_eval() { return true; }
```

Use `stack_arg_post_eval(n)` instead of `stack_arg(n)` to evaluate
the nth argument on demand.

Register the command in `ComTerp::add_defaults()` in `comterp.c`:

```cpp
add_command("mycommand", new MyFunc(this));
```

## The Postfix Buffer

ComTerp parses input into a flat array of `postfix_token` structs
(`_pfbuf`). This is the Fischer-LeBlanc pipeline: scanner → parser →
postfix token array → evaluator. The postfix buffer is not just an
implementation detail — it IS the wire protocol. What travels over a
DrawServ socket is the same postfix token stream that the local
evaluator processes.

`_pfcomvals` is the `ComValue[]` representation of `_pfbuf`, populated
as evaluation proceeds. `_pfoff` is the current offset into `_pfbuf`.
`_pfnum` is the total token count.

For post-eval commands, arguments are not evaluated before `execute()`
is called — they sit in `_pfcomvals` as unevaluated token sequences.
The methods `stack_arg_post_eval(n)`, `stack_key_post_eval(id)`,
`skip_arg_in_expr()`, and `skip_key_in_expr()` navigate this buffer.

The key traversal insight: the buffer is scanned backward (from the
command token toward lower indices). Keywords appear before (closer to
the command than) their corresponding fixed-format arguments. So when
scanning from `saved_offtop`:

```
saved_offtop
    ↓
[ :key(1) ][ val ][ arg0 ][ arg1 ]   stream[n|k|1]*
  keyword   kval   fixed   fixed     command (topmost)
←————————————————————————————————— direction of scan
```

`skip_key_in_expr()` skips a keyword token plus its value expression.
`skip_arg_in_expr()` skips one fixed-format argument expression.
Both decrement the offset (scan toward lower indices).

## Why the Pipeline Is So Clean: Fischer-LeBlanc and argoff

Most language implementations interpose several layers between source
text and execution: a parse tree (AST), a tree-walking or lowering pass
to produce an intermediate representation, and then either compilation
or interpretation of that IR. Each layer adds complexity and each
crossing between layers requires a translation.

ComTerp has none of that. The pipeline is:

```
source text → scanner → Fischer-LeBlanc parser → postfix token array → evaluator
```

The Fischer-LeBlanc parser (from the scanner/parser library developed
at Triple Vision in the late 1980s, acquired by Scott Johnston at
dissolution and carried forward into ComTerp) produces postfix-ordered
tokens directly as it parses. There is no separately allocated AST. The tree structure is implicit
in the postfix ordering — token sequence and argument counts
(`[narg|nkey|nids]`) encode the full topology without ever
materializing heap-allocated nodes with pointers. The "code conversion"
step that would normally transform an explicit parse tree into an
executable form is trivially thin — the parser is already building
postfix order as it goes, and the postfix buffer it emits *is* the
executable form.

This has two consequences that compound each other:

**The wire protocol is free.** Because the postfix buffer is the
executable form, sending an expression over a socket and evaluating it
locally are the same operation. There is no separate serialization
layer, no marshalling, no protocol adapter. `remote()` sends the
expression string, receives a ComTerp value string back, and evaluates
it — one pipeline, both directions.

**`argoff` unifies all lazy evaluation.** When the evaluator reaches a
post-eval command in the postfix buffer, instead of having already
evaluated and pushed all of the command's arguments onto the operand
stack, it pushes a single integer: `argoff` — the offset into the
static postfix buffer where the command's unevaluated arguments begin.

The command receives `argoff` and can do whatever it likes with the
buffer: evaluate one argument now and save the rest for later, evaluate
an argument repeatedly (as `while` does for its condition and body),
capture a slice as a `FuncObj` for deferred execution, or skip
arguments entirely (as `if` does for the untaken branch).

This is what makes `if`, `for`, `while`, `func`, stream literals, and
`remote()` all work through the same mechanism rather than requiring
special cases in the evaluator:

- `if(:then :else)` — evaluates only the selected branch via `argoff`
- `while(cond body)` — re-evaluates both from the buffer on each iteration
- `func` — captures the body token buffer as a `FuncObj` via `argoff`
- Stream literals — `StreamLiteralNextFunc` re-evaluates each element
  token lazily via `comterpserv()->run(tokbuf+offset, cnt)`
- `remote()` — the returned string is evaluated by the same pipeline

In Lisp, special forms and macros achieve some of this, but each is
baked into the evaluator as a special case. `argoff` is a uniform
mechanism: any `ComFunc` subclass opts in by returning `true` from
`post_eval()`, and the same `argoff` machinery handles it. No special
cases in the evaluator, no privileged syntax.

The simplicity compounds: the Fischer-LeBlanc parser encodes the tree
implicitly in a flat buffer → the buffer is directly addressable by
integer offset → `argoff` is just an integer → post-eval commands are
just commands with one extra method → lazy evaluation, streaming, and
distributed execution all fall out of the same mechanism.

## FuncObj and Lazy Evaluation

`FuncObj` (defined in `postfunc.h`) is the mechanism for storing an
unevaluated expression for later execution:

```cpp
int toklen;
postfix_token* tokbuf = copy_stack_arg_post_eval(0, toklen);
FuncObj* fo = new FuncObj(tokbuf, toklen);
```

Later, evaluate it:

```cpp
ComValue result = comterpserv()->run(fo->toks(), fo->ntoks());
```

`comterpserv()->run(postfix_token*, int)` saves and restores the
interpreter state via `push_servstate()`/`pop_servstate()`, loads the
token buffer, evaluates one expression, and returns the result. This
is the foundation for lazy stream evaluation — `StreamLiteralNextFunc`
uses it to evaluate one stream element per `next()` call.

## Stream Implementation Pattern

Every stream in ComTerp has two parts:

**The setup func** (e.g. `StreamFunc`, `RepeatFunc`, `IterateFunc`) —
called once, builds an `AttributeValueList` (AVL) holding the stream
state, creates a stream `ComValue` pointing to the next func:

```cpp
ComValue stream(nextfunc, avl);
stream.stream_mode(STREAM_INTERNAL);
push_stack(stream);
```

**The next func** (e.g. `StreamNextFunc`, `RepeatNextFunc`) — called
by `NextFunc::execute_impl()` on each `next()` call. Receives the
stream `ComValue`, retrieves the AVL, produces one value, updates
state, pushes result. Returns nil when exhausted.

The AVL is the stream's state object. Existing patterns:

- `IterateFunc`: AVL holds `[current, stop]` — two ints mutated in place
- `RepeatFunc`: AVL holds `[value, count]` — count decremented in place
- `StreamNextFunc`: AVL holds the list being consumed, removing from front
- `StreamLiteralNextFunc`: AVL holds `[FuncObj, nremaining, (offset,count)...]`
  — FuncObj wraps the token buffer, entries consumed from front

## ComValue Types

`ComValue` is the universal value type. Key types:

| Type | Test method | Notes |
|------|-------------|-------|
| `IntType` | `is_int()` | |
| `DoubleType` | `is_double()` | |
| `StringType` | `is_string()` | |
| `SymbolType` | `is_symbol()` | interned string |
| `KeywordType` | `is_keyword()` | keyword token, carries `keynarg_val()` |
| `CommandType` | `is_command()` | `obj_val()` → `ComFunc*` |
| `StreamType` | `is_stream()` | `stream_list()` → AVL, `stream_func()` → next func |
| `BlankType` | `is_blank()` | return of `empty()`, out-of-bounds `at()` |
| `UnknownType` | `is_unknown()` | end-of-stream signal from `NextFunc` |
| `ObjectType` | `is_object()` | `obj_val()` → `void*`, check with `is_object(ClassId)` |

`ComValue::nullval()` — nil. `ComValue::trueval()` — true (for bare
keyword defaults). `ComValue::blankval()` — BlankType.

## narg, nkey, and the Counting Convention

For a command `foo[narg|nkey|nids]*`:

- `narg` (`nargsfixed()`) — count of all non-keyword tokens: fixed-format
  arguments PLUS keyword values. Not just pure fixed-format args.
- `nkey` (`nkeys()`) — count of keyword name tokens only.
- Pure fixed-format arg count is NOT directly available without traversal,
  because bare keyword flags (`:flag` with no value) contribute 0 to
  `narg` while valued keywords (`:key val`) contribute 1. The only
  reliable way to count pure fixed-format args is to traverse the buffer
  using `skip_key_in_expr()` and `while (rescan > offtop)`.

For stream literal `(0 1 :key 99)` → `stream[3|1|1]*`:
- `narg=3` — tokens 0, 1, 99 (including keyword value)
- `nkey=1` — keyword `:key`
- pure fixed-format count = 2 (0 and 1), but requires traversal to know

## symbol_add and CLASS_SYMID

Symbol interning is pervasive. `symbol_add("name")` returns an integer
symid, creating the symbol if it doesn't exist. Cache the result in a
`static int`:

```cpp
static int foo_symid = symbol_add("foo");
```

`CLASS_SYMID("ClassName")` declares the class symbol machinery for
`ObjectType` ComValues. Required for any class stored as `obj_val()`.
Access via `ClassName::class_symid()`. Test with:

```cpp
val.is_object(FuncObj::class_symid())
FuncObj* fo = (FuncObj*)val.obj_val();
```

## AttributeList vs AttributeValueList

Two different list types are used throughout:

**`AttributeList`** — a property list of `Attribute` objects, each
holding a name (symid) and an `AttributeValue`. This is what `attrlist`
literals and `setattr()`/`getattr()` work with. Iterate with
`ALIterator`. Add with `add_attr(symid, value)`. Find with
`find(symid)` → `AttributeValue*`.

**`AttributeValueList`** — a flat list of `AttributeValue` objects,
no names. Used for stream state AVLs, list values, and internal
storage. Iterate with `ALIterator`. Access by index with `Get(n)`.
Append with `Append(new AttributeValue(...))`. Remove with
`Remove(av_ptr)` — note: invalidates any existing iterator.

After `Remove()`, always re-navigate from `First()` rather than
continuing with the existing iterator.

## Resource and Reference Counting

`AttributeList`, `AttributeValueList`, and many other objects derive
from `Resource` (InterViews reference counting). To keep an object
alive: `Resource::ref(obj)`. To release: `Resource::unref(obj)`.

When constructing a singleton `AttributeList` for a keyword stream
element:

```cpp
AttributeList* al = new AttributeList();
al->add_attr(key_symid, keyval);
Resource::ref(al);
ComValue result(AttributeList::class_symid(), (void*)al);
```

## The DrawServ Extension Pattern

DrawServ extends comdraw by adding `DrawServFunc` and `DrawLinkFunc`
subclasses. The distributed command propagation pattern:

1. A `DrawServCmd` (Unidraw Command subclass) wraps the operation
2. The corresponding `ComFunc` creates the `DrawServCmd` and calls
   `Execute()` locally
3. `DrawServCmd::Execute()` calls `Interpret()` which propagates the
   command string to connected peers via `DrawLink`

The `LinkBrushCmd` mixin pattern: `DrawServCmd` subclasses mix in the
`LinkBrushCmd` interface to get network propagation for free.

## The `:lock`/`:unlock` Pattern

DrawServ distributed selection uses a two-phase lock protocol exposed
to ComTerp via `select(:unlock key)` / `select(:lock key)`. The key
is a UUID-based session token. Between `:unlock` and `:lock`, remote
ownership checks are suspended, allowing a distributed command to
execute atomically from the perspective of remote peers.

## Debugging Tips

**Build timestamp** — the comterp banner prints `(built: DATE TIME)`.
This reflects when `src/comterp_/main.c` was last compiled, which
happens on `make clean` + full rebuild. For incremental builds, use
`ls -l $(which comterp)` to verify the binary timestamp.

**`postfix(expr)`** — returns the unevaluated postfix representation
of any expression as a string. Essential for understanding how the
parser saw your input. The format `command[narg|nkey|nids]*` shows
argument counts; `*` means post-eval.

**`errmsg(:clear)`** — clears both current and last error state.
Essential in test harnesses to isolate errors between test cases.
Pattern for testing parse errors:
```
errmsg(:clear)
run("expression_that_should_error" :str)
err=errmsg(:last)
```

**`stackheight()`** — returns current stack depth. Useful for
diagnosing stack leaks (every `execute()` must push exactly one value
after `reset_stack()`).

**`trace(1)`** — enables execution tracing.

## Test Infrastructure

Test files use `.comt` extension and live in `src/comterp_/tests/`
(ComTerp), `src/comdraw/tests/` (drawing commands), and
`src/drawserv_/tests/` (distributed tests).

The test harness pattern`:

```
ok=true
prev_ok=true

func check_fail(:ok flag) (
    if(prev_ok&&!flag print("    ^^^ FIRST FAIL ^^^\n"))
    global(prev_ok)=flag
    flag
)

// --- test N: description ---
// BEFORE patch: expected failure behavior
// AFTER patch:  expected passing behavior
ok=ok&&(expression==expected)
print("N description: %v\n" expression)
prev_ok=check_fail(:ok ok)

print("suitename: %v\n" ok)
ok
```

`run_all.comt` in each test directory aggregates suite results.

## Key Source Files

| File | Contents |
|------|----------|
| `src/ComTerp/comfunc.h/c` | `ComFunc` base class, stack access, arg/key navigation |
| `src/ComTerp/comterp.h/c` | `ComTerp` interpreter, postfix buffer, eval loop |
| `src/ComTerp/comterpserv.h/c` | `ComTerpServ` server mode, `run(tokbuf, ntoks)` |
| `src/ComTerp/strmfunc.h/c` | All stream commands and iterators |
| `src/ComTerp/postfunc.h/c` | `for`, `while`, `if`, `FuncObj`, `FuncObjFunc` |
| `src/ComTerp/listfunc.h/c` | `list()`, `attrlist()`, `at()`, `size()` |
| `src/ComUtil/_parser.c` | Fischer-LeBlanc parser, postfix token generation |
| `src/DrawServ/drawservfunc.h/c` | DrawServ ComFunc subclasses |
| `src/comterp_/main.c` | comterp executable entry point |
| `src/comdraw/main.c` | comdraw executable entry point |

## Why the Postfix Buffer Is Indexed Backward

ComTerp uses postfix (reverse Polish) evaluation. The parser emits tokens
in postfix order — operands first, operator last. For a command call
`foo(a b :key c)` the token buffer looks like:

```
index:  ...  N    N+1  N+2  N+3       N+4
token:  ...  a    b    c    :key(1)   foo[2|1|1]*
                                      ↑
                                   command (topmost)
```

When the evaluator reaches `foo`, its arguments are already in the buffer
at lower indices. To find them, post-eval commands scan *backward* from
the command token — that's why `saved_offtop` starts just below the
command and `skip_key_in_expr`/`skip_arg_in_expr` decrement the offset.

Keywords appear closer to the command than their fixed-format counterparts.
`a` and `b` were emitted first (deepest), `:key` and its value `c` were
emitted later (closer to the command). So when scanning backward from the
command, you hit keywords before fixed-format args — the reverse of the
order they appeared in the source expression.

The `offlimit` parameter passed to `skip_arg` and `skip_key` is the
absolute bottom — `-pfnum` — below which the scan must not go.

## When You Need to Deal With the Buffer Geometry

Most `ComFunc` subclasses never touch the buffer directly. `stack_arg(n)`,
`stack_key(id)`, `stack_arg_post_eval(n)`, and `stack_key_post_eval(id)`
handle the navigation for you. You only need to deal with the raw buffer
geometry when:

**You are implementing a new lazy/streaming construct** — anything that
needs to capture the unevaluated token sequences for later execution, like
stream literals, `func()`, or a new post-eval operator. You need to
traverse the buffer yourself to find each argument's offset and token
count, then copy those slices for later `comterpserv()->run()` calls.

**You are adding stream overdrive to a post-eval operator** — making
`cond()`, `if()`, or `switch()` re-evaluate their arguments across a
driving stream. You need to save the buffer position after the first
evaluation and resume from that position on each subsequent `next()` call.

**You are adding a new argument navigation method** — anything that needs
to locate a specific argument by position or keyword without evaluating it.

In all these cases the key facts to keep in mind:

1. **Keywords before fixed-format args** — scanning backward from the
   command, you hit keywords first. Use `skip_key_in_expr` for each
   keyword (known count: `nkeys()`).

2. **`nargsfixed()` includes keyword values** — it counts all non-keyword
   tokens: fixed-format args AND values that follow keywords. To get the
   true fixed-format (positional) count, subtract the keyword value count:
   ```cpp
   int nkeyvals = 0;
   // accumulate keynarg_val() for each keyword during the keyword scan
   int npositionals = nargsfixed() - nkeyvals;
   ```
   Bare keyword flags (`:flag` with no value) contribute 0 to `nkeyvals`.
   Valued keywords (`:key val`) contribute 1. This must be computed by
   traversal — there is no stored field for it.

3. **`offtop` is a relative negative offset** — `saved_offtop` starts at
   `argoffv.int_val() - comterp()->pfnum()`, typically `-1` (just below
   the command token). Each `skip_*` call decrements it further. The
   absolute bottom of the arg region after a full scan is the `offtop`
   value when all args have been skipped.

4. **Iterator invalidation after `Remove()`** — `AttributeValueList`
   iterators go stale after any `Remove()` call. Always re-navigate from
   `First()` after removing an entry. Or use `Get(n)` by index if the
   AVL structure is known and stable.

The stream literal implementation in `StreamFunc::execute_literal()` is
the worked example of all four of these at once.

## Generating Patches for git apply

When producing a patch for `git apply`, the `---` and `+++` header lines
must use `a/` and `b/` prefixes with paths relative to the repo root.
Files uploaded to Claude land at `/mnt/user-data/uploads/` and patched
copies are written to `/home/claude/`. Use `sed` to fix the paths:

```bash
diff -u /mnt/user-data/uploads/filename.c /home/claude/filename.c \
  | sed 's|/mnt/user-data/uploads/filename.c|a/src/SubDir/filename.c|; \
         s|/home/claude/filename.c|b/src/SubDir/filename.c|' \
  > filename.patch
```

For `_parser.c` specifically:

```bash
diff -u /mnt/user-data/uploads/_parser.c /home/claude/_parser.c \
  | sed 's|/mnt/user-data/uploads/_parser.c|a/src/ComUtil/_parser.c|; \
         s|/home/claude/_parser.c|b/src/ComUtil/_parser.c|' \
  > patch.patch
```

Apply with:

```bash
git apply patch.patch
```

## See Also

- `INTRODUCTION.md` — project overview and history
- `APPENDIX-A-DRAWING-EDITOR.md` -- history of drawing editor embedded in ivtools
- `APPENDIX-B-COMTERP-EXAMPLES.md` -- how to learn comterp command language

- `src/ComUtil/ARCHITECTURE.md` -- scanner/parser and utilities
- `src/ComTerp/ARCHITECTURE.md` -- ComTerp C++ design
- `src/ComTerp/HACKING.md` -- comterp programming

