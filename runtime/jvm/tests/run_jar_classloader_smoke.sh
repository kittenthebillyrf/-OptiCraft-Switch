#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="${TMPDIR:-/tmp}/opticraft-jar-classloader-smoke"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/runtime/jvm/include" \
  "$ROOT/runtime/jvm/src/opticraft_classfile.cpp" \
  "$ROOT/runtime/jvm/src/opticraft_jar_classloader.cpp" \
  "$ROOT/runtime/jvm/tests/jar_classloader_smoke.cpp" \
  -lz -o "$OUT"
"$OUT"
echo "OptiCraft JAR classloader smoke: PASS"
