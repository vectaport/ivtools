#!/bin/sh

set -e

# echo "Usage: ./autoget.sh [options]"
# echo "Options:"
# echo "  --warn=all    warn about various autoconf constructs"

## automake --foreign option engaged so these files are no longer mandatory:
# for f in NEWS AUTHORS ChangeLog; do
#     test -e "$f" || cp --verbose /dev/null "$f"
# done

## Unnecessary: m4/acltx_prog_ps2pdf.m4 and hence m4/ now in the repo.
# mkdir --parent --verbose m4

autoreconf --force --install --symlink $*
