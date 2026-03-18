#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

assert_executable "$WEBSERV_BIN"

shopt -s nullglob
error_files=("$ROOT_DIR"/tests/config/error_*.conf)

[[ ${#error_files[@]} -gt 0 ]] || fail "no error_* config files found"

for f in "${error_files[@]}"; do
  check_config_file "$f" 0
done

echo "[OK] error configs"
