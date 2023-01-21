from pathlib import Path
import multiprocessing
import _vimgcov


def GetCoverageGcovLines(filename):
    gcnos = list(map(str, Path('.').rglob("*.gcno")))
    files = _vimgcov.getcoverage(gcnos, multiprocessing.cpu_count(), filename)
    covered = []
    uncovered = []
    try:
        for lineno, unexecuted_block in files[filename]:
            if unexecuted_block:
                uncovered.append(lineno)
            else:
                covered.append(lineno)
    except KeyError:
        pass
    return (covered, uncovered)
