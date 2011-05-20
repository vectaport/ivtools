#!/bin/sh

set -e

for f in NEWS AUTHORS ChangeLog; do
    test -e "$f" || cp --verbose /dev/null "$f"
done

mkdir --parent --verbose m4

autoreconf --install --symlink

# Use --force to recreate things that don't seem to require recreation:
# autoreconf --install --symlink --make --force
