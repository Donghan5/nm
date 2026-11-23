# ft_nm

This project is part of the 42 school curriculum and focuses on Unix programming.
It reimplements the basic behavior of `nm`: reading an ELF file and displaying
its symbols, values, and type characters.

The program is written in C and parses ELF data directly using `<elf.h>`.
The current scope covers Linux x86 ELF32 and ELF64 object files, executables,
and shared libraries, with the additional `-r` and `-p` options.
The project subject is included in [nm.pdf](nm.pdf).

## Getting Started

Build on a little-endian x86 Linux host with GNU Make and a C compiler:

```sh
make
```

This creates `ft_nm` in the repository root. The build uses
`-Wall -Wextra -Werror` and has no libft dependency. It requires Linux/POSIX
headers and a libc providing `strlcpy`; older libc versions may need a port.
Reading ELF32 input with the native executable does not require a 32-bit libc.

| Command | Action |
|---|---|
| `make` | Build `ft_nm` |
| `make clean` | Remove object and dependency files |
| `make fclean` | Also remove `ft_nm` |
| `make re` | Remove build outputs and rebuild |

The compiler can be selected with `make CC=/usr/bin/gcc`.

## Usage

```text
./ft_nm [options] [files...]
```

```sh
./ft_nm                         # Read ./a.out
./ft_nm example.o               # Display an object's symbols
./ft_nm program libexample.so   # Process multiple files
./ft_nm -r example.o            # Reverse name order
./ft_nm -p example.o            # Keep symbol-table order
./ft_nm example.o -r            # Options may follow files
./ft_nm -- -example.o           # Read a filename beginning with '-'
```

If no file is supplied, including when only options are supplied, the program
reads `a.out` in the current directory. Multiple file arguments produce
filename headings and are processed in argument order.

### Options

| Option | Behavior |
|---|---|
| Default | Sort symbols by name using a bytewise comparison |
| `-r` | Sort names in reverse order |
| `-p` | Skip sorting and preserve symbol-table order |
| `--` | End option parsing |

Short options can be combined, as in `-rp`. When both `-r` and `-p` are present,
`-p` takes priority regardless of their order. Options before `--` apply to all
input files. Unsupported options print a diagnostic and usage, then exit with 1.

Bonus work is limited to `-r` and `-p`. The subject's other bonus options,
`-a`, `-g`, and `-u`, are not implemented.

### Output and Exit Status

Each displayed symbol has a value field, a type character, and a name.
Values use 8 hexadecimal digits for ELF32 and 16 for ELF64. Undefined symbols
have a blank value field. COMMON symbols display `st_size` rather than their
alignment value in `st_value`.

The implementation classifies text (`T/t`), data (`D/d`), BSS (`B/b`),
read-only data (`R/r`), undefined (`U`), absolute (`A/a`), common (`C`),
and weak symbols (`W/w`, `V/v`). Unclassified symbols use `?`.
Empty names and `STT_FILE`/`STT_SECTION` entries are filtered from the output.

The program exits with 0 when all files are processed successfully and 1 when
an option or file fails. A file error does not prevent subsequent files from
being processed. A valid input without a regular symbol table produces a
`no symbols` diagnostic and exits successfully. Empty files exit with 1 without
a diagnostic; directories are reported and also exit with 1.

## Project Structure

The project is organized into the following directories:

```text
.
├── Makefile
├── README.md
├── nm.pdf                       # Project subject
├── inc/                         # Headers, shared records, and declarations
│   ├── ft_nm.h
│   ├── functions.h
│   └── struct.h
├── src/
│   ├── main.c                   # Options, file loop, and processing pipeline
│   ├── elf/                     # ELF identification, readers, and validation
│   ├── symbol/                  # Collection, classification, sorting, printing
│   └── utils/                   # File mapping, cleanup, and diagnostics
└── tests/
    ├── common.sh                # Shared Bash test helpers
    ├── demo.sh                  # Live command and output demonstration
    ├── README.md                # Detailed Bash test guide
    ├── edge_case_test/           # Core fixtures and regression runner
    │   ├── fixtures/
    │   └── lifetime/            # Failure injection and resource audit
    └── eval_test/               # School fixtures and comparison runner
        └── libft_malloc.so      # Supplied shared-library test input
```

Build artifacts are generated under `build/`, and the executable is generated
as `ft_nm`. The test guide in `tests/README.md` describes the test suites and
their requirements.

## Project Architecture

The processing flow extends the original input → parse → sort → print design:

```text
Parse options and select files (a.out by default)
  → Open file, check metadata, mmap, close fd
  → Validate ELF identification, headers, and section tables
  → Find SHT_SYMTAB and its linked SHT_STRTAB
  → Validate and collect symbols into common t_symbol records
  → Check section payload ranges and classify symbols
  → Sort records, filter entries, and print
  → Free the symbol array and munmap the file
```

`src/elf/elf32.c` and `elf64.c` handle their respective ELF layouts.
They copy headers and entries into local structures with `memcpy`, validate
table ranges and section names, and resolve extended table counts.
The collectors find the first `SHT_SYMTAB` by section type, follow `sh_link`
to its string table, and resolve `SHN_XINDEX` through `SHT_SYMTAB_SHNDX`.

Both collectors produce the same `t_symbol` record, preserving the value,
size, binding, type, resolved section index, and original position.
Names are checked for an in-table NUL terminator and copied into the record.
Classification uses binding, special indexes, and section flags. Sorting moves
whole records so names remain associated with their metadata.

The file layer owns the mapping. The fd is closed after a successful mapping,
while the mapping remains available through classification and output.
The caller frees the collected array before releasing the mapping.

### Validation Policy

File ranges use `offset <= file_size && length <= file_size - offset`
to avoid overflowing an `offset + length` calculation. Table sizes, indexes,
entry sizes, string termination, and relevant integer conversions are checked
before the associated data is read.

An out-of-range general section payload produces a warning while symbol
processing continues. An out-of-range, nonempty `SHT_NOTE` payload is rejected.
Required symbol, string, and extended-index data must pass validation.
`SHT_NOBITS` has no file payload, and `SHT_NULL` fields are excluded from
the general payload check.

## Tests

Run these commands from the repository root:

```sh
make demo             # Show commands, actual output and exit codes
make demo32           # Show ELF32 file type, GNU nm and ft_nm output
make test             # Main evaluation suite (same as make eval_test)
make edge_case_test   # 130 edge checks
make lifetime_test    # 16 fd/mapping lifetime and failure-injection scenarios
make eval_test        # 137 school, shared-library, format, CLI and option checks
```

The evaluation suite compares supplied `tests/eval_test/*.so` files, including
`libft_malloc.so`, with GNU `nm` under default, `-r`, `-p` and combined options.
It also covers school fixtures and generated ELF32/64 inputs.

The test runners compile the current sources into temporary executables and
use `LC_ALL=C` for GNU `nm` comparisons. Sanitizers are optional and can be
enabled with `NM_TEST_SANITIZERS=address,undefined` where the runtimes are available.

`make demo` and `make demo32` display example output.
`make demo32` uses GNU `as --32` to build `build/demo32.o` without 32-bit libc.
`make clean` removes `build/demo32.o`.

See the [test guide](tests/README.md) for prerequisites, sanitizer settings,
coverage, exit codes, and the separate acceptance policy for malformed school
fixtures. Test results and failure diagnostics are printed to the terminal;
temporary comparison files and logs are removed when each runner exits.

## Scope and Limitations

- Supported inputs are little-endian i386 ELF32 and x86-64 ELF64 relocatable
  objects, executables (including PIE), and shared libraries. Archive containers,
  Mach-O, PE, other architectures, and big-endian ELF are unsupported.
- Symbol collection uses the regular symbol table. There is no dynamic-symbol
  mode or fallback to `SHT_DYNSYM`; use non-stripped inputs for symbol comparisons.
- Symbol names must fit in the fixed `PATH_MAX`-byte record buffer, including
  the terminating NUL. Longer names are rejected.
- Reference comparisons use GNU `nm` with `LC_ALL=C`. Diagnostic wording and BFD
  plugin output can differ. Full GNU `nm` option and symbol-type compatibility
  is outside the current scope.
- Equal names compare equal in `qsort`; their relative order is not guaranteed
  across libc implementations. Tests cover selected equal-name cases.
- Tests cover specified valid and malformed inputs. They do not establish
  correctness for every ELF variant, allocation failure, cleanup-system-call
  failure, or concurrent truncation of a mapped file.
