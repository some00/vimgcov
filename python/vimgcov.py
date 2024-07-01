from pathlib import Path
import multiprocessing
import _vimgcov


def GetCoverageGcovLines(filename):
    """
    Retrieves the covered and uncovered lines for the given filename using
    gcov.

    Args:
        filename (str): The name of the file to check coverage for.

    Returns:
        tuple: Two lists containing the covered and uncovered line numbers.
    """
    # Ensure the filename exists
    if not Path(filename).is_file():
        raise FileNotFoundError(f"File {filename} not found.")

    # Search for all .gcno files in the current directory and subdirectories
    gcnos = list(map(str, Path('.').rglob("*.gcno")))

    # Get coverage information using _vimgcov module
    files = _vimgcov.getcoverage(gcnos, multiprocessing.cpu_count(), filename)

    # Ensure files contains the requested filename
    if filename not in files:
        raise KeyError(f"Coverage data for file {filename} not found.")

    covered = []
    uncovered = []
    try:
        for lineno, unexecuted_block in files[filename]:
            if unexecuted_block:
                uncovered.append(lineno)
            else:
                covered.append(lineno)
    except KeyError:
        # Handle the case where the filename is not found in the coverage
        # results
        pass

    return covered, uncovered
