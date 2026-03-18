#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

require_cmd curl
assert_executable "$WEBSERV_BIN"

TMPDIR="$(mktemp_dir)"
cleanup() {
  stop_server
  rm -rf "$TMPDIR"
}
trap cleanup EXIT

LOG_FILE="$TMPDIR/server.log"
HEADERS_FILE="$TMPDIR/headers.txt"
BODY_FILE="$TMPDIR/body.txt"

start_server "$LOG_FILE"

echo "[HTTP] chunked upload"

# Open raw TCP socket (bash feature: /dev/tcp)
if ! exec 3<>"/dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"; then
  fail "cannot open /dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT} (need bash with /dev/tcp support)"
fi

# Headers
printf 'POST /chunked_test HTTP/1.1\r\n' >&3
printf 'Host: %s:%s\r\n' "$WEBSERV_HOST" "$WEBSERV_PORT" >&3
printf 'Transfer-Encoding: chunked\r\n' >&3
printf 'Content-Type: text/plain\r\n' >&3
printf '\r\n' >&3

# Chunk 1: "Hello " (6 bytes)
printf '6\r\nHello \r\n' >&3

# Chunk 2: "World!" (6 bytes)
printf '6\r\nWorld!\r\n' >&3

# End chunk
printf '0\r\n\r\n' >&3

# Read status line
IFS= read -r -t 3 status_line <&3 || true
status_line="${status_line%$'\r'}"

# Drain a bit (best-effort)
{
  IFS= read -r -t 0.2 _ || true
} <&3 || true

exec 3<&-
exec 3>&-

if [[ "$status_line" != HTTP/1.1*201* && "$status_line" != HTTP/1.0*201* ]]; then
  fail "chunked upload: expected HTTP 201, got: ${status_line:-<empty>}"
fi

code="$(request GET "$WEBSERV_BASE_URL/uploads/chunked_test" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET chunked uploaded file" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Hello World!" "$BODY_FILE" || fail "chunked body mismatch"

code="$(request DELETE "$WEBSERV_BASE_URL/chunked_test" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 204 "DELETE /chunked_test" "$HEADERS_FILE" "$BODY_FILE"

echo "[OK] http chunked"
