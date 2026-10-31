"""Compare raw records against an independent stdlib ELF parser for real .o files."""
import struct
import subprocess
import sys

binary, path = sys.argv[1:]
data = open(path, 'rb').read()
if data[:4] != b'\x7fELF' or data[5] != 1:
    raise SystemExit('FAIL: fixture is not little-endian ELF')
bits = 32 if data[4] == 1 else 64
if bits == 32:
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', data)
    sh_fmt, sym_fmt = '<IIIIIIIIII', '<IIIBBH'
else:
    header = struct.unpack_from('<16sHHIQQQIHHHHHH', data)
    sh_fmt, sym_fmt = '<IIQQQQIIQQ', '<IBBHQQ'
sh_offset, sh_size, sh_count = header[6], header[11], header[12]
sections = [struct.unpack_from(sh_fmt, data, sh_offset + i * sh_size) for i in range(sh_count)]
expected = []
for section in sections:
    if section[1] != 2:  # SHT_SYMTAB
        continue
    strings = sections[section[6]]
    names = data[strings[4]: strings[4] + strings[5]]
    for i in range(section[5] // section[9]):
        raw = struct.unpack_from(sym_fmt, data, section[4] + i * section[9])
        if bits == 32:
            name, value, _, info, _, index = raw
        else:
            name, info, _, index, value, _ = raw
        end = names.index(0, name)
        expected.append((i, bits, info & 15, info >> 4, index, value, names[name:end].hex()))
    break
completed = subprocess.run([binary, 'object', path], text=True, capture_output=True, timeout=10)
if completed.returncode:
    sys.stderr.write(completed.stderr)
    raise SystemExit('FAIL: real object harness failed')
actual = []
for line in completed.stdout.splitlines():
    fields = line.split(' ', 6)
    actual.append((*map(int, fields[:6]), fields[6]))
if actual != expected:
    raise SystemExit(f'FAIL: ELF{bits} records differ from independent reference\nexpected={expected}\nactual={actual}')
print(f'PASS: ELF{bits}/real-object ({len(actual)} records matched)')
