#pragma once
#include <map>
#include <vector>
#include <string>
#include <functional>

using lines_t = std::vector<std::tuple<unsigned/*lineno*/,
                                       bool/*unexecuted*/>>;
using files_t = std::map<std::string /*path*/, lines_t>;
using filename_selector_t = std::function<bool(const std::string&)>;

std::string parse_gcov_json(files_t& out,
                            const std::string& buf,
                            filename_selector_t filename_selector);
