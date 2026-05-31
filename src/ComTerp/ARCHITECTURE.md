# ComTerp Architecture

## Overview

ComTerp is a command interpreter with an unusual hybrid evaluation model:
postfix token execution with per-command opt-in lazy evaluation. This
combination gives it the efficiency of a stack-based VM with the expressive
power of a lazy interpreter — without the overhead of either continuations
or tree-walking.

## Postfix Execution Model

ComTerp parses input into a flat array of postfix tokens (`_pfbuf`), then
converts them into a parallel array of `ComValue` objects (`_pfcomvals`).
This array is immutable for the lifetime of an expression and shared across
all command invocations during evaluation.

Execution proceeds left-to-right through `_pfcomvals`. Non-command values
are pushed onto the eval stack. When a command is encountered,
`eval_expr_internals()` fires the corresponding `ComFunc::execute()` method
with arguments already on the stack.

## The Argoffval Bookmark

The central mechanism that makes lazy evaluation possible is a single
pushed integer — the **argoffval**.

When `load_sub_expr()` encounters a `post_eval` command in `_pfcomvals`,
it pushes the command's current index (`_pfoff`) onto the eval stack as an
integer `ComValue` before pushing the command itself:

```cpp
if (func && func->post_eval()) {
    ComValue argoffval(_pfoff);
    push_stack(argoffval);
}
```

Inside the `post_eval` command's `execute()` method, `stack_top()` retrieves
this integer — a bookmark into `_pfcomvals` pointing just past the command.
`skip_arg_in_expr()` uses this bookmark to walk backward through `_pfcomvals`
and locate the start of each argument sub-expression.

This costs one integer push per lazy command invocation. No sub-expression
copying, no re-parsing, no continuation allocation.

## Eager vs Lazy Commands

**Eager commands** (`post_eval() == false`, the default):
- Arguments are evaluated before `execute()` fires
- Arguments arrive on the eval stack as fully evaluated `ComValue` objects
- `stack_arg(n)` retrieves the nth argument

**Lazy commands** (`post_eval() == true`):
- Arguments are NOT pre-evaluated
- The argoffval bookmark is pushed before the command
- `stack_arg_post_eval(n)` navigates `_pfcomvals` via the bookmark,
  calls `post_eval_expr()` to evaluate argument n on demand, returns result
- `stack_key_post_eval(id)` does the same for keyword arguments

This opt-in mechanism is what makes `if()`, `while()`, `for()`, `func()`,
and `assign()` work correctly — they receive unevaluated sub-expressions
and choose if, when, and how many times to evaluate each one.

## Post-Eval Depth (pedepth)

`_pfcomvals` entries inside a `post_eval` command's argument scope carry a
`pedepth > 0`. `load_sub_expr()` computes these depths by scanning backward
from each `post_eval` command using `skip_func()`. During normal evaluation,
tokens with `pedepth > 0` are skipped — they are only evaluated when a
`post_eval` command explicitly calls `post_eval_expr()`.

Nested `post_eval` commands increment the depth further, allowing arbitrary
nesting of lazy constructs.

## ComValue and _flags

`ComValue` extends `AttributeValue` with command/keyword metadata:

- `_narg`, `_nkey` — argument and keyword counts for command tokens
- `_nids` — delimiter token type (used for bracket matching)
- `_pedepth` — post-eval nesting depth
- `_flags` — bitfield for per-instance boolean flags:
  - `COMVALUE_BQUOTE_FLAG (0x01)` — return symbol without lookup
  - `COMVALUE_LHS_ASSIGN_FLAG (0x02)` — set by AssignFunc on a global()
    command ComValue to indicate lhs assignment context

Since `_pfcomvals` is an immutable array of `ComValue` objects (one per
postfix token), flags set on a specific `ComValue` in the array persist
for the lifetime of the expression and are copied when that `ComValue` is
pushed onto the eval stack.

## func() command for user written commands

The func() command:

    funcobj=func(body :echo) -- encapsulate a body of commands into an executable object

is a comterp primitive for creating new user-defined commands.  It is one of
the commands with arguments that are post or lazy evaluated.  In general the
post evaluation is used for conditonally executing blocks of code in an if(),
for(), while(), or switch(). But in the func() case it takes the yet-to-be
evaluated block which is its argument and returns it in on the stack in a
funcobj.

The funcobj is invoked later by giving the variable that holds it a list of
keyword arguments in parentheses:

    f=func(print("%v is of type %v\n" x type(x)))
    f(:x 1.e7)

and you will see:

    1e+07 is of type DoubleType

The keyword argument's value becomes the value of a local variable within the
the funcobj's scope.  All other variables outside the func body are available
read-only within the func body.  Any variable assigned to within the func body
becomes a local variable within the scope of the func body.

The func returns information by pushing a result on the stack, the value
that results from executing the body of the func.

## func() improvements

Currently the narg() and arg() commands only work for the command line.
They can be extended to provide for fixed-format arguments for func() use.
Along with what would be nkey() and maybe even nids() commands as well. The
end result would be user-defined funcs that behaved exactly like built-in
primitives.  Right now at least one keyword argument is required to call
the funcobj.

A new local() command would allow a func body to escape and set a value
outside its scope.  global() could work the same way.  Have to consider
the func() within a func() (a factory script) when implementing local().
local() might allow access to all variables that aren't global.  A
companion command to local() and global() might only go up one level
when searching for variables (the func body that is calling the lower
level func body) but that seems complicated to use as a programmer.  If
an escape to a larger namespace is needed, having a two tier system of
global() and local() is enough, and otherwise variables created or update
within a func body are invisible unless returned.

## Global Variable Lhs/Rhs Context

`global(sym)=val` requires `global()` to behave differently depending on
whether it is the lhs or rhs of an assignment:

- **lhs**: push a bquoted, global-flagged symbol for `AssignFunc` to write to
- **rhs**: look up and push the symbol's value from the global table

`AssignFunc::execute()` uses `skip_arg_in_expr()` via the argoffval bookmark
to locate the start of its arg 0 sub-expression in `_pfcomvals`. If that
start token is a `global` command, it sets `COMVALUE_LHS_ASSIGN_FLAG` on
that specific `ComValue`.

When `post_eval_expr()` later evaluates the lhs sub-expression and fires
`GlobalSymbolFunc::execute()`, that command can peek at its own `ComValue`
on the eval stack via `stack_top(nargs()+nkeys())` and check
`lhs_assign()`. Only the outermost `global()` explicitly flagged by
`AssignFunc` sees the flag — inner `global()` calls in rvalue position
within the same lhs expression are unaffected.

## Symbol and Global Tables

ComTerp maintains two variable tables per interpreter instance:

- `localtable()` — per-invocation local variables, scoped to the current
  `func()` body or top-level expression
- `globaltable()` — persistent across expressions, shared across all
  interpreter instances on the same `ComTerp`

`lookup_symval()` checks local first, then global. The `global_flag` on a
`ComValue` forces writes and reads to bypass local and go directly to
`globaltable()`.


## Key Files

| File | Contents |
|------|----------|
| `comterp.c/h` | Core interpreter: `load_sub_expr()`, `post_eval_expr()`, `eval_expr_internals()`, symbol/global tables |
| `comvalue.c/h` | `ComValue` — the fundamental token/value type, extends `AttributeValue` |
| `comfunc.c/h` | `ComFunc` base class — `stack_arg()`, `stack_arg_post_eval()`, `skip_arg_in_expr()` |
| `assignfunc.c` | `AssignFunc` — lhs context detection, global flag injection |
| `symbolfunc.c` | `GlobalSymbolFunc`, `SplitStrFunc`, and other symbol/string commands |
| `ctrlfunc.c` | `IfFunc`, `ForFunc`, `WhileFunc`, `RunFunc` — all post_eval control flow |
| `boolfunc.c` | Comparison and boolean operators |
