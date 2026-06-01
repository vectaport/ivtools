#!/bin/bash
# comterp_listen -- shebang wrapper for "comterp listen" mode
#
# Usage as shebang: #!/usr/bin/env comterp_listen
#
# Finds a free listen port starting at 10002 (stepping by 10000 if taken).
# If --port portnum is supplied it is used as the starting port, and stripped
# from the args before passing to the script.
#
# CONVENTION: the resolved listen port is injected as arg(1) in the script
# (before any user-supplied arguments) so shebang scripts can access it
# via arg(1) to use in callback commands, e.g.:
#
#   callback_port=int(arg(1))
#
# The --port keyword arg can be used to request a specific base port:
#
#   drawmo --port 20002 --tests updown
#

port=10002
newargs=()
skip_next=false
for i in "$@"; do
    if $skip_next; then
        port="$i"
        skip_next=false
        continue
    fi
    if [ "$i" = "--port" ]; then
        skip_next=true
        continue
    fi
    newargs+=("$i")
done
while lsof -i :$port > /dev/null 2>&1; do
    port=$((port + 10000))
done
while true; do
    comterp listen $port "${newargs[0]}" $port "${newargs[@]:1}"
    status=$?
    if [ $status -ne 1 ]; then
        break
    fi
    port=$((port + 10000))
    while lsof -i :$port > /dev/null 2>&1; do
        port=$((port + 10000))
    done
done
