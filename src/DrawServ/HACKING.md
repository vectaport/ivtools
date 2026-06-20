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

## See Also

- `src/ComTerp/HACKING.md`
- `src/DrawServ/ARCHITECTURE.md`
