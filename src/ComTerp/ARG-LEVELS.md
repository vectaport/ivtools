# Argument levels: from C `argc`/`argv` down to `arg()`/`narg()` in a func body

How "arguments" relate across the layers — the C process boundary, a comterp
script's command line, any command invocation, and a func body — and where
`nkey`/keywords enter (and, just as important, where they don't).

## The four levels

| Level | Args come from | Read a positional | Count | Keyword channel | Value type | Lives for |
|---|---|---|---|---|---|---|
| **1. C process** | shell → `main(argc, argv)` | `argv[n]` | `argc` | none — a flag is just another `argv` string | `char*` | the process |
| **2. comterp script** | `set_args(argv)` → `_arg_strs` (each `argv[i]` interned by `symbol_add`) | `arg(n)` | `narg()` | none at this level | **`StringType`** (the argv string) | the script run |
| **3. comterp command** (every `ComFunc`) | operands eagerly evaluated onto the stack | `stack_arg(n)` | `nargsfixed()` / `nargs()` | `stack_key(id)`, counted by `nkeys()` | any `ComValue` | one `execute()` |
| **4. comterp func body** (a FuncObj fire) | the fire captures positionals → `_funcobj_argvals`; keywords → the alist | `arg(n)` | `narg()` | the keyword *is* a local, read by name (not `stack_key`) | any `ComValue` | the invocation (alist lifetime) |

## The bridge: `arg()`/`narg()` is one pair of commands, two sources

`arg(n)`/`narg()` are the *same* two commands at levels 2 and 4. Which source
they read is chosen at runtime by `funcobj_active()`:

- **top level** → the script's command line (`_arg_strs`, from C's `argv`); the
  value comes back as a **`StringType`** (argv is strings — you parse it yourself).
- **inside a func body** → the func's captured positionals (`_funcobj_argvals`);
  the value comes back as the **already-evaluated `ComValue`** (int, float, list…).

So C's `argv` reaches a script as `arg()`/`narg()`, and the identical
`arg()`/`narg()` inside a func reads that func's actuals. Same surface, two feeds.

## The level usually missed: the universal command invocation (level 3)

Levels 2 and 4 are both special cases of level 3 — *every* command (`rect`,
`print`, `+`, and the FuncObj fire itself) gets its operands the same way:
eagerly evaluated onto the stack, read with `stack_arg(n)` (positionals) and
`stack_key(id)` (keyword values), counted by the `ComFuncState` arity family.

- The **script** (level 2) is the degenerate top: its "operands" are the
  process's `argv`, not stack operands, surfaced through the *same* `arg()`/`narg()`
  spelling.
- The **func body** (level 4) is a command invocation whose positionals are
  *re-exposed* to the body via `arg()`/`narg()`, and whose keywords are turned
  into named locals instead of being queried with `stack_key`.

That universal middle — `stack_arg`/`stack_key` + `ComFuncState` — is the
machinery the other two borrow.

## Where `nkey` (keywords) lives, and where it doesn't

Keywords are a **comterp-command-level** concept (levels 3 and 4). They do **not**
exist at the C / script boundary:

- **Level 1 (C):** no keyword channel — `argv` is a flat list of strings; a flag
  like `-x` is just another `argv[i]`, with no key/value structure.
- **Level 2 (script):** inherits that flatness — the script command line is
  positional strings, so `arg()`/`narg()` see no keywords.
- **Level 3 (command):** where keywords appear — `:key value` operands, counted
  by `nkeys()`, read with `stack_key()`. Each keyword token carries `keynarg`
  (how many values follow — **0 for a bare flag**).
- **Level 4 (func body):** those keywords become func-scoped locals — `:x 5`
  means `x` is `5`; a bare `:flag` means `flag` is `true`; an *unsupplied*
  keyword reads `nil`. The body reads them **by name**, never with `stack_key`.

So crossing from level 2 into level 3/4 is exactly where "flat positional
strings" becomes "typed positionals + a keyword channel." The value type tracks
it too: `char*` (1) → `StringType` (2) → fully typed `ComValue` (3, 4).

## The two arity-count families (don't conflate them)

The counts above come in two families (see `POSTFIX-INDEXING.md` §1):

- **ComValue token** — baked at parse onto the postfix token: `narg`, `nkey`,
  `keynarg`, `nids`, `pedepth`. The fire reads these to learn the call's shape.
- **ComFuncState** — runtime, per-invocation: `nargs` (non-keyword incl.
  post-keyword), `nkeys`, `nargsfixed = nargs - nargskey` (the pre-keyword
  positionals — the *real* positional count), `nargskey` (post-keyword values),
  `nargstotal`, `nargspost`.

**The trap that bit the bare-flag fix:** `narg()` counts non-keyword args
*including* values after keywords, so the fixed-positional count is
`nargsfixed = narg - nargskey`, **not** `narg - nkey` — and `nargskey == nkey`
only when every keyword happens to carry exactly one value. A bare `:flag`
(`keynarg == 0`) breaks the shortcut. See `funcarg.comt` tests 7–8 and 11.
