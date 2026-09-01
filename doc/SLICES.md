# String slices: ground truth

How `str@lo:hi` is represented, why the string-consuming commands read
through `cstr()`, how `+` decides to write in place vs. copy, and how
`:` ended up building a generic, chainable, arity-disambiguated value
rather than a slice-specific one. Established by reading `comvalue.h`,
`comvalue.c`, `listfunc.c`, `numfunc.c`, `boolfunc.c`, `symbolfunc.c`,
`_lexscan.c`, `_scanner.c`, `parser.c`/`parser.h`, and
`ComUtil/optable.c`; line numbers refer to those files as of this
writing. Companion to `LANGUAGE.md`'s *Slices* section, which covers
the same ground for a user calling these
commands rather than someone reading or extending the C++ underneath.

## 1. Representation: no new ComValue type

A slice is a plain `StringType` `ComValue`, sharing the parent's own
symid — the underlying symbol-table entry's memory never moves or
gets copied for the sake of slicing. Three fields describe the window,
each one repurposing storage a `StringType` never otherwise needs:

- `sliceoff()`/`slicelen()` — offset and length of the window, stored
  in `_narg`/`_nkey` (`comvalue.h:160-167`). Those fields hold a
  command's argument/keyword counts for every other `ComValue`
  variant; `narg()`/`nkey()` themselves read 0 unconditionally for a
  `StringType` (`comvalue.c:167-168`), so nothing that reads the
  "real" meaning of those fields can observe the repurposing.
- `blocksz()` — chunk size in bytes, stored in `_nids`. Defaults to
  0, meaning "ordinary byte-granular slice." A future consumer
  reading a nonzero value would treat `sliceoff()`/`slicelen()` as
  counts of `blocksz()`-byte chunks instead of raw bytes, so a slice
  could eventually describe more than characters, Go-`[]T`-style.
  Pure storage today — nothing reads it.
- `sliced()` — a flag bit, `COMVALUE_SLICED_FLAG` (`0x10`) in
  `_flags` (`comvalue.h:51,155-158`). Distinguishes a genuine slice
  (window fields meaningful) from an ordinary `StringType` (window
  fields present but unused, always 0).

`coloned()` (`COMVALUE_COLONED_FLAG`, `comvalue.h:150-152`) is a
separate, unrelated flag on `ArrayType` values, marking a list built
by `:` rather than `,` — see §6.

## 2. `cstr()` is the only correct read

`ComValue::cstr(std::string& scratch)` (`comvalue.c:179-193`) is the
slice-aware text accessor. Not sliced: returns
`AttributeValue::string_ptr()` (the whole shared string) directly, no
copy. Sliced: copies `[sliceoff(), sliceoff()+slicelen())` into
`scratch` and returns `scratch.c_str()` — a real copy is unavoidable
here, because a slice's own end is not a real `'\0'` in the shared
backing string, and writing one there would reintroduce the bug §7
describes for a different reason.

Every migrated call site declares its own local `std::string scratch`
and calls `.cstr(scratch)` — never a shared/static buffer. `cstr()` is
non-virtual and safe to call on any `ComValue&`, including a raw
element of `_stack` (`comterp.c`) read directly rather than copied to
a local first.

### Why not a virtual `string_ptr()` instead

Making `AttributeValue::string_ptr()` virtual, overridden in `ComValue`
to narrow on `sliced()`, was tried first — every existing caller would
get slice-awareness for free, no migration needed. Reverted the same
day: the common pattern in this codebase, `ComValue& x = stack_arg(n)`,
is a raw reference into `_stack`, not a safe local copy — and `_stack`
grows via `dmm_realloc`, a raw C realloc of the whole `ComValue` array.
Virtual dispatch on such a reference does not reliably resolve to
`ComValue`'s own vtable (confirmed live: one such reference resolved to
plain `AttributeValue`'s instead). `ComValue` has to stay a trivially
relocatable POD for `_stack`'s realloc discipline to be safe at all — no
member with real construction/destruction, and by extension no
reliably-dispatching vtable either. `cstr()`'s explicit, non-virtual
shape is the direct consequence of that constraint, not a style choice.

## 3. What's migrated, what isn't, and why

Slice-aware via `cstr()`: `print()` (`iofunc.c`, both the `%s`-spec and
no-spec paths, plus `operator<<`'s own `StringType` case, `comvalue.c`
— covers `%v` and the REPL's bare-echo), `==`/`!=` (`EqualFunc`/
`NotEqualFunc`, `boolfunc.c`), `split()` (`SplitStrFunc`,
`symbolfunc.c`), `+` (`AddFunc`, `numfunc.c` — §4), `index()`
(`ListIndexFunc`, `listfunc.c`), and `<`/`>`/`<=`/`>=`
(`boolfunc.c` — §5). `symadd()` reads `cstr()` too, for a different
reason (§6).

`==`/`!=` also stopped short-cutting to `symbol_val()` identity for
`StringType` — valid only for two genuine symbols, or two ordinary
non-sliced strings (interning dedups by content, so identity happened
to double as text equality there by accident). A slice's symid is its
*parent's*, unrelated to its own effective text: `sl=="cde"` compares
false under identity even when `sl` prints as `"cde"`.

Deliberately **not** migrated: `substr()`. It already makes a copy —
conceptually a proto-slice, the same shape this feature generalizes
into a zero-copy view — and isn't meant to become slice-view-aware
itself; a copy of a slice's own window is exactly as correct as a copy
of anything else's.

## 4. Growable strings and in-place `+`

`string(cap [:spaces])` (`StringFunc::execute()`, `symbolfunc.c`)
allocates a fresh, writable, `cap+1`-byte symbol via `symbol_new()` —
`cap` bytes NUL- (or space-) filled plus a guaranteed terminator.
Unlike `symbol_add()`, `symbol_new()` never deduplicates and is never
added to `symbol_find()`'s reverse index (its own doc comment,
`symbols.c`) — it is a private, write-through buffer's storage, not a
shared literal's. `strcap(str)` (`StrCapFunc`) reads that capacity back
via `symbol_len()`.

`AddFunc::execute()`'s `StringType`/`SymbolType` case
(`numfunc.c:249-`) is where `+` decides in-place vs. copy. Operand `a`
must be `is_only_string()` (`StringType`, never `SymbolType` — a
symbol's characters are its identity, shared by every value holding
that symid; writing through one would corrupt everyone else's
view). Given that, the in-place condition is:

```
a.sliceoff() + a.slicelen() + b.length < symbol_len(a's symid)
```

When it holds, `b`'s bytes are `memmove`'d (not `memcpy` — see below)
straight into the spare trailing capacity of `a`'s own backing symid,
and the result is handed back as a slice over that *same* symid with
the extended length — zero copy. Otherwise, a fresh, larger buffer is
allocated via `symbol_add()` and both operands are copied in once.

### Why `symbol_add()`, not `symbol_new()`, on the copy path

The first version of the copy path used `symbol_new()` with 2x
amortized headroom, matching Go's own growth-on-realloc policy
exactly — reasonable by analogy, and wrong. `symadd()`'s `StringType`
branch (`SymAddFunc::execute()`, before its own fix in §6) reused a
`StringType` argument's own symid directly as a symbol id, with no
lookup — valid only because, pre-`string()`, every `StringType` value's
symid necessarily *did* come from `symbol_add()` already: deduped,
registered, findable by text. A `symbol_new()`-backed result broke
that silently — `global(symadd(global(p1)+global(p2)))` started
returning a symid `global()` couldn't bind against. Staying on
`symbol_add()` for the copy path means a plain concatenation gets no
free growth headroom and copies again on its own next append — same
as it always did. Only the true in-place path, writing into an
already-over-provisioned `string()` buffer, gets zero-copy growth.

### Why `memmove`, not `memcpy`, for the in-place write

`b` (the value being appended) can alias `a`'s own buffer — e.g.
`sl=buf@0:5; sl+buf` appends `buf`'s own unsliced text onto a slice of
itself, so the read range (`b`'s `cstr()`) and the write range
(`a`'s trailing capacity) can genuinely overlap. `memcpy`'s
non-overlap requirement is trivially satisfied when writing into a
*fresh* allocation (the copy path, always a brand-new buffer) but never
guaranteed when the destination is existing, potentially-shared
storage. `memmove` handles the overlap correctly regardless of
direction; the in-place path uses it exclusively.

### The aliasing tradeoff, precisely

Writing into a symid's own spare trailing capacity is inherently a
shared-memory write. Two consequences, both accepted by design, the
same tradeoff Go's own `append` has when two slices share a backing
array:

- Another slice or plain reference sharing that same symid sees the
  growth too — `a=a+b` mutates whatever else still points at `a`'s
  underlying buffer, not just `a` itself.
- The reverse direction is safe by construction: a slice can never
  write *past* its own bound (`sliceoff()+slicelen()`, checked against
  `symbol_len()`) — an append can only ever extend forward into space
  nothing has fenced off, never outside the symid's own allocation.

No amortized/doubling growth policy exists for a *plain* (non-`string()`
-backed) repeated-append loop — `for(...) a=a+x` starting from an
ordinary string still copies on every iteration, since an ordinary
string's own capacity always equals its length. Only a `string()`
buffer with real spare capacity gets the fast path. Open question, not
yet decided: whether that's worth a dedicated growable-string type
distinct from a plain interned `StringType`.

## 5. Ordering comparisons: a pre-existing gap, not a slice bug

`<`/`>`/`<=`/`>=` (`GreaterThanFunc`/`GreaterThanOrEqualFunc`/
`LessThanFunc`/`LessThanOrEqualFunc`, `boolfunc.c`) had no
`StringType` case at all before this work — `"abc">"abd"` fell through
to `default: result = ComValue::nullval()`, silently `nil` for any
string comparison, forever. Not something the slice epic broke; it had
simply never been added. Fixed by adding a `StringType` case to all
four, modeled on `EqualFunc`'s own: `cstr()` for both operands.

The pre-existing `SymbolType` case had a latent, unrelated bug this
work also fixed: it read `operand2.symbol_ptr()`/`cstr()`
unconditionally, with no check that operand2 was actually string-like.
A genuinely mismatched comparison (`"abc">3.14`) read the float's raw
union storage as if it were a symid — either a bogus comparison
against an unrelated interned symbol, or a null pointer into
`strcmp()`. Both the `SymbolType` and new `StringType` cases now guard
with `if (!operand2.is_string()) { result = ComValue::nullval(); break; }`
before touching operand2's text, bailing to `nil` the same way the old
default-case fallback always did for a mismatch.

## 6. `:` — generic, not slice-specific

`ColonListFunc` (`listfunc.c`) is registered as the `:` operator,
priority 78 (`optable.c`, above `@`'s 77 so `str@lo:hi` groups the
range before `at()` consumes it). What it builds is deliberately
generic — a plain `coloned()`-tagged 2-element `AttributeValueList`,
arity-disambiguated rather than type-disambiguated: two elements reads
as a range/slice, a future three-element use (`hr:min:sec`) would read
as a `TimeObj`. Nothing about `:` itself knows it's sometimes used for
slicing; `ListAtFunc`'s `@` is the one specific consumer that
recognizes a `coloned()` 2-element index and knows to build a slice
from it (`listfunc.c`, the `is_only_string() && nv.coloned()` branch).

A bare identifier operand is captured as its own **unresolved symbol**,
never looked up (`stack_arg(0, true)` / `stack_arg(1, true)` — the
`symbol=true` flag suppresses `lookup_symval()`, `comfunc.c:86-`) — so
`Dec:25` doesn't fail just because `Dec` was never assigned anything,
and `y:1` captures `y` as a symbol even if `y` is bound to something
else. `@`'s own slice-construction code resolves `lo`/`hi` itself, via
`lookup_symval()`, after receiving the unresolved pair.

### Chained `:` flattens, matching `,`

`1:2:3` parses left-associatively as `(1:2):3`. `ColonListFunc`
checks whether its `lo` operand is already a `coloned()` array with
`nested_insert()` false, and if so appends `hi` onto `lo`'s existing
list in place rather than wrapping it in a fresh 2-element list —
`{1,2,3}`, not `{{1,2},3}`. This mirrors `TupleFunc`'s own flattening
for comma-chains exactly, `nested_insert()` included: `eval_expr_internals`
(`comterp.c:1068-1077`) stamps `nested_insert(true)` on any array
pulled back off a symbol-table read, immediately before it becomes an
operator's first operand — the "this came from storage, don't extend it
in place" signal both `,` and `:` need. Without checking it,
`x=1:2; y=(x):3` would silently mutate `x` into `{1,2,3}` too, since
`lo` and `x` share the same underlying `AttributeValueList` once `x` is
read back — caught live (`x=1,2; y=(x),3` already left `x` untouched,
confirming the pattern being copied was real, before writing the `:`
side of it) rather than assumed from `,`'s shape alone.

A `coloned()` list prints without braces, colon-joined — `1:2:3`, not
`{1,2,3}` — the one place `coloned()` changes observable behavior
rather than tagging a value inertly (`operator<<`'s `ArrayType` case,
`comvalue.c`). `list(:colon)` builds an empty `coloned()` list directly,
for starting one programmatically without a literal `:`.

### The scanner surgery for `a:b`

`a:b` used to be swallowed pre-parser into one bogus identifier token,
because `_colon_ident` (`_lexscan.c`) made `:` a legal
identifier-continuation character. Now defaults to 0 — confirmed via
the full `run_all.comt` suite that nothing relied on the old behavior.
That alone fixes `a:b` and `a: b`; the harder case, `a:b` where a
`:keyword` lexeme would otherwise claim it, needed two more pieces:

- `_lexscan_last_tokend`/`_lexscan_last_toktype` (`_lexscan.c`), new
  globals updated once at `lexscan()`'s own return, tracking the
  previous token in true document-scan order. `_parser.c`'s lookahead
  can call `scanner()` (and so `lexscan()`) more than once per token it
  hands its caller, so the scanner's own `*bufptr`/`*toktype` output
  params can't be trusted to mean "the token right before this one" by
  the time a later call reads them back.
- `_scanner.c`'s keyword-merge state machine now skips the merge only
  when `:` is glued, zero whitespace, directly onto a preceding
  `TOK_IDENTIFIER`. Real `:keyword` usage always has a space before it
  (`at(r 0 :raw)`), so that spacing was never at risk either way.
  Numbers and closing delimiters were deliberately left out of the
  check — `at(sl 0:raw)` (no established meaning either way, unlike
  `at(r 0 :raw)`, which is real syntax) was tried and backed off from,
  to avoid taking on an ambiguity nothing forces a call on.

Both `_lexscan_last_tokend`/`_toktype` participate in `Parser`'s
per-client save/restore discipline (`check_parser_client()`/
`save_parser_client()`, `parser.c`/`.h`) — the same mechanism a dozen
other pieces of scanner/parser state already use, since comterp
supports multiple concurrent interpreter clients and these are plain
process-globals otherwise. Missed on the first pass; any future global
scanner/parser state needs the same three-call-site treatment
(`init()`, `check_parser_client()`, `save_parser_client()`) before
shipping.

Still unsupported on purpose: `lo :hi` — space before `:`, none after.
Genuinely indistinguishable at the scanner level from real
`:key val` syntax following a positional argument, so left alone
rather than guessed at. `colonlist(lo hi)` (a plain command call,
reaching `stack_arg(i, true)` the same unresolved way) is the working
substitute for that one spacing.

## 7. `symadd()`: symbols stay idempotent, strings don't anymore

`SymAddFunc::execute()` (`symbolfunc.c`) used to hand back a
`StringType` argument's own symid directly (`val.string_val()`), no
lookup — valid only because every `StringType` value used to come from
`symbol_add()` already (§4 explains why `string()` broke that). Fixed
to always call `symbol_add(val.cstr(scratch))`, properly interning the
*text*. `cstr()` being slice-aware fixes a second, independent bug for
free: `symadd()` on a slice used to intern the parent's whole text, not
the slice's own window.

The underlying shift, worth keeping in mind for any future work here: a
`SymbolType`'s identity and a `StringType`'s identity are no longer the
same kind of thing. Symbols stay idempotent by construction — one name,
one id, forever. Strings, now that they can be private, writable, and
growable, are not: two string values can hold identical text without
being "the same" object, so any command that used to lean on "a
string's symid is already a proper registered symbol" needs to
re-derive from the text instead of trusting the symid.

## 8. Surviving a boxed copy

A slice keeps its `sliced()` tag — and a colon-list its `coloned()` tag,
along with `narg`/`nkey`/`nids` — when the `ComValue` is boxed into a
plain `AttributeValue`-based container: a function keyword argument, a
captured free variable, or any `AttributeList`/`AttributeValueList`
entry.  `AttributeValue` carries that block itself, so a plain
`new AttributeValue(value)` copies it, and a generic render-hook slot
lets `AttributeList`'s own top-level print go through
`ComValue::operator<<` for the two types (`ArrayType`/`StringType`)
that need it.

## 9. Known gaps

- `:set`/`:ins`/`:del` through a slice — not yet supported; falls
  through to `nil`.
- Slicing a plain list/array — only `is_only_string()` triggers slice
  logic; `@lo:hi` on a list reads as an ordinary (non-slice) index
  today.
- `blocksz()` is pure storage — no chunked/non-byte-granular slice
  consumer exists yet.
- No `symid()` resolve flag — `symid()` deliberately uses
  `stack_arg(i, true)` (unresolved), so `symid(buf)` on a bare variable
  returns the id of the *name* `"buf"`, not `buf`'s own storage symid.

  The reliable way to get a slice or plain string's real symid today
  is `symid(symvar(buf))`, not a self-append trick — `symvar(v)`
  (symbolfunc.c) is `ComValue symv(stack_arg(0)); push_stack(symv);`:
  an ordinary eager command that resolves its own argument once via
  a real `stack_arg(0)` lookup, then hands the result back as the
  output of a *command call*, not a bare postfix token. `symid()`'s
  own rule is "grab a bare symbol as-is, otherwise take the
  already-evaluated input" — so wrapping with `symvar()` first means
  there's no longer a bare token for `symid()` to grab; it receives
  `buf`'s already-resolved string content instead, same symid `buf`
  and any slice of it share:

  ```
  s = "hello world"
  greeting = s@0:5
  symid(symvar(s)) == symid(symvar(greeting))   -- true, same backing symid
  ```

  `symvar()`'s own primary job is the opposite-looking case — the
  left-hand side of an assignment, when the target name itself is
  computed rather than written literally (`sym=symadd(name);
  symvar(sym)=val` assigns to the variable *named by* `sym`'s content,
  not to a variable literally called `sym`). Both uses are the same
  single mechanism (resolve once, hand back a command result instead
  of a bare token) meeting two consumers with opposite defaults:
  `symid()` defaults to grabbing a bare symbol as-is (right for a
  literal symbol-table query, wrong for a value-holding variable);
  assign's own LHS read defaults to *never* looking up its target
  (right for an ordinary literal `x=5`, wrong when the name is
  composed). `symvar()` doesn't know or care which — it just always
  resolves once — so the same wrapper reads as "stops the grab" from
  `symid()`'s side and "stops assign's own no-lookup default from
  targeting the wrong name" from assign's, depending purely on which
  consumer's default you needed to override.
