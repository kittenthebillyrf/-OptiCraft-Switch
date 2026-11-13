#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="$ROOT/build/world-catalog-test"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/runtime/world/include" \
  "$ROOT/runtime/world/src/world_save_catalog.cpp" \
  "$ROOT/runtime/world/tests/world_save_catalog_test.cpp" \
  -o "$OUT"
"$OUT"
