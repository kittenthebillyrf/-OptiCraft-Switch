#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="/tmp/opticraft-input-platform-test"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -pthread -I"$ROOT/runtime/input/include" -I"$ROOT/runtime/platform/include" "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" "$ROOT/runtime/input/src/input_adapter.cpp" "$ROOT/runtime/input/src/switch_input_bridge.cpp" "$ROOT/runtime/input/src/opticraft_input_api.cpp" "$ROOT/runtime/input/tests/input_platform_test.cpp" -o "$OUT"
"$OUT"
ABI_OUT="/tmp/opticraft-lwjgl-event-abi-test"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -I"$ROOT/runtime/input/include" "$ROOT/runtime/input/src/input_adapter.cpp" "$ROOT/runtime/input/src/switch_input_bridge.cpp" "$ROOT/runtime/input/src/lwjgl_event_buffers.cpp" "$ROOT/runtime/input/tests/lwjgl_event_buffers_test.cpp" -o "$ABI_OUT"
"$ABI_OUT"
echo "PASS platform-to-LWJGL input bridge tests"
echo "PASS LWJGL 2 keyboard/mouse event ABI tests"
