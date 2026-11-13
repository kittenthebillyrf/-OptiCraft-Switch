#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="$ROOT/build/native-registry-smoke"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror \
  -I"$ROOT/runtime/platform/include" -I"$ROOT/runtime/awt/include" -I"$ROOT/runtime/jvm/include" \
  "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" \
  "$ROOT/runtime/awt/src/opticraft_awt_bridge.cpp" \
  "$ROOT/runtime/awt/src/opticraft_switch_window_native.cpp" \
  "$ROOT/runtime/jvm/src/opticraft_native_registry.cpp" \
  "$ROOT/runtime/jvm/tests/native_registry_smoke.cpp" \
  -o "$OUT"
"$OUT"
echo "Native method registry smoke test: PASS"
