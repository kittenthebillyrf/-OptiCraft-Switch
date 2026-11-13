#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -I"$ROOT/runtime/world/include" \
 "$ROOT/runtime/world/src/world_anvil_chunk.cpp" "$ROOT/runtime/world/tests/world_anvil_chunk_test.cpp" -lz \
 -o "$ROOT/build/world-anvil-chunk-test"
"$ROOT/build/world-anvil-chunk-test"
