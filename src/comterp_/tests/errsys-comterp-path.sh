#!/bin/sh
# errsys-comterp-path.sh -- verify comerr_read()/err_open() genuinely read an
# external comterp.err file rather than always silently falling back to the
# compiled-in default_errmsgs table.
#
# Two bugs made this always fail before: (1) the RELLIBALLDIR/ABSLIBALLDIR
# path-substitution -D flags in ComUtil/Imakefile never took effect, because
# errsys.c referenced the macro names as quoted string literals ("RELLIBALLDIR")
# rather than bare tokens -- the preprocessor never expands an identifier
# found inside a string literal; (2) even with a correctly opened file,
# err_open()'s ErrorStreams[findex] = fptr registration was dead code behind
# #if 0, so err_read() always saw a NULL entry and used default_errmsgs
# regardless.
#
# COMTERP_PATH is checked before RELLIBALLDIR/ABSLIBALLDIR and exercises the
# identical downstream registration/read logic, so it's a faithful,
# install-independent way to test the fix. Builds a scratch copy of
# comterp.err with one message swapped for a distinctive marker, points
# COMTERP_PATH at it, and confirms the marker -- not the compiled default --
# appears in a triggered error message. Exit 0 on success, 1 on failure.

set -e
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT INT TERM

marker="TESTMARKER errsys read the external comterp.err"
srcerr="$(dirname "$0")/../../ComUtil/comterp.err"
sed 's/(%d) Unexpected end-of-file/'"$marker"'/' "$srcerr" > "$tmpdir/comterp.err"

printf '1;\n' > "$tmpdir/incomplete.comt"

out=$(COMTERP_PATH="$tmpdir" comterp run "$tmpdir/incomplete.comt" 2>&1) || true
echo "errsys-comterp-path: got -> $out"

case "$out" in
  *"$marker"*)
    echo "errsys-comterp-path: OK (external comterp.err was read)"
    exit 0 ;;
  *)
    echo "errsys-comterp-path: FAIL (expected marker text -- err_open still not reading the external file)"
    exit 1 ;;
esac
