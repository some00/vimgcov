cmake_minimum_required(VERSION 3.18)
project(vimgcov)

option(ENABLE_TESTS "Enable tests" ON)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost REQUIRED COMPONENTS headers filesystem)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RapidJSON REQUIRED IMPORTED_TARGET RapidJSON)

pybind11_add_module(_vimgcov
    src/vimgcov.cpp
    src/gcov_json_handler.cpp
)
target_link_libraries(_vimgcov PRIVATE
    Boost::headers
    Boost::filesystem
    PkgConfig::RapidJSON
)

if(ENABLE_TESTS)
    set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
    enable_testing()
    add_subdirectory(tests)
endif()
set_target_properties(_vimgcov PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)
