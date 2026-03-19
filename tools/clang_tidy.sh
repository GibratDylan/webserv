#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  cat <<'EOF'
Usage:
  tools/clang_tidy.sh [options] [files...]

Runs clang-tidy using the repo's .clang-tidy config.
Requires a compilation database (compile_commands.json).

Options:
  --build-db         Generate compile_commands.json using "bear -- make re".
  --auto-build-db    If compile_commands.json is missing, try to generate it.
  --all              Run on all project sources (src/**/*.cpp, include/**/*.hpp).
  --changed          Run on modified files (git diff) limited to src/include.
  --fix              Apply clang-tidy fixes (--fix --fix-errors).
  --no-werror         Do not treat warnings as errors.
  --jobs N           Parallelism hint (passed to clang-tidy via -j N when supported).
  -h, --help         Show help.

Examples:
  ./tools/clang_tidy.sh --build-db
  ./tools/clang_tidy.sh --changed
  ./tools/clang_tidy.sh --changed --fix
  ./tools/clang_tidy.sh --all
  ./tools/clang_tidy.sh src/server/Server.cpp include/server/Server.hpp
EOF
}

need_cmd() {
  command -v "$1" >/dev/null 2>&1
}

escape_regex() {
  # Escape a string so it can be safely embedded in an ERE regex.
  # shellcheck disable=SC2001
  echo "$1" | sed 's/[.[\\*^$()+?{}|]/\\&/g'
}

ensure_compdb() {
  if [[ -f "$ROOT_DIR/compile_commands.json" ]]; then
    return 0
  fi

  if [[ "${AUTO_BUILD_DB:-0}" == "1" ]]; then
    build_db
    return 0
  fi

  echo "[clang-tidy] Missing compile_commands.json" >&2
  echo "[clang-tidy] Run: ./tools/clang_tidy.sh --build-db" >&2
  echo "[clang-tidy] (requires 'bear' installed)" >&2
  return 2
}

build_db() {
  if ! need_cmd bear; then
    echo "[clang-tidy] 'bear' not found in PATH." >&2
    echo "[clang-tidy] Install it (ex: apt: sudo apt-get install bear)" >&2
    return 2
  fi

  echo "[clang-tidy] Generating compile_commands.json via: bear -- make re" >&2
  (cd "$ROOT_DIR" && bear -- make re >/dev/null)

  if [[ ! -f "$ROOT_DIR/compile_commands.json" ]]; then
    echo "[clang-tidy] Failed to generate compile_commands.json" >&2
    return 2
  fi

  echo "[clang-tidy] compile_commands.json generated" >&2
}

collect_all_files() {
  (cd "$ROOT_DIR" && find src include \
    -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) \
    -print)
}

collect_changed_files() {
  if ! need_cmd git; then
    echo "[clang-tidy] git not available; cannot use --changed" >&2
    return 2
  fi

  (cd "$ROOT_DIR" && git diff --name-only --diff-filter=ACMR | \
    grep -E '^(src|include)/.*\.(cpp|hpp|h)$' || true)
}

JOBS=""
MODE=""
DO_FIX=0
DO_WERROR=1
AUTO_BUILD_DB=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-db)
      build_db
      exit 0
      ;;
    --auto-build-db)
      AUTO_BUILD_DB=1
      shift
      ;;
    --all)
      MODE="all"
      shift
      ;;
    --changed)
      MODE="changed"
      shift
      ;;
    --fix)
      DO_FIX=1
      shift
      ;;
    --no-werror)
      DO_WERROR=0
      shift
      ;;
    --jobs)
      JOBS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    *)
      break
      ;;
  esac
done

if ! need_cmd clang-tidy; then
  echo "[clang-tidy] clang-tidy not found in PATH." >&2
  echo "[clang-tidy] Install it (ex: sudo apt-get install clang-tidy)" >&2
  exit 2
fi

ensure_compdb

FILES=()
if [[ $# -gt 0 ]]; then
  for f in "$@"; do
    # Accept relative paths; normalize them to repo-root relative.
    if [[ "$f" == /* ]]; then
      FILES+=("$f")
    else
      FILES+=("$ROOT_DIR/$f")
    fi
  done
else
  case "$MODE" in
    changed)
      mapfile -t rels < <(collect_changed_files)
      if [[ ${#rels[@]} -eq 0 ]]; then
        echo "[clang-tidy] No changed src/include .cpp/.hpp/.h files" >&2
        exit 0
      fi
      for r in "${rels[@]}"; do
        FILES+=("$ROOT_DIR/$r")
      done
      ;;
    all|"")
      mapfile -t rels < <(collect_all_files)
      for r in "${rels[@]}"; do
        FILES+=("$ROOT_DIR/$r")
      done
      ;;
    *)
      echo "[clang-tidy] Unknown mode: $MODE" >&2
      exit 2
      ;;
  esac
fi

ROOT_ESCAPED="$(escape_regex "$ROOT_DIR")"
HEADER_FILTER="^${ROOT_ESCAPED}/(src|include)/"

TIDY_ARGS=(
  "-p" "$ROOT_DIR"
  "--header-filter=${HEADER_FILTER}"
)

if [[ "$DO_WERROR" == "1" ]]; then
  TIDY_ARGS+=("-warnings-as-errors=*")
fi

if [[ -n "$JOBS" ]]; then
  # clang-tidy supports -j N on most installs.
  TIDY_ARGS+=("-j" "$JOBS")
fi

if [[ "$DO_FIX" == "1" ]]; then
  TIDY_ARGS+=("--fix" "--fix-errors")
fi

echo "[clang-tidy] Running on ${#FILES[@]} file(s)" >&2

EXIT_CODE=0
for file in "${FILES[@]}"; do
  if [[ ! -f "$file" ]]; then
    echo "[clang-tidy] Skip missing: $file" >&2
    continue
  fi
  clang-tidy "$file" "${TIDY_ARGS[@]}" || EXIT_CODE=$?
done

exit "$EXIT_CODE"
