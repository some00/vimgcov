#include <boost/process.hpp>
#include <functional>
#include "gcov_json_handler.hpp"

files_t process_files(
    std::function<std::unique_ptr<boost::process::child>(
        const std::string&,
        boost::process::async_pipe&,
        boost::process::async_pipe&,
        boost::asio::io_context&
    )> start_process,
    std::function<void(files_t&, const std::string&)> parse_json,
    std::deque<std::string> files,
    unsigned j
);
