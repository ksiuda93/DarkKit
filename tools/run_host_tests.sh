#!/usr/bin/env bash
# Host unit-test runner for DarkKit.
#
# The tests under test/ are standalone host programs: each has its own main()
# and uses plain assert()/CHECK on the pure-logic headers in src/, with small
# stub headers standing in for the ESP-IDF / Arduino APIs. They are NOT Unity
# suites, so we compile and run them directly with g++ instead of `pio test`.
#
# Usage: tools/run_host_tests.sh
# Exit code is non-zero if any test fails to compile or run.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT/test"

CXX="${CXX:-g++}"
CXXFLAGS="${CXXFLAGS:--std=c++17 -O1 -Wall}"
BIN="$(mktemp -d)"
trap 'rm -rf "$BIN"' EXIT

# badusb_ble pulls <HIDTypes.h> from the Arduino-ESP32 framework (unguarded)
# and needs ARDUINO_ARCH_ESP32 so its HIDINPUT/HIDOUTPUT macros are defined.
HIDDIR="$(find "${HOME}/.platformio/packages" -name HIDTypes.h 2>/dev/null | head -1 | xargs -r dirname)"

# test name                 extra per-test flags
TESTS=(
  "pc_monitor_protocol"     ""
  "infrared_file_utils"     ""
  "air_mouse_motion"        ""
  "air_mouse_hid"           ""
  "bluetooth_manager"       ""
  "badusb_ble"              "-DARDUINO_ARCH_ESP32 -I${HIDDIR}"
)

pass=0; fail=0; failed=()
i=0
while [ $i -lt ${#TESTS[@]} ]; do
  name="${TESTS[$i]}"; extra="${TESTS[$((i+1))]}"; i=$((i+2))
  printf '%-26s ' "$name"
  if ! $CXX $CXXFLAGS -I. -I.. $extra -o "$BIN/$name" "${name}_test.cpp" 2>"$BIN/$name.cerr"; then
    echo "COMPILE-FAIL"; sed 's/^/    /' "$BIN/$name.cerr" | head -12
    fail=$((fail+1)); failed+=("$name"); continue
  fi
  if "$BIN/$name" >"$BIN/$name.out" 2>&1; then
    echo "PASS"; pass=$((pass+1))
  else
    rc=$?; echo "RUN-FAIL (rc=$rc)"; sed 's/^/    /' "$BIN/$name.out" | tail -6
    fail=$((fail+1)); failed+=("$name")
  fi
done

echo "────────────────────────────────────────"
echo "pass=$pass fail=$fail"
if [ $fail -ne 0 ]; then
  echo "FAILED: ${failed[*]}"
  exit 1
fi
echo "All host tests passed."
