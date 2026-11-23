#!/usr/bin/env bash
set -euo pipefail

program=$1
elf32_fixture=$2
fixtures=$(dirname -- "${BASH_SOURCE[0]}")
export LC_ALL=C DEBUGINFOD_URLS=
log=$(mktemp)
trap 'rm -f -- "$log"' EXIT
failed=0

check() {
    local expected=$1 path=$2 status=0

    printf '\n=== %s (expected exit %d) ===\n' "$path" "$expected"
    valgrind --leak-check=full --errors-for-leak-kinds=definite,indirect,possible \
        --error-exitcode=99 --log-file="$log" "$program" "$path" >/dev/null || status=$?
    cat "$log"
    if ! grep -q 'ERROR SUMMARY:' "$log"; then
        printf 'FAIL: Memcheck did not complete for %s\n' "$path" >&2
        failed=1
    fi
    if [[ "$status" != "$expected" ]]; then
        printf 'FAIL: %s exited %d, expected %d\n' "$path" "$status" "$expected" >&2
        failed=1
    fi
}

check 0 "$elf32_fixture"
check 0 "$program"
for path in "$fixtures"/error_header "$fixtures"/header_offset_error "$fixtures"/*.so; do
    check 0 "$path"
done
for name in unterminated_string wrong_arch; do
    check 1 "$fixtures/$name"
done

# Memcheck's debuginfo reader aborts when ft_nm maps these truncated images.
for name in header header_copy header_and_prog header_and_prog_copy; do
    printf '\n=== %s (exit check only, no Memcheck) ===\n' "$fixtures/$name"
    status=0
    "$program" "$fixtures/$name" >/dev/null || status=$?
    if [[ "$status" != 1 ]]; then
        printf 'FAIL: %s exited %d, expected 1\n' "$fixtures/$name" "$status" >&2
        failed=1
    fi
done

exit "$failed"
