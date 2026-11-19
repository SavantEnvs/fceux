#!/usr/bin/env bash
# mayhem/test.sh — run the fceux known-answer self-test built by build.sh and diff each result line
# (ROM MD5, RAM/framebuffer CRCs after N emulated frames, savestate round-trip, Game Genie decode,
# mapper 218 rendering, ld65 symbol import) against mayhem/tests/golden.txt. One CTRF test per line.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
SRC="${SRC:-/mayhem}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

BIN="$SRC/mayhem/build/fceux_selftest"
GOLDEN="$SRC/mayhem/tests/golden.txt"
if [ ! -x "$BIN" ]; then echo "missing $BIN (run mayhem/build.sh)" >&2; emit_ctrf fceux-selftest 0 1; exit 1; fi
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
TMPDIR="$tmp" "$BIN" > "$tmp/out.txt" 2>"$tmp/err.txt"
rc=$?
passed=0; failed=0
while IFS= read -r want; do
  key=${want%% *}
  got=$(grep -m1 "^$key " "$tmp/out.txt" || true)
  if [ "$got" = "$want" ]; then passed=$((passed+1)); else failed=$((failed+1)); echo "FAIL: want '$want' got '$got'"; fi
done < "$GOLDEN"
[ $rc -eq 0 ] || { echo "selftest exited $rc"; failed=$((failed+1)); }
emit_ctrf fceux-selftest "$passed" "$failed"
