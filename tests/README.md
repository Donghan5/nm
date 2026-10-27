# Tests

| Day | Run | Documentation |
|---|---|---|
| 1 | `make test-1` / `make test-1-32` | [File handling](day1/README.md) |
| 2 | `make test-2` | [ELF parsing](day2/DAY2.md) |
| 3 | `make test-3` | [Symbol collection](day3/DAY3.md) |
| 4 | `make test-4` | [Symbol classification and output](day4/DAY4.md) |

Each day directory contains its test runner, fixtures, and documentation.

Day 1-3 link the current project sources against libc. They do not require a
`libft/` directory; `memcpy`, `strlen`, and `strlcpy` are libc calls in the
current implementation. The host libc must provide `strlcpy` for Day 1 and the CLI.

The default sanitizer selection is `address,undefined`. Day 1-3 accept
`NM_TEST_SANITIZERS=address|undefined|address,undefined|none`, with optional
per-runner overrides `DAY1_SANITIZERS`, `DAY2_SANITIZERS`, and `DAY3_SANITIZERS`.
Day 4 uses the existing `DAY4_SANITIZERS` setting. No automatic fallback occurs.
Omitted checks are not passes; Day 1-3 print explicit SKIP messages.

On the current host, GCC's UBSan linker script references the missing
`/usr/lib64/libubsan.so.1.0.0`. Clang's standalone UBSan runtime is also absent.
Full UBSan verification requires a matching runtime installed in the environment.
Until then, run the available ASan checks explicitly:

```sh
NM_TEST_SANITIZERS=address ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
    make test-1 test-2 test-3 CC=/usr/bin/gcc
```

`detect_leaks=0` is needed in the current traced environment because
LeakSanitizer reports that it cannot run under ptrace. The runners preserve
caller-supplied ASAN_OPTIONS/UBSAN_OPTIONS; leak detection remains enabled by
default. Restore the defaults on a host with the required runtime and leak
checking support. The separate `test-1-32` target remains sanitizer-free and
requires 32-bit libc development headers/libraries.
