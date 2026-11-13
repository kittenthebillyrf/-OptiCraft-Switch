#!/bin/sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
SRC="$ROOT/runtime/jvm/tools/opticraft_launch_preflight.cpp"
OUT="${TMPDIR:-/tmp}/opticraft-launch-preflight"
JAR="${1:-}"
if [ -z "$JAR" ]; then
  echo "usage: $0 /path/to/minecraft.jar" >&2
  exit 2
fi
c++ -std=c++17 -O2 -Wall -Wextra -Werror "$SRC" -lz -o "$OUT"
"$OUT" "$JAR"
