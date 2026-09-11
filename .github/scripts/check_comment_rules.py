#!/usr/bin/env python3
"""Lint added .c/.h comments against the two rules in AGENTS.md:

  1. No GitHub issue/PR numbers in source comments (#223, Fixes #94, ...).
  2. A comment describes the code as it is, not the story of how it got
     that way (no fix narration: "used to", "the bug", "diagnosed", ...).

Only lines *added* by the diff between BASE and HEAD are checked, and only
the comment-bearing portion of each line -- a `#` inside a string literal
(a hex color, an error message) is not a violation. .comt files are exempt,
matching AGENTS.md.

Rule 1 is a hard failure (exit 1): the pattern is unambiguous. Rule 2 is
inherently fuzzy, so it is reported as a non-blocking warning for a human
(or the next review pass) to judge -- AGENTS.md itself carves out an
exception for comments that genuinely need the long version.

Fails closed: a base ref that can't be resolved, or a changed file that
can't be read for any reason other than having been deleted/renamed away,
also exits 1 -- an unverified diff is not a passing one.

Usage: check_comment_rules.py [BASE] [HEAD]
  BASE defaults to origin/master, HEAD defaults to HEAD.
"""
import re
import subprocess
import sys

ISSUE_RE = re.compile(r'#\d+')

NARRATION_PATTERNS = [re.compile(p, re.IGNORECASE) for p in [
    r'\bused to\b',
    r'\bpreviously\b',
    r'\bthis (?:bug|fix)\b',
    r'\bthe bug\b',
    r'\bwas broken\b',
    r'\bsilently broke\b',
    r'\bdiagnosed\b',
    r'\bdiscovered\b',
    r'\bfixed by\b',
    r'\bprior to this\b',
    r'\bbefore this (?:fix|change|commit|patch)\b',
    r'\bregression\b',
    r'\bwe (?:caught|found)\b',
    r'\brather than the (?:old|previous|prior)\b',
    r'\brather than (?:through|via) (?:a|the)\b',
]]


def sh(*args):
    return subprocess.run(args, capture_output=True, text=True, check=True).stdout


def splice_lines(text):
    """Resolve backslash-newline line splicing (translation phase 2).

    Real compilers do this as a raw character-stream pass *before* any
    tokenizing -- including before recognizing a `//`/`/*` delimiter, so a
    `/` and `/` separated only by a spliced line ending do combine into a
    line comment (confirmed against g++: it even warns -Wcomment on this
    exact shape). Splicing has to happen first and separately for the same
    reason: peeking one character ahead to recognize a two-character
    delimiter only sees the right character if the splice already ran.

    Returns (logical_text, line_of) where line_of[k] is the 1-based
    original physical line number of logical_text[k], so a match found
    after splicing can still be attributed to the physical line a diff
    would report as added.
    """
    n = len(text)
    logical_chars = []
    line_of = []
    line_no = 1
    i = 0
    while i < n:
        if text[i] == '\\' and i + 1 < n and text[i + 1] == '\n':
            i += 2
            line_no += 1
            continue
        logical_chars.append(text[i])
        line_of.append(line_no)
        if text[i] == '\n':
            line_no += 1
        i += 1
    return logical_chars, line_of


def extract_comment_lines(text):
    """Map 1-based (original, pre-splice) line number -> comment-only text
    contributed by that line.

    A small state-machine C/C++ lexer over the spliced character stream:
    tracks // and /* */ comments, "string" and 'char' literals (with
    backslash escapes) so a `#123` inside a string, or a `//` inside a URL
    string, is never mistaken for comment content.
    """
    chars, line_of = splice_lines(text)
    m = len(chars)
    buf = {}

    def emit(ln, ch):
        buf.setdefault(ln, []).append(ch)

    state = 'CODE'
    k = 0
    while k < m:
        c = chars[k]
        nxt = chars[k + 1] if k + 1 < m else ''
        ln = line_of[k]
        if state == 'CODE':
            if c == '/' and nxt == '/':
                state = 'LINE_COMMENT'
                k += 2
            elif c == '/' and nxt == '*':
                state = 'BLOCK_COMMENT'
                k += 2
            elif c == '"':
                state = 'STRING'
                k += 1
            elif c == "'":
                state = 'CHAR'
                k += 1
            else:
                k += 1
        elif state == 'LINE_COMMENT':
            if c == '\n':
                state = 'CODE'
                k += 1
            else:
                emit(ln, c)
                k += 1
        elif state == 'BLOCK_COMMENT':
            if c == '*' and nxt == '/':
                state = 'CODE'
                k += 2
            else:
                emit(ln, c)
                k += 1
        elif state == 'STRING':
            if c == '\\' and k + 1 < m:
                k += 2
            elif c == '"':
                state = 'CODE'
                k += 1
            elif c == '\n':
                state = 'CODE'  # malformed, but don't hang the lexer
                k += 1
            else:
                k += 1
        elif state == 'CHAR':
            if c == '\\' and k + 1 < m:
                k += 2
            elif c == "'":
                state = 'CODE'
                k += 1
            elif c == '\n':
                state = 'CODE'
                k += 1
            else:
                k += 1
    return {ln: ''.join(chars) for ln, chars in buf.items()}


def added_lines_by_file(base, head):
    """Map changed .c/.h path -> set of new-file line numbers it added."""
    diff = sh('git', 'diff', '--unified=0', '--no-color', base, head,
              '--', '*.c', '*.h')
    files = {}
    cur_file = None
    cur_new_line = None
    hunk_re = re.compile(r'^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@')
    for line in diff.splitlines():
        if line.startswith('+++ '):
            path = line[4:]
            if path == '/dev/null':
                cur_file = None
            else:
                cur_file = path[2:] if path.startswith('b/') else path
                files.setdefault(cur_file, set())
        elif line.startswith('@@'):
            m = hunk_re.match(line)
            if m:
                cur_new_line = int(m.group(1))
        elif line.startswith('+') and not line.startswith('+++'):
            if cur_file is not None and cur_new_line is not None:
                files[cur_file].add(cur_new_line)
                cur_new_line += 1
        # '-' lines don't consume a new-file line number; -U0 has no context lines.
    return files


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else 'origin/master'
    head = sys.argv[2] if len(sys.argv) > 2 else 'HEAD'

    try:
        files = added_lines_by_file(base, head)
    except subprocess.CalledProcessError as e:
        # Fail the gate rather than fail open: a base ref that can't be
        # resolved (a too-shallow checkout, a rewritten branch) means no
        # comment was actually checked, and a silently green lint step is a
        # worse outcome than a CI failure someone has to look at. The
        # caller (ci.yml) is responsible for handing this script refs that
        # do resolve -- e.g. falling back to origin/master instead of a
        # zero-SHA "before" on a branch's first push.
        print(f"check_comment_rules: couldn't diff {base}..{head}: "
              f"{e.stderr.strip()}")
        return 1

    failures = []
    warnings = []
    errors = []
    for path, lines in files.items():
        if not lines:
            continue
        try:
            content = sh('git', 'show', f'{head}:{path}')
        except subprocess.CalledProcessError as e:
            stderr = e.stderr.strip()
            if 'does not exist' in stderr:
                continue  # file deleted or renamed away in this diff
            errors.append(f"{path}: couldn't read {head}:{path}: {stderr}")
            continue
        comments = extract_comment_lines(content)
        for ln in sorted(lines):
            text = comments.get(ln, '')
            if not text.strip():
                continue
            for m in ISSUE_RE.finditer(text):
                failures.append(
                    f"{path}:{ln}: comment references an issue/PR number "
                    f"({m.group(0)!r}) -- belongs in the commit message/PR "
                    f"description, not the source tree (AGENTS.md rule 1)")
            for pat in NARRATION_PATTERNS:
                m = pat.search(text)
                if m:
                    warnings.append(
                        f"{path}:{ln}: comment may narrate a fix "
                        f"({m.group(0)!r}) instead of describing current "
                        f"behavior -- review against AGENTS.md rule 2")
                    break  # one flag per line is enough

    if warnings:
        print("### Possible fix-narration in comments (AGENTS.md rule 2, "
              "non-blocking -- judgment call)")
        for w in warnings:
            print(f"  {w}")
        print()

    if errors:
        print("### Files this check couldn't read (blocking -- an unverified "
              "file is not a passing one)")
        for e in errors:
            print(f"  {e}")
        print()

    if failures:
        print("### Issue/PR numbers in comments (AGENTS.md rule 1, blocking)")
        for f in failures:
            print(f"  {f}")
        print()
        print(f"{len(failures)} comment(s) violate AGENTS.md's no-issue-number "
              "rule. Move that context to the commit message or PR "
              "description and describe the code itself in the comment.")

    if failures or errors:
        return 1

    print("check_comment_rules: no issue-number violations in added "
          f"comments ({len(warnings)} narration warning(s)).")
    return 0


if __name__ == '__main__':
    sys.exit(main())
