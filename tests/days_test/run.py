#!/usr/bin/env python3
"""One entry point for Day1-4 edge cases and positive controls."""
import argparse
from collections import Counter
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / 'tests'
ELF = ['src/elf/elf_ident.c', 'src/elf/elf32.c', 'src/elf/elf64.c']
COLLECT = ['src/symbol/symbol_collect.c', 'src/symbol/symbol_validate.c']
IDENT_CASES = 'ranges short-inputs valid-ident magic class endian version'.split()
TABLE_CASES = '''regular none exec shared extended-sections extended-truncated
extended-programs program program-truncated program-entry program-zero-offset
bad-magic wrong-class big-endian header-size header-version machine type
section-entry section-truncated section-offset zero-offset-count reserved-count
extended-small short-header null-args'''.split()
SYMBOL_CASES = '''regular abs common local missing dynsym-only link-oob link-type
entry-zero entry-wrong entry-partial symbol-payload string-payload name-oob
name-no-nul section-oob xindex-unsupported name-max-fit name-too-long
reader-contract string-contract'''.split()
TYPE_CASES = '''undefined absolute-global absolute-local common weak-object-defined
weak-object-undefined weak-function-defined weak-function-undefined weak-notype-defined
weak-notype-undefined text-global text-local text-notype data-global data-local
bss-global bss-local readonly-global readonly-local readonly-merge-strings
text-extra-group tls-data tls-bss dynamic-global dynamic-local init-array-global
init-array-local fini-array-global fini-array-local preinit-array-global
preinit-array-local note-global note-local note-nonalloc text-writable
index-out-of-range index-huge'''.split()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--require-32', action='store_true',
                        help='Treat unavailable 32-bit ABI as failure, not SKIP')
    parser.add_argument('--report', type=Path, help='Write per-case JSON results')
    args = parser.parse_args()
    cc = shlex.split(os.environ.get('CC', 'cc'))
    sanitizers = os.environ.get('NM_TEST_SANITIZERS', 'address,undefined')
    if sanitizers not in ('address', 'undefined', 'address,undefined', 'none'):
        parser.error('invalid NM_TEST_SANITIZERS')
    env = dict(os.environ, LC_ALL='C')
    env.setdefault('ASAN_OPTIONS', 'detect_leaks=1:halt_on_error=1')
    env.setdefault('UBSAN_OPTIONS', 'halt_on_error=1:print_stacktrace=1')
    sanitizer_flags = [] if sanitizers == 'none' else [
        '-fsanitize=' + sanitizers, '-fno-omit-frame-pointer']
    rows = []
    print('Sanitizers:', sanitizers, flush=True)
    if 'address' not in sanitizers:
        print('SKIP: ASan / LeakSanitizer (not selected)', flush=True)
    elif 'detect_leaks=0' in env['ASAN_OPTIONS'].split(':'):
        print('SKIP: LeakSanitizer (detect_leaks=0)', flush=True)
    if 'undefined' not in sanitizers:
        print('SKIP: UBSan (not selected)', flush=True)

    def record(group, name, status, details=''):
        rows.append(dict(group=group, name=name, status=status, details=details))
        if status != 'PASS':
            print(f'{status}: {group}/{name}\n{details}', flush=True)

    def execute(command):
        try:
            result = subprocess.run(list(map(str, command)), cwd=ROOT, env=env,
                                    capture_output=True, text=True, timeout=20)
            return result.returncode, result.stdout + result.stderr
        except (OSError, subprocess.TimeoutExpired) as error:
            return None, str(error)

    with tempfile.TemporaryDirectory(prefix='nm-days-test-') as temporary:
        tmp = Path(temporary)
        (tmp / 'regular').write_text('hello\n')
        (tmp / 'empty').touch()
        (tmp / 'directory').mkdir()
        base = cc + ['-Wall', '-Wextra', '-Werror', '-g', '-O1', '-I' + str(ROOT / 'inc'), '-I' + str(ROOT)]

        def compile_binary(name, sources, flags=None, sanitized=True):
            binary = tmp / name
            cmd = base + (sanitizer_flags if sanitized else []) + (flags or [])
            code, details = execute(cmd + [str(ROOT / source) for source in sources] + ['-o', binary])
            if code != 0:
                record('build', name, 'FAIL', details)
                return None
            return binary

        def run(group, name, command):
            code, details = execute(command)
            record(group, name, 'PASS' if code == 0 else 'FAIL', details)

        file_sources = ['tests/days_test/fixtures/file_test.c', 'src/file.c', 'src/utils/error.c']
        native = compile_binary('file-native', file_sources, ['-Wl,--wrap=fstat'])
        if native:
            for name in ['cleanup-zero', 'regular', 'empty', 'missing', 'directory',
                         'repeat', 'size-negative', 'size-max-valid']:
                path = tmp / (name if name in ['empty', 'missing', 'directory'] else 'regular')
                run('file-lifetime', name, [native, name, path])
        # SIZE_MAX+1 is unrepresentable with native signed 64-bit off_t.
        # Run the real conversion branch using 32-bit size_t and 64-bit off_t.
        probe = tmp / 'abi32'
        code, details = execute(cc + ['-m32', '-D_FILE_OFFSET_BITS=64',
                                     TESTS / 'days_test/fixtures/abi32_probe.c', '-o', probe])
        if code == 0:
            code, details = execute([probe])
        if code != 0:
            for name in ['size-negative', 'size-max-valid', 'size-overflow']:
                record('size32', name, 'FAIL' if args.require_32 else 'SKIP',
                       '32-bit ABI unavailable: ' + details)
        else:
            flags = ['-m32', '-D_FILE_OFFSET_BITS=64', '-DDAY1_TEST_32', '-Wl,--wrap=fstat64']
            binary = compile_binary('file32', file_sources, flags, sanitized=False)
            if binary:
                for name in ['size-negative', 'size-max-valid', 'size-overflow']:
                    run('size32', name, [binary, name, tmp / 'regular'])

        ident = compile_binary('ident', ['tests/days_test/fixtures/ident_test.c', ELF[0]])
        if ident:
            for name in IDENT_CASES:
                run('ident-boundaries', name, [ident, name])
        types = compile_binary('types', ['tests/days_test/fixtures/type_test.c', 'src/symbol/symbol_type.c'] + ELF)
        cli_sources = ['src/main.c', 'src/file.c'] + ELF + COLLECT + [
            'src/symbol/symbol_type.c', 'src/sort/sort_symbols.c',
            'src/print/print_symbols.c', 'src/utils/error.c']
        cli = compile_binary('ft_nm', cli_sources)
        for bits in (32, 64):
            table = compile_binary(f'table{bits}', ['tests/days_test/fixtures/table_test.c'] + ELF,
                                   [f'-DDAY2_BITS={bits}'])
            if table:
                for name in TABLE_CASES + (['huge-offset', 'huge-count'] if bits == 64 else []):
                    run('table-boundaries', f'ELF{bits}/{name}', [table, name])
            symbols = compile_binary(f'symbols{bits}', ['tests/days_test/fixtures/symbol_test.c'] + ELF + COLLECT,
                                     [f'-DDAY3_BITS={bits}'])
            if symbols:
                for name in SYMBOL_CASES + (['huge-offset', 'huge-size'] if bits == 64 else []):
                    run('symbol-boundaries', f'ELF{bits}/{name}', [symbols, name])
                fixture = tmp / f'symbol{bits}.o'
                code, details = execute(cc + [f'-m{bits}', '-fno-lto', '-fno-pic', '-c',
                                             TESTS / 'days_test/fixtures/symbol_fixture.c', '-o', fixture])
                if code != 0:
                    record('symbol-reference', f'ELF{bits}', 'FAIL', details)
                else:
                    run('symbol-reference', f'ELF{bits}',
                        [sys.executable, TESTS / 'days_test/fixtures/symbol_compare.py', symbols, fixture])
            if types:
                for name in TYPE_CASES:
                    run('classification-boundaries', f'ELF{bits}/{name}', [types, str(bits), name])
            if cli:
                fixture = tmp / f'output{bits}.o'
                code, details = execute(cc + [f'-m{bits}', '-c', TESTS / 'days_test/fixtures/output_fixture.s', '-o', fixture])
                if code != 0:
                    record('exact-output', f'ELF{bits}', 'FAIL', details)
                else:
                    run('exact-output', f'ELF{bits}',
                        [sys.executable, TESTS / 'days_test/fixtures/output_compare.py', cli, fixture])

    for group in dict.fromkeys(row['group'] for row in rows):
        counts = Counter(row['status'] for row in rows if row['group'] == group)
        print(f"{group}: {counts['PASS']} PASS / {counts['FAIL']} FAIL / {counts['SKIP']} SKIP")
    counts = Counter(row['status'] for row in rows)
    print(f"Result: {counts['PASS']} PASS / {counts['FAIL']} FAIL / {counts['SKIP']} SKIP")
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(dict(sanitizers=sanitizers, results=rows), indent=2))
    return 1 if counts['FAIL'] else (2 if counts['SKIP'] else 0)


if __name__ == '__main__':
    sys.exit(main())
