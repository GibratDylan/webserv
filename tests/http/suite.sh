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

echo "[HTTP] functional suite"

# --- 200 / 404 / 301 / 405 ---
code="$(request GET "$WEBSERV_BASE_URL/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Welcome to My Web Server" "$BODY_FILE" || fail "GET / body mismatch"

code="$(request GET "$WEBSERV_BASE_URL/does-not-exist" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 404 "GET /does-not-exist" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Content-Type:[[:space:]]*text/html' "$HEADERS_FILE" || fail "404 should be served as text/html"
grep -q "Site 8080: 404 Not Found" "$BODY_FILE" || fail "404 should use custom error_page body (marker missing)"

code="$(request GET "$WEBSERV_BASE_URL/redirect" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 301 "GET /redirect" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Location: /' "$HEADERS_FILE" || fail "redirect should set Location: /"

code="$(request PUT "$WEBSERV_BASE_URL/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 405 "PUT /" "$HEADERS_FILE" "$BODY_FILE"
allow_line="$(grep -i '^Allow:' "$HEADERS_FILE" | head -1 || true)"
[[ -n "$allow_line" ]] || fail "405 should include an Allow: header"
echo "$allow_line" | grep -qi 'GET' || fail "Allow: should include GET"
echo "$allow_line" | grep -qi 'POST' || fail "Allow: should include POST"
echo "$allow_line" | grep -qi 'DELETE' || fail "Allow: should include DELETE"

# --- autoindex ---
code="$(request GET "$WEBSERV_BASE_URL/files/" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /files/" "$HEADERS_FILE" "$BODY_FILE"
grep -Eq 'href="2\.txt"|>2\.txt<' "$BODY_FILE" || fail "autoindex should expose a link to 2.txt in /files/"

# Virtual location static file (location /files { root ./www/8080/files; })
code="$(request GET "$WEBSERV_BASE_URL/files/2.txt" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /files/2.txt" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Hello from /files virtual location" "$BODY_FILE" || fail "location /files should serve static files from its root"

# Virtual location CGI (same location root, but .py should be executed via global cgi /usr/bin/python3)
code="$(request GET "$WEBSERV_BASE_URL/files/loc_test.py?x=1" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "GET /files/loc_test.py (CGI in virtual location)" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Content-Type: text/plain' "$HEADERS_FILE" || fail "virtual location CGI should set Content-Type: text/plain"
grep -q "virtual-location-cgi-ok" "$BODY_FILE" || fail "virtual location CGI body mismatch"
grep -Eq '^SCRIPT_NAME=.*loc_test\.py' "$BODY_FILE" || fail "CGI should receive SCRIPT_NAME containing loc_test.py under virtual location"
grep -q "QUERY_STRING=x=1" "$BODY_FILE" || fail "CGI should receive correct QUERY_STRING under virtual location"

# --- upload / delete ---
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

code="$(request DELETE "$WEBSERV_BASE_URL/does-not-exist" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 404 "DELETE /does-not-exist" "$HEADERS_FILE" "$BODY_FILE"

# --- CGI ---
code="$(request GET "$WEBSERV_BASE_URL/cgi-bin/test.py?hello=world" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "CGI python" "$HEADERS_FILE" "$BODY_FILE"
grep -qi '^Content-Type: text/html' "$HEADERS_FILE" || fail "CGI should set Content-Type"
grep -q "CGI Python3 Test" "$BODY_FILE" || fail "CGI python body mismatch"

# Advanced CGI: POST form parsing + CONTENT_LENGTH
payload='name=Bob&email=bob%40example.com&message=Hello+CGI'
expected_len=${#payload}
code="$(curl -sS --http1.1 -X POST -D "$HEADERS_FILE" -o "$BODY_FILE" \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data "$payload" \
  "$WEBSERV_BASE_URL/cgi-bin/test.py" -w "%{http_code}")"
expect_code "$code" 200 "CGI python POST" "$HEADERS_FILE" "$BODY_FILE"
grep -q "<strong>Méthode HTTP:</strong> POST" "$BODY_FILE" || fail "CGI POST should show REQUEST_METHOD=POST"
grep -q "<td>name</td><td>Bob</td>" "$BODY_FILE" || fail "CGI POST should parse name=Bob"
grep -q "<td>email</td><td>bob@example.com</td>" "$BODY_FILE" || fail "CGI POST should decode email"
grep -q "<td>message</td><td>Hello CGI</td>" "$BODY_FILE" || fail "CGI POST should decode + as space"
grep -q "CONTENT_LENGTH</strong></td><td>${expected_len}</td>" "$BODY_FILE" || fail "CGI should receive correct CONTENT_LENGTH"
grep -Eq 'CONTENT_TYPE</strong></td><td>application/x-www-form-urlencoded' "$BODY_FILE" || fail "CGI should receive correct CONTENT_TYPE env"

# Cookies / session (CGI-managed Set-Cookie)
code="$(curl -sS --http1.1 -X GET -D "$HEADERS_FILE" -o "$BODY_FILE" \
  "$WEBSERV_BASE_URL/cgi-bin/test.py" -w "%{http_code}")"
expect_code "$code" 200 "CGI session cookie set" "$HEADERS_FILE" "$BODY_FILE"
session_id="$(grep -i '^Set-Cookie: session_id=' "$HEADERS_FILE" | head -1 | sed -E 's/^[Ss]et-[Cc]ookie:[[:space:]]*session_id=([^;]+).*/\1/')"
[[ -n "$session_id" ]] || fail "CGI should set session_id cookie"
grep -qi '^Set-Cookie: session_id=.*HttpOnly' "$HEADERS_FILE" || fail "session cookie should be HttpOnly"
grep -qi '^Set-Cookie: session_id=.*SameSite=Lax' "$HEADERS_FILE" || fail "session cookie should be SameSite=Lax"

# Second request with Cookie: should not re-set session_id cookie, and visits should increment
code="$(curl -sS --http1.1 -X GET -D "$HEADERS_FILE" -o "$BODY_FILE" \
  -H "Cookie: session_id=$session_id" \
  "$WEBSERV_BASE_URL/cgi-bin/test.py" -w "%{http_code}")"
expect_code "$code" 200 "CGI session cookie reuse" "$HEADERS_FILE" "$BODY_FILE"
if grep -qi '^Set-Cookie: session_id=' "$HEADERS_FILE"; then
  fail "CGI should not re-send session_id cookie when already provided"
fi
grep -q "<strong>session_id</strong></td><td>" "$BODY_FILE" || fail "CGI should receive session_id in HTTP_COOKIE"
grep -Fq "<strong>Session ID:</strong> <code>${session_id}</code>" "$BODY_FILE" || fail "CGI Session ID should match provided session_id cookie"

# Multiple cookies should be forwarded to CGI
code="$(curl -sS --http1.1 -X GET -D "$HEADERS_FILE" -o "$BODY_FILE" \
  -H "Cookie: session_id=$session_id; foo=bar" \
  "$WEBSERV_BASE_URL/cgi-bin/test.py" -w "%{http_code}")"
expect_code "$code" 200 "CGI multiple cookies" "$HEADERS_FILE" "$BODY_FILE"
grep -q "<strong>foo</strong></td><td>bar</td>" "$BODY_FILE" || fail "CGI should receive custom cookie foo=bar"

# Cookie-backed POST should persist username in session
code="$(curl -sS --http1.1 -X POST -D "$HEADERS_FILE" -o "$BODY_FILE" \
  -H "Cookie: session_id=$session_id" \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data 'username=alice' \
  "$WEBSERV_BASE_URL/cgi-bin/test.py" -w "%{http_code}")"
expect_code "$code" 200 "CGI session username" "$HEADERS_FILE" "$BODY_FILE"
grep -q "<strong> User:</strong> alice" "$BODY_FILE" || fail "CGI should persist username in session"

code="$(request GET "$WEBSERV_BASE_URL/cgi-bin/test.cat" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 200 "CGI cat" "$HEADERS_FILE" "$BODY_FILE"
grep -q "Test file for cat" "$BODY_FILE" || fail "CGI cat body mismatch"

# --- Additional HTTP status codes ---
code="$(request GET "$WEBSERV_BASE_URL/noauto" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 403 "GET /noauto (autoindex off, no index)" "$HEADERS_FILE" "$BODY_FILE"

printf '%s' '0123456789' >"$TMPDIR/big_payload.txt"
code="$(request POST "$WEBSERV_BASE_URL/small/too_big" "@$TMPDIR/big_payload.txt" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 413 "POST /small/too_big" "$HEADERS_FILE" "$BODY_FILE"

printf '%s' 'abcd' >"$TMPDIR/exact_4.txt"
code="$(request POST "$WEBSERV_BASE_URL/small/exact_4" "@$TMPDIR/exact_4.txt" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 201 "POST /small/exact_4 (4 bytes)" "$HEADERS_FILE" "$BODY_FILE"

printf '%s' 'abcde' >"$TMPDIR/exact_5.txt"
code="$(request POST "$WEBSERV_BASE_URL/small/exact_5" "@$TMPDIR/exact_5.txt" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 413 "POST /small/exact_5 (5 bytes)" "$HEADERS_FILE" "$BODY_FILE"

code="$(request GET "$WEBSERV_BASE_URL/small/anything" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 405 "GET /small/anything (allow_methods POST)" "$HEADERS_FILE" "$BODY_FILE"

code="$(request DELETE "$WEBSERV_BASE_URL/small/anything" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 405 "DELETE /small/anything (allow_methods POST)" "$HEADERS_FILE" "$BODY_FILE"

# 414 URI Too Long (use a path length that should exceed limits reliably)
long_path="/$(head -c 8000 /dev/zero | tr '\0' 'a')"
code="$(request GET "$WEBSERV_BASE_URL${long_path}" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 414 "GET long URI" "$HEADERS_FILE" "$BODY_FILE"

# 500 Internal Server Error via failing CGI (.false)
code="$(request GET "$WEBSERV_BASE_URL/cgi-bin/test.false" "" "$HEADERS_FILE" "$BODY_FILE")"
expect_code "$code" 500 "CGI false" "$HEADERS_FILE" "$BODY_FILE"

# 400 Bad Request via invalid Content-Length
if ! exec 4<>"/dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"; then
  fail "cannot open /dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"
fi
printf 'POST /bad-length HTTP/1.1\r\n' >&4
printf 'Host: %s:%s\r\n' "$WEBSERV_HOST" "$WEBSERV_PORT" >&4
printf 'Content-Length: abc\r\n' >&4
printf '\r\n' >&4
IFS= read -r -t 3 status_line <&4 || true
status_line="${status_line%$'\r'}"
exec 4<&-
exec 4>&-
case "$status_line" in
  HTTP/1.*\ 400\ *) ;;
  *) fail "bad Content-Length: expected 400, got: ${status_line:-<empty>}";;
esac

# 411 Length Required: POST without Content-Length and without Transfer-Encoding
if ! exec 7<>"/dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"; then
  fail "cannot open /dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"
fi
printf 'POST /no-length HTTP/1.1\r\n' >&7
printf 'Host: %s:%s\r\n' "$WEBSERV_HOST" "$WEBSERV_PORT" >&7
printf 'Content-Type: text/plain\r\n' >&7
printf '\r\n' >&7
printf 'hello' >&7
IFS= read -r -t 3 status_line <&7 || true
status_line="${status_line%$'\r'}"
exec 7<&-
exec 7>&-
case "$status_line" in
  HTTP/1.*\ 411\ *) ;;
  *) fail "no Content-Length: expected 411, got: ${status_line:-<empty>}";;
esac

# 505 HTTP Version Not Supported
if ! exec 5<>"/dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"; then
  fail "cannot open /dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"
fi
printf 'GET / HTTP/2.0\r\nHost: %s\r\n\r\n' "$WEBSERV_HOST" >&5
IFS= read -r -t 3 status_line <&5 || true
status_line="${status_line%$'\r'}"
exec 5<&-
exec 5>&-
case "$status_line" in
  HTTP/1.*\ 505\ *) ;;
  *) fail "HTTP version: expected 505, got: ${status_line:-<empty>}";;
esac

# 431 Request Header Fields Too Large (use a large value so we're well above buffers)
big_header_value="$(head -c 20000 /dev/zero | tr '\0' 'b')"
if ! exec 6<>"/dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"; then
  fail "cannot open /dev/tcp/${WEBSERV_HOST}/${WEBSERV_PORT}"
fi
printf 'GET / HTTP/1.1\r\n' >&6
printf 'Host: %s:%s\r\n' "$WEBSERV_HOST" "$WEBSERV_PORT" >&6
printf 'X-Big: %s\r\n' "$big_header_value" >&6
printf '\r\n' >&6
IFS= read -r -t 3 status_line <&6 || true
status_line="${status_line%$'\r'}"
exec 6<&-
exec 6>&-
case "$status_line" in
  HTTP/1.*\ 431\ *) ;;
  *) fail "big headers: expected 431, got: ${status_line:-<empty>}";;
esac

# --- Stress tests (server already running) ---
echo "[HTTP] stress"
bash "$SCRIPT_DIR/../stress/get_1000.sh"
bash "$SCRIPT_DIR/../stress/post_1000.sh"
bash "$SCRIPT_DIR/../stress/upload_1000.sh"

# Optional siege stress (skips if siege isn't installed)
START_SERVER=0 bash "$SCRIPT_DIR/../stress/siege.sh"

echo "[OK] http suite"
