#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

require_cmd curl

URL="${URL:-$WEBSERV_BASE_URL/upload.txt}"
EXPECTED="${EXPECTED:-201}"
TOTAL="${TOTAL:-1000}"

TMPDIR="$(mktemp_dir)"
trap 'rm -rf "$TMPDIR"' EXIT

echo "data=test" >"$TMPDIR/body.txt"

errors=0
for _ in $(seq 1 "$TOTAL"); do
  code=$(curl -sS -o /dev/null -w "%{http_code}" -X POST -d "@$TMPDIR/body.txt" "$URL" || echo 000)
  if [[ "$code" != "$EXPECTED" ]]; then
    errors=$((errors + 1))
  fi
done

if [[ "$errors" -ne 0 ]]; then
  fail "POST stress: $errors/$TOTAL requests returned != $EXPECTED"
fi

echo "[OK] POST stress: $TOTAL requests"
