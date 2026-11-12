#!/usr/bin/env sh
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)"
OUT="/tmp/opticraft-platform-abi-test"
c++ -std=c++17 -Wall -Wextra -Werror -pedantic "$ROOT/runtime/platform/host/opticraft_platform_host.cpp" "$ROOT/runtime/platform/tests/platform_abi_test.cpp" -o "$OUT"
"$OUT"
echo "PASS platform ABI host tests"
