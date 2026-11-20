#!/usr/bin/env bash
# School fixtures remain read-only; temporary comparison files are removed on exit.
source "$(dirname -- "${BASH_SOURCE[0]}")/../common.sh"
if [[ $# != 0 ]]; then printf 'Usage: bash tests/eval_test/run.sh\n' >&2; exit 1; fi
if ! compile cli "${cli_sources[@]}"; then finish; fi
# GNU nm prefixes every diagnostic with its own program name and so does ft_nm.
# Only that prefix may differ, so normalising it lets stderr be compared for real
# instead of only passing when both streams happen to be empty.
normalize_err() {
    sed "s|^$1: |@diag: |" "$2" > "$3"
}
check_eval() {
    local name=$1 mode=$2 refcode=0 code=0 valid=false; shift 2
    local path=${!#}
    [[ $# != 0 ]] || path=a.out
    if timeout 10s nm "$@" > "$TASK_TMP/$name.nm.out" 2> "$TASK_TMP/$name.nm.err"; then :; else refcode=$?; fi
    if timeout 10s "$work/cli" "$@" > "$TASK_TMP/$name.ft.out" 2> "$TASK_TMP/$name.ft.err"; then :; else code=$?; fi
    printf '%s\n' "$refcode" > "$TASK_TMP/$name.nm.status"
    printf '%s\n' "$code" > "$TASK_TMP/$name.ft.status"
    normalize_err nm "$TASK_TMP/$name.nm.err" "$TASK_TMP/$name.nm.cmperr"
    normalize_err ft_nm "$TASK_TMP/$name.ft.err" "$TASK_TMP/$name.ft.cmperr"
    if [[ "$mode" == exact ]]; then
        # refcode must be 0: an accepted fixture that GNU nm rejects is a broken
        # fixture, not a passing comparison. ft_nm must then match that exit code,
        # stdout byte for byte, and stderr once the program name is normalised.
        # The grep keeps the normalisation from hiding ft_nm borrowing "nm:".
        if [[ "$refcode" == 0 && "$code" == "$refcode" ]] && \
            cmp -s "$TASK_TMP/$name.nm.out" "$TASK_TMP/$name.ft.out" && \
            cmp -s "$TASK_TMP/$name.nm.cmperr" "$TASK_TMP/$name.ft.cmperr" && \
            ! grep -q '^nm: ' "$TASK_TMP/$name.ft.err"; then valid=true; fi
    else
        # Refuse malformed inputs without reproducing BFD plugin output.
        printf 'ft_nm: %s: file format not recognized\n' "$path" > "$TASK_TMP/$name.expectederr"
        if [[ "$refcode" == 1 && "$code" == 1 && ! -s "$TASK_TMP/$name.ft.out" ]] && \
            cmp -s "$TASK_TMP/$name.expectederr" "$TASK_TMP/$name.ft.err"; then valid=true; fi
    fi
    if grep -Eq 'Sanitizer|runtime error:' "$TASK_TMP/$name.ft.err"; then valid=false; fi
    local detail
    if [[ "$mode" == exact ]]; then
        detail="GNU nm comparison: stdout, stderr (program name aside) and exit match (nm=$refcode, ft_nm=$code)"
    else
        detail="Malformed ELF: reject with exit 1, empty stdout, filename diagnostic (nm=$refcode, ft_nm=$code)"
    fi
    if "$valid"; then
        record PASS "$name" "$detail"
    else
        record FAIL "$name" "Expected: $detail"
        diff -u "$TASK_TMP/$name.nm.out" "$TASK_TMP/$name.ft.out" || true
        if [[ "$mode" == exact ]]; then
            diff -u "$TASK_TMP/$name.nm.cmperr" "$TASK_TMP/$name.ft.cmperr" || true
        else
            diff -u "$TASK_TMP/$name.expectederr" "$TASK_TMP/$name.ft.err" || true
        fi
    fi
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
# Compare supplied shared libraries without modifying the original files.
for path in "$ROOT"/tests/eval_test/*.so; do
    [[ -f "$path" ]] || continue
    name=$(basename "$path")
    for option in default r p rp pr rr pp r-p p-r; do
        opts=()
        case "$option" in default) ;; r-p) opts=(-r -p) ;; p-r) opts=(-p -r) ;; *) opts=("-$option") ;; esac
        check_eval "$name-$option" exact "${opts[@]}" "$path"
    done
done
# These two .c files are source inputs, not malformed ELF fixtures.
for variant in easy_test not_so_easy_test-32 not_so_easy_test-64; do
    file=easy_test
    arch_flags=()
    case "$variant" in
        not_so_easy_test-32) file=not_so_easy_test; arch_flags=(-m32) ;;
        not_so_easy_test-64) file=not_so_easy_test; arch_flags=(-m64) ;;
    esac
    if timeout 30s "${compiler[@]}" -Wall -Wextra -Werror "${arch_flags[@]}" "$ROOT/tests/eval_test/$file.c" \
        -o "$work/$variant" > "$TASK_TMP/build-$variant.log" 2>&1; then
        for option in default r p; do
            opts=()
            [[ "$option" == default ]] || opts=("-$option")
            check_eval "$variant-$option" exact "${opts[@]}" "$work/$variant"
        done
    else
        record FAIL "build/$variant"
        cat "$TASK_TMP/build-$variant.log"
    fi
done
# Main comparisons cover ELF32/64 objects, executables, PIE and shared libraries.
for bits in 32 64; do
    if ! timeout 30s "${compiler[@]}" -m"$bits" -c "$ROOT/tests/eval_test/symbols.s" -o "$work/symbols$bits.o" > "$TASK_TMP/fixture$bits.log" 2>&1; then
        record FAIL "build/symbols$bits"
        cat "$TASK_TMP/fixture$bits.log"
        continue
    fi
    emulation=elf_x86_64
    [[ "$bits" == 64 ]] || emulation=elf_i386
    for kind in exec pie so; do
        flags=()
        case "$kind" in pie) flags=(-pie --no-dynamic-linker) ;; so) flags=(-shared) ;; esac
        if ! timeout 30s ld -m "$emulation" "${flags[@]}" -e zebra_text "$work/symbols$bits.o" -o "$work/$kind$bits" > "$TASK_TMP/link-$kind$bits.log" 2>&1; then
            record FAIL "build/$kind$bits"
            cat "$TASK_TMP/link-$kind$bits.log"
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
    # Symbol-less links: GNU nm writes "no symbols" to stderr and still exits 0,
    # a diagnostic path that an empty-stderr comparison could never reach.
    # ld -s drops the symbol table at link time, so this needs no tool beyond the
    # ld the suite already requires and can never be skipped.
    for kind in exec pie so; do
        flags=()
        case "$kind" in pie) flags=(-pie --no-dynamic-linker) ;; so) flags=(-shared) ;; esac
        name=nosym-$kind$bits
        if ! timeout 30s ld -m "$emulation" -s "${flags[@]}" -e zebra_text \
            "$work/symbols$bits.o" -o "$work/$name" > "$TASK_TMP/link-$name.log" 2>&1; then
            record FAIL "build/$name"
            cat "$TASK_TMP/link-$name.log"
            continue
        fi
        for option in default r p; do
            opts=()
            [[ "$option" == default ]] || opts=("-$option")
            check_eval "$name-$option" exact "${opts[@]}" "$work/$name"
        done
    done
done
# A fixture that failed to build is already reported as a build failure above;
# running these cases against the missing file would only add cascading noise.
if [[ -f "$work/symbols32.o" && -f "$work/symbols64.o" ]]; then
    check_eval multiple exact "$work/symbols32.o" "$work/symbols64.o"
    check_eval multiple-bonus exact -rp "$work/symbols32.o" "$work/symbols64.o"
    check_eval option-between-files exact "$work/symbols32.o" -p "$work/symbols64.o"
fi
if [[ -f "$work/symbols64.o" ]]; then
    check_eval option-after-file exact "$work/symbols64.o" -r
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
fi
finish
