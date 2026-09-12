#!/bin/bash
# Q7.6 helper: sample voluntary/involuntary context switches of the CHILD
# process for several ITERS values. This reads the same task_struct fields
# (nvcsw / nivcsw) that the csprobe kernel module exposes via /proc/csprobe.
cd "$(dirname "$0")"
for N in "$@"; do
  gcc -O2 -DITERS=$N -o ipc-cs hw1-ipc.c
  ./ipc-cs 0 > /tmp/ipcout.txt &
  BG=$!
  sleep 0.4
  CHILD=$(awk '/child pid/ {print $4}' /tmp/ipcout.txt)
  LAST=""
  while [ -d "/proc/$CHILD" ]; do
    R=$(awk '/_ctxt_switches/ {printf "%s ", $2}' /proc/$CHILD/status 2>/dev/null)
    [ -n "$R" ] && LAST="$R"
    sleep 0.1
  done
  wait $BG 2>/dev/null
  echo "ITERS=$N  ->  child (voluntary involuntary) = $LAST"
done
