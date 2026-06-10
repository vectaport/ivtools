# Drum Streams Notation (DSN)

A lightweight, space-separated text system for writing drum grooves as time streams.

Each token represents one time slot (step).  
Spaces are required and meaningful.

---

## 1. Time Grid

```
.   metronome tick (always present, base grid)
```

---

## 2. Hi-Hat System

```
:   closed hi-hat (default groove layer)
^   open hi-hat
v   close hi-hat (transition / action)
```

Default assumption: closed hi-hat unless modified.

---

## 3. Core Drum Voices

```
*   kick
x   snare
o   high tom
0   mid tom
@   floor tom
```

---

## 4. Cymbals

```
+   crash
=   ride
```

---

## 5. Cymbal Accents (combined hits)

```
$   kick + cymbal (crash or ride)
!   snare + cymbal (crash or ride)
```

---

## 6. Combined Rules

A single token may represent multiple simultaneous actions.

```
X   kick + hi-hat
S   snare + hi-hat
```

Where:
- X = * + :
- S = x + :

---

## 7. Time Behavior Rules

- Each token is one fixed time step
- Spaces separate time steps
- Tokens are atomic events or clusters
- . always represents the metronome tick

---

## 8. Example: Basic Rock Groove

```
X . S . X . S .
```

---

## 9. Example: Expressive Hi-Hat Motion

```
X : S ^ X v S :
```

Meaning:
- : = closed hi-hat groove
- ^ = open hi-hat
- v = close gesture
- . = metronome pulse

---

## 10. Example: Cymbal Accents

```
$ . S . X . ! .
```

---

## 11. Example: Fill

```
X . x o 0 @ ! .
```

---

## 12. Design Philosophy

- . = metronome tick (time skeleton)
- : = hi-hat groove layer
- * x = core kit
- ^ v = hi-hat motion
- $ ! = cymbal-driven accents
- space = time step separator

---

## 13. Optional Extensions

```
|   bar separator
~   ghost note modifier
```

