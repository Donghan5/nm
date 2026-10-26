#!/usr/bin/env bash
set -eu
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ft-nm-day2.XXXXXX")
trap 'rm -rf -- "$TASK_TMP"' EXIT
export LC_ALL=C
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1

if ! "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$ROOT/inc" -I"$ROOT" "$ROOT/tests/day2/day2_elf_test.c" \
    "$ROOT/src/elf/elf_ident.c" -o "$TASK_TMP/day2_elf_test" \
    > "$TASK_TMP/build.log" 2>&1; then
    printf 'FAIL: day2 test build (runtime tests were not executed)\n'
    cat "$TASK_TMP/build.log"
    exit 1
fi
passed=0
failed=0
for bits in 32 64; do
    if ! "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
        -fsanitize=address,undefined -fno-omit-frame-pointer -DDAY2_BITS="$bits" \
        -I"$ROOT/inc" -I"$ROOT" "$ROOT/tests/day2/day2_table_test.c" \
        "$ROOT/src/elf/elf_ident.c" "$ROOT/src/elf/elf32.c" "$ROOT/src/elf/elf64.c" \
        "$ROOT/libft/ft_memcpy.c" -o "$TASK_TMP/table-$bits" \
        > "$TASK_TMP/build-$bits.log" 2>&1; then
        printf 'FAIL: ELF%s table test build (runtime tests were not executed)\n' "$bits"
        cat "$TASK_TMP/build-$bits.log"
        exit 1
    fi
done
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
for bits in 32 64; do
    cases=(regular none exec shared extended-sections extended-truncated extended-programs
        program program-truncated program-entry program-zero-offset
        bad-magic wrong-class big-endian header-size header-version machine type
        section-entry section-truncated section-offset zero-offset-count reserved-count
        extended-small short-header null-args)
    if [ "$bits" = 64 ]; then cases+=(huge-offset huge-count); fi
    for name in "${cases[@]}"; do
        if timeout 10s "$TASK_TMP/table-$bits" "$name" > "$TASK_TMP/table.log" 2>&1; then
            printf 'PASS: ELF%s/%s\n' "$bits" "$name"
            passed=$((passed + 1))
        else
            printf 'FAIL: ELF%s/%s\n' "$bits" "$name"
            cat "$TASK_TMP/table.log"
            failed=$((failed + 1))
        fi
    done
done
printf '\nResult: %s passed, %s failed\n' "$passed" "$failed"
printf 'Scope: identification, ELF32/ELF64 table getters and readers; section payload validation is not covered.\n'
test "$failed" -eq 0
