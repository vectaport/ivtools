#!/bin/bash
# devrun.sh -- run a comterp-family binary against the IN-TREE libraries,
# so a partial build can be tested without a "make install".
#
# Every in-tree dylib is built with an install_name of /usr/local/lib/...,
# so even a binary linked inside the tree loads the INSTALLED library at run
# time.  DYLD_LIBRARY_PATH overrides that, but it cannot be passed on the
# command line of a shell wrapper: /bin/bash is SIP-protected, so dyld
# scrubs DYLD_* out of the environment it is exec'd with -- which is why
# "DYLD_LIBRARY_PATH=... comterp_run foo.comt" silently tests the installed
# library instead.  Setting the variable *inside* this script works, because
# the scrub happens on entry, not on the way out to an unprotected child.
#
# usage:
#   ./devrun.sh comterp run src/comterp_/tests/run_all.comt
#   ./devrun.sh comterp_run foo.comt        # same as: comterp run foo.comt
#   ./devrun.sh --check                     # report which libs would be used
#
# IVDEV_BIN=/path/to/comterp overrides the binary (default: the installed
# one, which is fine unless your change is in a program's own main.c).

set -e
root=$(cd "$(dirname "$0")" && pwd)
cpu=$(sed -n 's/^ *CPU *= *//p' "$root/config/config.mk" 2>/dev/null | head -1)
cpu=${cpu:-DARWIN}

libpath=$(ls -d "$root"/src/*/"$cpu" 2>/dev/null | tr '\n' ':')
if [ -z "$libpath" ]; then
  echo "devrun.sh: no $root/src/*/$cpu build directories -- build first" >&2
  exit 1
fi
export DYLD_LIBRARY_PATH="$libpath$DYLD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$libpath$LD_LIBRARY_PATH"

if [ "$1" = "--check" ]; then
  bin=${IVDEV_BIN:-$(command -v comterp)}
  echo "CPU        : $cpu"
  echo "binary     : $bin"
  echo "in-tree libs it will actually load:"
  DYLD_PRINT_LIBRARIES=1 "$bin" -e 'exit' 2>&1 |
    sed -n "s|^dyld\[[0-9]*\]: <[^>]*> \($root.*\)|  \1|p" | sort -u
  exit 0
fi

prog=$1; shift || true
case "$prog" in
  comterp_run) set -- run "$@"; prog=comterp ;;
esac
bin=${IVDEV_BIN:-$(command -v "$prog")}
[ -x "$bin" ] || { echo "devrun.sh: cannot find $prog" >&2; exit 1; }
exec "$bin" "$@"
