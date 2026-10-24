# Day 3 Symbol Collection Tests

Run:

```sh
make test-3
```

Requires little-endian x86 Linux, a C compiler with AddressSanitizer and UndefinedBehaviorSanitizer, Python 3, and `timeout`. The compiler must generate both ELF32 and ELF64 object files with `-m32`/`-m64`. The fixture uses no system headers and is compiled without linking, so 32-bit libc development libraries are not required. Test harness executables use the native ABI for both file formats.

## Milestone mapping

| Day 3 requirement | Checks |
|---|---|
| Find SHT_SYMTAB by type | Synthetic fixtures omit section names; discovery still works |
| Follow sh_link | Valid string table, out-of-bounds link, wrong linked section type |
| Validate entry sizes/counts | Zero/wrong entry size, partial entry, table payload beyond file |
| Validate st_name and NUL | Offset at table end, unterminated final byte, valid empty name |
| Preserve common records | Name, value, binding, type, section index, class, original order, initial code |
| Special section indices | SHN_UNDEF, SHN_ABS, SHN_COMMON accepted; invalid index rejected |
| SHN_XINDEX handling | Explicit rejection tested; successful extended-index resolution is NOT implemented/certified |
| Real ELF32/ELF64 .o extraction | Compile a small fixture and compare all raw SYMTAB entries against an independent Python struct-based parser |
| No symbol table vs corruption | Missing SYMTAB and DYNSYM-only return empty success; malformed SYMTAB returns false |

## Files

- `day3_symbol_test.c`: format-parameterized synthetic fixtures and a real-object collection/dump harness.
- `day3_fixture.c`: small relocatable-object source with function/data/BSS/read-only/weak/undefined cases.
- `day3_compare.py`: independent reference reader using Python's standard library; compares the first regular symbol table entry-for-entry, including the initial null entry.
- `test_day3.sh`: strict builds, per-case timeouts, sanitizers, real fixture generation, and summary.

The runner links actual ELF32/ELF64, identification, symbol collection/validation, and required libft implementations. It excludes unfinished symbol classification, sorting, printing, and main. It does not replace project functions with mocks or suppress warnings. Build failures report runtime tests not executed.

## Contracts and coverage

Collection returns true on success and false for malformed or unsupported input. Successful absence produces `symbols == NULL`, `count == 0`; DYNSYM is not used as a fallback. Failed collection must release any allocated array and reset outputs. Malformed second-symbol cases exercise cleanup after at least one successful record was stored. Success transfers the array to the caller, which frees it. Names are copied into the current fixed-size record buffer.

Each format has 21 synthetic cases. ELF64 adds huge offset/size rejection. Two real-object comparisons give 46 cases total. The synthetic suite checks large ELF64 values (above 32-bit address range), unaligned table storage, exact maximum name length fitting the current buffer, rejection of an overlong name, reader index/entry checks, and string boundary/NUL behavior.

Overlong-name rejection documents the current fixed-buffer policy; it is not proof that all valid ELF symbol names are supported. SHN_XINDEX rejection documents the current unsupported path rather than marking extended-index support complete.

Native tests do not execute 32-bit size_t narrowing paths. They do not validate all reservation values, allocation-failure injection, multiple-SYMTAB merge policies, every malformed string-table invariant, NULL arguments to the collector, all section payloads/SHT_NOBITS, or final nm classification/filtering/output. Comparing raw collected records does not imply final system nm output compatibility.

Each synthetic case is a separate process with a 10-second timeout. Real-object comparison has a 15-second outer timeout and 10-second collector timeout. Artifacts live in a unique temporary directory and are removed on exit. Failure to generate a required real object is reported as UNAVAILABLE, not PASS, and makes the target exit unsuccessfully.

## Latest verification

`make test-3`: **46 passed, 0 failed, 0 unavailable**. Both real fixtures matched all 11 records. ASan/UBSan were enabled. The agent ran the suite outside the restricted sandbox to allow LeakSanitizer to operate. Implementation sources were not modified.
