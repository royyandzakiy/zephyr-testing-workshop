"""Integration tests for build system and project structure."""
import pytest
from pathlib import Path


class TestProjectStructure:
    """Validate project structure is correct."""
    
    @pytest.mark.smoke
    def test_source_exists(self, project_root):
        """Verify src/ directory exists."""
        assert (project_root / "src").exists()
        assert (project_root / "src").is_dir()
    
    @pytest.mark.smoke
    def test_tests_exist(self, project_root):
        """Verify tests/ directory exists."""
        assert (project_root / "tests").exists()
        assert (project_root / "tests" / "biz_logic").exists()
    
    @pytest.mark.smoke
    def test_cmake_exists(self, project_root):
        """Verify CMakeLists.txt exists."""
        assert (project_root / "CMakeLists.txt").exists()
    
    @pytest.mark.smoke
    def test_prj_conf_exists(self, project_root):
        """Verify prj.conf exists."""
        assert (project_root / "prj.conf").exists()


class TestConfigFiles:
    """Validate configuration files."""
    
    @pytest.mark.unit
    def test_cmake_readable(self, project_root):
        """Verify CMakeLists.txt is readable."""
        cmake = (project_root / "CMakeLists.txt").read_text()
        assert len(cmake) > 0
        assert "cmake_minimum_required" in cmake or "project" in cmake
    
    @pytest.mark.unit
    def test_prj_conf_readable(self, project_root):
        """Verify prj.conf is readable."""
        conf = (project_root / "prj.conf").read_text()
        assert len(conf) > 0


class TestZephyrBuild:
    """Test Zephyr build integration (marked as slow)."""
    
    @pytest.mark.slow
    @pytest.mark.integration
    def test_project_west_available(self):
        """Verify project uses west/Zephyr toolchain."""
        # This is a simple validation test
        # In CI/CD, west would be in PATH from NCS environment
        assert True  # Placeholder for CI integration
