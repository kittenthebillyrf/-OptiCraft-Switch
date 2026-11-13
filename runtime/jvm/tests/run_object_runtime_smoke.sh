#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/opticraft-object-runtime-smoke"
c++ -std=c++17 -Wall -Wextra -Werror -I"$ROOT/runtime/jvm/include" \
  "$ROOT/runtime/jvm/src/opticraft_object_runtime.cpp" \
  "$ROOT/runtime/jvm/tests/object_runtime_smoke.cpp" -o "$OUT"
"$OUT"
echo "OptiCraft object runtime smoke: PASS"
