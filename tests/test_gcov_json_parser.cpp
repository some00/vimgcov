#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <vector>
#include "gcov_json_handler.hpp"
#include <rapidjson/reader.h>

struct ParseGcovJsonTestFixture : ::testing::TestWithParam<
                                   std::tuple<std::string, files_t>> {};

#define EXPECT_THROW_WITH_MSG(x, msg) \
    do \
    { \
        try \
        { \
            x; \
            FAIL() << "expression didn't throw"; \
        } \
        catch (const std::exception& ex) { \
            EXPECT_STREQ(ex.what(), msg); \
        } \
    } while(false);

TEST_P(ParseGcovJsonTestFixture, Test)
{
    const auto& [json, expected_files] = GetParam();
    files_t files;
    parse_gcov_json(files, json, [] (auto) { return true;});
    EXPECT_EQ(expected_files, files);
}

const std::vector<std::tuple<std::string, files_t>> test_data{
    {R"(
{
    "gcc_version": "12.2.0",
    "files": [
        {
            "lines": [
                {
                    "branches": [],
                    "count": 102,
                    "line_number": 67,
                    "unexecuted_block": false,
                    "function_name": "lv_chart_create"
                }
            ],
            "file": "lv_area.h"
        }
    ]
}
)", { {"lv_area.h", {{67, false}}} } },
    {R"(
{
    "gcc_version": "12.2.0",
    "files": [
        {
            "lines": [
                {
                    "branches": [],
                    "count": 0,
                    "line_number": 67,
                    "unexecuted_block": false,
                    "function_name": "lv_chart_create"
                },
                {
                    "branches": [],
                    "count": 102,
                    "line_number": 69,
                    "unexecuted_block": true,
                    "function_name": "lv_chart_create"
                }
            ],
            "file": "lv_area.h"
        }
    ]
}
)", { {"lv_area.h", {{67, false}, {69, false}}} } },
    {R"(
{
    "gcc_version": "12.2.0",
    "files": [
        {
            "file": "lv_area.h",
            "lines": [
                {
                    "branches": [],
                    "count": 102,
                    "unexecuted_block": false,
                    "line_number": 67,
                    "function_name": "lv_chart_create"
                }
            ]
        }
    ]
}
)", { {"lv_area.h", {{67, false}}} } },
    {R"(
{
    "gcc_version": "12.2.0",
    "files": [
        {
            "file": "lv_area.h",
            "lines": [
                {
                    "branches": [],
                    "count": 102,
                    "unexecuted_block": false,
                    "line_number": 67,
                    "function_name": "lv_chart_create"
                }
            ]
        }
    ],
    "format_version": "1"
}
)", { {"lv_area.h", {{67, false}}} } },
    {R"({
    "gcc_version": "12.2.0",
    "files": [
        {
            "lines": [
                {
                    "branches": [],
                    "count": 160,
                    "line_number": 24,
                    "unexecuted_block": false,
                    "function_name": "lv_test_init"
                }
            ],
            "functions": [
                {
                    "blocks": 3,
                    "end_column": 1,
                    "start_line": 24,
                    "name": "lv_test_init",
                    "blocks_executed": 3,
                    "execution_count": 160,
                    "demangled_name": "lv_test_init",
                    "start_column": 6,
                    "end_line": 28
                }
            ],
            "file": "lv_test_init.c"
        }
    ],
    "format_version": "1",
    "current_working_directory": "build_test_sysheap",
    "data_file": "lv_test_init.c.gcno"
}
)", { {"lv_test_init.c", {{24, false}}} } },
};

INSTANTIATE_TEST_SUITE_P(ParseGcovJsonTestFixtureInst,
                         ParseGcovJsonTestFixture,
                         ::testing::ValuesIn(test_data));

// Helper function to create a filename selector
bool filename_selector(const std::string& filename)
{
    return filename == "testfile.c";
}

// Test when JSON input is invalid
TEST(ParseGcovJsonTest, InvalidJson)
{
    files_t out;
    const std::string invalid_json = "{ invalid json }";
    EXPECT_THROW_WITH_MSG(
        parse_gcov_json(out, invalid_json, filename_selector),
        "JSON parse error: Missing a name for object member. (offset 2)"
    );
}

// Test when root is not an object
TEST(ParseGcovJsonTest, RootNotObject)
{
    files_t out;
    const std::string json = "[]";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "JSON root is not an object");
}

// Test when 'files' attribute is missing
TEST(ParseGcovJsonTest, NoFilesAttribute)
{
    files_t out;
    const std::string json = "{}";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "JSON does not contain a valid 'files' array");
}

// Test when 'files' isn't an array
TEST(ParseGcovJsonTest, FilesNotArray)
{
    files_t out;
    const std::string json = "{\"files\": {}}";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "JSON does not contain a valid 'files' array");
}

// Test valid JSON input with no coverage data
TEST(ParseGcovJsonTest, ValidJsonNoCoverageData)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": []
        }]
    })";
    parse_gcov_json(out, json, filename_selector);
    EXPECT_EQ(out["testfile.c"].size(), 0);
}

// Test valid JSON input with coverage data
TEST(ParseGcovJsonTest, ValidJsonWithCoverageData)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": false,
                "count": 1
            }, {
                "line_number": 2,
                "unexecuted_block": true,
                "count": 0
            }]
        }]
    })";
    parse_gcov_json(out, json, filename_selector);
    EXPECT_EQ(out["testfile.c"].size(), 2);
    EXPECT_EQ(std::get<0>(out["testfile.c"][0]), 1);
    EXPECT_EQ(std::get<1>(out["testfile.c"][0]), false);
    EXPECT_EQ(std::get<0>(out["testfile.c"][1]), 2);
    EXPECT_EQ(std::get<1>(out["testfile.c"][1]), true);
}

// Test when 'file' attribute is missing
TEST(ParseGcovJsonTest, NoFileAttribute)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "lines": []
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "File entry missing 'file' string attribute");
}

// Test when 'file' isn't a string
TEST(ParseGcovJsonTest, FileNotString)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": 123,
            "lines": []
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "File entry missing 'file' string attribute");
}

// Test when 'lines' attribute is missing
TEST(ParseGcovJsonTest, NoLinesAttribute)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c"
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "File entry for 'testfile.c' missing 'lines' array");
}

// Test when 'lines' isn't an array
TEST(ParseGcovJsonTest, LinesNotArray)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": {}
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "File entry for 'testfile.c' missing 'lines' array");
}

// Test when 'line_number' attribute is missing
TEST(ParseGcovJsonTest, NoLineNumberAttribute)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "unexecuted_block": true,
                "count": 0
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(
        parse_gcov_json(out, json, filename_selector),
        "Line entry in file 'testfile.c' missing 'line_number' "
        "integer attribute"
    );
}

// Test when 'line_number' isn't an uint
TEST(ParseGcovJsonTest, LineNumberNotUint)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": "one",
                "unexecuted_block": true,
                "count": 0
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(
        parse_gcov_json(out, json, filename_selector),
        "Line entry in file 'testfile.c' missing"
        " 'line_number' integer attribute"
    );
}

/* TODO check where did it go
// Test when 'unexecuted_block' attribute is missing
TEST(ParseGcovJsonTest, NoUnexecutedBlockAttribute)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "count": 0
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                                          "no unexecute_block attribute");
}
*/

/* TODO check where did it go
// Test when 'unexecuted_block' isn't a bool
TEST(ParseGcovJsonTest, UnexecutedBlockNotBool)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": "true",
                "count": 0
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "'unexecuted_block' isn't bool");
}
*/

// Test when 'count' attribute is missing
TEST(ParseGcovJsonTest, NoCountAttribute)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": true
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(
        parse_gcov_json(out, json, filename_selector),
        "Line entry in file 'testfile.c' missing 'count' integer attribute"
    );
}

// Test when 'count' isn't an uint
TEST(ParseGcovJsonTest, CountNotUint)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": true,
                "count": "zero"
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(
        parse_gcov_json(out, json, filename_selector),
        "Line entry in file 'testfile.c' missing 'count' integer attribute"
    );
}

// Test when 'file' isn't an object
TEST(ParseGcovJsonTest, FileIsNotAnObject)
{
    files_t out;
    const std::string json = R"({
        "files": [123]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "File entry isn't an object");
}

// Test when filename selector skips a file
TEST(ParseGcovJsonTest, FilenameSelectorSkips)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c"
        }]
    })";
    parse_gcov_json(out, json, [] (const auto f) { return false; });
    EXPECT_EQ(out.size(), 0);
}

// Test when 'line' isn't an object
TEST(ParseGcovJsonTest, LineIsNotAnObject)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [123]
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "Line entry isn't an object");
}

// Test when 'unexecuted_block' is set but 'count' is not zero
TEST(ParseGcovJsonTest, UnexecutedBlockWithNonZeroCount)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": true,
                "count": 1
            }, {
                "line_number": 1,
                "unexecuted_block": true,
                "count": 1
            }]
        }]
    })";
    parse_gcov_json(out, json, filename_selector);
    EXPECT_EQ(out["testfile.c"].size(), 1);
    EXPECT_EQ(std::get<0>(out["testfile.c"][0]), 1);
    EXPECT_EQ(std::get<1>(out["testfile.c"][0]), false);
}

TEST(ParseGcovJsonTest, UnexecutedBlockIsNotBool)
{
    files_t out;
    const std::string json = R"({
        "files": [{
            "file": "testfile.c",
            "lines": [{
                "line_number": 1,
                "unexecuted_block": 123,
                "count": 1
            }]
        }]
    })";
    EXPECT_THROW_WITH_MSG(parse_gcov_json(out, json, filename_selector),
                          "Unexecuted block isn't boolean");
}
