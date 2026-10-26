# Day 2 ELF Tests

Run `make test-2`. Requirements: Linux on an x86 little-endian host, a C compiler with AddressSanitizer and UndefinedBehaviorSanitizer, and `timeout`.

The suite links actual project implementations and libft memcpy without modifying source files or suppressing diagnostics. Each case/group runs in a separate process with a 10-second timeout. Build failures are reported as runtime tests not executed. Temporary fixtures and executables are removed on exit.

## Return value contract

| Function category | Valid / success | Invalid / failure | Caller check |
|---|---|---|---|
| ELF32/ELF64 boolean validators and readers | `true` | `false` | `if (!check_...(...))` or `if (!read_...(...))` |
| Identification/range validators (currently int) | `1` | `0` | `if (!check_...(...))` |
| ELF32/ELF64 getters | `0` | `-1` | `if (get_...(...) < 0)` |

Getter signatures take `const t_file *` and `t_section_table *`. On success the output contains the original file pointer, validated byte offset, and resolved section count. A valid file with no section table returns success with offset/count zero and the original file pointer. Failure outputs must not be consumed. Returning `false` from an int getter incorrectly signals success.

No program headers is valid without requiring e_phentsize to be nonzero. Present program tables require the expected entry size and full file range. Extended program count is resolved from section zero's sh_info; extended section count is resolved from its sh_size. Multiplication must not overflow. Getters currently support little-endian x86 ELF files and reject big-endian input, even though the identification helper recognizes both endian encodings.

## Executable coverage

`day2_elf_test.c` provides seven identification/range groups:

| Group | Coverage |
|---|---|
| ranges | Empty/exact/out-of-bounds ranges and SIZE_MAX wrap cases |
| short-inputs | Every length below EI_NIDENT, ending at a protected guard page |
| valid-ident | ELF32/ELF64 and little/big-endian identification combinations |
| magic, class, endian, version | Every byte value at the relevant identification positions |

`day2_table_test.c` is compiled twice with DAY2_BITS=32/64 to select the ELF format, not the host ABI. Both variants call the real getters and readers and use synthetic byte fixtures. No 32-bit system development libraries are required for this native test target.

Each variant has 26 cases covering valid ET_REL/ET_EXEC/ET_DYN headers, no sections, unaligned table storage, exact file-end boundaries, normal/extended section counts, extended program counts, missing/truncated headers and tables, invalid magic/class/endian/version/machine/type/entry sizes, inconsistent offset/count, reserved section counts, and NULL getter arguments. Successful nonempty cases also check copied section bytes, last valid/first invalid/SIZE_MAX indices, NULL reader arguments, and forged offset/count overflow.

ELF64 adds huge-offset and huge-extended-count rejection cases (UINT64_MAX), giving 7 + 26 + 28 = 61 cases/groups total. Extended fixtures have SHN_LORESERVE section entries or PN_XNUM program entries; invalid extended section counts below the threshold are also rejected.

## Scope and remaining work

These are synthetic parser integration tests, not a complete ft_nm program comparison. ET_EXEC/ET_DYN fixtures exercise parser contracts rather than providing loader-ready executables. File opening, symbol parsing, sorting, printing, and system nm output comparison are outside this suite.

The native suite exercises both ELF formats with the host's size_t. It does not prove rejection before narrowing in a 32-bit executable; that needs a separate 32-bit ABI run. Native huge-value tests validate rejection without implying that the 32-bit conversion path ran.

Individual section payload validation, SHT_NOBITS handling, e_shstrndx/SHN_XINDEX validation, real object/executable/shared-library fixtures, and exhaustive malformed extended-header combinations remain additional work. Passing this suite does not by itself complete every Day 2 milestone.

## Latest verification

`make test-2`: 61 passed, 0 failed, with AddressSanitizer and UndefinedBehaviorSanitizer enabled. Executed by the agent outside the restricted sandbox because LeakSanitizer cannot run under its ptrace restrictions. No implementation files were modified.
