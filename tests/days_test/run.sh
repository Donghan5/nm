#!/usr/bin/env bash
# High-value edge cases plus minimal positive controls. No Python dependency.
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
require32=false
if [[ "${1:-}" == --require-32 ]]; then require32=true; shift; fi
if [[ $# != 0 ]]; then printf 'Usage: bash tests/days_test/run.sh [--require-32]\n' >&2; exit 1; fi
printf 'hello\n' > "$work/regular"
: > "$work/empty"
mkdir "$work/directory"
file_sources=("$fixtures/file_test.c" "$ROOT/src/utils/file.c" "$ROOT/src/utils/error.c")
if compile file "${file_sources[@]}" -Wl,--wrap=fstat; then
    for name in cleanup-zero empty missing directory repeat size-negative; do
        path="$work/regular"
        case "$name" in empty|missing|directory) path="$work/$name" ;; esac
        run_case "file-$name" "$work/file" "$name" "$path"
    done
fi
if timeout 30s "${compiler[@]}" -m32 -D_FILE_OFFSET_BITS=64 "$fixtures/abi32_probe.c" \
    -o "$work/probe32" > "$TASK_TMP/abi32.log" 2>&1 && \
    timeout 10s "$work/probe32" >> "$TASK_TMP/abi32.log" 2>&1; then
    if timeout 30s "${compiler[@]}" "${base[@]}" -m32 -D_FILE_OFFSET_BITS=64 \
        -DDAY1_TEST_32 -Wl,--wrap=fstat64 "${file_sources[@]}" -o "$work/file32" \
        > "$TASK_TMP/build-file32.log" 2>&1; then
        for name in size-max-valid size-overflow; do
            run_case "abi32-$name" "$work/file32" "$name" "$work/regular"
        done
    else
        record FAIL build/file32
        cat "$TASK_TMP/build-file32.log"
    fi
else
    if "$require32"; then record FAIL abi32-unavailable; else record SKIP abi32-unavailable; fi
fi
if compile ident "$fixtures/ident_test.c" "${elf[0]}"; then
    for name in ranges short-inputs magic class endian; do
        run_case "ident-$name" "$work/ident" "$name"
    done
fi
if compile types "$fixtures/type_test.c" "$ROOT/src/symbol/symbol_type.c" "${elf[@]}"; then
    for bits in 32 64; do
        for name in undefined common weak-object-undefined weak-function-defined \
            text-writable note-nonalloc index-out-of-range index-huge; do
            run_case "type$bits-$name" "$work/types" "$bits" "$name"
        done
    done
fi
for bits in 32 64; do
    if compile "table$bits" -DDAY2_BITS="$bits" "$fixtures/table_test.c" "${elf[@]}"; then
        for name in regular extended-sections extended-truncated program-truncated \
            section-entry section-truncated section-offset null-args; do
            run_case "table$bits-$name" "$work/table$bits" "$name"
        done
        if [[ "$bits" == 64 ]]; then
            run_case table64-huge-offset "$work/table64" huge-offset
            run_case table64-huge-count "$work/table64" huge-count
        fi
    fi
    if compile "symbols$bits" -DDAY3_BITS="$bits" "$fixtures/symbol_test.c" "${elf[@]}" "${collect[@]}"; then
        for name in regular link-oob link-type entry-zero entry-partial symbol-payload \
            string-payload name-oob name-no-nul section-oob xindex-unsupported \
            name-max-fit name-too-long; do
            run_case "symbols$bits-$name" "$work/symbols$bits" "$name"
        done
    fi
    if compile "extended$bits" -DTEST_BITS="$bits" "$fixtures/extended_test.c" "${elf[@]}" "${collect[@]}"; then
        for name in valid nonzero-companion missing duplicate shstr-range section-name-oob; do
            run_case "extended$bits-$name" "$work/extended$bits" "$name"
        done
    fi
done
if compile cli "${cli_sources[@]}"; then
    for bits in 32 64; do
        if timeout 30s "${compiler[@]}" -m"$bits" -c "$fixtures/output_fixture.s" \
            -o "$work/output$bits.o" > "$TASK_TMP/fixture$bits.log" 2>&1; then
            for option in default r p; do
                opts=()
                [[ "$option" == default ]] || opts=("-$option")
                name="output$bits-$option"
                if timeout 10s nm "${opts[@]}" "$work/output$bits.o" \
                    > "$TASK_TMP/$name.ref" 2> "$TASK_TMP/$name.referr" && \
                    timeout 10s "$work/cli" "${opts[@]}" "$work/output$bits.o" \
                    > "$TASK_TMP/$name.out" 2> "$TASK_TMP/$name.err" && \
                    cmp -s "$TASK_TMP/$name.ref" "$TASK_TMP/$name.out" && \
                    cmp -s "$TASK_TMP/$name.referr" "$TASK_TMP/$name.err"; then
                    record PASS "$name"
                else
                    record FAIL "$name"
                fi
            done
        else
            record FAIL "fixture$bits"
        fi
        if [[ -x "$work/extended$bits" ]]; then
            for kind in range note nobits; do
                path="$work/payload$bits-$kind.o"
                if ! timeout 10s "$work/extended$bits" "emit-$kind" "$path"; then
                    record FAIL "payload$bits-generate-$kind"
                    continue
                fi
                code=0
                if timeout 10s "$work/cli" "$path" > "$TASK_TMP/payload$bits-$kind.out" \
                    2> "$TASK_TMP/payload$bits-$kind.err"; then :; else code=$?; fi
                valid=false
                case "$kind" in
                    note)
                        if [[ "$code" == 1 && ! -s "$TASK_TMP/payload$bits-$kind.out" ]] && \
                            grep -q 'file format not recognized' "$TASK_TMP/payload$bits-$kind.err"; then valid=true; fi ;;
                    range)
                        if [[ "$code" == 0 && -s "$TASK_TMP/payload$bits-$kind.out" ]] && \
                            grep -q 'has a section extending past end of file' "$TASK_TMP/payload$bits-$kind.err"; then valid=true; fi ;;
                    nobits)
                        if [[ "$code" == 0 && -s "$TASK_TMP/payload$bits-$kind.out" && ! -s "$TASK_TMP/payload$bits-$kind.err" ]]; then valid=true; fi ;;
                esac
                if grep -Eq 'Sanitizer|runtime error:' "$TASK_TMP/payload$bits-$kind.err"; then valid=false; fi
                if "$valid"; then record PASS "payload$bits-$kind"; else record FAIL "payload$bits-$kind"; fi
            done
        fi
    done
fi
finish
