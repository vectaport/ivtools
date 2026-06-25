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

## The `arg()`/`narg()` patch (in flight)

**Strategy (S. Johnston):** *let the positionals do what the guard stops, and
tie `narg()`/`arg()` to looking up — and running — the positionals in the
postfix buffer.*

Touchpoints:

1. **`comterp.c`, the FuncObj invocation** (the `val.is_object(FuncObj::...)`
   branch, ~line 440). It currently bails on any positional:
   ```c
   if(val.narg()!=val.nkey()) {
     fprintf(stderr, "free format args not yet supported for custom funcs (%s)\n", funcname);
     push_stack(ComValue::nullval()); return;
   }
   ```
   Remove that guard; let the positionals through. Keyword pairs still build the
   `AttributeList` that becomes the body's locals (via `_alist`); the positional
   spans stay addressable in the read-only postfix buffer (`_pfbuf`).
2. **`arg()`/`narg()`** (`iofunc.c`, `GetArgFunc`/`NumArgFunc`). Today they read
   the *script* argv (`comterp->arg_str(n)` / `narg_str()` over `_arg_strs`).
   Add the func-invocation path: when inside a func, `arg(n)` post-evals the
   n-th positional span via `post_eval_expr(tokcnt, offtop, pedepth)` against
   `_pfbuf`; `narg()` is the positional count. Fall back to the script argv at
   top level (so `drawmo`'s `arg(1)` is untouched).
3. The **bell is free** because `_pfbuf` is read-only — re-indexing the same
   immutable span re-runs it, no consumption, no captured side array.

Tests to satisfy: `f=func(arg(0)+arg(1)) f(3 4)` → 7; re-readable
`c=func(arg(0),arg(1),arg(0),arg(1)) c(beep ding)` → beep ding beep ding;
multi-body `f=func(a0=arg(0) a1=arg(1) a2=arg(2) a0,a1,a2) l=f(1 2 3)` with
`l==(1,2,3)` true. (Note `==` (45) binds tighter than `,` (35), so the literal
needs the parens.)

## Confirmed: `assign` guards against re-binding a func-bound variable

`AssignFunc::execute` (`assignfunc.c:47`) opens with:

```c
if (operand1.is_command() && stack_arg_post_eval_size(0)==1) {
    cout << "WARNING:  assignment to command \"" << ... << "\" without args not allowed ...";
    reset_stack(); push_stack(ComValue::nullval()); return;
}
```

So a bare `x=…` where `x` holds a `FuncObj` (which resolves as a command) is
**blocked** — warn + nil. Backquote bypasses it: `` `x `` makes `operand1` a
`SymbolType`, so `is_command()` is false, and control reaches the `SymbolType`
branch, which removes the old value and inserts the new (`localtable`/
`globaltable`). That is exactly why rebinding a funcobj-bound name needs `` ` ``.

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
