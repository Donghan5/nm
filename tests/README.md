# Tests

All test runners use Bash and build current sources into temporary binaries.
Python is not required. Run from the repository root:

```sh
make test            # Main suite; alias for eval_test
make eval_test       # 110 school, format, CLI and bonus checks
make edge_case_test       # 75 edge checks
make lifetime_test   # 16 failure-injection and resource-lifetime checks
```

## Main evaluation suite

The eight original school binaries are read-only. Each is tested with default,
`-r` and `-p`: accepted inputs require exact stdout/stderr and successful exit;
rejected inputs require exit 1, empty stdout and a filename-specific format error.
GNU nm's BFD plugin diagnostics are not reproduced.

The two school C sources are compiled and compared with GNU nm under the same
three options. Generated ELF32/64 objects, executables, PIE and shared libraries
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
Each runner prints PASS/FAIL/SKIP and a /tmp/nm-edge.* log directory containing
results.tsv, build logs and runtime outputs; evaluation also saves both tools'
exit statuses. Temporary binaries are removed on exit; logs are not permanent.
Exit 0 means pass, 1 failure, 2 skipped checks without failures.

The former Python eval-auto runner was removed in favor of the main Bash suite.
This reorganization does not claim equivalent coverage: its separate Valgrind,
permission-error and build-cycle matrix is no longer an active test target.
Historical results in milestone.md describe the suites used at that time.
These tests do not prove correctness for every ELF, allocation failure,
close/munmap failure, concurrent truncation, libc or locale.
