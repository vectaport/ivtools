# DrawServ Test Suite

See `src/comterp_/tests/TESTING.md` for the full coverage taxonomy,
scoring methodology, and scripting conventions. This document covers
only what is different or additional at the DrawServ layer.

## What is Different Here

DrawServ tests are **integration tests**, not unit tests. They require:

- A working X11 display (drawserv opens a window on startup)
- `drawserv`, `comterp_listen`, and supporting binaries on `PATH`
- `lsof` or `nc` for port probing (falls back gracefully)
- Free TCP ports in the 20000+ range

Tests are not `.comt` scripts run directly under `comterp`. They are
run via the `drawmo` orchestrator, which is itself a ComTerp script
(`#! /usr/bin/env comterp_listen`) that launches live drawserv
instances, communicates with them via `remote()` over TCP, and shuts
them down. Each test function manages its own process lifecycle.

## Running Tests

```bash
# run all tests
./drawmo

# run a specific test
./drawmo --tests updown

# run multiple tests
./drawmo --tests updown,updown1

# show help
./drawmo --help
```

`drawmo` exits 0 on full pass, 1 on any failure. All test progress and
failure messages go to stderr; the exit code is the CI signal.

### Runs in GitHub Actions CI (headless, under xvfb)

The hosted CI (`.github/workflows/ci.yml`) runs the full suite under xvfb and
blocks on it. This was thought to need a real X server, but the blocker turned
out to be networking, not the GUI: drawmo dialed `"localhost"`, which on the
runner resolves to IPv6 `::1` first, while the comterp/drawserv acceptors bind
IPv4 only (`ACE_INET_Addr(port)` → `INADDR_ANY`). Every callback hit
`ECONNREFUSED`, so the child `drawserv` looked like it "never called back" when in
fact it had started fine. The harness now uses `127.0.0.1` literals (no resolver
in the path), so the child maps its window under xvfb and calls back over IPv4
loopback exactly as on a real display.

The deeper fix -- have the listener accept IPv6 too (dual-stack), so `"localhost"`
works regardless of resolver order -- is ACE-build-dependent (`ACE_HAS_IPV6`) and
is left to the in-tree ACE-lite work (issue #147). Until then, distributed
ivtools across IPv6-preferring hosts should use IPv4 literals.

## Port Convention

drawmo listens on port 10002 for callbacks from drawserv instances.
drawserv instances start at port 20002, stepping by 10000 per instance
(20002, 30002, ...). `find_open_port` checks that both `ds_port` and
`ds_port-1` (the import port) are free before using them.

## Infrastructure Functions

These are defined at the top of `drawmo` and shared across all tests:

`find_open_port(:base_port n)` — scans upward from `base_port` in
steps of 10 until both `ds_port` and `ds_port-1` are free.

`kill_port(:kill_port_num n)` — kills the process listening on port n,
using `lsof` if available, falling back to `fuser`.

`updown` launch pattern — each test uses the same structure:
1. Find a free port pair
2. Set `global(drawserv_up)=false`
3. Launch drawserv with `-runexpr` that calls back to drawmo on
   `callback_port` to set `global(drawserv_up)=true`
4. Spin in `update(poll_usec)` until the callback fires or timeout
5. Connect via `socket()`, run assertions via `remote()`
6. Shut down with `remote(sock "exit" :nowait)` and `close(sock)`

The spin loop uses `update()` rather than `usleep()` so drawmo's own
ComTerp event loop keeps processing incoming connections during the wait.

## What `remote()` Does, and Why Shutdown Has Three Steps

`remote(sock cmd)` writes the command plus a newline, then reads **one
newline-terminated line** back, one byte at a time, and — unless you pass
`:str` — runs that line through the *local* interpreter and returns its value.
One line per call, no more, no less.

So a command that answers with exactly one line can be asked on any socket.
`size(select())` and `grid(:table)` are such commands. `:nowait` skips the read
entirely, which is right only when nothing is coming back, or when something
else is reading that socket.

The shutdown sequence in step 6 is three things and needs all three:

1. `remote(sock "exit" :nowait)` — `exit` sends **no reply**, so a blocking
   `remote()` on it waits for a line that never comes.
2. `close(sock)` — after the peer exits, this end sits in `CLOSE_WAIT` still
   holding the port.
3. `kill_port()` only for one that did not go. It kills **by port**, and
   `lsof -ti` lists every process holding that port, this end included — so
   killing a port you are still connected to kills the test itself.

Shut down by asking, not by killing; check with `pgrep -f '[d]rawserv -comdraw
<port>'` and kill only what stayed. Killing by port while connected is silent:
the test dies with SIGTERM and reports nothing, which reads exactly like a hang.

## Test Inventory

### updown

**What:** Launch a drawserv, verify it responds to `sid()`, shut it down.

**Checks:**
- drawserv starts and calls back within 60 seconds
- `socket()` connects successfully
- `remote(sock "sid()")` returns a non-blank value

**Purpose:** Smoke test. If this fails, nothing else will work.

### updown1

**What:** Launch a drawserv, verify the initial state of its tables,
shut it down.

**Checks:**
- drawserv starts and calls back
- `remote(sock "drawlink(:table)")` returns an empty list (`size==0`)
- `remote(sock "sid(:table)")` returns a list with exactly 1 entry
  (the freshly-opened empty drawing)

**Note on serialization:** `sid(:table)` returns a list of attrlists.
For a freshly-launched drawserv with one drawing, that is a singleton
list serialized as `{(:key ... :sid ...),}` — the trailing comma is
required for the remote parser to reconstruct it as a list rather than
a bare attrlist. This relies on the `comvalue.c` singleton list
trailing-comma fix and the attrlist-in-list parenthesization fix. The
assertion is `size(sidtable)==1`.

**Purpose:** Verifies that a freshly-launched drawserv has a clean
drawlink table and exactly one sid entry. This is the baseline against
which connected-peer tests will diff.

### sel

Two spokes on a hub, which is the arrangement where an answer between spokes is
not delivered by the node that produced it. Spoke 1 draws and holds a graphic;
spoke 2 reaches for it and must be **refused**, with the refusal routed home
across the hub; spoke 1 lets go and spoke 2 reaches again and must be
**granted**, likewise across the hub. Before the refusal named its asker it went
back one hop and stopped, leaving the spoke that asked stuck in
`WaitingToBeSelected` with no retry to rescue it — a request is only made from
`NotSelected`.

## agreetest — counting runs rather than trusting one

`agreetest` is not a drawmo test and is run on its own:

```
./agreetest --runs 10 --kids 4
```

It runs the same scenario n times and tallies how many came out consistent:
every node holding the one graphic it drew, every node seeing all of them. The
four-node case answers differently to identical input, so a single run of it will
tell you a change fixed something or broke something when it did neither — a
claim about anything distributed should move the tally rather than produce one
good run. It went 0 of 4 to 8 of 8 across one fix, and that number is what
located the bug.

It is shell rather than comterp, unlike everything else here. Driving several
drawservs and reading answers back from each is worth getting right in a demo,
but in a test whose whole job is to be trusted it is one more thing that can be
wrong about the test rather than about drawserv.

## What Belongs Here vs comterp_/tests

Functions registered only in DrawServ (`sid()`, `drawlink()`,
`select()` with `:lock`/`:unlock`, distributed brush/color commands)
must be tested here — they are unknown to `comterp`.

Serialization round-trips that only need `print(:str)`/`run(:str)` can
be tested in `src/comterp_/tests/` without drawserv. Use the DrawServ
layer only when the test requires a live drawing session, TCP
communication, or a DrawServ-specific command.

## Adding a New Test

1. Write a `funcname_test=func(...)` following the updown launch pattern above,
   in **`updown.comt`** (link/propagation tests) or **`gstests.comt`**
   (graphic-state tests). `drawmo` itself keeps only the arg parsing, the shared
   helpers (`kill_port`, `find_open_port`, `grid_has_id`), the two `run()` loaders
   that pull in those files, and the dispatch block. (The loaders are separate
   top-level statements with no trailing `;` on purpose: a func binds its symbol
   only when evaluated, but a caller resolves that symbol at parse time, so each
   file must finish loading before the dispatch is parsed.)
2. Add it to the `switch` in the argument parser:
   `:tests_flag tests_flag=true; ...`
3. Add it to the `if(t==\`name || t==\`all ...)` dispatch block
4. Document it in this file under Test Inventory
5. Update the usage `print()` strings at the top of `drawmo`

Keep each test function self-contained — it finds its own port, launches
its own drawserv, and cleans up on both pass and fail paths.
