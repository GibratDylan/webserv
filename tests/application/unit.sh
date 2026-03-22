#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

OUT_BIN="obj/application_handler_unit_tests"
trap 'rm -f "$OUT_BIN"' EXIT

# Reuse objects produced by `make re` (part of `make test-all`).
# Link everything except obj/main.o to avoid multiple entry points.
OBJS=( $(find obj -name '*.o' ! -name 'main.o' | sort) )

c++ -std=c++98 -Wall -Wextra -Werror -Iinclude \
	tests/application/handlers_unit.cpp "${OBJS[@]}" -o "$OUT_BIN"

"$OUT_BIN"
