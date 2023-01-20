find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
add_executable(test_gcov_json_parser
    test_gcov_json_parser.cpp
    ${source_dir}/gcov_json_handler.cpp
)
target_link_libraries(test_gcov_json_parser PRIVATE
    GTest::gtest
    GTest::gtest_main
    spdlog::spdlog
)
target_compile_definitions(test_gcov_json_parser PRIVATE
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    spdlog_FOUND
)
target_include_directories(test_gcov_json_parser PRIVATE ${source_dir})
add_test(NAME test_gcov_json_parser COMMAND test_gcov_json_parser)
