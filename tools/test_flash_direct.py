# SPDX-License-Identifier: GPL-2.0-or-later

"""Test the Linux flashing helper with mock tools, without USB access."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


FLASH_SCRIPT = Path(__file__).resolve().parents[1] / "flash-direct.sh"
MOCK_TOOL = """
import json
import os
from pathlib import Path
import sys

name = Path(sys.argv[0]).name
args = sys.argv[1:]
with open(os.environ['FLASH_TEST_LOG'], 'a') as log:
    log.write(json.dumps([name, *args]) + '\\n')
if name == 'sudo':
    if args == ['-v']:
        sys.exit(int(os.environ.get('FLASH_TEST_AUTH_EXIT', '0')))
    assert args[0] == 'picotool'
    tool = str(Path(__file__).parent / 'picotool')
    os.execv(tool, [tool, *args[1:]])
if name == 'picotool' and args[0] == 'load':
    sys.exit(int(os.environ.get('FLASH_TEST_LOAD_EXIT', '0')))
"""


class FlashDirectTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.bin = self.root / "bin"
        self.bin.mkdir()
        self.log = self.root / "commands.jsonl"
        self.firmware = self.root / "standalone firmware.uf2"
        self.firmware.write_bytes(b"mock firmware")
        for name in ["sudo", "picotool", "sleep"]:
            tool = self.bin / name
            tool.write_text(f"#!{sys.executable}\n" + MOCK_TOOL)
            tool.chmod(0o755)

    def run_flash(self, *args, **settings):
        env = os.environ.copy()
        env.update(PATH=str(self.bin), FLASH_TEST_LOG=str(self.log), **settings)
        result = subprocess.run(
            ["/bin/sh", str(FLASH_SCRIPT), *map(str, args)],
            env=env, capture_output=True, text=True,
        )
        commands = [json.loads(line) for line in self.log.read_text().splitlines()] if self.log.exists() else []
        return result, commands

    def test_requires_explicit_firmware(self):
        result, commands = self.run_flash()
        self.assertEqual(result.returncode, 2)
        self.assertEqual(commands, [])

    def test_missing_firmware_does_not_authenticate_or_flash(self):
        result, commands = self.run_flash(self.root / "missing.uf2")
        self.assertEqual(result.returncode, 1)
        self.assertEqual(commands, [])

    def test_authentication_failure_stops_before_delay_or_flash(self):
        result, commands = self.run_flash(self.firmware, FLASH_TEST_AUTH_EXIT="7")
        self.assertEqual(result.returncode, 7)
        self.assertEqual(commands, [["sudo", "-v"]])

    def test_failed_load_or_verification_never_reboots(self):
        result, commands = self.run_flash(self.firmware, FLASH_TEST_LOAD_EXIT="9")
        self.assertEqual(result.returncode, 9)
        self.assertIn(["picotool", "load", "-u", "-v", str(self.firmware)], commands)
        self.assertFalse(any("reboot" in command for command in commands))

    def test_authenticates_then_delays_and_verifies_before_reboot(self):
        result, commands = self.run_flash(self.firmware)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(commands, [
            ["sudo", "-v"],
            ["sleep", "5"],
            ["sudo", "picotool", "load", "-u", "-v", str(self.firmware)],
            ["picotool", "load", "-u", "-v", str(self.firmware)],
            ["sudo", "picotool", "reboot"],
            ["picotool", "reboot"],
        ])


if __name__ == "__main__":
    unittest.main()
