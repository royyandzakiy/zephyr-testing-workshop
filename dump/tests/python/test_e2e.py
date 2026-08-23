"""End-to-end tests for full application workflows."""
import subprocess
import os
import pytest
from pathlib import Path


class TestNativeSimE2E:
    """End-to-end tests running full application on native_sim."""
    
    @pytest.mark.e2e
    @pytest.mark.slow
    def test_native_sim_boots(self, project_root):
        """Verify native_sim binary boots and produces expected output."""
        binary = project_root / "build_native_sim/zephyr-ztest-emul-button-hal/zephyr/zephyr.exe"
        
        if not binary.exists():
            pytest.skip(
                f"Binary not found: {binary}\n"
                "Build with: west build -b native_sim -s . --pristine"
            )
        
        # Run binary with timeout and immediate EOF
        try:
            result = subprocess.run(
                str(binary),
                capture_output=True,
                text=True,
                timeout=2,
                input=""  # Send EOF immediately
            )
            output = result.stdout
            stderr = result.stderr
        except subprocess.TimeoutExpired as e:
            # Get partial output before timeout (convert bytes to str if needed)
            output = e.stdout.decode() if isinstance(e.stdout, bytes) else (e.stdout or "")
            stderr = e.stderr.decode() if isinstance(e.stderr, bytes) else (e.stderr or "")
        
        # Verify boot messages appeared
        assert "Booting nRF Connect SDK" in output or "nRF Connect SDK" in output, \
            f"Boot message not found in output.\nStdout:\n{output}\nStderr:\n{stderr}"
        
        assert "Zephyr OS" in output or "zephyr" in output.lower(), \
            f"Zephyr OS message not found.\nStdout:\n{output}"


class TestZtestE2E:
    """End-to-end tests for Zephyr ztest execution."""
    
    @pytest.mark.e2e
    @pytest.mark.slow
    def test_twister_execution(self, project_root):
        """Verify twister can execute tests successfully."""
        # Try to run twister - if it fails, skip the test
        result = subprocess.run(
            "west twister -n -T tests/biz_logic --outdir build_tests_twister",
            cwd=project_root,
            shell=True,
            capture_output=True,
            text=True,
            timeout=60,
            env={**os.environ}  # Use current environment
        )
        
        # If return code is 127, command not found - skip
        if result.returncode == 127 or "command not found" in result.stderr.lower():
            pytest.skip("west command not found - requires NCS environment")
        
        # Otherwise, verify tests passed
        output = result.stdout + result.stderr
        assert result.returncode == 0, \
            f"Twister failed:\n{result.stdout}\n{result.stderr}"
        assert "passed" in output.lower(), \
            f"No passed tests found in output:\n{output}"
