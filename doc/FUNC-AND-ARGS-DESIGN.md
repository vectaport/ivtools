# FuncObj, `arg()`/`narg()`, and the func-as-verb model — design record

Design notes from the func/arguments design session (S. Johnston + Claude).
Some of this is implemented, some is the in-flight `arg()`/`narg()` patch, and
the last section is a future direction. Written down so the reasoning survives.

## The one-line shape

> **A func is a verb with a stream of expressions that works on objects.**

- `func()` packages a body — *one or many* expressions — into a callable
  `FuncObj`: a composite command (verb).
- Invoking it runs that body on the objects you pass in.
- It hands a result back out.

## Three different things, one engine

These are **three distinct concepts** that happen to share the same lazy,
deferred-token machinery. Do not collapse them into one (the whole point of
naming concepts: same mechanism ≠ same thing):

| Concept | Stored as | Drained how |
|---|---|---|
| **stream literal** | a `FuncObj` | *lazily* — `next()` one element at a time (streaming) |
| **func bodies** | a `FuncObj` | *eagerly* — invoked, run to the last value (not streaming) |
| **func actual parameter list** | **eagerly-evaluated values on the stack** (postfix order) | indexed on demand by `arg(n)` = `stack_arg(n)` |

`FuncObj` is the underlying abstraction for the first two: a held span of tokens
to evaluate later. The actual parameter list is the odd one out — **by default
it is *not* deferred**: a FuncObj invocation is an eager evaluation, so the
positionals are computed up front and sit on the stack in postfix order, and
`arg(n)` is simply `stack_arg(n)` reading the already-computed value. The
deferred-span reading of the actual-param list (a proto-`FuncObj` post-eval-
indexed out of the read-only postfix buffer) is the **`:posteval` future**
(§ below), not today's behavior.

## The func IO contract

**In — exactly two channels:**

- **positionals → `arg(n)`, indexed and re-readable.** A FuncObj invocation is
  **eager**: the positionals are evaluated up front and sit on the stack in
  postfix order, so `arg(n)` is just `stack_arg(n)` and `narg()` is the
  positional count. You can read `arg(2)` first, `arg(0)` three times, any order
  — but because the values are *already computed*, re-reading returns the **same
  value** each time; it does **not** re-run the expression. So `c(beep ding)`
  with `arg(0),arg(1),arg(0),arg(1)` in the body yields the four computed values
  but **beeps/dings once**, not "beep ding beep ding". The true re-*firing* bell
  — re-running a positional's expression on each read — needs deferred
  evaluation, which is the **`:posteval`** future keyword (§ below). A func
  positional is a real `ComValue` (the script-level `arg()` returns a string
  symbol; this one returns the value).
- **keywords → func-scoped variables, auto-created.** There is *no querying* of
  a keyword inside a body — no `stack_key`. `f(:x 5)` simply means `x` is `5` as
  a local. The keyword **is** the variable.

  *Read-side caveat (the slot is not zero-initialized).* The scope shadows on
  **write**, and for any **passed** keyword — but it does **not** shield an
  *unset, unwritten* name on **read**: reading such a name falls through
  `_alist` → localtable → globaltable to whatever the outer scope holds. So
  "an unsupplied keyword reads nil" — the basis of the
  `if(x==nil :then default …)` optional-parameter idiom — holds **only when the
  name isn't also a live outer variable**. It is an *uninitialized variable* in
  the exact C/C++ sense: if code (or a test) branches on `x==nil`, **set `x=nil`
  first** rather than assume the slot starts empty. (This is precisely what bit
  `funcarg.comt` test 10 once it was spliced into the flat `run()` scope behind
  a file that had left `x` set — see that test's nil-init and `run_all.comt`.)

**Out — one channel by default, because the scope does not leak:**

- a plain `x=…` inside the body stays *local* — scratch, not output;
- to get a value out you `return` it — and that return may be an **attrlist**,
  which is how you hand back several named results at once — or you deliberately
  escape the scope: `` `x=… `` to write the outer symbol, or `global(x)=…` for
  the global.

Nice twisted symmetry: keywords arrive **spread** as separate locals, but
multiple results leave **gathered** as one returned attrlist. In as names, out
as a namespace.

## Body execution is eager and finite — not streaming

The body is *invoked*: fired once, run through its finite blocks to the last
value, done. It is **not** streaming, by the clean test —

> it isn't streaming because it's **finite**; there's no chance of it lasting
> longer than the program.

A stream earns the name by the *possibility* of never ending (open-ended,
produced forever on demand). A func body can't: it's a fixed, bounded set of
blocks that *will* terminate inside the call. Running them in sequence isn't
streaming, it's just running them; firing the func once isn't streaming either.

Streaming returns one level up and **orthogonal**: you can *overdrive* the func
across a stream — one finite invocation per element. That streams the
*invocations*, not the body.

## `func()` the command

- Takes its **body span** (one body, or many space-separated expressions run in
  sequence, last value out) plus exactly **one predefined keyword, `:echo`**
  (`help(func)`: "echo the postfix version of parsed body"). `:echo` rides last
  and is *thinned* by `func()` — consumed so it never leaks into the body.
- **No formal positions.** The bodies are NOT positional args. (Don't conflate
  `func()`'s bodies with the funcobj's *call-time* positionals — different list,
  different end, different name.)
- The multiple-bodies idiom *can* imitate a formal arg list —
  `func(a0=arg(0) a1=arg(1) a2=arg(2) a0,a1,a2)` reads like declaring three
  parameters — but that's a thing you *may* do, never *must*. `arg(n)` is the
  bell; ring `arg(2)` first, `arg(0)` thrice, bind no names at all. No formal
  parameter structure is imposed; you bolt one on only if you feel like it.

## The `arg()`/`narg()` patch (landed — eager)

**Strategy (S. Johnston):** *let the positionals do what the guard stops, and
tie `narg()`/`arg()` to the eager actuals.*

What landed (on `comterp-arg-narg-for-func`):

1. **`comterp.c`, the FuncObj invocation** (the `val.is_object(FuncObj::...)`
   branch). The guards that bailed on positionals are removed. Keyword pairs
   build the `AttributeList` that becomes the body's locals (via `_alist`),
   split by each keyword's `keynarg` (`0` = bare flag → `true`); the fixed
   positionals are **captured eagerly** into a per-invocation channel on the
   ComTerp (`_funcobj_argvals` / `_funcobj_nargs` / `_funcobj_active`), saved and
   restored around `ef.exec` so nesting and recursion work.
2. **`arg()`/`narg()`** (`iofunc.c`, `GetArgFunc`/`NumArgFunc`). Serve from the
   funcobj channel when inside a body (`funcobj_arg(n)` / `funcobj_narg()`), and
   fall back to the script argv (`arg_str(n)` / `narg_str()` over `_arg_strs`) at
   top level (so `drawmo`'s `arg(1)` is untouched).
3. **Re-readable, not re-firing.** The invocation is *eager*: positionals are
   evaluated up front and captured, so re-reading `arg(0)` returns the **same
   already-computed value** — no consumption, but no re-run either. A
   side-effecting positional fires **once**, not per read.

> The "Three different things" table above imagined the actuals as a deferred
> span post-eval-indexed out of the read-only `_pfbuf` (a captured-side-array-free
> "bell"). That is **not** what landed — the landed form is eager capture. The
> `_pfbuf` re-index / re-firing bell is the future `:posteval` keyword.

Tests it satisfies (`funcarg.comt`): `f=func(arg(0)+arg(1)) f(3 4)` → 7;
re-readable `c=func(arg(0),arg(1),arg(0),arg(1)) c(5 6)` → `(5,6,5,6)` (and a
side-effecting `c(beep ding)` beeps/dings **once**, not "beep ding beep ding" —
that awaits `:posteval`); multi-body
`f=func(a0=arg(0) a1=arg(1) a2=arg(2) a0,a1,a2) l=f(1 2 3)` with `l==(1,2,3)`.
(Note `==` (45) binds tighter than `,` (35), so the literal needs the parens.)

## Corrected: `assign` blocks clobbering a registered command — *not* rebinding a funcobj

`AssignFunc::execute` (`assignfunc.c:49`) opens with:

```c
ComValue operand1(stack_arg(0, true));   // LHS, symbol-preserving
if (operand1.is_command() && stack_arg_post_eval_size(0)==1) {
    cout << "WARNING:  assignment to command \"" << ... << "\" without args not allowed ...";
    reset_stack(); push_stack(ComValue::nullval()); return;
}
```

What this actually blocks is assigning to a **bare registered command** —
`print=5` / `func=5` → *"assignment to command \"print\" without args not
allowed"*. `operand1` is read symbol-preserving; a registered command resolves
to a `CommandType` token (`is_command()` true) and the size-1 check catches the
bare form, so a built-in can't be clobbered.

It does **not** block reassigning a funcobj-bound *variable*. A funcobj variable
on the LHS is a plain `SymbolType` (not `is_command()`), so the guard is skipped
and the assignment proceeds normally. Verified:

- `f=func(19); f=func(20); f==20` → **true** (funcobj → funcobj)
- `f=func(19); f=5; f==5` → **true** (funcobj → value)
- `f=5; f=func(20); f==20` → **true** (value → funcobj)

No backquote is needed to rebind a funcobj-bound name. (An earlier draft of this
section claimed the opposite — that a funcobj LHS was blocked and `` ` `` was
required — which was wrong.)

## Future: `DuckType` — run-time method dispatch

comterp is a **verb-based** language, and that buys a method system almost for
free:

- Store **attrlists of `FuncObj`s keyed by method name** (`add`, `minus`, …).
- Dispatch on the **1 or 2 operand types already on the stack** at the moment a
  symbol is being converted into a `ComFunc` to push — i.e. resolve `add(a b)`
  to the right `add` FuncObj by the type(s) of its operands.
- A `DuckType` then holds a whole collection of what `add()`, `minus()`, etc.
  *mean* — single- or double-dispatch on operand type — and that collection can
  **change at run-time**.

This is multiple dispatch / open methods, realized in the existing
symbol→ComFunc conversion seam, with attrlists as the method tables. Tracked as
its own issue.
