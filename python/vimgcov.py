from pathlib import Path
import multiprocessing
import _vimgcov
import tempfile
import subprocess
import os
import json
from collections import defaultdict


DEPS_DIR = Path("./target/debug/deps")
# TODO make it configurable
LLVM_PROFDATA = "llvm-profdata"
LLVM_COV = "llvm-cov"


def debug(*args, **kwargs):
    with open("/tmp/vimgcov.log", "a") as f:
        print(*args, file=f, **kwargs)


def get_llvm_rust_coverage_lines_native(filename):
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


def convert_dict_to_arrays(cover_dict):
    covered_lines = []
    uncovered_lines = []
    for lineno, covered in cover_dict.items():
        if covered:
            covered_lines.append(lineno)
        else:
            uncovered_lines.append(lineno)
    return covered_lines, uncovered_lines


def llvm_cov(filename, file, cover_dict, profdata):
    if not file.is_file():
        return
    if not os.access(str(file), os.X_OK):
        return
    proc = subprocess.Popen([
        LLVM_COV, "export",
        "-instr-profile", profdata,
        "-format=text", str(file),
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    try:
        data_array = json.loads(stdout.decode())["data"]
    except json.JSONDecodeError:
        return
    llvm_cov_parse(data_array, filename, cover_dict)


def llvm_cov_parse(data_array, filename, cover_dict):
    for data in data_array:
        for file in data["files"]:
            if file["filename"] != str(filename):
                continue
            for segment in file["segments"]:
                if not segment[3] or not segment[4] or segment[5]:
                    continue
                cover_dict[segment[0]] |= bool(segment[2])
        for function in data["functions"]:
            if str(filename) not in function["filenames"]:
                continue
            for region in function["regions"]:
                for lineno in range(region[0], region[2]):
                    cover_dict[lineno] |= bool(region[4])


def get_llvm_rust_coverage_lines(filename):
    if not DEPS_DIR.is_dir():
        raise FileNotFoundError("No deps directory found")
    cover_dict = defaultdict(lambda: False)
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
        for file in DEPS_DIR.iterdir():
            llvm_cov(filename, file, cover_dict, profdata)
    return convert_dict_to_arrays(cover_dict)


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
        if False:
            return get_llvm_rust_coverage_lines(filename)
        else:
            return get_llvm_rust_coverage_lines_native(filename)
    else:
        return get_gcc_coverage_gcov_lines(filename)
