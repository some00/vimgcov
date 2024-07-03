import pytest
from unittest.mock import patch
from vimgcov import GetCoverageGcovLines


@pytest.fixture
def mock_getcoverage():
    with patch("_vimgcov.getcoverage") as mock:
        yield mock


@pytest.fixture
def temp_file(tmp_path):
    temp_files = []

    def temp_file_factory(name):
        temp_file_path = tmp_path / name
        temp_file_path.touch()
        temp_files.append(temp_file_path)
        return temp_file_path

    try:
        yield temp_file_factory
    finally:
        for temp_file in temp_files:
            temp_file.unlink()


def test_get_coverage_gcov_lines_file_not_found():
    """
    Test to check if the function raises a FileNotFoundError
    when the specified file does not exist.
    """
    with pytest.raises(FileNotFoundError):
        GetCoverageGcovLines("nonexistentfile.c")


def test_get_coverage_gcov_lines_no_coverage_data(temp_file, mock_getcoverage):
    """
    Test to check if the function raises a KeyError when there is no
    coverage data available for the specified file.
    """
    mock_getcoverage.return_value = {}
    with pytest.raises(KeyError):
        GetCoverageGcovLines(str(temp_file("nosuchfile.c")))


def test_get_coverage_gcov_lines_valid_file(temp_file, mock_getcoverage):
    """
    Test to verify that the function correctly returns the covered and
    uncovered lines when valid coverage data is available for the specified
    file.
    """
    temp_file = str(temp_file("testfile.c"))
    mock_getcoverage.return_value = {
        temp_file: [(1, False), (2, True), (3, False), (4, True)]
    }
    covered, uncovered = GetCoverageGcovLines(temp_file)
    assert covered == [1, 3]
    assert uncovered == [2, 4]
