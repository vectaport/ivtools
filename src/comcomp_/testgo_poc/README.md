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
3. Feeds the captured value back into a ComTerp ring
   (`feed(string(n AnyType))`, built via the same bridge before the
   flowgraph runs) and reads it back out with `next()`, to confirm it
   landed. An `AnyType` ring boxes a value whole rather than converting
   it (`src/comterp_/tests/feedring.comt` test 30), so it can hold
   whatever came back from the flowgraph unchanged.
4. Compares what landed in the ring against the value ComTerp produced
   directly, as ground truth.

## Building and running

Same prerequisite as `bridge_poc`/`compile_poc` (a built ivtools tree).
Needs Go >= 1.26 (flowgraph's own requirement); `go build`/`go run` will
fetch that toolchain automatically if it isn't already installed.

    cd src/comcomp_/testgo_poc
    go build -o testgo_poc .
    ./testgo_poc            # defaults to the expression "42"
    ./testgo_poc '100+23'

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
- Every value still crosses the Go/C++ boundary as printed text via
  `comterp_bridge_eval`, both pulling the original value out and pushing
  the result back in. The next step discussed is a direct accessor onto
  a ring's backing buffer (`ComTerp::localvalue()` +
  `AttributeValue::stream_list()`'s `[buf,head,tail,count,wrap]` AVL,
  per `strmfunc.h`) so Go can read/write ring slots in shared memory
  instead of going through text each time -- safe without a lock under
  a single-writer guarantee, though the head/tail counters still need
  atomic/volatile access across the language boundary.
