# ComUtil Architecture

## Origins

ComUtil was written in March-April 1989 at Triple Vision, Inc. under a
Small Business Innovation Research (SBIR) grant. The original authors
were Scott E. Johnston and Robert C. Fitch. The work was subsequently
transferred to Vectaport Inc. and has been maintained there since 1993.

The goal of the SBIR work was a portable toolkit for building embedded
command interpreters with compiler-quality parsing, extensible operator
tables, robust error reporting, and a memory manager suitable for systems
without full Unix memory management. The complete compiler chain —
lexical scanner, parser, and postfix code generator — was designed from
the start to be embeddable in C applications without OS dependencies.

## Compiler Chain

The ComUtil compiler chain follows the Fischer/LeBlanc model (see
*Crafting a Compiler*, Fischer and LeBlanc, 1988, written at UW-Madison):

### Lexical Scanner (`_lexscan.c`)

`lexscan()` tokenizes input into a stream of typed tokens: identifiers,
integers, floats, strings, characters, operators, and keywords. It is
driven by a function-pointer interface (`infunc`, `eoffunc`, `errfunc`,
`outfunc`) so the input and output sources are fully pluggable — the same
scanner works against a FILE*, an in-memory string, a socket, or any
other source that provides the `fgets`-style interface.

The continuation prompt mechanism (`_continuation_prompt`,
`_continuation_prompt_disabled`) allows the scanner to emit a `> ` prompt
when more input is needed to complete a partially parsed expression,
making interactive use natural.

### Parser (`_parser.c`)

`parser()` implements a shunting-yard algorithm (operator precedence
parsing) that reads tokens from `lexscan()` and emits them in postfix
order into `_pfbuf`. Written April 1989, with significant extensions
in 1993-1995 for the ComTerp keyword argument mechanism and post-eval
depth tracking.

The output is a flat array of `postfix_token` structs (`_pfbuf`) — the
"compiled" form of the expression. Each token records its type, symbol
id, argument counts (`narg`, `nkey`, `nids`), and line number. This
postfix buffer is the boundary between the parser (ComUtil) and the
evaluator (ComTerp).

### Error System (`errsys.c`, `errfile.c`)

`err_open()`, `err_submit()`, `err_print()`, `err_clear()` — a structured
error reporting system that looks up human-readable error messages from
a format-string file (`.err`) using `#define ERR_xxx` macro constants.
This allowed error messages to be maintained separately from code, and
supported both immediate and deferred error reporting. Written March 1989.

### Symbol Table (`symbols.c`)

`symbol_add()`, `symbol_pntr()`, `symbol_find()` — a hash-based symbol
table mapping string names to integer ids and back. Written April 1989 by
Robert C. Fitch. Extended in 2010 for Wave Semiconductor use. The symbol
table is global and persistent for the lifetime of the process, making
symbol ids stable references usable in postfix tokens and ComValue objects.

## Memory Manager (`dmm.c`, `mblock.c`)

Dynamic Memory Manager — written by Robert C. Fitch, March 1989.

Designed for embedded systems without full Unix memory management, `dmm`
provides a portable heap allocator with `dmm_malloc()`, `dmm_free()`,
`dmm_realloc()`, and size-aware variants used by the parser to grow
`_pfbuf` as needed. The `mblock.c` companion provides block memory
allocation for fixed-size objects.

On modern Unix systems `dmm` wraps `malloc`/`free`, but the interface
allows it to be replaced with a fixed-pool allocator for embedded targets
where `malloc` is unavailable or undesirable.

## Extended Doubly-Linked List (`xdll.c`)

`xdllink` — an extended doubly-linked list supporting arbitrary node
structures. Written 1989. This data structure was designed to be the
foundation of a graph representation intended as the real target of the
postfix code generator — a data flow graph suitable for simulation.

The graph structure was never completed as part of the original SBIR work.
The postfix buffer (`_pfbuf`) became the de-facto "code generation" output,
feeding directly into the ComTerp evaluator rather than a graph simulator.

A real flow graph simulator was not built until 2005-2007, when the
`_parser.c` copyright was extended, and flow-graph execution was
implemented separately outside ComUtil.

## Operator Table (`optable.c`)

`op_table` — the operator precedence and associativity table. Defines
the built-in operators (`+`, `-`, `*`, `/`, `=`, `==`, etc.) with their
precedence levels, arity (unary/binary), and associativity. The shunting-
yard algorithm in `parser()` uses this table to determine output order.
New operators can be registered at runtime via `opr_tbl_insert()`.

## Pluggable I/O Interface

A key architectural decision is that all I/O in the compiler chain is
abstracted through function pointers:

```c
infunc  -- char* (*infunc)(char* buf, int n, void* ptr)  -- like fgets
eoffunc -- int (*eoffunc)(void* ptr)                     -- like feof
errfunc -- int (*errfunc)(void* ptr)                     -- like ferror
outfunc -- int (*outfunc)(const char* s, void* ptr)      -- like fputs
```

This makes the entire compiler chain embeddable in any C or C++ application
without modification — the calling application supplies the I/O functions
appropriate to its environment. In ComTerp this is used to plug in socket
I/O, in-memory string I/O, and stdin/stdout for different execution contexts.

## Relationship to ComTerp

ComUtil provides the parse half of the interpreter; ComTerp provides the
evaluate half. The boundary is `_pfbuf` — the postfix token array produced
by `parser()` and consumed by `ComTerp::load_sub_expr()`. ComUtil knows
nothing about ComTerp's evaluation model; ComTerp knows nothing about how
the postfix was produced. This clean separation means ComUtil could in
principle feed any postfix evaluator, not just ComTerp.

## Key Files

| File | Contents |
|------|----------|
| `_lexscan.c` | Lexical scanner; pluggable I/O; continuation prompt |
| `_parser.c` | Shunting-yard parser; postfix token output to `_pfbuf` |
| `errsys.c` | Structured error reporting system |
| `errfile.c` | Error message format string lookup from `.err` file |
| `symbols.c` | Global hash-based symbol table |
| `dmm.c` | Dynamic memory manager for embedded systems |
| `mblock.c` | Block memory allocator |
| `xdll.c` | Extended doubly-linked list (foundation for deferred graph work) |
| `optable.c` | Operator precedence/associativity table |
| `util.c` | String utilities including `restore_escapes()` |
| `comutil.h` | Public API for all ComUtil functions |
