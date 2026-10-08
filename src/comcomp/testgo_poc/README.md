# testgo_poc

Proof of concept that a ComValue can round-trip through a live
[flowgraph](https://github.com/vectaport/flowgraph) graph and back into
ComTerp, carried as Go's `any` across every hub and pipe on the Go side.

Given a `.comt` expression, this:

1. Evaluates it in a real ComTerp instance via the same `extern "C"`
   bridge shim `bridge_poc`/`compile_poc` use (`shim.h`/`shim.cc`,
   copied in here so this POC builds standalone).
2. Parses the printed result as an int64 (only primitive family wired up
   so far) and pushes it through a flowgraph: an `Array` hub streams it
   once, an identity `OneOf` hub passes it through unchanged, a `Sink`
   hub's `Sinker.Sink` captures it -- `any` the whole way, no
   stringifying between hubs.
3. Writes the captured value directly into the backing memory of a
   ComTerp ring (`feed(string(n AnyType))`, built via the bridge before
   the flowgraph runs) -- no `feed()` call, no text crossing the bridge
   for this leg. `comterp_bridge_ring_info()` (`shim.h`/`shim.cc`) hands
   Go the ring's base pointer plus live `head`/`tail`/`count`/`wrap`
   pointers (`AttributeValue::int_ref()`'s own storage, not copies), and
   `comterp_bridge_encode_int()` packs a value into one slot's 40-byte
   `AnyType` chunk the same way `ComValue::comval_encode()` does
   internally (`comvalue.c`) -- so a slot Go writes is indistinguishable
   from one `feed()` wrote. `directPushInt()` (`main.go`) then applies
   the same fullness check and wraparound `ring_push_elt()`
   (`strmfunc.c`) does, just from Go against those live ints.
4. Reads the ring back out with `next()` (through the bridge, as an
   independent check) and compares it against the value ComTerp produced
   directly, as ground truth.

## Building and running

Same prerequisite as `bridge_poc`/`compile_poc` (a built ivtools tree).
Needs Go >= 1.26 (flowgraph's own requirement); `go build`/`go run` will
fetch that toolchain automatically if it isn't already installed.

    cd src/comcomp/testgo_poc
    go build -o testgo_poc .
    ./testgo_poc            # defaults to the expression "42"
    ./testgo_poc '100+23'
    ./testgo_poc '7-50'     # negative values round-trip too

## What this doesn't cover yet

- Only int64 has a Go-side representation; no float/string/symbol/etc.
  The design question (see thread discussion) is making the union of
  types flowgraph's `any` carries and what ComTerp's `AnyType` ring can
  box fully inclusive of each other, not just proving one.
- The flowgraph hub (`OneOf` + an identity `Transformer`) has to be used
  instead of the `Pass` HubCode: `Pass`'s generic nil-FireFunc fallback
  (flowgraph.go) copies values through without recognizing EOS, so a
  `Pass` hub's goroutine never terminates and `fg.Run()` hangs forever.
  `OneOf`'s fire func (`oneOfFire`) does that EOS recognition itself.
- Pulling the original value out of ComTerp, and reading the ring back
  out afterward to verify, still cross the boundary as printed text via
  `comterp_bridge_eval` -- only the push leg is direct so far. The ring
  lookup is by top-level variable name (`ComTerp::localvalue()`); nothing
  stops Go from polling `*info.count` instead of calling `next()` to
  read a slot directly too, once decode coverage matches encode.
- `comterp_bridge_encode_int`/`comterp_bridge_decode_int` only cover
  `IntType` -- same single-primitive-family scope as `compile_poc`'s
  native arithmetic. Growing this to the rest of `AttributeValue`'s
  union (`attrvalue.h`) is the union-of-types work the thread discussed.
- The current single-writer story is "Go is the only pusher in this
  program" -- true here because nothing else touches the ring, not
  because of any lock. `*info.tail`/`*info.count` are plain `int`s
  written through C pointers with no atomics; a real concurrent
  single-writer/single-reader split (ComTerp's own `feed()` running on
  one goroutine's assumption while Go reads, or vice versa) would need
  to revisit that, same as any lock-free SPSC ring.
