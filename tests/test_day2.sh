#!/usr/bin/env bash
set -eu
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ft-nm-day2.XXXXXX")
trap 'rm -rf -- "$TASK_TMP"' EXIT
export LC_ALL=C
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1

if ! "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$ROOT/inc" -I"$ROOT" "$ROOT/tests/day2_elf_test.c" \
    "$ROOT/src/elf/elf_ident.c" -o "$TASK_TMP/day2_elf_test" \
    > "$TASK_TMP/build.log" 2>&1; then
    printf 'FAIL: day2 test build (runtime tests were not executed)\n'
    cat "$TASK_TMP/build.log"
    exit 1
fi
passed=0
failed=0
for name in ranges short-inputs valid-ident magic class endian version; do
    if timeout 10s "$TASK_TMP/day2_elf_test" "$name" \
        > "$TASK_TMP/$name.log" 2>&1; then
        printf 'PASS: %s\n' "$name"
        passed=$((passed + 1))
    else
        printf 'FAIL: %s\n' "$name"
        cat "$TASK_TMP/$name.log"
        failed=$((failed + 1))
    fi
done
printf '\nResult: %s passed, %s failed\n' "$passed" "$failed"
printf 'Scope: ELF identification and ranges only; ELF headers/sections are not tested yet.\n'
test "$failed" -eq 0
