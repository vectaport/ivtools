# Comment review rules for Greptile

This is a static copy of the two comment rules from `AGENTS.md` (repo root,
"Code comments" section), formatted for Greptile's custom-context knowledge
base. `AGENTS.md` is the source of truth -- if the two ever disagree, fix
this file to match it. There is no automatic sync, so re-copy this file's
body whenever those rules change in `AGENTS.md`.

Paste the section below into this repo's Greptile knowledge base (Custom
Context) so review passes flag violations automatically, the same way the
in-repo CI lint (`.github/scripts/check_comment_rules.py`) catches them
mechanically -- the CI check only looks at literal `#<digits>` tokens and a
handful of narration phrases; Greptile's judgment is the backstop for the
subtler phrasing the mechanical check misses.

---

## Instructions for the reviewer

When reviewing a diff that adds or edits a comment in a `.c` or `.h` file
(`.comt` test files are exempt), check it against both rules below and
raise a review finding for either violation.

### Rule 1 -- no GitHub issue/PR numbers in source comments

A comment must never contain a GitHub issue or PR reference: `#223`,
`(#485)`, `Fixes #94`, `Closes #501`, `PR #493`, "see issue 147", etc. That
context belongs only in the commit message and PR description -- external,
mutable state has no business embedded in the source tree.

### Rule 2 -- comments describe the code as it is, not its history

A comment must read exactly as it would if the code had been written
correctly the first time: state the invariant, the mechanism, or the
trade-off a future reader needs. Flag a comment that instead narrates:

- how the code used to be broken or what the old behavior was,
- how a bug was diagnosed, found, or reasoned about,
- how or why a fix was made, or a contrast against a prior design
  ("rather than the old approach", "instead of the previous helper",
  "switched back to", etc.).

This applies even to a comment written specifically to explain a bug fix --
the fix's story belongs in the commit message and PR description, not the
comment. The one exception (per `AGENTS.md`): a comment may stay long-form
when a spot genuinely needs it to head off a likely misreading that would
cause real chaos -- that's a judgment call, not a loophole for narration.

### Examples

Bad:
```c
// Fixed #485: this used to return a bare int, we now wrap it in brackets.
```

Bad (references a removed helper -- still narrating history):
```c
// Stamp the wrapper here instead of through the old helper, which
// silently dropped it on an unelided return.
```

Good:
```c
// Stamped in place: a copy of this value discards the wrapper
// (AttributeValue::assignval() never copies it), so read it by
// reference rather than assign it elsewhere.
```
