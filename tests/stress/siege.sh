#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

START_SERVER="${START_SERVER:-1}"
REQUIRE_SIEGE="${REQUIRE_SIEGE:-0}"

CONCURRENCY="${CONCURRENCY:-25}"
REPS="${REPS:-10}"
TIMEOUT="${TIMEOUT:-}" # siege format, e.g. 10S, 1M (when set, overrides REPS)

if ! command -v siege >/dev/null 2>&1; then
  if [[ "$REQUIRE_SIEGE" == "1" ]]; then
    fail "missing dependency: siege"
  fi
  echo "[SKIP] siege not installed"
  exit 0
fi

TMPDIR="$(mktemp_dir)"
cleanup() {
  if [[ "$START_SERVER" == "1" ]]; then
    stop_server
  fi
  rm -rf "$TMPDIR"
}
trap cleanup EXIT

if [[ "$START_SERVER" == "1" ]]; then
  require_cmd curl
  assert_executable "$WEBSERV_BIN"
  LOG_FILE="$TMPDIR/server.log"
  start_server "$LOG_FILE"
fi

if [[ -n "$TIMEOUT" ]]; then
  echo "[SIEGE] load test (c=$CONCURRENCY t=$TIMEOUT)"
else
  echo "[SIEGE] load test (c=$CONCURRENCY r=$REPS)"
fi

URLS_FILE="$TMPDIR/urls.txt"
cat >"$URLS_FILE" <<EOF
$WEBSERV_BASE_URL/
$WEBSERV_BASE_URL/files/
$WEBSERV_BASE_URL/cgi-bin/test.py
EOF

set +e
if [[ -n "$TIMEOUT" ]]; then
  siege -q -b -c "$CONCURRENCY" -t "$TIMEOUT" -f "$URLS_FILE"
else
  siege -q -b -c "$CONCURRENCY" -r "$REPS" -f "$URLS_FILE"
fi
rc=$?
set -e

if [[ $rc -ne 0 ]]; then
  fail "siege returned non-zero exit code: $rc"
fi

echo "[OK] siege"
