#include "gcov_json_handler.hpp"
#include "rapidjson/document.h"

#ifdef spdlog_FOUND
#include <spdlog/spdlog.h>
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define TRACE(...) (void)0
#endif

using rapidjson::Document;

std::string parse_gcov_json(files_t& out,
                            const std::string& buf,
                            filename_selector_t filename_selector)
{
    Document d;
    if (d.Parse(buf.c_str()).HasParseError())
        return "parse error";
    if (!d.IsObject())
        return "root isn't object";
    if (!d.HasMember("files"))
        return "no 'files' attribute";
    if (!d["files"].IsArray())
        return "'files' isn't array";
    const auto files = d["files"].GetArray();
    for (const auto& file : files)
    {
        if (!file.IsObject())
            return "file isn't object";

        if (!file.HasMember("file"))
            return "no 'file' attribute";
        if (!file["file"].IsString())
            return "'file' isn't string";
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
        auto itl = lines_out.begin();

        if (!file.HasMember("lines"))
            return "no lines attribute";
        if (!file["lines"].IsArray())
            return "'lines' isn't array";
        const auto lines = file["lines"].GetArray();
        for (const auto& line : lines)
        {
            if (!line.IsObject())
                return "line isn't object";

            if (!line.HasMember("unexecuted_block"))
                return "no unexecute_block attribute";
            if (!line["unexecuted_block"].IsBool())
                return "'unexecuted_block' isn't bool";
            auto unexecute_block = line["unexecuted_block"].GetBool();

            if (!line.HasMember("line_number"))
                return "no 'line_number' attribute";
            if (!line["line_number"].IsUint())
                return "'line_number' isn't uint";
            const auto line_number = line["line_number"].GetUint();

            if (!line.HasMember("count"))
                return "no 'count' attribute";
            if (!line["count"].IsUint())
                return "'count isn't uint";
            const auto count = line["count"].GetUint();
            unexecute_block &= !count;

            const auto itl = std::lower_bound(
                    lines_out.begin(), lines_out.end(),
                    line_number,
                    [] (const auto& t, const auto line_number) {
                        return std::get<0>(t) < line_number; }
            );
            if (itl != lines_out.end() && std::get<0>(*itl) == line_number)
                std::get<1>(*itl) &= unexecute_block;
            else
                lines_out.insert(itl, {line_number, unexecute_block});
        }
    }
    return {};
}
