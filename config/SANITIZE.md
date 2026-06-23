# Building ivtools with AddressSanitizer

AddressSanitizer (ASan) instruments every memory access and faults *at* the
offending instruction, with a backtrace and the allocation site.  It catches
heap-buffer-overflows, use-after-free, and similar — the kind of corruption that
is invisible to `lldb` and macOS Guard Malloc (both perturb the heap layout
enough to hide it) but shows up far away as a crash or wrong result.

It is not wired into the build by default (it roughly doubles build time and
slows the program); you turn it on for a debugging session and turn it back off.

## How the flag has to be threaded through the build

ASan needs `-fsanitize=address` on **three** kinds of command line:

1. **compile** of every translation unit whose accesses you want checked,
2. **link of the program** (`a.out`) — to pull in the ASan runtime,
3. **link of each instrumented shared library** (`.dylib`).

Two gotchas specific to this imake build:

- Use the **`OTHER_*`** flag hooks in the Imakefile, **not `EXTRA_*`**.  The imake
  config (`config/params.def`) re-defines `EXTRA_CCFLAGS` / `EXTRA_CCLDFLAGS`
  after the Imakefile body, so an `EXTRA_*` assignment in an Imakefile is
  silently overridden (you'll see the default `-Wl,-bind_at_load` in the
  generated `DARWIN/Makefile` instead of your flag).  `OTHER_CCFLAGS` /
  `OTHER_CCLDFLAGS` win, and both still feed `CCFLAGS`/`CCLDFLAGS`, so nothing
  the build needs is lost.

- The shared-library (`-dynamiclib`) link rule does **not** use the compile or
  link flag variables — only `$(DYLDLIBS)`.  So the dylib link only gets ASan if
  you add it to the rule itself: `SharedLibraryCmdDarwin` in
  `config/site.def.DARWIN`.  (Putting it on the *compiler-name* `CCDriver` also
  works because the dylib rule expands `CCDriver`, but that's abusing the
  compiler-name variable for flags — don't.)

## Recipe

Instrument the program directory **and** every library directory holding code
you want checked.  For a bug in `libComTerp` driven from `comterp`, that is
`src/comterp_` plus `src/ComTerp` (add `src/ComUtil`, `src/Attribute`, … if the
suspect write could be there).

1. In each library Imakefile (`src/ComTerp/Imakefile`, …), add:

       OTHER_CCFLAGS = -fsanitize=address

   In the program Imakefile (`src/comterp_/Imakefile`), add both:

       OTHER_CCFLAGS = -fsanitize=address
       OTHER_CCLDFLAGS = -fsanitize=address

2. In `config/site.def.DARWIN`, add `-fsanitize=address` to both
   `SharedLibraryCmdDarwin` definitions, right after `CCDriver`:

       ...CCDriver -fsanitize=address $(DYLDLIBS) -dynamiclib...

3. Regenerate the makefiles and rebuild each touched dir (clean, so every object
   is recompiled with the flag):

       cd src/ComTerp/DARWIN  && make Makefile && make clean && make
       cd src/comterp_/DARWIN && make Makefile && make clean && make

   Confirm it took: `otool -L a.out | grep asan` should list
   `libclang_rt.asan_osx_dynamic.dylib`.

4. Run against the freshly built (uninstalled) libraries:

       ASAN_OPTIONS=detect_leaks=0 \
       DYLD_LIBRARY_PATH=src/ComTerp/DARWIN:src/Attribute/DARWIN:src/ComUtil/DARWIN \
       src/comterp_/DARWIN/a.out run yourscript.comt

   On a hit ASan prints `ERROR: AddressSanitizer: heap-buffer-overflow ...` with
   the faulting line, the size and direction of the overrun, and the buffer's
   allocation site.

## Turning it back off

Revert the three edits and rebuild:

    git checkout src/comterp_/Imakefile src/ComTerp/Imakefile config/site.def.DARWIN
    cd src/ComTerp/DARWIN  && make Makefile && make clean && make
    cd src/comterp_/DARWIN && make Makefile && make clean && make

`otool -L a.out | grep asan` should now print nothing.

## Worked example

This is how the `ComTerpServ::load_string` overflow was found: a multi-KB input
line (see `src/comterp_/tests/bigbuf.comt`) ran `comterp` straight off the rails
in `ComValueTable::find` — but only intermittently, and never under `lldb` or
Guard Malloc.  ASan named it immediately:

    ERROR: AddressSanitizer: heap-buffer-overflow ... WRITE of size 1 ...
        #0 ComTerpServ::load_string(char const*) comterpserv.c:133
      0 bytes after 1024-byte region ... allocated by ... comterpserv.c:88

i.e. `load_string` writing past the fixed `_instr` buffer — the line the crash
was nowhere near.

## Worked example 2 — a use-after-free that needs real timing

The `ComTerpServ::runfile` reactor-reentrancy UAF (the script interpreter freed
out from under itself when stdin hits EOF inside an `update()` poll's
`handle_events()`) only reproduces with drawserv's real timing — pure-comterp
poller reproducers never hit the race.  The trick is to **force** the race:
run the `drawmo` orchestrator in listen mode with **stdin closed** so the
reactor dispatches the fd-0 EOF during the very first `update()` poll.

Because `drawmo` shells out to a (non-instrumented) `drawserv`, that child must
still resolve the ASan symbols in the instrumented dylibs it loads, so add
`DYLD_INSERT_LIBRARIES` pointing at the ASan runtime:

    ASAN_OPTIONS=detect_leaks=0:abort_on_error=0 \
    DYLD_LIBRARY_PATH=src/ComTerp/DARWIN:src/Attribute/DARWIN:src/ComUtil/DARWIN \
    DYLD_INSERT_LIBRARIES=$(dirname $(otool -L src/comterp_/DARWIN/a.out \
        | awk '/asan/{print $1}'))/libclang_rt.asan_osx_dynamic.dylib \
    src/comterp_/DARWIN/a.out listen 10002 \
        src/drawserv_/tests/drawmo 10002 --tests gsexim < /dev/null

Pre-fix this aborts within the first poll; ASan names it exactly:

    ERROR: AddressSanitizer: heap-use-after-free ... comterp.c:932 in ComTerp::push_stack
      freed by ... ~ComTerpServ() <- ComterpHandler::destroy() comhandler.c:108
              <- handle_close <- handle_events <- UpdateFunc::execute ctrlfunc.c:461
      previously allocated by ... ComterpHandler::ComterpHandler <- main.c:185

Post-fix (the `running()` bracket restored in `ComTerpServ::runfile`) the same
command runs clean — the stdin-EOF is deferred instead of freeing the live
interpreter.
