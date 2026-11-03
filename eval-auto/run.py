#!/usr/bin/env python3
"""AI-authored Linux ELF mandatory evaluation; no project source mutations."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--require-32', action='store_true', help='compatibility: always required')
parser.add_argument('--memory', action='store_true', help='compatibility: always enabled')
parser.add_argument('--bonus', action='store_true',
                    help='also verify implemented -r/-p bonus options')
args = parser.parse_args()
os.environ['LC_ALL'] = 'C'
logs = Path(tempfile.mkdtemp(prefix='nm-eval-auto-'))
work = logs / 'fixtures'
work.mkdir()
results = []
cc = shlex.split(os.environ.get('CC', 'cc'))
nm = shlex.split(os.environ.get('NM', 'nm'))
binary = ROOT / 'ft_nm'
sanitized = logs / 'sanitized-ft_nm'
sanitizer_ready = None


def record(ident, status, reason=''):
    results.append(dict(id=ident, status=status, reason=reason))
    print(f'{status}: {ident} {reason}', flush=True)


def run(ident, command, cwd=work, seconds=15):
    try:
        p = subprocess.run(list(map(str, command)), cwd=cwd,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           timeout=seconds)
        code, out, err = p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired as exc:
        code, out, err = 124, exc.stdout or b'', exc.stderr or b''
    except OSError as exc:
        code, out, err = 127, b'', str(exc).encode()
    (logs / (ident + '.stdout')).write_bytes(out)
    (logs / (ident + '.stderr')).write_bytes(err)
    (logs / (ident + '.status')).write_text(str(code) + '\n')
    (logs / (ident + '.command.json')).write_text(json.dumps(list(map(str, command))))
    return code, out, err


def build(ident, command):
    result = run(ident, command, ROOT, 120)
    record(ident, 'PASS' if result[0] == 0 else 'FAIL',
           '' if result[0] == 0 else 'build log: ' + str(logs / (ident + '.stderr')))
    return result[0] == 0


def check_memory(ident, paths, expected):
    global sanitizer_ready
    if not shutil.which('valgrind'):
        record(ident + '.memory', 'FAIL', 'required tool unavailable: valgrind')
        return
    mem = run(ident + '.memory', ['valgrind', '--leak-check=full',
              '--show-leak-kinds=all', '--errors-for-leak-kinds=all',
              '--error-exitcode=99', binary] + list(paths), seconds=60)
    if b'Valgrind: debuginfo reader:' not in mem[2]:
        good = mem[0] == expected[0] and mem[1] == expected[1] and (
            b'ERROR SUMMARY: 0 errors' in mem[2])
        record(ident + '.memory', 'PASS' if good else 'FAIL', 'Valgrind')
        return
    # Valgrind itself parses mmap images. Use independent instrumentation
    # of the same project sources when its debug reader rejects an input.
    if sanitizer_ready is None:
        sanitizer_ready = build('MEM.sanitizer-build', ['make', '-B', 'all',
            f'NAME={sanitized}', f'OBJ_DIR={logs / "san-obj"}',
            f'DEP_DIR={logs / "san-dep"}',
            'CFLAGS=-Wall -Wextra -Werror -g -O1 -fsanitize=address,undefined '
            '-fno-omit-frame-pointer -fno-pie -no-pie'])
    if not sanitizer_ready:
        record(ident + '.memory', 'FAIL', 'sanitizer fallback build failed')
        return
    mem = run(ident + '.sanitizer', ['env',
              'ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:exitcode=99',
              'UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1:exitcode=99',
              'LSAN_OPTIONS=exitcode=99', sanitized] + list(paths), seconds=60)
    record(ident + '.memory', 'PASS' if mem == expected else 'FAIL',
           'ASan + UBSan + LeakSanitizer (Valgrind debug reader fallback)')


def compare(ident, paths, malformed=False):
    if paths:
        run(ident + '.file', ['file'] + list(paths))
    ref = run(ident + '.nm', nm + list(paths))
    got = run(ident + '.ft', [binary] + list(paths))
    # Only tool-name prefixes are permitted to differ in diagnostics.
    referr = ref[2].replace(b'nm:', b'TOOL:')
    goterr = got[2].replace(b'ft_nm:', b'TOOL:')
    referr = referr.replace(b'No such file or directory', b'No such file')
    goterr = goterr.replace(b'No such file or directory', b'No such file')
    safe = got[0] >= 0 and got[0] not in (124, 127)
    if malformed:
        # Deliberately independent of BFD's acceptance of damaged ELF.
        good = safe and got[0] in (0, 1) and (
            (ref[0] == 0 and got[0] == 0 and ref[1] == got[1]) or
            (ref[0] != 0 and got[0] == 1 and not got[1] and bool(got[2])))
    else:
        good = safe and ref[0] == got[0] and ref[0] not in (124, 127) and (
            ref[1] == got[1] and referr == goterr)
    record(ident, 'PASS' if good else 'FAIL', f'nm={ref[0]}, ft_nm={got[0]}')
    check_memory(ident, paths, got)


try:
    for tool, command in [('compiler', cc), ('nm', nm), ('make', ['make']),
                          ('file', ['file']), ('ld', ['ld'])]:
        run('environment.' + tool, command + ['--version'], ROOT)
    metadata = {f: hashlib.sha256((ROOT / f).read_bytes()).hexdigest()
                for f in ['nm.pdf', 'eval.pdf'] if (ROOT / f).exists()}
    metadata['LC_ALL'] = 'C'
    metadata['manual_review'] = ['repository ownership', 'code explanation',
                                 'absence of shortcuts', 'submission layout']
    (logs / 'environment.json').write_text(json.dumps(metadata, indent=2))
    if not build('PRE.build', ['make', '-B', 'all']):
        raise RuntimeError('project build failed; runtime tests not executed')
    before = binary.stat().st_mtime_ns
    if build('PRE.incremental', ['make', 'all']):
        record('PRE.no-relink', 'PASS' if before == binary.stat().st_mtime_ns else 'FAIL')
    school = ROOT / 'tests/eval_test'
    if build('EASY.build', cc + [school / 'easy_test.c', '-o', work / 'easy']):
        compare('EASY.64', ['easy'])
    valid = []
    for bits in (64, 32):
        probe = run(f'ABI.{bits}', cc + [f'-m{bits}', school / 'not_so_easy_test.c',
                                        '-o', work / f'less{bits}'], seconds=60)
        if probe[0] != 0:
            record(f'LESS.{bits}', 'FAIL',
                   'compiler/linker ABI unavailable; see ABI log')
        else:
            compare(f'LESS.{bits}', [f'less{bits}'])
            valid.append(f'less{bits}')
        # Freestanding fixtures require no multilib libc, including ELF32.
        source = work / f'symbols{bits}.s'
        source.write_text('''.text
.globl _start
_start: ret
.local local_text
local_text: ret
.weak weak_function
weak_function: ret
.data
.globl global_data
global_data: .long 42
.local local_data
local_data: .long 7
.long missing_symbol
.weak weak_missing
.long weak_missing
.bss
.globl global_bss
global_bss: .zero 8
.section .rodata
.globl readonly_symbol
readonly_symbol: .long 3
.comm common_symbol,16,8
.globl absolute_symbol
.set absolute_symbol,0x1234
.section .note.GNU-stack,"",@progbits
''')
        if bits == 64:
            source.write_text(source.read_text().replace('.long missing_symbol',
                              '.quad missing_symbol').replace('.long weak_missing',
                              '.quad weak_missing'))
        obj = f'symbols{bits}.o'
        built = run(f'OBJ.{bits}.build', cc + [f'-m{bits}', '-c', source, '-o', work / obj])
        if built[0] != 0:
            for group in ('OBJ', 'SO', 'EXEC', 'PIE'):
                record(f'{group}.{bits}', 'FAIL',
                       'assembler unavailable; see OBJ build log')
            continue
        compare(f'OBJ.{bits}', [obj])
        valid.append(obj)
        emulation = 'elf_i386' if bits == 32 else 'elf_x86_64'
        for group, flags in [('SO', ['-shared']), ('EXEC', []), ('PIE', ['-pie'])]:
            # Link controls resolve external references to absolute zero;
            # the relocatable fixture retains its undefined symbols.
            name = f'{group.lower()}{bits}'
            linkflags = flags + ['-Bsymbolic', '--defsym=missing_symbol=0',
                                 '--defsym=weak_missing=0']
            linked = run(f'{group}.{bits}.build', ['ld', '-m', emulation] + linkflags +
                         ['-o', work / name, work / obj])
            if linked[0] != 0:
                record(f'{group}.{bits}', 'FAIL', 'fixture link failed; see build log')
            else:
                compare(f'{group}.{bits}', [name])
                valid.append(name)
    if valid:
        shutil.copyfile(work / valid[0], work / 'a.out')
        compare('CLI.default', [])
        compare('CLI.multiple', valid)
        compare('CLI.mixed', [valid[0], 'missing', valid[-1]])
        shutil.copyfile(work / valid[0], work / 'stripped')
        if build('OTHER.strip', ['strip', work / 'stripped']):
            compare('OTHER.stripped', ['stripped'])
    if args.bonus:
        # Reuse real ELF32/64 objects, executables and shared libraries.
        # Compare raw output: sorting or whitespace normalization hides bugs.
        for name in valid:
            for label, options in [('r', ['-r']), ('p', ['-p']),
                                   ('rp', ['-rp']), ('pr', ['-pr'])]:
                compare(f'BONUS.{name}.{label}', options + [name])
        if valid:
            for label, options in [('separate-rp', ['-r', '-p']),
                                   ('separate-pr', ['-p', '-r']),
                                   ('repeat-r', ['-rr']), ('repeat-p', ['-pp'])]:
                compare('BONUS.' + label, options + [valid[0]])
            for flag in ('-r', '-p'):
                label = flag[1:]
                compare(f'BONUS.default.{label}', [flag])
                compare(f'BONUS.multiple.{label}', [flag] + valid)
                compare(f'BONUS.mixed.{label}',
                        [flag, valid[0], 'missing', valid[-1]])
                compare(f'BONUS.trailing.{label}', [valid[0], flag])
                compare(f'BONUS.between.{label}',
                        [valid[0], flag, valid[-1]])
                compare(f'BONUS.stripped.{label}', [flag, 'stripped'])
                compare(f'BONUS.malformed.{label}',
                        [flag, school / 'header'], malformed=True)
            # A dash-prefixed filename must become an operand after --.
            shutil.copyfile(work / valid[0], work / '-r')
            compare('BONUS.end-options', ['--', '-r'])
            compare('BONUS.end-options-p', ['-p', '--', '-r'])
    for name in ('error_header', 'header_offset_error', 'header', 'header_copy',
                 'header_and_prog', 'header_and_prog_copy', 'unterminated_string', 'wrong_arch'):
        compare('ERR.' + name, [school / name], malformed=True)
    (work / 'empty').touch()
    (work / 'text').write_text('not an ELF\n')
    (work / 'directory').mkdir()
    for name in ('empty', 'text', 'directory', 'missing'):
        compare('ERR.' + name, [name])
    (work / 'a.out').unlink(missing_ok=True)
    compare('CLI.default-missing', [])
    if valid:
        blob = (work / valid[0]).read_bytes()
        for length in (1, 16, 40):
            name = f'truncated{length}'
            (work / name).write_bytes(blob[:length])
            compare('ERR.' + name, [name], malformed=True)
        denied = work / 'denied'
        denied.write_bytes(blob)
        denied.chmod(0)
        if os.access(denied, os.R_OK):
            record('ERR.permission', 'FAIL', 'current identity bypasses mode 000; run as unprivileged user')
        else:
            compare('ERR.permission', ['denied'])
        denied.chmod(0o600)
except (RuntimeError, OSError) as exc:
    record('HARNESS', 'FAIL', str(exc))
finally:
    (logs / 'results.json').write_text(json.dumps(results, indent=2))
    counts = {s: sum(r['status'] == s for r in results) for s in ('PASS', 'FAIL')}
    print(f'Result: {counts}; logs and fixtures: {logs}')
raise SystemExit(1 if counts['FAIL'] else 0)
