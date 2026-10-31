#!/usr/bin/env bash
# Shared build, timeout and reporting helpers; sourced by both Bash runners.
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
read -r -a compiler <<< "${CC:-cc}"
base=(-Wall -Wextra -Werror -g -O1 -I"$ROOT/inc" -include "$ROOT/inc/ft_nm.h")
sanitize=()
selection=${NM_TEST_SANITIZERS:-address,undefined}
case "$selection" in
    address|undefined|address,undefined) sanitize=(-fsanitize="$selection" -fno-omit-frame-pointer) ;;
    none) ;;
    *) printf 'Invalid NM_TEST_SANITIZERS: %s\n' "$selection" >&2; exit 1 ;;
esac
export LC_ALL=C
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
export UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/nm-edge.XXXXXX")
work="$TASK_TMP/build"
mkdir "$work"
trap 'rm -rf -- "$work"' EXIT
passed=0 failed=0 skipped=0
printf 'Sanitizers: %s\n' "$selection"
[[ "$selection" == *address* ]] || printf 'SKIP: ASan/LSan not selected\n'
[[ "$selection" == *undefined* ]] || printf 'SKIP: UBSan not selected\n'
if [[ ":$ASAN_OPTIONS:" == *:detect_leaks=0:* ]]; then
    printf 'SKIP: LeakSanitizer (detect_leaks=0)\n'
fi
elf=("$ROOT/src/elf/elf_ident.c" "$ROOT/src/elf/elf32.c" "$ROOT/src/elf/elf64.c")
collect=("$ROOT/src/elf/get_elf_section.c" "$ROOT/src/symbol/symbol_collect_32.c"
    "$ROOT/src/symbol/symbol_collect_64.c" "$ROOT/src/symbol/symbol_validate.c"
    "$ROOT/src/symbol/symbol_tools.c")
cli_sources=("$ROOT/src/main.c" "$ROOT/src/utils/file.c" "$ROOT/src/utils/error.c"
    "${elf[@]}" "${collect[@]}" "$ROOT/src/symbol/symbol_type.c"
    "$ROOT/src/symbol/symbol_sort.c" "$ROOT/src/symbol/symbol_print.c")
fixtures="$ROOT/tests/days_test/fixtures"
record() {
    local status=$1 name=$2
    printf '%s\t%s\n' "$status" "$name" >> "$TASK_TMP/results.tsv"
    case "$status" in
        PASS) passed=$((passed + 1)) ;;
        FAIL) failed=$((failed + 1)); printf 'FAIL: %s (%s)\n' "$name" "$TASK_TMP" ;;
        SKIP) skipped=$((skipped + 1)); printf 'SKIP: %s\n' "$name" ;;
    esac
}
compile() {
    local name=$1; shift
    if timeout 60s "${compiler[@]}" "${base[@]}" "${sanitize[@]}" "$@" \
        -o "$work/$name" > "$TASK_TMP/build-$name.log" 2>&1; then
        return 0
    fi
    record FAIL "build/$name"
    cat "$TASK_TMP/build-$name.log"
    return 1
}
run_case() {
    local name=$1; shift
    if timeout 10s "$@" > "$TASK_TMP/$name.log" 2>&1; then
        record PASS "$name"
    else
        record FAIL "$name"
        cat "$TASK_TMP/$name.log"
    fi
}
finish() {
    printf 'Result: %s PASS / %s FAIL / %s SKIP\nLogs: %s\n' "$passed" "$failed" "$skipped" "$TASK_TMP"
    [[ "$failed" == 0 ]] || exit 1
    [[ "$skipped" == 0 ]] || exit 2
}
