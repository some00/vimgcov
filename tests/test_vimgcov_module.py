import pytest
import subprocess
import _vimgcov


@pytest.fixture
def cpp_code():
    return """
    #include <iostream>

    void foo() {
        std::cout << "foo" << std::endl;
    }

    void unused_function() {
        std::cout << "This function is not used" << std::endl;
    }

    int main() {
        foo();
        return 0;
    }
    """


def test_getcoverage(tmp_path, cpp_code):
    test_cpp_file = tmp_path / "test.cpp"
    test_binary = tmp_path / "test_binary"
    gcno_file = tmp_path / "test_binary-test.gcno"

    test_cpp_file.write_text(cpp_code)

    print("[DEBUG] Compiling test binary...")
    compile_result = subprocess.run([
        "g++", "--coverage", str(test_cpp_file), "-o", str(test_binary)
    ], capture_output=True, text=True)
    print("[DEBUG] Compile stdout:", compile_result.stdout)
    print("[DEBUG] Compile stderr:", compile_result.stderr)

    if compile_result.returncode != 0:
        pytest.fail("Compilation failed")

    print("[DEBUG] Checking file existence...")
    print("Binary exists:", test_binary.exists())
    print("GCNO file exists:", gcno_file.exists())

    print("[DEBUG] Running test binary...")
    try:
        subprocess.run(["chmod", "+x", str(test_binary)], check=True)
        run_result = subprocess.run([str(test_binary)], capture_output=True,
                                    text=True)
        print("[DEBUG] Run stdout:", run_result.stdout)
        print("[DEBUG] Run stderr:", run_result.stderr)
    except Exception as e:
        print("[DEBUG] Exception while running binary:", str(e))

    print("[DEBUG] Checking coverage files...")
    print("GCDA file exists:", any(
        f.suffix == ".gcda" for f in tmp_path.iterdir()))

    print("[DEBUG] Running getcoverage...")
    try:
        coverage_data = _vimgcov.getcoverage(
            gcnos=[str(gcno_file)],
            j=1,
            path=str(test_cpp_file)
        )
        print("[DEBUG] Coverage data:", coverage_data)
    except Exception as e:
        print("[DEBUG] Exception in getcoverage:", str(e))

    expected_coverage = {
        str(test_cpp_file): [
            (4, False), (5, False), (6, False),
            (8, True), (9, True), (10, True),
            (12, False), (13, False), (14, False)
        ]
    }

    assert coverage_data == expected_coverage
