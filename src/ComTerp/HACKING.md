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

## ComTerpServ vs ComTerp

`ComTerpServ` subclasses `ComTerp` and adds server-mode execution with
its own `runfile()`. When updating error handling or execution loop
behavior in `ComTerp::run()` or `ComTerp::runfile()`, check whether
`ComTerpServ::runfile()` in `comterpserv.c` also needs the same
change. The two `runfile()` implementations are parallel but not
identical — `ComTerpServ::runfile()` handles line-by-line buffering
and has its own `err_str` call sites.
