# Day 1 File Handling Tests

## Dedicated 32-bit conversion suite

Run `make test-1-32` to build with `-m32 -D_FILE_OFFSET_BITS=64` and run only the three size boundary cases. This exercises the actual implementation with 32-bit size_t and 64-bit off_t, including rejection of 4,294,967,296 (SIZE_MAX + 1). It reuses the existing C harness and wraps fstat64. No large file is created. All three cases must pass; an N/A result is a failure in this mode.

This target requires an x86 compiler with 32-bit libc headers/libraries and a Linux environment capable of executing 32-bit binaries. A separate probe checks compilation, linking, type widths, and execution first. Missing support produces ENVIRONMENT ERROR and a nonzero exit status, not a test pass. The native `make test-1` target remains available independently.

On x86_64 WSL, install the multilib development packages for the Linux distribution inside WSL. On x86_64 Fedora, install the corresponding i686 glibc development libraries and compiler support. ARM environments require a suitable x86 cross-toolchain and execution environment; this target does not configure either automatically.

The 32-bit suite intentionally omits sanitizers to avoid requiring separate 32-bit sanitizer runtimes. Continue using `make test-1` for sanitizer checks. This target tests integer conversion, not ELF32 parsing.

## Native suite

Run:

```sh
bash tests/day1/test_day1.sh
```

Requires Linux with `/proc/self/fd`, a C compiler, AddressSanitizer/UndefinedBehaviorSanitizer, and `timeout`. To select a compiler, use `CC=clang bash tests/day1/test_day1.sh`.

The harness links the actual `src/file.c`, project headers, and required libft functions. It excludes the empty ELF parsers and unfinished main. A full project build must be checked separately. If the test build fails, runtime tests are reported as not executed.

| Test | Checks |
|---|---|
| cleanup-zero | Safe cleanup and repeated cleanup of a zero-initialized structure |
| regular | Success return, size, filename, mapped bytes, reads after fd closure, and cleanup |
| empty | Rejection of an empty file followed by safe cleanup |
| missing | Rejection of a nonexistent file followed by safe cleanup |
| directory | Rejection of a directory followed by safe cleanup |
| repeat | 64 load/cleanup cycles using the same structure to check fd leaks and lifetime issues |
| size-negative | Reject a negative st_size without changing the output size |
| size-max-valid | Preserve the largest nonnegative size representable in both off_t and size_t |
| size-overflow | Reject SIZE_MAX + 1, or report N/A (not applicable) if off_t cannot represent it |

Size boundary cases use GNU-compatible linker `--wrap=fstat` support to inject metadata into the real `check_file_size()` function. Other cases use the real fstat. No huge file is created or mapped. These tests target Linux with signed off_t; the compiler's native type widths determine which boundaries are reachable. On typical 64-bit Linux, all nonnegative off_t values fit in size_t, so the overflow case is reported as N/A with the reason on the same line. N/A means this input cannot be represented on the current platform; it is neither a pass nor a failure and does not verify overflow rejection. Exit code 77 communicates this result to the runner. Rejected metadata must leave the initially zero output size unchanged.

The expected API contract is `0` on success and a negative value on failure. The caller initializes the structure to zero. Cleanup must be safe after both success and failure, reset `f_data` to `NULL` and `f_size` to `0`, and remain safe when repeated. This reset contract is a Day 1 design target, not a guarantee provided by the current implementation.

Each runtime case runs in a separate process with a 10-second timeout. Fixtures and the test executable are stored in a temporary directory created for each run and removed on exit. Sanitizers check invalid frees, invalid memory accesses, and related errors. Counting file descriptors checks fd leaks; it does not prove the absence of all mmap leaks.

Empty files, nonexistent files, and directories are tested as separate inputs, but the tests do not require the current return value to identify each error category. Error messages and program exit status require integration tests after main is connected. Forced fstat/mmap failures, very large files, permission errors, and comparison against the project PDF are outside this suite's scope.
