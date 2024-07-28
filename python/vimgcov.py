from pathlib import Path
import multiprocessing
import _vimgcov
import tempfile
import subprocess
import os


DEPS_DIR = Path("./target/debug/deps")
# TODO make it configurable
LLVM_PROFDATA = "llvm-profdata"
LLVM_COV = "llvm-cov"


def debug(*args, **kwargs):
    with open("/tmp/vimgcov.log", "a") as f:
        print(*args, file=f, **kwargs)


def get_llvm_rust_coverage_lines(filename):
    if not DEPS_DIR.is_dir():
        raise FileNotFoundError("No deps directory found")
    with tempfile.TemporaryDirectory() as directory:
        profdata = os.path.join(directory, "a.profdata")
        proc = subprocess.Popen([
            LLVM_PROFDATA, "merge",
            "-o", profdata,
            *map(str, Path(".").rglob("*.profraw")),
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        if proc.returncode != 0:
            print(stdout.decode())
            print(stderr.decode())
            return

        def filter_file(file):
            return file.is_file() and os.access(str(file), os.X_OK)
        executables = list(map(str, filter(filter_file, DEPS_DIR.iterdir())))
        files = _vimgcov.getllvmcoverage(executables,
                                         multiprocessing.cpu_count(),
                                         filename, profdata)
    return process_return_value(filename, files)


def get_gcc_coverage_gcov_lines(filename):
    # Search for all .gcno files in the current directory and subdirectories
    gcnos = list(map(str, Path('.').rglob("*.gcno")))

    # Get coverage information using _vimgcov module
    files = _vimgcov.getcoverage(gcnos, multiprocessing.cpu_count(), filename)

    return process_return_value(filename, files)


def process_return_value(filename, files):
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

    if Path(filename).suffix == ".rs":
        return get_llvm_rust_coverage_lines(filename)
    else:
        return get_gcc_coverage_gcov_lines(filename)


if __name__ == "__main__":
    from pprint import pprint
    pprint(GetCoverageGcovLines("/home/tkonya/tmp/serde/"
                                "serde/src/ser/impls.rs"))
