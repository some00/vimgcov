cmake_minimum_required(VERSION 3.18)
project(foo)

option(ENABLE_TESTS "Enable tests" ON)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost REQUIRED COMPONENTS headers filesystem)
find_package(pybind11 REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RapidJSON REQUIRED IMPORTED_TARGET RapidJSON)

pybind11_add_module(_foo
    src/vimgcov.cpp
    src/gcov_json_handler.cpp
)
target_link_libraries(_foo PRIVATE
    Boost::headers
    Boost::filesystem
    PkgConfig::RapidJSON
    fmt::fmt
)

if(ENABLE_TESTS)
    set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
    enable_testing()
    add_subdirectory(tests)
endif()


find_package(spdlog REQUIRED)
target_compile_definitions(_foo PRIVATE
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    spdlog_FOUND
)
target_link_libraries(_foo PRIVATE
    spdlog::spdlog
)
