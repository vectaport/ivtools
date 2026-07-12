#!/bin/sh
# gitcommitid.sh -- print a #define COMMIT_ID line for a main.c to #include
#
# Usage: gitcommitid.sh <repo-root>
#
# COMMIT_ID is the build_stamp() (ComUtil/util.c) identity shown on each
# editor's startup banner, proving which commit a running binary was built
# from.  This used to be PATCH_KEY, a manually-bumped literal (first 8 of a
# made-up uuid) hardcoded in each main.c -- easy to forget to bump, and with
# no way to verify it against anything real.  This script replaces that: it
# prints the actual current git commit (first 8 hex chars), so anyone (or
# any tool) can verify a build by running `git rev-parse --short=8 HEAD`
# themselves and comparing, no bookkeeping required.
#
# Appends "-dirty" if the working tree has uncommitted changes, so a build
# from a dirty tree doesn't silently claim to be an exact match for a
# clean commit.  Falls back to "unknown" if not run from a git checkout at
# all (e.g. a release tarball with no .git).
#
# Regenerated on every build (see the Imakefile's gitcommitid.h rule)
# rather than committed -- a file can't contain the hash of the commit it's
# part of (the hash is computed from the tree's content, including this
# file), so this can only ever be a build-time value, never a source
# literal.

root="$1"

commit_id=$(cd "$root" 2>/dev/null && git rev-parse --short=8 HEAD 2>/dev/null)

if [ -z "$commit_id" ]; then
    commit_id="unknown"
else
    if ! (cd "$root" && git diff --quiet 2>/dev/null && git diff --cached --quiet 2>/dev/null); then
        commit_id="${commit_id}-dirty"
    fi
fi

echo "#define COMMIT_ID \"$commit_id\""
