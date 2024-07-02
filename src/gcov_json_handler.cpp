#include "gcov_json_handler.hpp"
#include "rapidjson/document.h"

#ifdef spdlog_FOUND
#include <spdlog/spdlog.h>
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define TRACE(...) (void)0
#endif

using rapidjson::Document;

void parse_gcov_json(files_t& out,
                     const std::string& buf,
                     filename_selector_t filename_selector)
{
    Document d;
    if (d.Parse(buf.c_str()).HasParseError())
        throw parse_exception{"parse error"};
    if (!d.IsObject())
        throw parse_exception{"root isn't object"};
    if (!d.HasMember("files"))
        throw parse_exception{"no 'files' attribute"};
    if (!d["files"].IsArray())
        throw parse_exception{"'files' isn't array"};
    const auto files = d["files"].GetArray();
    for (const auto& file : files)
    {
        if (!file.IsObject())
            throw parse_exception{"file isn't object"};

        if (!file.HasMember("file"))
            throw parse_exception{"no 'file' attribute"};
        if (!file["file"].IsString())
            throw parse_exception{"'file' isn't string"};
        const auto filename = file["file"].GetString();
        if (!filename_selector(filename))
            continue;
        auto itf = out.find(filename);
        if (itf == out.end())
        {
            out.insert({filename, {}});
            itf = out.find(filename);
        }
        auto& lines_out = itf->second;

        if (!file.HasMember("lines"))
            throw parse_exception{"no lines attribute"};
        if (!file["lines"].IsArray())
            throw parse_exception{"'lines' isn't array"};
        const auto lines = file["lines"].GetArray();
        for (const auto& line : lines)
        {
            if (!line.IsObject())
                throw parse_exception{"line isn't object"};

            if (!line.HasMember("unexecuted_block"))
                throw parse_exception{"no unexecute_block attribute"};
            if (!line["unexecuted_block"].IsBool())
                throw parse_exception{"'unexecuted_block' isn't bool"};
            auto unexecute_block = line["unexecuted_block"].GetBool();

            if (!line.HasMember("line_number"))
                throw parse_exception{"no 'line_number' attribute"};
            if (!line["line_number"].IsUint())
                throw parse_exception{"'line_number' isn't uint"};
            const auto line_number = line["line_number"].GetUint();

            if (!line.HasMember("count"))
                throw parse_exception{"no 'count' attribute"};
            if (!line["count"].IsUint())
                throw parse_exception{"'count isn't uint"};
            const auto count = line["count"].GetUint();
            unexecute_block &= !count;

            const auto itl = std::lower_bound(
                    lines_out.begin(), lines_out.end(),
                    std::make_tuple(line_number, false)
            );
            if (itl != lines_out.end() && std::get<0>(*itl) == line_number)
                std::get<1>(*itl) &= unexecute_block;
            else
                lines_out.insert(itl, {line_number, unexecute_block});
        }
    }
}
