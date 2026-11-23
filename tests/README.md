# Tests

All test runners use Bash and build current sources into temporary binaries.
Python is not required. Run from the repository root:

```sh
make test            # Main suite; alias for eval_test
make eval_test       # 140 school, shared-library, format, CLI and bonus checks
make edge_case_test  # 132 edge checks
make lifetime_test   # 16 failure-injection and resource-lifetime checks
make test_all        # All three suites in one run
make proof           # Live differential scan of every ELF file on this host
```

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

The school easy_test.c is compiled with the compiler's default architecture.
not_so_easy_test.c is compiled explicitly with -m32 and -m64, as required by the
evaluation sheet. All three binaries are compared with GNU nm under default,
-r and -p; unavailable multilib support is a build failure, not a skipped pass.
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

`edge_case_test` covers file/size boundaries, actual 32-bit size conversion,
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

Requires Bash, an x86 little-endian Linux host, C compiler, GNU nm/ld,
coreutils (timeout/cmp), sed, grep and /proc. libc must provide strlcpy.
Multilib libc and kernel support for -m32 is needed by the actual 32-bit ABI
boundary checks, the main evaluation's -m32 C build and the ELF32 fixtures. The
generated input binaries are inspected, not executed. On Debian or Ubuntu,
install `gcc-multilib`.

No check is ever skipped: every `Result` line reads `0 SKIP`, so an absent skip
is visible evidence rather than an absent line. A host without multilib is a
local toolchain gap, not a defect in ft_nm, and is reported two ways. The two
actual 32-bit ABI checks in `edge_case_test` emit `WARN`, which names both
checks that did not run and leaves the exit status at 0. `eval_test` and
`lifetime_test` still fail, because the evaluation sheet requires their -m32
builds and a 32-bit fixture they cannot substitute. Warnings are never test
results and never mask one; a check that can run either passes or fails.

ASan, UBSan and LeakSanitizer are enabled by default; only the two actual 32-bit
ABI checks omit sanitizers. A ptrace-restricted sandbox can prevent LSan from
running. Run in a supported environment to complete leak coverage.

```sh
NM_TEST_SANITIZERS=none make test
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make test
```

Disabling sanitizers or leak detection removes that coverage, and each runner
says so on its first lines; it does not change any check's PASS/FAIL result.
Each runner prints each test's subject and PASS/FAIL/WARN result directly to the
terminal. Evaluation cases report the stdout/stderr/exit comparison or malformed
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
