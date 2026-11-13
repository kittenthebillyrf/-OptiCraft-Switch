#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
mkdir -p "$ROOT/build"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic -I"$ROOT/runtime/world/include" \
 "$ROOT/runtime/world/src/opticraft_texture_atlas.cpp" "$ROOT/runtime/world/tests/world_texture_loader_test.cpp" -lz \
 -o "$ROOT/build/world-texture-loader-test"
"$ROOT/build/world-texture-loader-test"
