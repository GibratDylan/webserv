#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

bash "$SCRIPT_DIR/unit.sh"
bash "$SCRIPT_DIR/suite.sh"
bash "$SCRIPT_DIR/chunked_upload.sh"
