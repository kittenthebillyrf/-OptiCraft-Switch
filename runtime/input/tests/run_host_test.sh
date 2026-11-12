#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="/tmp/opticraft-input-platform-test"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -pthread -I"$ROOT/runtime/input/include" -I"$ROOT/runtime/platform/include" "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" "$ROOT/runtime/input/src/input_adapter.cpp" "$ROOT/runtime/input/src/switch_input_bridge.cpp" "$ROOT/runtime/input/src/opticraft_input_api.cpp" "$ROOT/runtime/input/tests/input_platform_test.cpp" -o "$OUT"
"$OUT"
echo "PASS platform-to-LWJGL input bridge tests"
