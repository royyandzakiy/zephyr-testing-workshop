# tests/pytest_raw/conftest.py
#
# pytest against the native_sim binary, with no twister and no
# twister_harness. About sixty lines, and every one of them is something
# twister was doing for you in tests/shell_pytest/.
#
# Worth doing once. It is the difference between "the harness is magic" and
# "the harness spawns a process and reads lines off a pipe".
#
# Run it with:
#   cd apps/05-pytest-advanced/tests/shell_pytest
#   west build -b native_sim/native -p
#   cd ../pytest_raw && pytest

import os
import queue
import subprocess
import threading
import time
from pathlib import Path

import pytest

HERE = Path(__file__).parent
DEFAULT_EXE = HERE / '..' / 'shell_pytest' / 'build' / 'zephyr' / 'zephyr.exe'

PROMPT = 'uart:~$'


def _binary() -> Path:
    return Path(os.environ.get('ZEPHYR_EXE', DEFAULT_EXE)).resolve()


class RawShell:
    """The smallest thing that can be called a device adapter.

    twister_harness.Shell does all of this and handles serial ports, resets,
    flashing and timeouts on top. Here there is one process and one pipe.
    """

    def __init__(self, exe: Path):
        self._proc = subprocess.Popen(
            [str(exe)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        self._lines: queue.Queue = queue.Queue()
        # A reader thread, because reading a pipe blocks and a test that hangs
        # forever is worse than a test that fails.
        self._reader = threading.Thread(target=self._pump, daemon=True)
        self._reader.start()
        self.read_until(PROMPT, timeout=10.0)

    def _pump(self):
        for line in self._proc.stdout:
            self._lines.put(line.rstrip('\r\n'))

    def read_until(self, needle: str, timeout: float = 5.0):
        deadline = time.time() + timeout
        seen = []
        while time.time() < deadline:
            try:
                line = self._lines.get(timeout=0.1)
            except queue.Empty:
                continue
            seen.append(line)
            if needle in line:
                return seen
        raise TimeoutError(f'never saw {needle!r}; got {seen}')

    def exec_command(self, cmd: str, timeout: float = 5.0):
        self._proc.stdin.write(cmd + '\n')
        self._proc.stdin.flush()
        lines = self.read_until(PROMPT, timeout=timeout)
        # Drop the echo of the command itself and the trailing prompt.
        return [ln for ln in lines if cmd not in ln and PROMPT not in ln]

    def close(self):
        self._proc.terminate()
        try:
            self._proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self._proc.kill()


@pytest.fixture(scope='module')
def raw_shell():
    exe = _binary()
    if not exe.exists():
        pytest.skip(f'{exe} not built; see the header of this file')

    sh = RawShell(exe)
    yield sh
    sh.close()
