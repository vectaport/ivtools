# Postfix indexing & arity: ground truth

What the source says about how the postfix buffer is laid out, how arity counts
work, how the buffer is traversed, and how stream-literal consumption is built on
top of it. Established by reading `comterp.c`, `comfunc.c`, `comfunc.h`,
`comvalue.h`, and `strmfunc.c`; line numbers refer to those files.

## 1. Two arity families on two different objects

Look-alike names with different meanings; conflating them is the core hazard.

### ComValue token counts — in the postfix token

`narg()`, `nkey()`, `nids()`, `pedepth()` are four integers baked into each
postfix token at code-conversion (parse) time and carried in the pfbuf
(`ComValue(postfix_token*)`). **The buffer walk reads these.**

- `narg()` — non-keyword args of this token, **including args that follow
  keywords**. It is *not* "args before keywords."
- `nkey()` — keyword args of this token.
- `nids()` — the header comment says "(not used)", but that comment is stale: it
  is read at `comterp.c:297` (`sv.nids()!=1`) on the delim/dot path. Treat it as
  live.
- `pedepth()` — post-eval scope depth, set by the backward pre-pass (§2).

### ComFunc / ComFuncState invocation counts — runtime state

Pushed on the `_fsstack` when a command executes; read by `execute()` bodies via
`stack_arg` etc. (Documented in HACKING `## nargspost() vs nargsfixed()`.)

- `nargs()` — non-keyword args incl. post-keyword. For post_eval, unreliable
  before `reset_stack()`.
- `nkeys()` — keyword args.
- `nargskey()` — args that *follow* keywords (post-keyword positionals),
  `<= nkeys()`.
- `nargsfixed() = nargs() - nargskey()` — the pre-keyword, order-dependent
  positionals. **This** is the "args before keywords" count, not `nargs()`.
- `nargstotal() = nargs() + nkeys()` — everything.
- `nargspost()` — post-eval token-scope count; **includes the argoffval
  bookmark**. Not a caller-supplied-arg count.

The three meanings people attach to "nargs" are three *different members* —
`nargstotal` (all whitespace-separated incl. keywords), `nargs` (all non-keyword
incl. post-keyword), `nargsfixed` (pre-keyword only) — not three readings of one
call. The walk reads the *token* family; `execute()` bodies read the *funcstate*
family.

## 2. The pfbuf is stored reversed; the walk has two passes

`load_sub_expr` (`comterp.c:490`) runs in both directions, for different jobs:

- **Backward pre-pass** (`j = _pfnum-1 .. 0`) computes `pedepth` for each token,
  using `skip_func` to find how far each post-eval command's scope reaches. This
  is a lookahead: a token's operands appear *before* the command in postfix, so
  you can only know a token is inside a post-eval region after seeing the
  post-eval command that follows it — which forces a backward pass.
- **Forward push pass** (`_pfoff = 0 .. _pfnum`) pushes operands onto the stack
  until a CommandType/funcobj at depth 0 is hit.

So storage is reversed, but both directions are in use, and the backward pass is
the forward pass's precomputed lookahead — not a competing traversal.

## 3. The arity-aware traversal exists and is linear

`ComTerp::skip_func` / `skip_key` / `skip_arg` (`comterp.c:693-`) is a correct,
mutually-recursive, arity-aware skip over the reversed buffer. `skip_func` reads
the token's `narg`/`nkey`, steps `offset` backward, and recurses to consume each
operand's whole subtree. Within one span it visits each token once: it is
**linear, not quadratic**.

### The `narg`-meaning correction lives inside skip_func

`skip_func` (lines 702-712): `nargs = sv->narg()` (incl. post-keyword); the loop
decrements `nkeys` on a keyword and `nargs` on a non-keyword, with
`nargs -= tokcnt ? 1 : 0` discounting a post-keyword positional that a keyword
already consumed (so it isn't double-counted against `narg`). This line is the
"`narg` includes post-keyword args" subtlety encoded as a correction. It is the
place a mis-walk would hide, and it can only be validated by probing across
keyword arrangements (positional-only, trailing keywords, interleaved keywords,
keyword-with-value) via `postfix()`, not by eye.

## 4. The quadratic cost is in the accessors, not the traversal

The traversal is linear. The quadratic cost is the access pattern in the
`stack_arg_post_eval*` family (`comfunc.c:129-311`), which is **stateless across
calls**:

```
recover offtop from the argoff anchor (argoff.int_val() - _pfnum)
skip ALL keywords        (for i < nkeys())
skip from nargsfixed() down to n   (for j = nargsfixed(); j > n; j--)
read argument n
```

Reading args `0..nargsfixed-1` re-skips the keyword block plus all later
positionals on every fetch: `nargsfixed + (nargsfixed-1) + ... + 1` skip passes —
O(n²). The shape is "re-derive position from a fixed anchor on each access"
rather than "advance a saved cursor." A side effect of this shape: the *first*
argument fetched (`n=0`) is the most expensive (`nargsfixed` skips down) and each
later one is cheaper, so a consumer reading `0,1,2,...` starts slow and speeds
up.

`stack_arg_post_eval`, `stack_arg_post`, `stack_arg_post_eval_size`,
`print_stack_arg_post_eval`, and `copy_stack_arg_post_eval` all use this shape.

### The linear template already exists, used by one method

`stack_arg_post_eval_nargsfixed` (`comfunc.c:212`) walks **once**,
`j = nargsfixed() .. 1`, caching `offtopbuf[j-1]`/`argcntbuf[j-1]` for every
positional in a single descent, then consumes from the cache. The fix for the
quadratic methods is to share one descent the way this method does — not to write
a new traversal.

## 5. What this says about the argoff anchor

The argoff anchor and the per-access re-scan are load-bearing together: the
accessors recover position from `argoff.int_val() - _pfnum` precisely because
they keep no cursor between calls. So the universal argoff push is not
compensating for a *missing* traversal — the traversal exists — it exists because
the post-eval accessors are anchor-relative by construction. The accessor O(n²)
is removable by migrating to the single-descent cache pattern above, independent
of whether the push is later narrowed to post-eval-only.

## 6. Broadcast documents the discovery-order hazard

`strmfunc.c:200-212` (broadcast literal construction) contains an in-source
comment recording the discovery-order hazard directly: `skip_arg_in_expr` walks
the buffer **backward**, so positional `pi=0` is the *last* source arg, while
`copy_post_eval_expr`'s tokbuf is in **forward** order. A naive running
`elem_offset` is correct only when all positionals are the same width and
silently wrong for mixed-width elements like `(1 (2 3))`. The code fixes this by
collecting `possizes[pi]` and doing a reverse-accumulation pass (lines 213-228).

Takeaways:
- The mixed-width / discovery-order bug is real; any rewrite must preserve the
  reverse-accumulation rather than regress to a running offset.
- It confirms `nargsfixed()` is the right positional count here (line 168-169
  comment), consistent with §1.
- `possizes` is `new int[npositionals]` only when `npositionals>0`; the reverse
  loop and `delete[]` are guarded correctly, but the `npositionals==0` path (pure
  keyword call) should be confirmed to leave `nelem`/`elem_offset` sane.

## 7. How stream literals are laid out in the postfix

Reading `postfix()` of a literal shows the structure directly:

- `postfix((0 1 ... 39))` → 29 element tokens **in source order**, then a single
  `stream[29|0|1]*` command token. Elements are stored contiguously, forward;
  nothing needs to be searched for to *find* them.
- `postfix((... 100*4/33 ... a=0*3000 ... 3*xxx++ ...))` shows compound elements
  (`100 4 mpy 33 div`, `a 0 3000 mpy assign*`) sitting flush among bare ints. So
  **element stride is variable, not 1** — element boundaries are an arity-balance
  property, not a fixed position. Splitting the buffer into elements is a
  paren-matching / stack-depth computation (what `skip_arg` does), not a pattern
  match.
- The trailing `*` on a command token marks it **post-eval** (e.g. `assign*`,
  `stream*`). This is how post-eval status is read off the postfix.

## 8. Stream-literal representation and consumption (as built)

`StreamLiteralNextFunc::execute` (`strmfunc.c:834-924`), `execute_literal`, and
the `NextFunc::execute_impl` driver (`strmfunc.c:525`) together implement lazy
stream literals as follows.

### Construction: reverse-built directory, traversal paid once

`execute_literal` copies the whole literal's token region once into a `FuncObj`
(`copy_post_eval_expr(total, offtop)` → `FuncObj(tokbuf, total)`,
`strmfunc.c:178-182`), then builds a **directory** over it in a single backward
pass (the `possizes` + reverse-accumulation, 213-228). The directory is an
`AttributeValueList`:

```
[0]    FuncObj(tokbuf, ntoks)        -- the copied token buffer
[1]    nremaining                    -- starts at element count
[2..]  per-element entries           -- positional = (offset,count) = 2 slots;
                                         keyword = KeywordType marker [+ off,count]
```

The variable-stride work (§7) is therefore done **once**, at construction, and
stored. This is the key cost decision: you pay the postfix traversal either once
(reverse-build the directory) or as a triangular per-element cost at consumption
time. Paying it once, against the token copy you are already making, is the right
trade.

The FuncObj copy is also what decouples the **parser rate** from the lazy
consumer: the parser fills the buffer ahead of consumption and moves on, so a
lazy stream cannot alias the live parse region. The copy is the bounded, once-up-
front buffer the cursor drains; it is not a live producer/consumer queue. Because
every stream literal owns its FuncObj copy, the stream never aliases the volatile
REPL `_pfbuf`, so lifetime needs no escape-on-assignment trigger — the copy is
paid once, uniformly, at construction.

### Consumption: cheap head removal to exhaustion

`next()` evaluates one element by handing its precomputed `(offset,count)` span to
`comterpserv()->run(tokbuf + offset, cnt)` (line 913) — the evaluator runs the
element's tokens in place. Because each span is precomputed, `next()` cannot land
mid-subtree; the boundary invariant is established once at construction, not
recomputed per call.

The directory **diminishes**: each `next()` removes the consumed front entries
and decrements `nremaining` (lines 874-922). Removal of a positioned element is
O(1) (`AttributeValueList::Remove(ALIterator&)` splices a node), and the live
front stays shallow, so the per-call cost is constant. After each removal the
code re-navigates from the list head (`First(); Next(); Next()`) rather than
holding an iterator across calls. This re-navigation is intentional and correct:
it is O(few) because removal keeps the front shallow, and — crucially — it never
carries a live iterator across the lazy boundary, where an interleaved consumer or
a fork could invalidate it. A saved iterator would work in the single-consumer
case and corrupt silently under interleaving, which is exactly the cross-product /
fork scenario streams must support. Cheap-and-robust is preferred over
cheaper-and-fragile.

### Copy forks at the current position

Because consumption physically removes entries, the remaining directory *is* the
stream's position. `$$s` / `stream(s)` therefore copies a stream at its current
state of consumption: it deep-copies the **remaining** directory (the
`AttributeValueList` copy ctor does `Append(new AttributeValue(...))` per entry,
`attrlist.c`), giving the fork an independent diminishing directory, while the
FuncObj token buffer at `[0]` is shared. Draining one fork does not disturb the
other. This is why **removal is the correct design** rather than an immutable
directory plus a cursor index: under removal, "copy at current position" is just a
deep copy of what remains; an immutable-directory design would instead have to
copy a cursor value, a different mechanism. The two are the same decision viewed
from opposite ends, and copy-at-current-position is the one the stream contract
requires.

(For the FuncObj at `[0]` to be safely shared across forks rather than leaked or
double-freed, it must be reference-counted. See the ObjectType Resource-by-contract
work, which makes every ObjectType payload — FuncObj included — a `Resource`.)

`stream-info.comt` (tests 7-10) exercises this end to end: the raw directory
shrinks on `next()`, exhaustion yields nil, and a mid-stream `$$` fork continues
from the current position independently.

## 9. Stale-doc fixes to make alongside

- `comvalue.h` `nids()` "(not used)" comment — it is used (`comterp.c:297`).
- HACKING `## nargspost() vs nargsfixed()` documents only the funcstate family.
  Add the ComValue token family (`narg/nkey/nids/pedepth`) and state plainly that
  the *walk* reads the token family while `execute()` bodies read the funcstate
  family.
