# Day 2 Tests

Run `make test-2`. Requirements: Linux, a C compiler with AddressSanitizer and UndefinedBehaviorSanitizer, and `timeout`.

This suite links the actual `src/elf/elf_ident.c` without modifying implementation files or suppressing compiler diagnostics. A failed build means runtime tests did not run. Each group runs in a separate process with a 10-second timeout. Temporary artifacts are removed on exit.

| Group | Coverage |
|---|---|
| ranges | Empty ranges, exact boundaries, out-of-bounds ranges, SIZE_MAX and addition-wrap cases |
| short-inputs | Every length from 0 through EI_NIDENT - 1, ending at a protected guard page |
| valid-ident | All ELF32/ELF64 and little/big-endian identification combinations |
| magic | Every byte value at each magic position |
| class | Every byte value for EI_CLASS |
| endian | Every byte value for EI_DATA |
| version | Every byte value for EI_VERSION |

The entry point under test is `check_elf_ident()`. The caller supplies a valid t_file whose data describes the advertised readable bytes. The suite does not independently call the magic helper; it must either remain internal with a documented size precondition or validate its own input length.

Acceptance here means valid identification bytes, not a complete valid ELF file or supported parsing endianness. The ELF32/ELF64 parsers are currently empty. Header sizes, machine/type/version fields, section tables, multiplication overflow, extended section counts, section payload bounds, and SHT_NOBITS require additional tests once the parser API exists. Passing this suite alone does not complete Day 2.
