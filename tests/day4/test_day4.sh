#!/usr/bin/env bash
set -eu
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ft-nm-day4.XXXXXX")
trap 'rm -rf -- "$TASK_TMP"' EXIT
export LC_ALL=C
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
sanitizers=${DAY4_SANITIZERS-address,undefined}
sanitizer_flags=()
case "$sanitizers" in
    address,undefined|address|undefined)
        sanitizer_flags=("-fsanitize=$sanitizers" -fno-omit-frame-pointer)
        ;;
    none) ;;
    *) printf 'Invalid DAY4_SANITIZERS: %s\n' "$sanitizers" >&2; exit 1 ;;
esac
printf 'Sanitizers: %s\n' "$sanitizers"
passed=0
failed=0
unavailable=0
"${CC:-cc}" --version | head -n 1
nm --version | head -n 1
printf 'LC_ALL=%s; native test runner reads both ELF32 and ELF64 sections.\n' "$LC_ALL"
cases=(undefined absolute-global absolute-local common
    weak-object-defined weak-object-undefined weak-function-defined weak-function-undefined
    weak-notype-defined weak-notype-undefined text-global text-local text-notype
    data-global data-local bss-global bss-local readonly-global readonly-local
    readonly-merge-strings text-extra-group tls-data tls-bss index-out-of-range index-huge)
if "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
    "${sanitizer_flags[@]}" \
    -I"$ROOT/inc" -I"$ROOT" "$ROOT/tests/day4/day4_type_test.c" \
    "$ROOT/src/symbol/symbol_type.c" "$ROOT/src/elf/elf_ident.c" \
    "$ROOT/src/elf/elf32.c" "$ROOT/src/elf/elf64.c" \
    -o "$TASK_TMP/type-test" > "$TASK_TMP/build.log" 2>&1; then
    for bits in 32 64; do
        for name in "${cases[@]}"; do
            if timeout 10s "$TASK_TMP/type-test" "$bits" "$name" > "$TASK_TMP/run.log" 2>&1; then
                printf 'PASS: ELF%s/%s\n' "$bits" "$name"
                passed=$((passed + 1))
            else
                printf 'FAIL: ELF%s/%s\n' "$bits" "$name"
                cat "$TASK_TMP/run.log"
                failed=$((failed + 1))
            fi
        done
    done
else
    printf 'FAIL: classification build; all 50 runtime cases NOT RUN.\n'
    cat "$TASK_TMP/build.log"
    failed=$((failed + 1))
fi
# A CLI path is explicit: do not accidentally test an old workspace binary.
for bits in 32 64; do
    if ! "${CC:-cc}" -m"$bits" -c "$ROOT/tests/day4/day4_fixture.s" \
        -o "$TASK_TMP/fixture-$bits.o" > "$TASK_TMP/fixture.log" 2>&1; then
        printf 'UNAVAILABLE: ELF%s fixture generation.\n' "$bits"
        cat "$TASK_TMP/fixture.log"
        unavailable=$((unavailable + 1))
    elif [ -z "${DAY4_NM:-}" ]; then
        printf 'UNAVAILABLE: ELF%s exact output; set DAY4_NM to a freshly built ft_nm.\n' "$bits"
        unavailable=$((unavailable + 1))
    elif python3 "$ROOT/tests/day4/day4_compare.py" "$DAY4_NM" "$TASK_TMP/fixture-$bits.o"; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
    fi
done
printf '\nResult: %s passed, %s failed, %s unavailable\n' "$passed" "$failed" "$unavailable"
if [ "$failed" -ne 0 ]; then exit 1; fi
# Incomplete output coverage must not look like complete Day 4 success.
if [ "$unavailable" -ne 0 ]; then exit 2; fi
