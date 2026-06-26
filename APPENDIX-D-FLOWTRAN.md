## Flowtran: Flowgraph Composition Language *(reserved, ivtools-4.0)*

ComTerp is designed to evolve into a flowgraph composition language —
**flowtran** — that can drive the
[ipl simulator](https://ipl.sourceforge.net/ipl/IPL_Home.html) directly
(when linked) and export executable
[vectaport/flowgraph](https://github.com/vectaport/flowgraph) goroutines.

The flowtran syntax is reserved in the current parser. This section
documents the intended design.

### Concepts

A flowgraph is made up of **hubs** and **pipes**. A hub is a unit of
computation; pipes are the typed channels connecting hubs. This maps
directly to the `vectaport/flowgraph` Hub and Stream model and to the
ipl (pipe|invo),connector model.  What was called first pipe, then
invocation in ipl, became node and edge in vectaport/fgbase then Hub
and Stream in vectaport/flowgraph.  In ipl it was realized that the
nodes between edges were the place of buffering, so they were the
pipe holding value.  You would think the edges/Stream should be the
pipe but buffering is done at the node/Hub and edge/Stream is just
the instantaneous interconnect, the wire.

ComTerp streams (`..`, `**`, `$$`, `,,`) are a separate concept —
they are evaluation-time value sequences within ComTerp expressions.
Flowtran pipes are persistent communication channels between hubs in
a running flowgraph. The two levels interact: ComTerp stream algebra
is used to *construct* flowgraphs (replicating hubs and wiring pipes),
while the flowgraph itself executes asynchronously once assembled.

### Hub definition: `def()`

```
def(hubname[srclist][dstlist](hubbody))
```

Defines a hub type. `srclist` is the list of input pipes; `dstlist` is
the list of output pipes. `hubbody` is a ComTerp expression that runs
when the hub fires. A hub with no destinations has an empty `dstlist`.

By convention, input pipes are named `a`, `b`, `c` and output pipes
`x`, `y`, `z` in hub definitions — the Steve Johnson compiler tradition.
At invocation sites, pipes are given names that make sense from both
ends of the connection.

Def's with no body must link to a language primitive and are just there to
declare the API.   Otherwise def's can be a comterp expression that is a
FuncObj to be fired when AllOf or OneOf is satisfied on inputs arriving,
the result getting marshalled out on the destination list.  Or the body can
be an ongoing nested declaration of a flowgraph connected up to the streams
in the srclist and dstlist.  The primitives are flowtran's bottoming out,
the comterp expressions are the comterp programmer's way of bottoming out,
everything above that is a flowgraph.

Any side effects from an embedded comterp fragment would not fit the model 
so they will all be pre-evaluated down to the code fragment waiting for stream
input.  For example:

```
def(area[r][a](a=pi()*r*r))
```

will get evaluated at "compile" time, and symbols not in the stream lists will
be evaluated and stored in an internalized representation, i.e.:

```
area[r][a](a=3.141519*r*r)
```

A less useful but more illustrative example might be a hub that returned the
date it was compiled forever:

```
datecompiled[][d](d=print("%v" date :str))
```

which becomes:

```
datecompiled[][d](d=" 6-Jun-2026")
```

The two canonical Invocation Language primitives illustrate the full syntax:

```
def(ARBIT[a | b][x](x=a||b))        // OneOf input, single output
def(STRVAL[a b][x | y](if(a :then x=b :else y=b)))  // AllOf input, OneOf output
```

The `|` operator separates **firing groups** within a srclist or
dstlist. Space separates pipes within a group (AllOf — all must be
present). `|` separates groups (OneOf — any one group fires the hub).
The rule "no bare values after keywords begin" applies within each
group, as elsewhere in ComTerp.

```
def(sink[a][](print(a)))              // AllOf-1 input, no output
def(double[a][x](x=a*2))             // AllOf-1 in, AllOf-1 out
def(add[a b][x](x=a+b))              // AllOf-2 in, AllOf-1 out
def(merge[a | b][x](x=a||b))         // OneOf-2 in (either fires), AllOf-1 out
def(route[a b][x | y](if(a :then x=b :else y=b)))  // AllOf-2 in, OneOf-2 out
```

`|` between two sublists means the hub fires on whichever group
arrives — useful for multiplexing normal data flow with a heartbeat
or control path:

```
def(watchdog[data | heartbeat][x](x=data||heartbeat))
// fires on normal data OR on the watchdog pulse, whichever arrives
```

Pipes in `srclist` and `dstlist` can include keywords for configuration
values that are set at wiring time rather than flowing per-token:

```
def(scale[a :factor][x](x=a*factor))
```

### Hub invocation: `hubname[srclist][dstlist]`

```
hubname[srclist][dstlist]
```

Instantiates a hub and wires it into the flowgraph. The hub name leads
— you read the topology as *what* fires, then what flows in, then what
flows out. A stream symbol is created the first time it appears in any
srclist or dstlist and updated as both ends become connected.

The two NCL primitives invoked in a small flowgraph:

```
ARBIT[raw_a | raw_b][arbitrated]
STRVAL[direction arbitrated][to_x | to_y]
```

`arbitrated` is the stream connecting `ARBIT`'s output to `STRVAL`'s
input — named from the perspective of what it carries. `direction` is
a control input; `to_x` and `to_y` are the two steered outputs.
Reading top to bottom: arbitrate between two raw inputs, then steer
the result based on a direction signal.

A simple pipeline:

```
double[raw][scaled]
sink[scaled][]
```

`scaled` is created as `double`'s output and simultaneously wired as
`sink`'s input. Pipes are fully connected when both ends are bound.

### Range replication: `<<` and `>>`

`<<` and `>>` are added to the operator table as flowtran range
operators, used to replicate hub names and pipe names across a range.
Combined with ComTerp's `..` stream, they expand to indexed variants:

```
sink<<0..9>>=sink[input<<0..9>>][]
```

Expands to `sink0` through `sink9`, each wired to `input0` through
`input9`. The ten input pipes are left unconnected on their source
end — other hubs will wire into them. This is the `setbuf` pattern:
declare the pipes where they are consumed, let the producer side wire
to them later.

`<<` and `>>` apply to any identifier in a hub or pipe position:

```
add<<0..3>>[a<<0..3>> b<<0..3>>][sum<<0..3>>]
```

Creates four `add` hubs, each with its own `a`, `b`, and `sum` pipes.

### Relationship to existing reserved syntax

The following are already reserved in the current parser and will
be activated for flowtran:

- `[]` — hub srclist and dstlist delimiters
- `<>` — reserved (angle bracket template fill-in, complementing `<<`/`>>`)
- `<<` / `>>` — added to the operator table as flowtran range operators

It is hoped that not using `[` and ` ]` and `<<` and `>>` before now
in the public use of comterp will make any conflicts between comterp and
flowtran resolvable.


