#!/bin/bash
# comterp_listen -- shebang wrapper for comterp listen mode
#
# Usage:
#   comterp_listen [portnum] [script.comt] [args...]
#   #!/usr/bin/env comterp_listen   (as shebang in .comt scripts)
#
# If the first argument is a port number, use it; otherwise default to
# port 10002 and pass all arguments as the script file and its args.
# This allows .comt scripts to use comterp_listen as a shebang interpreter:
#
#   #!/usr/bin/env comterp_listen        # listens on default port 10002
#   #!/usr/bin/env comterp_listen 10003  # listens on port 10003
#
# The script opens an ACE acceptor on the port, registers a stdin handler,
# runs the optional script file, then enters the ACE reactor event loop
# accepting connections from both the port and stdin until exit.

if [[ "$1" =~ ^[0-9]+$ ]]; then
    port="$1"
    shift
    comterp listen "$port" "$@"
else
    comterp listen 10002 "$@"
fi
