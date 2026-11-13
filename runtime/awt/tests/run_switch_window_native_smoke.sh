#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="$ROOT/build/switch-window-native-smoke"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror \
  -I"$ROOT/runtime/platform/include" -I"$ROOT/runtime/awt/include" \
  "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" \
  "$ROOT/runtime/awt/src/opticraft_awt_bridge.cpp" \
  "$ROOT/runtime/awt/src/opticraft_switch_window_native.cpp" \
  "$ROOT/runtime/awt/tests/switch_window_native_smoke.cpp" \
  -o "$OUT"
"$OUT"
echo "SwitchWindow native bridge smoke test: PASS"
