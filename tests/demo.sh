#!/usr/bin/env bash
# Display actual commands, symbol output, diagnostics and exit statuses.
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export LC_ALL=C
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/nm-demo.XXXXXX")
trap 'rm -rf -- "$sandbox"' EXIT
show() {
    local code=0
    printf '\n$'
    printf ' %q' "$@"
    printf '\n'
    "$@" || code=$?
    printf '[exit %s]\n' "$code"
}
"${AS:-as}" --64 "$ROOT/tests/eval_test/symbols.s" -o "$sandbox/example64.o"
"${AS:-as}" --32 "$ROOT/tests/eval_test/symbols.s" -o "$sandbox/example32.o"
printf '\n=== Default symbol output: compare GNU nm and ft_nm ===\n'
show nm "$sandbox/example64.o"
show "$ROOT/ft_nm" "$sandbox/example64.o"
printf '\n=== Reverse name order (-r) ===\n'
show "$ROOT/ft_nm" -r "$sandbox/example64.o"
printf '\n=== Symbol-table order (-p); -p also takes priority over -r ===\n'
show "$ROOT/ft_nm" -p "$sandbox/example64.o"
show "$ROOT/ft_nm" -rp "$sandbox/example64.o"
printf '\n=== ELF32/64 address widths and multiple-file headings ===\n'
show "$ROOT/ft_nm" "$sandbox/example32.o" "$sandbox/example64.o"
printf '\n=== Error followed by a valid file: processing continues, exit 1 ===\n'
show "$ROOT/ft_nm" "$sandbox/missing.o" "$sandbox/example64.o"
printf '\n=== Malformed ELF is rejected ===\n'
show "$ROOT/ft_nm" "$ROOT/tests/eval_test/header"
printf '\n=== Default a.out ===\n'
cp "$sandbox/example64.o" "$sandbox/a.out"
cd "$sandbox"
show "$ROOT/ft_nm"
