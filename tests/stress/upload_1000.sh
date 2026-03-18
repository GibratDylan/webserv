#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

require_cmd curl
require_cmd xargs
require_cmd dd

URL="${URL:-$WEBSERV_BASE_URL/upload}"
TOTAL="${TOTAL:-1000}"
PARALLEL="${PARALLEL:-50}"

TMPDIR="$(mktemp_dir)"
trap 'rm -rf "$TMPDIR"' EXIT

payload="$TMPDIR/bigfile.bin"
# 100 KiB
DD_COUNT="${DD_COUNT:-100}"
dd if=/dev/zero of="$payload" bs=1k count="$DD_COUNT" status=none

errors_file="$TMPDIR/errors.txt"
: >"$errors_file"

export URL payload errors_file
seq "$TOTAL" | xargs -P"$PARALLEL" -I{} bash -c '
  code=$(curl -sS -o /dev/null -w "%{http_code}" -X POST --data-binary "@$payload" "$URL" || echo 000)
  case "$code" in
    2*|3*) exit 0;;
    *) echo "UPLOAD error: got $code" >>"$errors_file"; exit 0;;
  esac
'

if [[ -s "$errors_file" ]]; then
  echo "--- errors (first 20) ---" >&2
  head -20 "$errors_file" >&2 || true
  fail "upload stress: some requests failed"
fi

echo "[OK] upload stress: $TOTAL requests ($PARALLEL parallel)"
