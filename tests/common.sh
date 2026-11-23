#!/usr/bin/env bash
# Shared build, timeout and reporting helpers; sourced by both Bash runners.
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
read -r -a compiler <<< "${CC:-cc}"
base=(-Wall -Wextra -Werror -g -O1 -I"$ROOT/inc")
sanitize=()
selection=${NM_TEST_SANITIZERS:-none}
case "$selection" in
    address|undefined|address,undefined) sanitize=(-fsanitize="$selection" -fno-omit-frame-pointer) ;;
    none) ;;
    *) printf 'Invalid NM_TEST_SANITIZERS: %s\n' "$selection" >&2; exit 1 ;;
esac
export LC_ALL=C
# Test builds use temporary outputs and must work with a read-only ccache directory.
export CCACHE_DISABLE=${CCACHE_DISABLE:-1}
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
export UBSAN_OPTIONS=${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/nm-edge.XXXXXX")
work="$TASK_TMP/build"
mkdir "$work"
trap 'rm -rf -- "$TASK_TMP"' EXIT
passed=0 failed=0 skipped=0 warned=0
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
fixtures="$ROOT/tests/edge_case_test/fixtures"
describe_case() {
    case "$1" in
        file-*) printf 'File handling: %s' "${1#file-}" ;;
        ident-*) printf 'ELF identification: %s' "${1#ident-}" ;;
        table*) printf 'ELF header/section table: %s' "$1" ;;
        symbols*) printf 'Symbol/string table boundaries or output: %s' "$1" ;;
        extended*) printf 'Extended indexes and section names: %s' "$1" ;;
        payload*) printf 'Section payload policy: %s' "$1" ;;
        elf*) printf 'FD/mapping lifetime and injected failures: %s' "$1" ;;
        *) printf '%s' "$1" ;;
    esac
}
record() {
    local status=$1 name=$2 detail=${3:-}
    [[ -n "$detail" ]] || detail=$(describe_case "$name")
    printf '[%s] %s — %s\n' "$status" "$name" "$detail"
    case "$status" in
        PASS) passed=$((passed + 1)) ;;
        FAIL)
            failed=$((failed + 1))
            # Display available diagnostics before temporary files are removed.
            local file
            for file in "$TASK_TMP/$name.log" "$TASK_TMP/build-$name.log" \
                "$TASK_TMP/$name.err" "$TASK_TMP/$name.ft.err"; do
                if [[ -s "$file" ]]; then cat "$file"; fi
            done
            ;;
        SKIP) skipped=$((skipped + 1)) ;;
        # WARN is not a test result: it reports a host capability the suite needs
        # but cannot provide for itself. It never changes the exit status, so the
        # detail must name every check that did not run.
        WARN) warned=$((warned + 1)) ;;
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
    # No fixture may report "untestable": every case either runs or fails. Any
    # platform limitation is a compile-time assertion inside the fixture.
    if timeout 10s "$@" > "$TASK_TMP/$name.log" 2>&1; then
        record PASS "$name"
    else
        record FAIL "$name"
    fi
}
finish() {
    # The counts are always printed, zeros included, so "nothing was skipped or
    # warned" is visible evidence rather than an absent line.
    printf 'Result: %s PASS / %s FAIL / %s SKIP / %s WARN\n' \
        "$passed" "$failed" "$skipped" "$warned"
    [[ "$warned" == 0 ]] || printf 'Warnings above report checks that did not run on this host.\n'
    [[ "$failed" == 0 ]] || exit 1
    [[ "$skipped" == 0 ]] || exit 2
}
