#!/usr/bin/env bash
# School fixtures remain read-only; generated binaries and logs live in /tmp.
source "$(dirname -- "${BASH_SOURCE[0]}")/../days_test/common.sh"
if [[ $# != 0 ]]; then printf 'Usage: bash tests/eval_test/run.sh\n' >&2; exit 1; fi
if ! compile cli "${cli_sources[@]}"; then finish; fi
check_eval() {
    local name=$1 mode=$2 path=$3 refcode=0 code=0 valid=false; shift 3
    if timeout 10s nm "$@" "$path" > "$TASK_TMP/$name.nm.out" 2> "$TASK_TMP/$name.nm.err"; then :; else refcode=$?; fi
    if timeout 10s "$work/cli" "$@" "$path" > "$TASK_TMP/$name.ft.out" 2> "$TASK_TMP/$name.ft.err"; then :; else code=$?; fi
    printf '%s\n' "$refcode" > "$TASK_TMP/$name.nm.status"
    printf '%s\n' "$code" > "$TASK_TMP/$name.ft.status"
    if [[ "$mode" == exact ]]; then
        if [[ "$refcode" == 0 && "$code" == 0 ]] && \
            cmp -s "$TASK_TMP/$name.nm.out" "$TASK_TMP/$name.ft.out" && \
            cmp -s "$TASK_TMP/$name.nm.err" "$TASK_TMP/$name.ft.err"; then valid=true; fi
    else
        # Refuse malformed inputs without reproducing BFD plugin output.
        printf 'ft_nm: %s: file format not recognized\n' "$path" > "$TASK_TMP/$name.expectederr"
        if [[ "$refcode" == 1 && "$code" == 1 && ! -s "$TASK_TMP/$name.ft.out" ]] && \
            cmp -s "$TASK_TMP/$name.expectederr" "$TASK_TMP/$name.ft.err"; then valid=true; fi
    fi
    if "$valid"; then record PASS "$name"; else record FAIL "$name"; fi
}
for file in error_header header_offset_error header header_copy header_and_prog \
    header_and_prog_copy unterminated_string wrong_arch; do
    mode=reject
    case "$file" in error_header|header_offset_error) mode=exact ;; esac
    for option in default r p; do
        opts=()
        [[ "$option" == default ]] || opts=("-$option")
        check_eval "$file-$option" "$mode" "$ROOT/tests/eval_test/$file" "${opts[@]}"
    done
done
# These two .c files are source inputs, not malformed ELF fixtures.
for file in easy_test not_so_easy_test; do
    if timeout 30s "${compiler[@]}" -Wall -Wextra -Werror "$ROOT/tests/eval_test/$file.c" \
        -o "$work/$file" > "$TASK_TMP/build-$file.log" 2>&1; then
        for option in default r p; do
            opts=()
            [[ "$option" == default ]] || opts=("-$option")
            check_eval "$file-$option" exact "$work/$file" "${opts[@]}"
        done
    else
        record FAIL "build/$file"
        cat "$TASK_TMP/build-$file.log"
    fi
done
finish
