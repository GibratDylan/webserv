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

echo "[HTTP] basic functional tests"

code="$(request GET "$WEBSERV_BASE_URL/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Welcome to My Web Server" "$BODY_FILE" || fail "GET / body mismatch"

code="$(request GET "$WEBSERV_BASE_URL/does-not-exist" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 404 "GET /does-not-exist" "$HEADERS_FILE" "$BODY_FILE"

code="$(request GET "$WEBSERV_BASE_URL/redirect" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 301 "GET /redirect" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Location: /' "$HEADERS_FILE" || fail "redirect should set Location: /"

code="$(request GET "$WEBSERV_BASE_URL/files/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /files/" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Index of" "$BODY_FILE" || fail "autoindex expected for /files/"

# upload -> GET -> DELETE
printf '%s\n' "hello-from-test" >"$TMPDIR/payload.txt"
code="$(request POST "$WEBSERV_BASE_URL/test-ut.txt" "@$TMPDIR/payload.txt" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 201 "POST /test-ut.txt" "$HEADERS_FILE" "$BODY_FILE"

code="$(request GET "$WEBSERV_BASE_URL/uploads/test-ut.txt" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /uploads/test-ut.txt" "$HEADERS_FILE" "$BODY_FILE"
diff -u "$TMPDIR/payload.txt" "$BODY_FILE" >/dev/null || fail "uploaded content mismatch"

code="$(request DELETE "$WEBSERV_BASE_URL/test-ut.txt" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 204 "DELETE /test-ut.txt" "$HEADERS_FILE" "$BODY_FILE"

code="$(request GET "$WEBSERV_BASE_URL/uploads/test-ut.txt" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 404 "GET after DELETE" "$HEADERS_FILE" "$BODY_FILE"

code="$(request PUT "$WEBSERV_BASE_URL/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 405 "PUT / (method not allowed)" "$HEADERS_FILE" "$BODY_FILE"

# CGI
code="$(request GET "$WEBSERV_BASE_URL/cgi-bin/test.py?hello=world" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "CGI python" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Content-Type: text/html' "$HEADERS_FILE" || fail "CGI should set Content-Type"
grep -q "CGI Python3 Test" "$BODY_FILE" || fail "CGI python body mismatch"

code="$(request GET "$WEBSERV_BASE_URL/cgi-bin/test.cat" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "CGI cat" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Test file for cat" "$BODY_FILE" || fail "CGI cat body mismatch"

echo "[OK] http basic"
