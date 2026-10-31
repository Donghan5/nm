#!/usr/bin/env bash
# Reduced failure-injection suite, using the same real-CLI lifetime audit.
source "$(dirname -- "${BASH_SOURCE[0]}")/../common.sh"
wrap="$ROOT/tests/days_test/lifetime/wrap.c"
mkdir "$work/fixtures"
printf 'not ELF\n' > "$work/fixtures/text"
wrapflags=()
for function in open close fstat mmap munmap symbol_type print_symbols; do
    wrapflags+=("-Wl,--wrap=$function")
done
if ! timeout 60s "${compiler[@]}" "${base[@]}" "${sanitize[@]}" -Dmain=nm_cli_main \
    -c "$ROOT/src/main.c" -o "$work/main.o" > "$TASK_TMP/build-main.log" 2>&1; then
    record FAIL build/main
    finish
fi
if ! compile audit "$work/main.o" "${cli_sources[@]:1}" "$wrap" "${wrapflags[@]}"; then finish; fi
if ! compile reference "${cli_sources[@]}"; then finish; fi
value() { sed -nE 's/.*"'"$1"'":(-?[0-9]+).*/\1/p' "$TASK_TMP/audit.json"; }
check_lifetime() {
    local name=$1 fault=$2 fail_at=$3 error=$4 poison=$5; shift 5
    local paths=("$@") controls=("$@") code=0 refcode=0 expected_maps
    local valid=true envargs=(NM_FAULT="$fault" NM_FAIL_AT="$fail_at" NM_FAULT_ERRNO="$error"
        NM_AUDIT_REPORT="$TASK_TMP/audit.json")
    if [[ "$poison" == yes ]]; then envargs+=(NM_POISON_CLOSE=1); fi
    if [[ "$fault" != none ]]; then controls[$((fail_at - 1))]="$work/fixtures/missing"; fi
    if timeout 30s "$work/reference" "${controls[@]}" > "$TASK_TMP/$name.ref" 2> "$TASK_TMP/$name.referr"; then :; else refcode=$?; fi
    if timeout 30s env -u NM_POISON_CLOSE "${envargs[@]}" "$work/audit" "${paths[@]}" \
        > "$TASK_TMP/$name.out" 2> "$TASK_TMP/$name.err"; then :; else code=$?; fi
    if [[ "$code" != "$refcode" ]] || ! cmp -s "$TASK_TMP/$name.ref" "$TASK_TMP/$name.out"; then valid=false; fi
    if [[ ! -f "$TASK_TMP/audit.json" ]]; then
        record FAIL "$name"
        return
    fi
    cp "$TASK_TMP/audit.json" "$TASK_TMP/$name.audit.json"
    if [[ $(value errors) != 0 || $(value peak) -gt 1 ]]; then valid=false; fi
    if [[ "$fault" == none ]]; then
        if [[ $(value injected) != 0 ]] || ! cmp -s "$TASK_TMP/$name.referr" "$TASK_TMP/$name.err"; then valid=false; fi
    else
        expected_maps=$((${#paths[@]} - 1))
        if [[ $(value injected) != 1 || $(value maps) != "$expected_maps" || $(value print_calls) != "$expected_maps" ]]; then valid=false; fi
        if [[ "$fault" == fstat && $(value map_attempts) != "$expected_maps" ]]; then valid=false; fi
        case "$error" in 5) message='Input/output error' ;; 12) message='Cannot allocate memory' ;; esac
        printf 'ft_nm: %s: %s\n' "${paths[$((fail_at - 1))]}" "$message" > "$TASK_TMP/$name.expectederr"
        if ! cmp -s "$TASK_TMP/$name.expectederr" "$TASK_TMP/$name.err"; then valid=false; fi
    fi
    if "$valid"; then record PASS "$name"; else record FAIL "$name"; fi
    rm -f "$TASK_TMP/audit.json"
}
for bits in 32 64; do
    fixture="$work/fixtures/normal$bits.o"
    if ! timeout 30s "${compiler[@]}" -m"$bits" -c "$fixtures/output_fixture.s" -o "$fixture" \
        > "$TASK_TMP/fixture$bits.log" 2>&1; then
        record FAIL "fixture$bits"
        continue
    fi
    check_lifetime "elf$bits-normal" none 1 5 no "$fixture"
    check_lifetime "elf$bits-fstat-errno" fstat 1 5 yes "$fixture"
    check_lifetime "elf$bits-mmap-errno" mmap 1 12 yes "$fixture"
    check_lifetime "elf$bits-fstat-middle" fstat 2 5 no "$fixture" "$fixture" "$fixture"
    check_lifetime "elf$bits-mmap-middle" mmap 2 12 no "$fixture" "$fixture" "$fixture"
    check_lifetime "elf$bits-postmap-error" none 1 5 no "$work/fixtures/text" "$fixture"
    repeated=()
    for ((i=0; i<100; i++)); do repeated+=("$fixture"); done
    check_lifetime "elf$bits-repeat100" none 1 5 no "${repeated[@]}"
    check_lifetime "elf$bits-mmap-repeat100" mmap 50 12 no "${repeated[@]}"
done
finish
