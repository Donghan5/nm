"""Compare exact stdout and check stderr/status separately; never normalize spacing."""
import difflib
import os
from pathlib import Path
import subprocess
import sys

candidate, fixture = sys.argv[1:]
env = dict(os.environ, LC_ALL="C")
try:
    reference = subprocess.run(["nm", fixture], capture_output=True, timeout=10, env=env)
    actual = subprocess.run([str(Path(candidate).resolve()), fixture],
                            capture_output=True, timeout=10, env=env)
except (OSError, subprocess.TimeoutExpired) as error:
    print(f"FAIL: {error}")
    sys.exit(1)
failed = False
for name, result in (("nm", reference), ("candidate", actual)):
    if result.returncode != 0 or result.stderr:
        print(f"FAIL: {name}: status={result.returncode}, stderr={result.stderr!r}")
        failed = True
if actual.stdout != reference.stdout:
    print("FAIL: exact stdout (filter/order/value width/padding/type/name)")
    print("".join(difflib.unified_diff(
        reference.stdout.decode(errors="backslashreplace").splitlines(True),
        actual.stdout.decode(errors="backslashreplace").splitlines(True),
        fromfile="system nm", tofile="candidate")), end="")
    failed = True
if not failed:
    print(f"PASS: exact nm output: {Path(fixture).name}")
sys.exit(int(failed))
