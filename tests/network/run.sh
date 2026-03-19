#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

BIN="$ROOT_DIR/tests/network/network_tests"
SRC="$ROOT_DIR/tests/network/network_tests.cpp"

c++ -std=c++98 -Wall -Wextra -Werror \
  -I"$ROOT_DIR/include" \
  "$SRC" \
  "$ROOT_DIR/src/network/TcpSocket.cpp" \
  "$ROOT_DIR/src/network/IOMultiplexer.cpp" \
  -o "$BIN"

"$BIN"
