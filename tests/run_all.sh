#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Note: a binary named "tests/tester" exists in the repo but is not part of the
# automated test run below (legacy/manual tool unless wired explicitly).

bash "$SCRIPT_DIR/config/valid.sh"
bash "$SCRIPT_DIR/config/error.sh"
bash "$SCRIPT_DIR/network/run.sh"
bash "$SCRIPT_DIR/http/run.sh"

echo "[OK] all tests passed"
