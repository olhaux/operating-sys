#!/bin/bash
# verify-q7.sh -- runs the complete Q7.4 + Q7.5 sequence in one go.
#
# Must be run on a real Linux machine (the VirtualBox VM), NOT under WSL2:
# WSL2 ships no /lib/modules/$(uname -r)/build, so the module cannot be built
# there. Requires sudo.
#
#   cd q7-ipc && ./verify-q7.sh
#
# Everything it prints is what Q7.4 and Q7.5 ask you to screenshot.

set -u
cd "$(dirname "$0")"

hr() { printf '\n=== %s ===\n' "$1"; }
KREL=$(uname -r)

hr "0. preflight"
echo "kernel: $KREL"

if [ ! -d "/lib/modules/$KREL/build" ]; then
  echo "FAIL: /lib/modules/$KREL/build is missing -- kernel headers not installed."
  echo "      sudo apt install -y linux-headers-$KREL"
  echo "      (If you are on WSL2, this will not work. Use the VirtualBox VM.)"
  exit 1
fi
echo "headers: OK"

# An unsigned module cannot load while Secure Boot is enforcing.
if command -v mokutil >/dev/null 2>&1; then
  echo "secure boot: $(mokutil --sb-state 2>/dev/null || echo 'unknown')"
else
  echo "secure boot: mokutil not installed (usually fine in VirtualBox)"
fi

hr "1. build the module"
make -C csprobe clean >/dev/null 2>&1
make -C csprobe || { echo "FAIL: module did not build"; exit 1; }
ls -l csprobe/csprobe.ko
modinfo csprobe/csprobe.ko | grep -E '^(filename|license|description|parm)' || true

hr "2. Q7.4 -- load the module"
sudo insmod csprobe/csprobe.ko || { echo "FAIL: insmod"; exit 1; }
lsmod | grep csprobe
echo "--- dmesg (expect: csprobe: loaded) ---"
sudo dmesg | tail -5
echo ">>> SCREENSHOT 7.4a: the dmesg line above"

hr "3. before a target pid is set"
cat /proc/csprobe

hr "4. Q7.5 -- start the IPC program and sample the CHILD"
make all >/dev/null 2>&1 || true
./hw1-ipc 0 > /tmp/ipc-run.txt 2>&1 &
IPCPID=$!
sleep 1
CHILD=$(awk '/child pid/ {print $4}' /tmp/ipc-run.txt)
if [ -z "${CHILD:-}" ]; then
  echo "FAIL: could not read the child pid"; sudo rmmod csprobe; exit 1
fi
echo "child pid = $CHILD  (parent = $(awk '/parent pid/ {print $4}' /tmp/ipc-run.txt))"

echo "$CHILD" | sudo tee /sys/module/csprobe/parameters/target_pid >/dev/null
echo "target_pid set to $(cat /sys/module/csprobe/parameters/target_pid)"

for n in 1 2 3; do
  printf '\n--- /proc/csprobe -- reading %d of 3 ---\n' "$n"
  cat /proc/csprobe
  echo ">>> SCREENSHOT 7.5-$n"
  [ "$n" -lt 3 ] && sleep 7
done

hr "5. after the child exits"
wait $IPCPID 2>/dev/null
sleep 1
cat /proc/csprobe          # expect: pid <n> not found (task exited?)
echo "--- the program's own output ---"
cat /tmp/ipc-run.txt

hr "6. unload the module"
sudo rmmod csprobe
echo "--- dmesg (expect: csprobe: unloaded) ---"
sudo dmesg | tail -5
echo ">>> SCREENSHOT 7.4b: the dmesg line above"

hr "done"
echo "Q7.6: now run  make ctxsw  and use YOUR numbers in the report."
