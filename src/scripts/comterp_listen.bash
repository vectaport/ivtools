#!/bin/bash
if [[ "$1" =~ ^[0-9]+$ ]]; then
    port="$1"
    shift
    comterp listen "$port" "$@"
else
    comterp listen 10002 "$@"
fi
