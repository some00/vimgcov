#include "gcov_json_handler.hpp"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#ifdef spdlog_FOUND
#include <spdlog/spdlog.h>
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#else
#define TRACE(...) (void)0
#endif

namespace {
rapidjson::Document parse_json(const std::string& buf)
{
    rapidjson::Document doc;
    doc.Parse(buf.c_str());

    if (doc.HasParseError())
        throw parse_exception{
            "JSON parse error: " + std::string(rapidjson::GetParseError_En(
            doc.GetParseError())) + " (offset " + std::to_string(
            doc.GetErrorOffset()) + ")"};
    return doc;
}

void add_line(lines_t& lines_out, unsigned line_number, bool unexecute_block)
{
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

void parse_gcov_json(files_t& out,
                     const std::string& buf,
                     filename_selector_t filename_selector)
{
    auto doc = parse_json(buf);

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

        auto [itf, _] = out.insert({filename, {}});
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
            add_line(lines_out, line_number, unexecute_block);
        }
    }
}

void parse_llvm_json(files_t& out,
                     const std::string& buf,
                     filename_selector_t filename_selector)
{
    auto doc = parse_json(buf);

    if (!doc.IsObject())
        throw parse_exception{"JSON root is not an object"};

    if (!doc.HasMember("data") || !doc["data"].IsObject())
        throw parse_exception{"JSON does not contain a valid 'data' object"};
    const auto& data = doc["data"];

    if (!data.HasMember("files") || !data["files"].IsArray())
        throw parse_exception{"JSON does not contain a valid 'files' array"};
    const auto& files = data["files"].GetArray();

    for (const auto& file : files)
    {
        if (!file.HasMember("filename") || !file["filename"].IsString())
            throw parse_exception{"File object without 'filename' attribute"};
        const auto& filename = file["filename"].GetString();

        if (filename_selector && !filename_selector(filename))
        {
            TRACE("Skipping file: {}", filename);
            continue;
        }

        auto [itf, _] = out.insert({filename, {}});
        auto& lines_out = itf->second;

        if (!file.HasMember("segments") || !file["segments"].IsArray())
            throw parse_exception{"File object without 'segments' array"};

        const auto& segments = file["segments"].GetArray();
        for (const auto& segment_json : segments)
        {
            if (!segment_json.IsArray())
                throw parse_exception{"Segment isn't an array"};
            const auto& segment = segment_json.GetArray();
            enum SegmentIndices
            {
                LINE, COL, COUNT, HAS_COUNT, IS_REGION_ENTRY, IS_GAP_REGION
            };
            if (segment.Size() < 6 ||
                !segment[LINE].IsUint() ||
                !segment[COUNT].IsUint() ||
                !segment[HAS_COUNT].IsBool() ||
                !segment[IS_REGION_ENTRY].IsBool() ||
                !segment[IS_GAP_REGION].IsBool())
                throw parse_exception{"Invalid segment array"};
            if (!segment[HAS_COUNT].GetBool() ||
                !segment[IS_REGION_ENTRY].GetBool() ||
                segment[IS_GAP_REGION].GetBool())
                continue;
            add_line(lines_out, segment[LINE].GetUint(),
                     !segment[COUNT].GetUint()
            );
        }

        if (!file.HasMember("functions") || !file["functions"].IsArray())
            throw parse_exception{"File object without 'functions' array"};
        const auto& functions = file["functions"].GetArray();
        for (const auto& function : functions)
        {
            if (!function.HasMember("filename") ||
                function["filename"].IsString())
                throw parse_exception{
                    "Function object without 'filename' string"};
            if (filename_selector &&
                !filename_selector(function["filename"].GetString()))
                continue;
            if (!function.HasMember("regions") ||
                !function["regions"].IsArray())
                throw parse_exception{
                    "Function object without 'regions' array"};
            const auto& regions = function["regions"].GetArray();
            for (const auto& region_json : regions)
            {
                enum RegionIndices
                {
                    LINE_START, COLUMN_START, LINE_END, COLUMN_END,
                    EXECUTION_COUNT, FILE_ID, EXPANDED_FILE_ID,
                };

                if (!region_json.IsArray())
                    throw parse_exception{"Region is not an array"};
                const auto& region = region_json.GetArray();
                if (region.Size() < 7 ||
                    !region[LINE_START].IsUint() ||
                    !region[EXECUTION_COUNT].IsUint())
                    throw parse_exception{"Invalid region array"};
                add_line(lines_out,
                        region[LINE_START].GetUint(),
                        !region[EXECUTION_COUNT].GetUint()
                );
            }
        }
    }
}
