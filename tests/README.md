# Tests

All test runners use Bash and build current sources into temporary binaries.
Python is not required. Run from the repository root:

```sh
make test            # Main suite; alias for eval_test
make eval_test       # 137 school, shared-library, format, CLI and bonus checks
make edge_case_test  # 130 edge checks
make lifetime_test   # 16 failure-injection and resource-lifetime checks
make test_all        # All three suites in one run
make valgrind        # Memcheck on the ELF32 example and valid/malformed evaluation fixtures
make proof           # Live differential scan of every ELF file on this host
```

`make valgrind` checks the binary fixtures in `tests/eval_test`, the generated
ELF32 object, and `ft_nm` itself. Valid inputs must exit 0 and malformed inputs
must exit 1. Memcheck errors and reported definite, indirect, or possible leaks
fail the target. The four truncated fixtures named `header`, `header_copy`,
`header_and_prog`, and `header_and_prog_copy` run directly because Valgrind's
debuginfo reader aborts when they are mapped.

## Live differential proof

`make proof` runs ft_nm and GNU nm over every ELF file under `/usr/lib`, `/lib`,
`/usr/bin`, `/bin` and `/usr/sbin` on the machine it is run on, comparing stdout
byte for byte (order included), stderr with only the program name normalised,
and the exit status. It shows a live progress bar, the file being compared and
running counters, then a final verdict. Roughly 3,200 files and 90 seconds on a
stock Ubuntu install.

Nothing is precomputed: the numbers come from the run on screen. The file list
is filtered by what `file(1)` reports, so an `ar` archive — not an ELF file and
outside the subject's scope — is counted as skipped, never as a pass, and the
skipped count is printed. A disagreement is re-run once before being recorded;
a result that does not reproduce is reported as nondeterministic rather than
quietly counted either way. Mismatches print the offending path and a diff, and
the command exits 1.

## Main evaluation suite

The eight original school binaries are read-only. Each is tested with default,
`-r` and `-p`: accepted inputs require exact stdout/stderr and successful exit;
rejected inputs require exit 1, empty stdout and a filename-specific format error.
GNU nm's BFD plugin diagnostics are not reproduced.

Both programs prefix each diagnostic with their own name, so stderr is compared
with that prefix normalised and nothing else: a warning's wording, its filename
and its ordering must match GNU nm exactly. ft_nm is separately required not to
emit a bare `nm:` prefix, which the normalisation would otherwise hide.

Supplied `tests/eval_test/*.so` libraries (currently `libft_malloc.so`) are
compared with GNU nm under default, `-r`, `-p`, `-rp`, `-pr`, `-rr`, `-pp`,
`-r -p` and `-p -r`, requiring exact stdout, stderr and successful exit.

The school `easy_test.c` is compiled with the compiler's default architecture,
and `not_so_easy_test.c` is compiled with `-m64`. Both binaries are compared
with GNU nm under default, `-r` and `-p`.
Generated ELF32/64 objects, executables, PIE and shared libraries
are compared under default, `-r`, `-p`, `-rp`, `-pr`, `-rr`, `-pp`, `-r -p` and
`-p -r`. Comparisons preserve output order, whitespace and address padding.
Symbol-less links of the executable, PIE and shared library (`ld -s`) are
compared under default, `-r` and `-p`: GNU nm writes `no symbols` to stderr and
still exits 0, so these cover the diagnostic path that a stdout-only or
empty-stderr comparison cannot reach. They use only the ld the suite already
requires, so they can never be skipped for want of a tool.
Additional cases cover multiple files, options after/between files, `--` protecting
a dash-prefixed filename, and default `a.out` with and without bonus options.
Only implemented bonus options `-r` and `-p` are covered; `-a/-g/-u` are unsupported.
The generated assembly has no libc dependency, including when linked as ELF32.

## Edge and resource checks

`edge_case_test` covers file/size boundaries,
ELF identification, ELF header field validation, header/table truncation and
offsets, program-header bounds, symbol/string bounds and reader contracts,
extended indexes, section-name bounds and payload/NOTE/NOBITS policy.
Minimal valid controls detect unconditional rejection. Normal CLI output
comparisons belong to the main suite.

Each fixture asserts one specific status rather than merely "some failure", and
rejects any case name it does not recognise, so a typo in `run.sh` fails loudly
instead of silently re-running the positive control. A case name valid only for
one word size (`huge-*`) is rejected by the other build for the same reason.
No fixture can report a boundary as untestable: a platform requirement is a
compile-time assertion, so each case either runs or fails.

Test builds use the same self-contained translation units as the Makefile: no
header is force-included, so a missing `#include` in `src/` fails the suite
instead of being masked.

`lifetime_test` retains 16 ELF32/64 scenarios: normal processing, injected
fstat/mmap failures, mixed inputs, and 100-file repetition with/without failure.
The wrappers check fd/mapping balance, errno preservation, valid unmapping and
mapping lifetime through classification and printing.

## Environment and logs

Requires Bash, an x86 little-endian Linux host, a C compiler, GNU as/ld/nm,
coreutils (timeout/cmp), sed, grep and /proc. libc must provide strlcpy.
GNU as and ld produce the ELF32 fixtures from assembly without multilib.

Sanitizers are disabled by default. To enable them where their runtimes are
available, run:

```sh
NM_TEST_SANITIZERS=address,undefined make test_all
```

If a ptrace-restricted sandbox prevents LeakSanitizer from running, set
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` to run without leak detection.
Each runner prints the sanitizer selection, individual results, and totals.
Evaluation cases report the stdout/stderr/exit comparison or malformed
input rejection rule, including both exit codes. Resource cases report input
count, injected fault, audit errors and peak mappings. Failures show diagnostics
and evaluation output differences. Final counts remain as a short overview.
Comparison/build files exist only during execution and the entire temporary
directory is removed on exit; no test logs or reports are retained.
Exit 0 means pass and 1 failure; nothing is skipped, so exit 2 cannot occur.
Warnings do not affect the exit status, so a run that warns still exits 0.

## Live demonstration

```sh
make demo
```

Builds ft_nm and temporary ELF32/64 objects, then shows actual shell commands,
symbol output, diagnostics and exit codes. Demonstrates GNU nm versus ft_nm,
default sorting, -r, -p, -rp priority, address widths, multiple files, continued
processing after a missing file, malformed ELF rejection and default a.out.
Expected error examples print exit 1 and the demonstration continues. This is a
presentation command, not an assertion suite. Generated files are removed on exit.

The former Python eval-auto runner was removed in favor of the main Bash suite.
This reorganization does not claim equivalent coverage: its separate Valgrind,
permission-error and build-cycle matrix is no longer an active test target.
Historical results in milestone.md describe the suites used at that time.
These tests do not prove correctness for every ELF, allocation failure,
close/munmap failure, concurrent truncation, libc or locale.
