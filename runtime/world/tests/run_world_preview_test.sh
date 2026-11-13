#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="$ROOT/build/world-preview-test"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/runtime/world/include" \
  "$ROOT/runtime/world/src/world_save_preview.cpp" \
  "$ROOT/runtime/world/tests/world_save_preview_test.cpp" -lz \
  -o "$OUT"
"$OUT"
