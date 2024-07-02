#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rapidjson/reader.h>
#include <memory>

#include "gcov_json_handler.hpp"

namespace py = pybind11;

files_t getcoverage(
    std::deque<std::string> gcnos,
    unsigned j,
    const std::string& path)
{
    struct per_proc_t
    {
        boost::process::async_pipe std_out_pipe;
        std::string std_out;
        std::unique_ptr<boost::process::child> child;
        boost::process::async_pipe std_err_pipe;
        std::string std_err;
        std::string gcno;
    };
    files_t rv;
    std::deque<per_proc_t> per_proc;
    boost::asio::io_context ctx;
    auto push = [&] {
        boost::process::async_pipe ap_err{ctx};
        boost::process::async_pipe ap_out{ctx};
        std::string buf;
        std::string err;
        auto c = std::make_unique<boost::process::child>(
            boost::process::search_path("gcov"),
            "--stdout",
            "--json-format",
            gcnos.back(),
            boost::process::std_out > ap_out,
            boost::process::std_err > ap_err,
            ctx
        );
        per_proc.emplace_back(per_proc_t{.std_out_pipe=std::move(ap_out),
                                         .std_out=std::move(buf),
                                         .child=std::move(c),
                                         .std_err_pipe=std::move(ap_err),
                                         .std_err=std::move(err),
                                         .gcno=std::move(gcnos.back()),
                                         });
        auto& pp = per_proc.back();
        boost::asio::async_read(pp.std_out_pipe,
                                boost::asio::dynamic_buffer(pp.std_out),
                                [] (auto, auto) {});
        boost::asio::async_read(pp.std_err_pipe,
                                boost::asio::dynamic_buffer(pp.std_err),
                                [] (auto, auto) {});
        gcnos.pop_back();
    };
    auto pop = [&] {
        auto it = per_proc.begin();
        const auto& [_, buf, child, __, err, gcno] = *it;
        child->wait();
        if (child->exit_code() != 0)
        {
            std::cerr << "-----------------------------------------------\n" <<
                "error in gcov process: " << child->exit_code() << "\n" <<
                err << std::endl;
            per_proc.erase(it);
            return;
        }
        try
        {
            parse_gcov_json(
                rv, buf, [&path] (const auto& x) { return x == path; }
            );
        }
        catch (const parse_exception& ex)
        {
            std::cerr << "error in gcov json file: " << ex.what() << std::endl;
        }
        per_proc.erase(it);
    };

    while (!gcnos.empty())
    {
        ctx.restart();
        while (per_proc.size() < j && !gcnos.empty())
            push();
        ctx.run();
        while (!per_proc.empty())
            pop();
    }
    return rv;
}


PYBIND11_MODULE(_vimgcov, m)
{
    m.def("getcoverage", getcoverage);
}
