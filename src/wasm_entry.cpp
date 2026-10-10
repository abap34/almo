#include <emscripten.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "render.hpp"

namespace {

struct ParserState {
    almo::IncrementalParser parser;
    std::string response;
};

std::unordered_map<uint32_t, std::unique_ptr<ParserState>> parsers;
uint32_t next_handle = 1;

std::string json_string(const std::string& value) {
    // HTML contains many quotes. Emit short JSON escapes without a stream or
    // formatting allocation per quote; this sits on every preview update.
    constexpr char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(value.size() + 2);
    result += '"';
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 0x20) {
                    result += "\\u00";
                    result += hex[ch >> 4];
                    result += hex[ch & 15];
                } else {
                    result += static_cast<char>(ch);
                }
        }
    }
    result += '"';
    return result;
}

std::string range_json(almo::SourceRange range) {
    return "{\"begin\":" + std::to_string(range.begin) +
           ",\"end\":" + std::to_string(range.end) + "}";
}

std::string diagnostic_json(const almo::Diagnostic& diagnostic) {
    return "{\"severity\":" + json_string(
               diagnostic.severity == almo::DiagnosticSeverity::Warning ? "warning" : "error") +
           ",\"range\":" + range_json(diagnostic.range) +
           ",\"message\":" + json_string(diagnostic.message) + "}";
}

std::string error_json(const char* phase, const almo::Diagnostic& diagnostic) {
    return "{\"ok\":false,\"phase\":" + json_string(phase) +
           ",\"diagnostics\":[" + diagnostic_json(diagnostic) + "]}";
}

constexpr const char* invalid_handle =
    R"({"ok":false,"phase":"input","diagnostics":[{"severity":"error","range":{"begin":0,"end":0},"message":"Invalid parser handle"}]})";
constexpr const char* internal_error =
    R"({"ok":false,"phase":"internal","diagnostics":[{"severity":"error","range":{"begin":0,"end":0},"message":"Failed to update parser"}]})";

}  // namespace

extern "C" {

// A handle owns one parser and its response buffer. Handles are never recycled.
// Zero means creation failed. All functions are synchronous on the module thread.
EMSCRIPTEN_KEEPALIVE
uint32_t almo_incremental_parser_create() {
    try {
        if (next_handle == 0) return 0;
        const uint32_t handle = next_handle;
        parsers.emplace(handle, std::make_unique<ParserState>());
        ++next_handle;
        return handle;
    } catch (...) {
        return 0;
    }
}

// Input and diagnostic ranges use UTF-8 bytes. Copy the returned JSON before
// calling again with this handle or destroying it; the caller must not free it.
// Parse errors leave the last successful source/AST intact. A render error can
// occur after parsing has committed; a later update may still use the same handle.
EMSCRIPTEN_KEEPALIVE
const char* almo_incremental_parser_set_source(uint32_t handle, const char* utf8,
                                              std::size_t length) {
    const auto found = parsers.find(handle);
    if (found == parsers.end()) return invalid_handle;
    auto& state = *found->second;
    const char* phase = "parse";
    try {
        try {
            if (!utf8 && length != 0) {
                state.response = error_json("input", {almo::DiagnosticSeverity::Error, {},
                                                       "Missing source buffer"});
                return state.response.c_str();
            }
            const double parse_start = emscripten_get_now();
            const auto update = state.parser.set_source(std::string(utf8 ? utf8 : "", length));
            const double render_start = emscripten_get_now();
            phase = "render";
            auto ast = state.parser.clone_ast();
            almo::move_footnote_to_end(ast);
            const auto html = ast.to_html();
            const double render_end = emscripten_get_now();

            std::string response = "{\"ok\":true,\"html\":" + json_string(html) +
                                   ",\"diagnostics\":[";
            const char* separator = "";
            for (const auto& diagnostic : update.diagnostics) {
                response += separator + diagnostic_json(diagnostic);
                separator = ",";
            }
            response += "],\"reparsedRanges\":[";
            separator = "";
            for (const auto range : update.reparsed_ranges) {
                response += separator + range_json(range);
                separator = ",";
            }
            response += "],\"timings\":{\"parseMs\":" + std::to_string(render_start - parse_start) +
                        ",\"renderMs\":" + std::to_string(render_end - render_start) + "}}";
            state.response = std::move(response);
        } catch (const almo::ParseException& error) {
            state.response = error_json(phase, error.diagnostic());
        } catch (const std::exception& error) {
            state.response = error_json(phase, {almo::DiagnosticSeverity::Error, {}, error.what()});
        }
        return state.response.c_str();
    } catch (...) {
        // Even response construction failures must not unwind into JavaScript.
        return internal_error;
    }
}

EMSCRIPTEN_KEEPALIVE
void almo_incremental_parser_destroy(uint32_t handle) {
    parsers.erase(handle);
}

EMSCRIPTEN_KEEPALIVE
const char* almo_render(const char* md, int len) {
    static std::string result_buf;
    std::string input(md, len);

    auto result = almo::Parser{}.parse(input);
    auto& ast = result.ast;

    almo::move_footnote_to_end(ast);

    result_buf = ast.to_html();
    return result_buf.c_str();
}

}
