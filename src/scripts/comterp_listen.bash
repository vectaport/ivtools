#!/bin/bash
# comterp_listen -- shebang wrapper for comterp listen mode
#
# Usage:
#   comterp_listen [portnum] [script.comt] [args...]
#   #!/usr/bin/env comterp_listen   (as shebang in .comt scripts)
#
# If the first argument is a port number, use it as the listen port;
# otherwise default to port 10002 and pass all arguments as the script
# file and its args.
#
# When used as a shebang, the OS supplies the script path as the first
# argument, so the script listens on the default port 10002:
#
#   #!/usr/bin/env comterp_listen
#
# To specify a port, invoke directly on the command line:
#
#   comterp_listen 10003 script.comt
#
# Note: passing a port number in the shebang line itself
# (e.g. #!/usr/bin/env comterp_listen 10003) is not portable --
# on Linux the kernel passes all shebang arguments as a single token.
# Use #!/usr/bin/env -S comterp_listen 10003 if GNU coreutils >= 8.30
# is available and Linux portability is required.

if [[ "$1" =~ ^[0-9]+$ ]]; then
    port="$1"
    shift
    comterp listen "$port" "$@"
else
    comterp listen 10002 "$@"
fi
