#!/bin/sh
# Headless wire-protocol test: the heart of ivtools -- "the command language is
# the wire protocol" -- exercised over a real loopback TCP socket, with no X and
# no drawserv.  Starts a `comterp server`, then sends an expression over the
# socket with netcat and checks the *evaluated value* comes back.  That netcat
# (not comterp) is a perfectly good client is the whole point: every ComTerp
# value serializes back to valid ComTerp syntax, so the REPL and the wire are the
# same thing -- any TCP client speaks it.
#
# This is the network-protocol half of what the drawserv/drawmo suite covers,
# minus the GUI/X coupling that makes drawmo CI-hostile, so it runs headless on a
# hosted CI runner.  Exit 0 on success, 1 on failure.  Needs comterp built WITH
# ACE, plus lsof and nc, on PATH.
port=${1:-20015}
srvlog=$(mktemp)
comterp server "$port" </dev/null >"$srvlog" 2>&1 &
srv=$!
trap 'kill "$srv" 2>/dev/null; rm -f "$srvlog"' EXIT INT TERM

# wait for the acceptor to actually be listening before connecting
i=0
while [ "$i" -lt 40 ]; do
  lsof -iTCP:"$port" -sTCP:LISTEN >/dev/null 2>&1 && break
  sleep 0.25; i=$((i + 1))
done
if ! lsof -iTCP:"$port" -sTCP:LISTEN >/dev/null 2>&1; then
  echo "remote_loopback: server never listened on port $port"
  echo "--- server log ---"; cat "$srvlog"
  exit 1
fi

# send a distinctive expression over the socket; the server evaluates it and
# writes the value back.  (printf ...; sleep 1) holds the connection open long
# enough to read the reply across nc variants.
out=$( (printf '111+222\n'; sleep 1) | nc -w 2 localhost "$port" 2>&1)
echo "remote_loopback: 111+222 over TCP returned -> [$(printf '%s' "$out" | tr -d '\r\n')]"
case "$out" in
  *333*) echo "remote_loopback: OK (server evaluated the expression over the wire)"; exit 0 ;;
  *)     echo "remote_loopback: FAIL (expected 333 in the reply)"
         echo "--- server log ---"; cat "$srvlog"; exit 1 ;;
esac
