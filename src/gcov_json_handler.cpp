#include "gcov_json_handler.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#ifdef spdlog_FOUND
#include <spdlog/spdlog.h>
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define TRACE(...) (void)0
#endif


void parse_gcov_json(files_t& out,
                     const std::string& buf,
                     filename_selector_t filename_selector)
{
    rapidjson::Document doc;
    doc.Parse(buf.c_str());

    if (doc.HasParseError())
        throw parse_exception{
            "JSON parse error: " + std::string(rapidjson::GetParseError_En(
            doc.GetParseError())) + " (offset " + std::to_string(
            doc.GetErrorOffset()) + ")"};

    if (!doc.IsObject())
        throw parse_exception{"JSON root is not an object"};

    if (!doc.HasMember("files") || !doc["files"].IsArray())
        throw parse_exception{"JSON does not contain a valid 'files' array"};

    const auto& files = doc["files"];
    for (const auto& file : files.GetArray())
    {
        if (!file.IsObject())
            throw parse_exception{"File entry isn't an object"};
        if (!file.HasMember("file") || !file["file"].IsString())
            throw parse_exception{
                "File entry missing 'file' string attribute"};

        const auto &filename = file["file"].GetString();
        if (filename_selector && !filename_selector(filename))
        {
            TRACE("Skipping file: {}", filename);
            continue;
        }

        auto itf = out.find(filename);
        if (itf == out.end())
        {
            out.insert({filename, {}});
            itf = out.find(filename);
        }
        auto& lines_out = itf->second;

        if (!file.HasMember("lines") || !file["lines"].IsArray())
            throw parse_exception{
                "File entry for '" + std::string(filename) +
                "' missing 'lines' array"};

        const auto &lines = file["lines"];
        for (const auto &line : lines.GetArray())
        {
            if (!line.IsObject())
                throw parse_exception{"Line entry isn't an object"};
            if (!line.HasMember("line_number") || !line["line_number"].IsInt())
                throw parse_exception{
                    "Line entry in file '" + std::string(filename) +
                    "' missing 'line_number' integer attribute"};
            const auto line_number = line["line_number"].GetUint();

            if (!line.HasMember("count") || !line["count"].IsInt())
                throw parse_exception{
                    "Line entry in file '" + std::string(filename) +
                    "' missing 'count' integer attribute"};
            if (!line.HasMember("unexecuted_block") ||
                !line["unexecuted_block"].IsBool())
                throw parse_exception{"Unexecuted block isn't boolean"};
            auto unexecute_block = line["unexecuted_block"].GetBool();


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
