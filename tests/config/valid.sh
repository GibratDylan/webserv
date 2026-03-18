#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

assert_executable "$WEBSERV_BIN"

shopt -s nullglob
valid_files=("$ROOT_DIR"/tests/config/valid_*.conf "$ROOT_DIR"/config/default.conf "$ROOT_DIR"/tests/config/test.conf)

[[ ${#valid_files[@]} -gt 0 ]] || fail "no valid config files found"

for f in "${valid_files[@]}"; do
  check_config_file "$f" 1
done

echo "[OK] valid configs"
