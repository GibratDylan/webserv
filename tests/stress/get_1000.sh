#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "$SCRIPT_DIR/../lib/common.sh"

require_cmd curl
require_cmd xargs

URL="${URL:-$WEBSERV_BASE_URL/}"
EXPECTED="${EXPECTED:-200}"
TOTAL="${TOTAL:-1000}"
PARALLEL="${PARALLEL:-100}"

errors_file="$(mktemp)"
trap 'rm -f "$errors_file"' EXIT

export URL EXPECTED errors_file

seq "$TOTAL" | xargs -P"$PARALLEL" -I{} bash -c '
  code=$(curl -sS -o /dev/null -w "%{http_code}" "$URL" || echo 000)
  if [ "$code" != "$EXPECTED" ]; then
    echo "GET error: got $code" >>"$errors_file"
  fi
'

if [[ -s "$errors_file" ]]; then
  echo "--- errors (first 20) ---" >&2
  head -20 "$errors_file" >&2 || true
  fail "GET stress: some requests failed"
fi

echo "[OK] GET stress: $TOTAL requests ($PARALLEL parallel)"
