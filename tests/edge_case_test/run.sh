#!/usr/bin/env bash
# High-value edge cases plus minimal positive controls. No Python dependency.
source "$(dirname -- "${BASH_SOURCE[0]}")/../common.sh"
if [[ $# != 0 ]]; then printf 'Usage: bash tests/edge_case_test/run.sh\n' >&2; exit 1; fi
printf 'hello\n' > "$work/regular"
: > "$work/empty"
mkdir "$work/directory"
file_sources=("$fixtures/file_test.c" "$ROOT/src/utils/file.c" "$ROOT/src/utils/error.c")
if compile file "${file_sources[@]}" -Wl,--wrap=fstat; then
    for name in cleanup-zero regular empty missing directory repeat size-negative; do
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
    # The school environment provides multilib, so a host that cannot build -m32
    # is a local toolchain gap rather than a defect in ft_nm. Report it as a
    # warning that names the checks which did not run, so reduced coverage stays
    # visible without failing the suite. The main suite still treats its own
    # -m32 builds as failures, as the evaluation sheet requires them.
    record WARN abi32-unavailable "multilib -m32 unavailable, so abi32-size-max-valid and abi32-size-overflow did not run (install gcc-multilib)"
    cat "$TASK_TMP/abi32.log"
fi
if compile ident "$fixtures/ident_test.c" "${elf[0]}"; then
    for name in ranges short-inputs magic class endian version valid-ident; do
        run_case "ident-$name" "$work/ident" "$name"
    done
fi
for bits in 32 64; do
    if compile "table$bits" -DDAY2_BITS="$bits" "$fixtures/table_test.c" "${elf[@]}"; then
        for name in regular none exec shared null-args \
            bad-magic wrong-class big-endian header-size header-version machine type \
            section-entry section-truncated section-offset zero-offset-count \
            reserved-count extended-sections extended-small extended-truncated \
            extended-programs program program-truncated program-entry \
            program-zero-offset short-header; do
            run_case "table$bits-$name" "$work/table$bits" "$name"
        done
        if [[ "$bits" == 64 ]]; then
            run_case table64-huge-offset "$work/table64" huge-offset
            run_case table64-huge-count "$work/table64" huge-count
        fi
    fi
    if compile "symbols$bits" -DDAY3_BITS="$bits" "$fixtures/symbol_test.c" "${elf[@]}" "${collect[@]}"; then
        for name in regular abs common local missing dynsym-only \
            reader-contract string-contract \
            link-oob link-type entry-zero entry-wrong entry-partial symbol-payload \
            string-payload name-oob name-no-nul section-oob xindex-unsupported \
            name-max-fit name-too-long; do
            run_case "symbols$bits-$name" "$work/symbols$bits" "$name"
        done
        if [[ "$bits" == 64 ]]; then
            for name in huge-offset huge-size; do
                run_case "symbols64-$name" "$work/symbols64" "$name"
            done
        fi
    fi
    if compile "extended$bits" -DTEST_BITS="$bits" "$fixtures/extended_test.c" "${elf[@]}" "${collect[@]}"; then
        for name in valid nonzero-companion missing duplicate shstr-range section-name-oob; do
            run_case "extended$bits-$name" "$work/extended$bits" "$name"
        done
    fi
done
if compile cli "${cli_sources[@]}"; then
    for bits in 32 64; do
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
