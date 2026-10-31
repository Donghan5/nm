# Tests

This is the single test guide. All runners use Bash and compile the current project
sources into temporary binaries; they do not require Python or reuse a stale `ft_nm`.

## Commands

Run from the repository root:

```sh
make days_test       # 97 core edge cases
make lifetime_test   # 16 fd/mapping lifetime cases
make eval_test       # 24 school-binary cases + 6 compiled-source cases
```

Require a working 32-bit process ABI rather than skipping its two boundary checks:

```sh
bash tests/days_test/run.sh --require-32
```

Requirements: Bash, an x86 little-endian Linux host, a C compiler, GNU binutils
(`nm` and the assembler used by the compiler), coreutils (`timeout`, `cmp`), sed,
grep, and `/proc`. The 32-bit conversion tests also require 32-bit libc development
files and kernel execution support. The project uses libc `strlcpy`.

## Sanitizers and results

The default is ASan, UBSan, and LeakSanitizer. The two actual 32-bit conversion
checks intentionally run without sanitizers. Configure the shared runners with:

```sh
NM_TEST_SANITIZERS=address,undefined make eval_test
NM_TEST_SANITIZERS=none make days_test
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make lifetime_test
```

Accepted selections are `address`, `undefined`, `address,undefined`, and `none`.
Caller-provided `ASAN_OPTIONS` and `UBSAN_OPTIONS` are preserved. Omitted sanitizer
checks are reported explicitly and must not be interpreted as verified.
Some sandboxes block LeakSanitizer and 32-bit system calls; run in a supported
environment to complete these checks. Disabling leak detection does not resolve
the missing leak coverage.

Every runner prints its PASS/FAIL/SKIP summary and a `/tmp/nm-edge.*` directory.
`results.tsv` records individual outcomes; other files contain build and runtime
logs. The evaluation runner also saves both tools' stdout, stderr, and exit status.
The lifetime runner saves `*.audit.json` counters. Temporary binaries are removed
on exit, while logs remain for inspection. Temporary files are not guaranteed to
persist across sessions.

Exit status is 0 for a complete pass, 1 for failure, and 2 for skipped test groups
without failures. An unavailable 32-bit ABI is one skipped group covering two
unexecuted checks; `--require-32` makes that a failure. Build failures are failures,
not successful runtime tests. Runtime cases have 10-second timeouts; lifetime
cases use 30 seconds and compilation uses 30 or 60 seconds.

## Core edge cases: days_test

The previous 194-case runner was reduced to 97 high-value checks. Normal controls
remain to detect overly strict rejection. Removed Python comparators, duplicate
cases, and historical per-day documentation are no longer part of the suite.

| Group | Cases | Scope |
|---|---:|---|
| File lifetime | 6 | Empty/missing files, directories, repeated cleanup, negative size |
| Actual 32-bit size conversion | 2 | Preserve SIZE_MAX; reject SIZE_MAX + 1 |
| ELF identification | 5 | Short input, ranges, magic, class, byte order |
| ELF32/64 headers and tables | 18 | Positive control, extended counts, truncation, offsets, entry size, integer limits |
| ELF32/64 symbol boundaries | 26 | Links, indexes, payload ranges, NUL termination, name-length limits |
| ELF32/64 classification | 16 | Undefined/common/weak, flag precedence, invalid indexes |
| Extended indexes and section names | 12 | Large real indexes, duplicate/missing companions, zero companion entries, section-name ranges |
| Real CLI output | 6 | ELF32/64 default, -r, -p; byte-for-byte comparison with nm |
| Real CLI payload policy | 6 | General range warning, invalid NOTE rejection, NOBITS exception |
| **Total** | **97** | |

This is a lightweight regression suite. Its pass count does not mean that every
case in the previous 194-case suite has been rerun.

## Resource lifetime: lifetime_test

GNU ld `--wrap` instruments the actual CLI without replacing project logic.
For each ELF class, the runner checks eight scenarios:

- Normal processing.
- Injected fstat EIO, preserving errno across close.
- Injected mmap ENOMEM, preserving errno across close.
- fstat failure in the middle of three inputs, followed by continued processing.
- mmap failure in the middle of three inputs, followed by continued processing.
- A mapped non-ELF input followed by a valid file.
- One hundred valid inputs.
- mmap failure at input 50 of one hundred, followed by continued processing.

The audit verifies balanced open/close and mmap/munmap calls, valid unmap
addresses/lengths, no double unmap or unmap of MAP_FAILED, and an active mapping
during symbol classification and printing. It compares actual fd counts and
fixture mappings in `/proc` before and after CLI execution. At most one file
mapping may remain active at a time. Fault counters, diagnostics, stdout, and exit
status are checked. Successful close calls deliberately overwrite errno with
EBUSY to test preservation of the original failure.

The old 96-case matrix was reduced to these 16 scenarios. EOVERFLOW/EACCES and
first/last-file fault combinations passed earlier checks but are not retained in
the lightweight runner. Allocation failures, close/munmap failure recovery,
asynchronous truncation, and all possible resource paths are outside this scope.

## School fixtures: eval_test

The original eight school binary fixtures and two C sources remain unchanged.
The runner tests each input with default output, `-r`, and `-p`.

| Inputs | Cases | Acceptance criterion |
|---|---:|---|
| `error_header`, `header_offset_error` | 6 | nm and ft_nm both exit 0; stdout and stderr match exactly |
| `header`, `header_copy`, `header_and_prog`, `header_and_prog_copy`, `unterminated_string`, `wrong_arch` | 18 | nm rejects; ft_nm exits 1 with empty stdout and a filename-specific format-error diagnostic |
| Compiled `easy_test.c`, `not_so_easy_test.c` | 6 | stdout, stderr, and successful exit match nm exactly |
| **Total** | **30** | |

Classification follows the observed reference behavior, not the fixture names:
`error_header` and `header_offset_error` are accepted by the tested system nm.
The C files are compiled into temporary executables; they are not passed to nm
as malformed binaries.

The malformed-input criterion is intentionally separate from exact output
comparison. BFD plugin messages and tool-name differences are not reproduced;
reference outputs are retained in logs. Signals, timeouts, sanitizer diagnostics,
unexpected acceptance, or an incorrect diagnostic fail the test. Fixture changes
or another system nm version may require reviewing the expected classification.

## Limits and history

These suites cover the stated inputs, not every valid or malformed ELF, locale,
libc sorting behavior, kernel failure, or BFD plugin path. Supplemental Day5,
NOTE, extended-index, and same-name-symbol investigations are recorded in
`../milestone.md` and `../edit.log`; they are not all retained in the lightweight
suite. Bonus work is limited to the user's chosen `-r` and `-p` scope.
