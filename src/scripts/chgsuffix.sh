#!/bin/sh
#set -x
if [ "$1" \!= "$2" ]; then
    # recursively change suffix in $1 for suffix in $2
    find . -type f -name "*.$1" -print -o -type d -name TIFF -prune | \
    sed "s/^\(.*\)$1/mv \1$1 \1$2" | \
    sh
fi
