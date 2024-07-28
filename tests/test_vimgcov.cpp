#include "vimgcov.hpp"
#include <gtest/gtest.h>
#include <fmt/core.h>

TEST(test_vimcov, process_files)
{
    const auto rv = process_files(
        [] (const auto& file, auto& ap_err, auto& ap_out, auto& ctx) {
            return std::make_unique<boost::process::child>(
                PYTHON_EXECUTABLE, "-c", fmt::format("print({}, end='')", file),
                boost::process::std_out > ap_out,
                boost::process::std_err > ap_err,
                ctx
            );
        },
        [] (auto& files, const auto& buf) {
            files[buf] = lines_t{ std::stoul(buf) };
        },
        {"1", "2", "3"},
        1
    );
    const auto& expected = files_t {
        {"1", { {0, false} }},
        {"2", { {0, false}, {0, false} }},
        {"3", { {0, false}, {0, false}, {0, false} }}
    };
    EXPECT_EQ(rv, expected);
}
