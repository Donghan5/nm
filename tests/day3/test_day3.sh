#!/usr/bin/env bash
set -eu
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ft-nm-day3.XXXXXX")
trap 'rm -rf -- "$TASK_TMP"' EXIT
export LC_ALL=C
sanitizers=${DAY3_SANITIZERS:-${NM_TEST_SANITIZERS-address,undefined}}
source "$ROOT/tests/sanitizers.sh"
passed=0
failed=0
unavailable=0
for bits in 32 64; do
    if ! "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
        "${sanitizer_flags[@]}" -DDAY3_BITS="$bits" \
        -I"$ROOT/inc" -I"$ROOT" "$ROOT/tests/day3/day3_symbol_test.c" \
        "$ROOT/src/elf/elf_ident.c" "$ROOT/src/elf/elf32.c" "$ROOT/src/elf/elf64.c" \
        "$ROOT/src/symbol/symbol_collect.c" "$ROOT/src/symbol/symbol_validate.c" \
        -o "$TASK_TMP/symbol-$bits" > "$TASK_TMP/build.log" 2>&1; then
        printf 'FAIL: Day 3 ELF%s test build; runtime tests not executed.\n' "$bits"
        cat "$TASK_TMP/build.log"
        exit 1
    fi
    cases=(regular abs common local missing dynsym-only link-oob link-type entry-zero
        entry-wrong entry-partial symbol-payload string-payload name-oob name-no-nul
        section-oob xindex-unsupported name-max-fit name-too-long reader-contract string-contract)
    if [ "$bits" = 64 ]; then cases+=(huge-offset huge-size); fi
    for name in "${cases[@]}"; do
        if timeout 10s "$TASK_TMP/symbol-$bits" "$name" > "$TASK_TMP/run.log" 2>&1; then
            printf 'PASS: ELF%s/%s\n' "$bits" "$name"
            passed=$((passed + 1))
        else
            printf 'FAIL: ELF%s/%s\n' "$bits" "$name"
            cat "$TASK_TMP/run.log"
            failed=$((failed + 1))
        fi
    done
    if ! "${CC:-cc}" -m"$bits" -fno-lto -fno-pic -c "$ROOT/tests/day3/day3_fixture.c" \
        -o "$TASK_TMP/fixture-$bits.o" > "$TASK_TMP/fixture.log" 2>&1; then
        printf 'UNAVAILABLE: ELF%s real object generation; this check did not run.\n' "$bits"
        cat "$TASK_TMP/fixture.log"
        unavailable=$((unavailable + 1))
    elif timeout 15s python3 "$ROOT/tests/day3/day3_compare.py" \
        "$TASK_TMP/symbol-$bits" "$TASK_TMP/fixture-$bits.o"; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
    fi
done
printf '\nResult: %s passed, %s failed, %s unavailable\n' "$passed" "$failed" "$unavailable"
# Missing real-object coverage cannot certify the full Day 3 criteria.
test "$failed" -eq 0 && test "$unavailable" -eq 0
