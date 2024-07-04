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


# TODO investigate
@pytest.mark.skip(reason="Github Actions throws Bad file descrtiptor")
def test_getcoverage(tmp_path, cpp_code):
    # Create paths for test files
    test_cpp_file = tmp_path / "test.cpp"
    test_binary = tmp_path / "test_binary"

    # Write the test C++ code to the temporary file
    test_cpp_file.write_text(cpp_code)

    # Compile the C++ file with coverage enabled
    subprocess.run([
        "g++", "--coverage", str(test_cpp_file), "-o", str(test_binary)
    ], check=True)

    # Run the binary to generate coverage data
    subprocess.run([str(test_binary)], check=True)

    # Call the getcoverage function
    coverage_data = _vimgcov.getcoverage(
        gcnos=[str(tmp_path / "test_binary-test.gcno")],
        j=1,
        path=str(test_cpp_file)
    )

    # Define the expected coverage data
    # First element of each tuple is the line number and the second is wether
    # it is unexecuted.
    expected_coverage = {
        str(test_cpp_file): [
            (4, False),   # foo function (executed)
            (5, False),   # foo function body
            (6, False),   # End of foo function
            (8, True),    # unused_function (not used)
            (9, True),    # unused_function body
            (10, True),   # End of unused_function
            (12, False),  # main function
            (13, False),  # call of foo function
            (14, False)   # return statement
        ]
    }

    # Check the output against the expected coverage
    assert coverage_data == expected_coverage
