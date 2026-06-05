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

**Purpose:** Verifies that a freshly-launched drawserv has a clean
drawlink table and exactly one sid entry. This is the baseline against
which connected-peer tests will diff.

## What Belongs Here vs comterp_/tests

Functions registered only in DrawServ (`sid()`, `drawlink()`,
`select()` with `:lock`/`:unlock`, distributed brush/color commands)
must be tested here — they are unknown to `comterp`.

Serialization round-trips that only need `print(:str)`/`run(:str)` can
be tested in `src/comterp_/tests/` without drawserv. Use the DrawServ
layer only when the test requires a live drawing session, TCP
communication, or a DrawServ-specific command.

## Adding a New Test

1. Write a `funcname_test=func(...)` in `drawmo` following the updown
   launch pattern above
2. Add it to the `switch` in the argument parser:
   `:tests_flag tests_flag=true; ...`
3. Add it to the `if(t==\`name || t==\`all ...)` dispatch block
4. Document it in this file under Test Inventory
5. Update the usage `print()` strings at the top of `drawmo`

Keep each test function self-contained — it finds its own port, launches
its own drawserv, and cleans up on both pass and fail paths.
