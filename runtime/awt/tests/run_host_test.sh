#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="$ROOT/build/awt-bridge-smoke"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror \
  -I"$ROOT/runtime/platform/include" -I"$ROOT/runtime/awt/include" \
  "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" \
  "$ROOT/runtime/awt/src/opticraft_awt_bridge.cpp" \
  "$ROOT/runtime/awt/tests/awt_bridge_smoke.cpp" \
  -o "$OUT"
"$OUT"
echo "AWT bridge smoke test: PASS"
