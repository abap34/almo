#pragma once

#include <map>
#include <string>
#include <vector>

#include "../../build/ast.hpp"
#include "../../build/parse.hpp"
#include "../../build/reader.hpp"
#include "../../build/render.hpp"
#include "framework.hpp"

namespace almo_test {

namespace fixtures {

inline constexpr char kExampleInputPath[] = "example/input.py";
inline constexpr char kJudgeSampleInputPath[] =
    "example/helloalmo/in/sample.txt";
inline constexpr char kJudgeSampleOutputPath[] =
    "example/helloalmo/out/sample.txt";
inline constexpr char kJudgeInputGlob[] = "example/helloalmo/in/*.txt";
inline constexpr char kJudgeOutputGlob[] = "example/helloalmo/out/*.txt";
inline constexpr char kFixtureRoot[] = "tests/fixtures";

}  // namespace fixtures

inline std::map<std::string, std::string> default_meta() {
    return {
        {"template_file", "__default__"},
        {"theme", "light"},
        {"css_setting", "light"},
        {"editor_theme", "ace/theme/chrome"},
        {"syntax_theme", "github.min"},
        {"title", "ALMO Test"},
        {"description", "Test document"},
        {"author", "test"},
        {"twitter_id", ""},
        {"github_id", ""},
        {"mail", ""},
        {"ogp_url", "https://example.com/ogp.png"},
        {"tag", "test"},
        {"url", "https://example.com"},
        {"site_name", "ALMO"},
        {"twitter_site", ""},
    };
}

inline std::vector<std::string> markdown_lines(const std::string& text) {
    return split(text, "\n");
}

inline almo::Markdown parse_markdown(const std::vector<std::string>& lines) {
    almo::Markdown ast;
    almo::MarkdownParser parser(lines);
    parser.process(ast);
    return ast;
}

inline almo::Markdown parse_markdown(const std::string& text) {
    return parse_markdown(markdown_lines(text));
}

inline almo::ParseSummary summarize_markdown(
    const std::vector<std::string>& lines) {
    auto meta = default_meta();
    return almo::md_to_summary(lines, meta);
}

inline almo::ParseSummary summarize_markdown(const std::string& text) {
    return summarize_markdown(markdown_lines(text));
}

inline std::string fixture_path(const std::string& relative_path) {
    return std::string(fixtures::kFixtureRoot) + "/" + relative_path;
}

inline std::vector<std::string> load_fixture_lines(
    const std::string& relative_path) {
    return read_file(fixture_path(relative_path));
}

inline std::string load_fixture_text(const std::string& relative_path) {
    return join(load_fixture_lines(relative_path), "\n");
}

}  // namespace almo_test
