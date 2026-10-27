#!/usr/bin/env bash
set -eu

ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
TASK_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ft-nm-day1.XXXXXX")
trap 'rm -rf -- "$TASK_TMP"' EXIT
export LC_ALL=C
mode=${1:-native}
build_flags=()
if [ "$mode" = 32 ]; then
    # Keep the conversion-only suite independent of 32-bit sanitizer runtimes.
    build_flags=(-m32 -D_FILE_OFFSET_BITS=64 -DDAY1_TEST_32 -Wl,--wrap=fstat64)
    if ! "${CC:-cc}" -m32 -D_FILE_OFFSET_BITS=64 \
        "$ROOT/tests/day1/day1_32_probe.c" -o "$TASK_TMP/probe" \
        > "$TASK_TMP/probe.log" 2>&1; then
        printf 'ENVIRONMENT ERROR: 32-bit compiler/libc support is unavailable; no tests ran.\n'
        printf 'See tests/day1/README.md for setup requirements.\n'
        cat "$TASK_TMP/probe.log"
        exit 1
    fi
    if ! timeout 10s "$TASK_TMP/probe"; then
        printf 'ENVIRONMENT ERROR: cannot execute the required 32-bit ABI; no tests ran.\n'
        exit 1
    fi
    printf '32-bit conversion suite: size_t=32 bits, off_t=64 bits (sanitizers disabled)\n'
elif [ "$mode" != native ]; then
    printf 'Usage: bash tests/day1/test_day1.sh [native|32]\n' >&2
    exit 1
else
    sanitizers=${DAY1_SANITIZERS:-${NM_TEST_SANITIZERS-address,undefined}}
    source "$ROOT/tests/sanitizers.sh"
    build_flags=("${sanitizer_flags[@]}" -Wl,--wrap=fstat)
fi

printf 'hello\n' > "$TASK_TMP/regular"
: > "$TASK_TMP/empty"
mkdir "$TASK_TMP/directory"

# Build the actual headers and implementation without bypassing warnings or errors.
if ! "${CC:-cc}" -Wall -Wextra -Werror -g -O1 \
    "${build_flags[@]}" \
    -I"$ROOT/inc" -I"$ROOT" \
    "$ROOT/tests/day1/day1_file_test.c" "$ROOT/src/file.c" \
    "$ROOT/src/utils/error.c" \
    -o "$TASK_TMP/day1_file_test" > "$TASK_TMP/build.log" 2>&1; then
    printf 'FAIL: day1 test build (runtime tests were not executed)\n'
    cat "$TASK_TMP/build.log"
    exit 1
fi

passed=0
failed=0
not_applicable=0
run_case() {
    local name=$1 path=$2
    local status
    if timeout 10s "$TASK_TMP/day1_file_test" "$name" "$path" \
        > "$TASK_TMP/$name.log" 2>&1; then
        printf 'PASS: %s\n' "$name"
        passed=$((passed + 1))
    else
        status=$?
        if [ "$status" -eq 77 ] && [ "$mode" = native ]; then
            printf 'N/A: %s - ' "$name"
            cat "$TASK_TMP/$name.log"
            not_applicable=$((not_applicable + 1))
        else
            printf 'FAIL: %s\n' "$name"
            cat "$TASK_TMP/$name.log"
            failed=$((failed + 1))
        fi
    fi
}

if [ "$mode" = native ]; then
    run_case cleanup-zero "$TASK_TMP/regular"
    run_case regular "$TASK_TMP/regular"
    run_case empty "$TASK_TMP/empty"
    run_case missing "$TASK_TMP/missing"
    run_case directory "$TASK_TMP/directory"
    run_case repeat "$TASK_TMP/regular"
fi
run_case size-negative "$TASK_TMP/regular"
run_case size-max-valid "$TASK_TMP/regular"
run_case size-overflow "$TASK_TMP/regular"
printf '\nResult: %s passed, %s failed, %s not applicable\n' "$passed" "$failed" "$not_applicable"
test "$failed" -eq 0
