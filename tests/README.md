# Tests

All test runners use Bash and build current sources into temporary binaries.
Python is not required. Run from the repository root:

```sh
make test            # Main suite; alias for eval_test
make eval_test       # 122 school, shared-library, format, CLI and bonus checks
make edge_case_test       # 75 edge checks
make lifetime_test   # 16 failure-injection and resource-lifetime checks
```

## Main evaluation suite

The eight original school binaries are read-only. Each is tested with default,
`-r` and `-p`: accepted inputs require exact stdout/stderr and successful exit;
rejected inputs require exit 1, empty stdout and a filename-specific format error.
GNU nm's BFD plugin diagnostics are not reproduced.

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
Additional cases cover multiple files, options after/between files, `--` protecting
a dash-prefixed filename, and default `a.out` with and without bonus options.
Only implemented bonus options `-r` and `-p` are covered; `-a/-g/-u` are unsupported.
The generated assembly has no libc dependency, including when linked as ELF32.

## Edge and resource checks

`edge_case_test` retains file/size boundaries, actual 32-bit size conversion,
ELF identification, header/table truncation and offsets, symbol/string bounds,
extended indexes, section-name bounds and payload/NOTE/NOBITS policy.
Minimal valid controls remain to detect unconditional rejection. General symbol
classification and normal CLI output comparisons belong to the main suite.

`lifetime_test` retains 16 ELF32/64 scenarios: normal processing, injected
fstat/mmap failures, mixed inputs, and 100-file repetition with/without failure.
The wrappers check fd/mapping balance, errno preservation, valid unmapping and
mapping lifetime through classification and printing.

## Environment and logs

Requires Bash, an x86 little-endian Linux host, C compiler, GNU nm/ld,
coreutils (timeout/cmp), grep and /proc. libc must provide strlcpy. Actual 32-bit
ABI boundary checks additionally need multilib libc and kernel support.
The main evaluation's -m32 C build also requires multilib libc development files;
the generated input binaries are inspected, not executed.

ASan, UBSan and LeakSanitizer are enabled by default; only the two actual 32-bit
ABI checks omit sanitizers. A ptrace-restricted sandbox can prevent LSan from
running. Run in a supported environment to complete leak coverage.

```sh
bash tests/edge_case_test/run.sh --require-32
NM_TEST_SANITIZERS=none make test
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 make test
```

Disabling sanitizers or leak detection removes that coverage. Missing 32-bit ABI
support is reported as SKIP (or FAIL with --require-32), never PASS.
Each runner prints each test's subject and PASS/FAIL/SKIP result directly to the
terminal. Evaluation cases report the stdout/stderr/exit comparison or malformed
input rejection rule, including both exit codes. Resource cases report input
count, injected fault, audit errors and peak mappings. Failures show diagnostics
and evaluation output differences. Final counts remain as a short overview.
Comparison/build files exist only during execution and the entire temporary
directory is removed on exit; no test logs or reports are retained.
Exit 0 means pass, 1 failure, 2 skipped checks without failures.

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
