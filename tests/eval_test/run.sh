#!/usr/bin/env bash
# School fixtures remain read-only; generated binaries and logs live in /tmp.
source "$(dirname -- "${BASH_SOURCE[0]}")/../common.sh"
if [[ $# != 0 ]]; then printf 'Usage: bash tests/eval_test/run.sh\n' >&2; exit 1; fi
if ! compile cli "${cli_sources[@]}"; then finish; fi
check_eval() {
    local name=$1 mode=$2 refcode=0 code=0 valid=false; shift 2
    local path=${!#}
    [[ $# != 0 ]] || path=a.out
    if timeout 10s nm "$@" > "$TASK_TMP/$name.nm.out" 2> "$TASK_TMP/$name.nm.err"; then :; else refcode=$?; fi
    if timeout 10s "$work/cli" "$@" > "$TASK_TMP/$name.ft.out" 2> "$TASK_TMP/$name.ft.err"; then :; else code=$?; fi
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
    if grep -Eq 'Sanitizer|runtime error:' "$TASK_TMP/$name.ft.err"; then valid=false; fi
    if "$valid"; then record PASS "$name"; else record FAIL "$name"; fi
}
for file in error_header header_offset_error header header_copy header_and_prog \
    header_and_prog_copy unterminated_string wrong_arch; do
    mode=reject
    case "$file" in error_header|header_offset_error) mode=exact ;; esac
    for option in default r p; do
        opts=()
        [[ "$option" == default ]] || opts=("-$option")
        check_eval "$file-$option" "$mode" "${opts[@]}" "$ROOT/tests/eval_test/$file"
    done
done
# These two .c files are source inputs, not malformed ELF fixtures.
for file in easy_test not_so_easy_test; do
    if timeout 30s "${compiler[@]}" -Wall -Wextra -Werror "$ROOT/tests/eval_test/$file.c" \
        -o "$work/$file" > "$TASK_TMP/build-$file.log" 2>&1; then
        for option in default r p; do
            opts=()
            [[ "$option" == default ]] || opts=("-$option")
            check_eval "$file-$option" exact "${opts[@]}" "$work/$file"
        done
    else
        record FAIL "build/$file"
        cat "$TASK_TMP/build-$file.log"
    fi
done
# Main comparisons cover ELF32/64 objects, executables, PIE and shared libraries.
for bits in 32 64; do
    if ! timeout 30s "${compiler[@]}" -m"$bits" -c "$ROOT/tests/eval_test/symbols.s" -o "$work/symbols$bits.o" > "$TASK_TMP/fixture$bits.log" 2>&1; then
        record FAIL "build/symbols$bits"; continue
    fi
    emulation=elf_x86_64
    [[ "$bits" == 64 ]] || emulation=elf_i386
    for kind in exec pie so; do
        flags=()
        case "$kind" in pie) flags=(-pie --no-dynamic-linker) ;; so) flags=(-shared) ;; esac
        if ! timeout 30s ld -m "$emulation" "${flags[@]}" -e zebra_text "$work/symbols$bits.o" -o "$work/$kind$bits" > "$TASK_TMP/link-$kind$bits.log" 2>&1; then
            record FAIL "build/$kind$bits"
        fi
    done
    for path in "$work/symbols$bits.o" "$work/exec$bits" "$work/pie$bits" "$work/so$bits"; do
        [[ -f "$path" ]] || continue
        name=$(basename "$path")
        for option in default r p rp pr rr pp r-p p-r; do
            opts=()
            case "$option" in default) ;; r-p) opts=(-r -p) ;; p-r) opts=(-p -r) ;; *) opts=("-$option") ;; esac
            check_eval "$name-$option" exact "${opts[@]}" "$path"
        done
    done
done
check_eval multiple exact "$work/symbols32.o" "$work/symbols64.o"
check_eval multiple-bonus exact -rp "$work/symbols32.o" "$work/symbols64.o"
check_eval option-after-file exact "$work/symbols64.o" -r
check_eval option-between-files exact "$work/symbols32.o" -p "$work/symbols64.o"
cp "$work/symbols64.o" "$work/-r"
check_eval dash-filename exact -- "$work/-r"
# Change directory so -- really protects a filename beginning with a dash.
previous=$PWD
cd "$work"
check_eval dash-relative exact -- -r
cd "$previous"
cp "$work/symbols64.o" "$work/a.out"
previous=$PWD
cd "$work"
check_eval default-aout exact
check_eval default-aout-bonus exact -rp
cd "$previous"
finish
