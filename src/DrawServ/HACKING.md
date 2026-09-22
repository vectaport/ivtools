## Adding a New DrawServ Distributed Command

DrawServ distributes graphic state changes (brush, color, font, etc.) to
connected instances by generating a ComTerp script that the remote
interpreter evaluates.  The mechanism uses the `DrawServCmd` mixin:

### Pattern

Any command that needs distributed propagation inherits from both the
base Unidraw command class AND `DrawServCmd`:

```cpp
class LinkBrushCmd : public BrushCmd, public DrawServCmd {
public:
    virtual const char* dist_script();
protected:
    std::string _dist_script_buf;
};
```

`DrawServCmd` is a pure mixin in `src/DrawServ/linkcmd.h`:

```cpp
class DrawServCmd {
public:
    virtual const char* dist_script() = 0;
    // return ComTerp script to distribute, or empty string if none.
};
```

### dist_script() structure

The generated script follows a fixed pattern — save the current
selection, unlock and reselect the LocallySelected graphics, apply
the command, re-lock:

```
s=select();select(grid("uuid1"),grid("uuid2") :unlock "XXXXXXXX");
brush(pat,width);
select(s :lock "XXXXXXXX")
```

The session key (`%08X` of `drawserv->sessionidkey()`) gates the
unlock/lock so only the originating instance's selection is affected.

### Implementation steps

1. **drawclasses.h** — add a new class ID, incrementing from the last:
   ```cpp
   #define LINK_BRUSH_CMD      9806
   #define LINK_COLOR_CMD      9807   // next in sequence
   ```

2. **linkcmd.h** — declare the new class inheriting from both the base
   command and `DrawServCmd`.  Include the base command's header.

3. **linkcmd.c** — implement constructors, `dist_script()`, `Copy()`,
   `GetClassId()`, `IsA()`.  The `dist_script()` body:
   - Guard on editor, selection, and linklist presence
   - Iterate `LinkSelection::LocallySelected` comps, collect grid UUIDs
   - Build the select/unlock/command/lock script into `_dist_script_buf`
   - Return `_dist_script_buf.c_str()`

4. **DrawServ::ExecuteCmd()** — call `dist_script()` when the command
   is a `DrawServCmd` and distribute the script to all connected links
   via `SendAllHandler`.

### Wire protocol model

`dist_script()` returns a ComTerp expression string.  The receiving
instance evaluates it in its own interpreter — the REPL IS the wire
protocol.  This means the script must be valid ComTerp and must
produce the same visual result as the local command.

## Linkfreeze Protocol

Before finalizing a link formation (`DrawLinkFunc::execute()` in
`drawfunc.c`), a node floods a "freeze" request across its own
already-established (`two_way`) links and waits for every neighbor to
echo an ack, proving its local fragment is quiescent, then runs
`cycletest()` against that now-guaranteed-stable view. Without this, two
nodes closing links into the same fragment concurrently can each see a
locally-stale, no-cycle answer and jointly close a cycle that neither
could see forming alone (issue #575). `DrawServ::freeze_fragment()` /
`unfreeze_fragment()` / `freeze_request_handle()` / `freeze_ack_handle()`
/ `freeze_release_handle()` (`drawserv.c`) implement the flood; see their
header doc comments in `drawserv.h` for the wave/echo mechanics (a
parallel, breadth-first request flood down an implicit spanning tree,
with acks climbing back up the same tree).

**Instant/non-blocking against peers.** A node already holding a freeze
declines an incoming dial (`one_way`) outright rather than waiting for
its own hold to clear — waiting would risk a symmetric deadlock against a
peer doing the same wait for the same reason.

**Retry against itself.** The one exception is an outgoing dial
(`new_link`) finding its own freeze already active: that's virtually
always its own accept of some other, unrelated inbound link, still
pending that link's own two_way confirmation — not a peer to deadlock
against, just this node's own event loop settling itself. So instead of
declining, it pumps the reactor briefly and retries, bounded, before
giving up.

**Deferred release on accept.** An accepting node's freeze only actually
resolves once that link reaches `two_way`, a later, separate message —
releasing right after cycletest would thaw the node while that decision
is still uncommitted. The hold is carried on the `DrawLink` itself
(`pending_freeze()`/`has_pending_freeze()`/`clear_pending_freeze()` in
`drawlink.h`) and released when that message arrives, or by `linkdown()`
as a safety net if the link is torn down first (including cleaning up a
relay's `_freeze_parent`/`_freeze_sent_to` bookkeeping if the link that
dies was playing one of those roles in a freeze this node is currently a
party to — `DrawServ::freeze_link_down()`).

## Redundant-Link Tie-Breaking

`DrawServ::sessionid_register_handle()` (`drawserv.c`) detects a redundant
link when a peer's session id arrives already on record via a different
link -- both links lead to the same peer, so one is superfluous. A sid
echoed back to its own originator is not itself a cycle (propagation
reaching around a non-redundant path to its source is normal); it only
means the *link* is redundant if that link's own peer is reachable some
other way too, exactly like any other sid.

Which of the two redundant links gets benched can't go by arrival order:
the two nodes on either end of either link decide independently and can
each see a different one arrive first. Comparing `linkid()` instead works
network-wide, not just for the one pair that triggered the check -- a
link's initiator mints its linkid once and the far end copies it
unchanged, so every node comparing the same pair of links, anywhere in
the fragment, reaches the identical answer. The one safety exception:
never bench a node's own last active link over this comparison. Leaving
both connections up costs a little redundant broadcast traffic once;
losing a node's only path out is a real disconnection.

## See Also

- `src/ComTerp/HACKING.md`
- `src/DrawServ/ARCHITECTURE.md`
