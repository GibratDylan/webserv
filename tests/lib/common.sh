#!/usr/bin/env bash
set -euo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ROOT_DIR="$(cd "$TESTS_DIR/.." && pwd)"

: "${WEBSERV_BIN:=$ROOT_DIR/webserv}"
: "${WEBSERV_CONF:=$ROOT_DIR/tests/config/test.conf}"
: "${WEBSERV_HOST:=127.0.0.1}"
: "${WEBSERV_PORT:=8080}"

WEBSERV_BASE_URL="http://${WEBSERV_HOST}:${WEBSERV_PORT}"

fail() {
  echo "[FAIL] $*" >&2
  exit 1
}

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || fail "missing dependency: $1"
}

assert_executable() {
  [[ -x "$1" ]] || fail "binary not found or not executable: $1 (run: make)"
}

mktemp_dir() {
  mktemp -d 2>/dev/null || mktemp -d -t webserv-tests
}

port_in_use() {
  local port="$1"
  if command -v ss >/dev/null 2>&1; then
    ss -ltn 2>/dev/null | awk '{print $4}' | grep -qE "(:|\\])${port}$"
    return $?
  fi
  if command -v lsof >/dev/null 2>&1; then
    lsof -nP -iTCP:"$port" -sTCP:LISTEN >/dev/null 2>&1
    return $?
  fi
  return 1
}

start_server() {
  local log_file="$1"

  require_cmd curl
  assert_executable "$WEBSERV_BIN"

  if port_in_use "$WEBSERV_PORT"; then
    fail "port $WEBSERV_PORT already in use"
  fi

  "$WEBSERV_BIN" "$WEBSERV_CONF" >"$log_file" 2>&1 &
  SERVER_PID=$!

  # Wait server up
  for _ in {1..60}; do
    if curl -sS -o /dev/null "$WEBSERV_BASE_URL/"; then
      return 0
    fi
    sleep 0.1
  done

  echo "--- server.log ---" >&2
  tail -200 "$log_file" >&2 || true
  fail "server did not start on ${WEBSERV_HOST}:${WEBSERV_PORT}"
}

stop_server() {
  if [[ -z "${SERVER_PID:-}" ]]; then
    return 0
  fi

  kill -INT "$SERVER_PID" >/dev/null 2>&1 || true
  for _ in {1..30}; do
    if ! kill -0 "$SERVER_PID" >/dev/null 2>&1; then
      break
    fi
    sleep 0.1
  done
  kill -KILL "$SERVER_PID" >/dev/null 2>&1 || true
  wait "$SERVER_PID" >/dev/null 2>&1 || true
  unset SERVER_PID
}

check_config_file() {
  local file="$1"
  local expect_ok="$2" # 1 ok, 0 fail

  assert_executable "$WEBSERV_BIN"

  set +e
  "$WEBSERV_BIN" --check-config "$file" >/dev/null 2>&1
  local rc=$?
  set -e

  if [[ "$expect_ok" == "1" && "$rc" -ne 0 ]]; then
    fail "config should parse OK but failed: $file"
  fi
  if [[ "$expect_ok" == "0" && "$rc" -eq 0 ]]; then
    fail "config should fail parsing but succeeded: $file"
  fi
}

request() {
  local method="$1"; shift
  local url="$1"; shift
  local data="${1-}"; shift || true
  local headers_file="$1"; shift
  local body_file="$1"; shift

  : >"$headers_file"
  : >"$body_file"

  if [[ -n "$data" ]]; then
    curl -sS --http1.1 -X "$method" -D "$headers_file" -o "$body_file" --data-binary "$data" "$url" -w "%{http_code}"
  else
    curl -sS --http1.1 -X "$method" -D "$headers_file" -o "$body_file" "$url" -w "%{http_code}"
  fi
}

expect_code() {
  local got="$1" expected="$2" label="$3" headers_file="$4" body_file="$5"

  if [[ "$got" != "$expected" ]]; then
    echo "--- last response headers ---" >&2
    cat "$headers_file" >&2 || true
    echo "--- last response body (first 2000 bytes) ---" >&2
    head -c 2000 "$body_file" >&2 || true
    echo >&2
    fail "$label: expected HTTP $expected, got $got"
  fi
}
