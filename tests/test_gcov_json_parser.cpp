#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <vector>
#include "gcov_json_handler.hpp"
#include <rapidjson/reader.h>


struct test_gcov_json_parser : ::testing::TestWithParam<
                                 std::tuple<std::string, files_t>> {};


TEST_P(test_gcov_json_parser, test)
{
    const auto& [json, expected_files] = GetParam();
    files_t files;
    const auto err = parse_gcov_json(files, json, [] (auto) { return true;});
    ASSERT_EQ(err, "");
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

INSTANTIATE_TEST_SUITE_P(test_gcov_json_parser_inst,
                         test_gcov_json_parser,
                         ::testing::ValuesIn(test_data));
