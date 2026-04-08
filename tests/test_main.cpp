#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "../build/ast.hpp"
#include "../build/parse.hpp"
#include "../build/reader.hpp"
#include "../build/render.hpp"

namespace {

void expect_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <class T>
void expect_eq(const T& actual, const T& expected, const std::string& message) {
    if (!(actual == expected)) {
        throw std::runtime_error(message);
    }
}

void expect_contains(const std::string& text, const std::string& needle,
                     const std::string& message) {
    expect_true(text.find(needle) != std::string::npos, message);
}

std::map<std::string, std::string> default_meta() {
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

std::vector<std::string> markdown_lines(const std::string& text) {
    return split(text, "\n");
}

void test_reader_navigation() {
    almo::Reader reader({"Hello", "World"});

    expect_eq(reader.get_row(), std::string("Hello"),
              "Reader should start on the first row");
    expect_true(reader.is_line_begin(),
                "Reader should begin at the start of the line");

    reader.move_next_char(3);
    expect_eq(reader.col, 3, "Reader should move within a line");
    expect_true(!reader.is_line_begin(),
                "Reader should no longer be at line begin after moving");

    reader.move_next_line();
    expect_eq(reader.get_row(), std::string("World"),
              "Reader should advance to the next row");

    reader.move_next_line();
    expect_true(reader.is_eof(), "Reader should report EOF after the last row");
}

void test_utils() {
    expect_eq(join({"a", "b", "c"}, ","), std::string("a,b,c"),
              "join should concatenate strings with a separator");

    const auto parts = split("alpha::beta::gamma", "::");
    expect_eq(parts.size(), static_cast<std::size_t>(3),
              "split should return all segments");
    expect_eq(parts[1], std::string("beta"),
              "split should preserve middle segments");

    expect_eq(ltrim("  left"), std::string("left"),
              "ltrim should remove leading whitespace");
    expect_eq(rtrim("right  \n"), std::string("right"),
              "rtrim should remove trailing whitespace");
    expect_eq(remove_listdef("- item"), std::string("item"),
              "remove_listdef should remove list markers");

    const std::string stripped =
        _remove_comment("visible\n<!-- hidden\nstill hidden -->\nafter");
    expect_true(stripped.find("hidden") == std::string::npos,
                "comment removal should drop comment contents");
    expect_contains(stripped, "visible",
                    "comment removal should preserve visible text");
    expect_contains(stripped, "after",
                    "comment removal should preserve following text");

    expect_eq(escape("<tag>", EscapeFormat::HTML),
              std::string("&lt;tag&gt;"),
              "HTML escaping should encode angle brackets");
    expect_contains(escape("\"", EscapeFormat::JSON), "\\u0022",
                    "JSON escaping should escape quotes");
}

void test_parser_outputs() {
    const std::vector<std::string> lines = {
        "# Title",
        "",
        "Paragraph with **bold** and `code`.",
        "",
        "```cpp",
        "int main() { return 0; }",
        "```",
    };

    almo::Markdown ast;
    almo::MarkdownParser parser(lines);
    parser.process(ast);

    expect_eq(ast.nodes_byclass("Header").size(), static_cast<std::size_t>(1),
              "Parser should create one header node");
    expect_eq(ast.nodes_byclass("CodeBlock").size(), static_cast<std::size_t>(1),
              "Parser should create one code block");
    expect_eq(ast.nodes_byclass("InlineStrong").size(),
              static_cast<std::size_t>(1),
              "Parser should detect inline strong syntax");
    expect_eq(ast.nodes_byclass("InlineCodeBlock").size(),
              static_cast<std::size_t>(1),
              "Parser should detect inline code syntax");

    const std::string html = ast.to_html();
    expect_contains(html, "<h1", "HTML output should contain a heading");
    expect_contains(html, "<strong>", "HTML output should contain strong text");
    expect_contains(html, "language-cpp",
                    "HTML output should preserve code block language");

    const std::string json = ast.to_json();
    expect_contains(json, "\"class\":\"Markdown\"",
                    "JSON output should serialize the root node");

    const std::string dot = ast.to_dot();
    expect_contains(dot, "digraph G",
                    "DOT output should contain the graph header");
}

void test_rendering_and_pyodide() {
    {
        auto meta = default_meta();
        const auto summary =
            almo::md_to_summary(markdown_lines("# Plain document"), meta);
        expect_true(summary.html.find("pyodide.js") == std::string::npos,
                    "Plain documents should not include the Pyodide loader");
    }

    {
        auto meta = default_meta();
        const std::vector<std::string> lines = {
            ":::code",
            "print('hello')",
            ":::",
        };
        const auto summary = almo::md_to_summary(lines, meta);
        expect_contains(summary.html, "pyodide.js",
                        "Executable code blocks should include Pyodide");
        expect_contains(summary.html, "runBlock(",
                        "Executable code blocks should render a run button");
    }

    {
        auto meta = default_meta();
        const std::vector<std::string> lines = {
            "Footnote ref[^1]",
            "",
            "[^1]: note",
        };
        const auto summary = almo::md_to_summary(lines, meta);
        const auto body_pos = summary.html.find("Footnote ref");
        const auto footnote_pos = summary.html.find("class=\"footnote\"");
        expect_true(body_pos != std::string::npos && footnote_pos != std::string::npos,
                    "Rendered HTML should contain both body and footnote");
        expect_true(body_pos < footnote_pos,
                    "Footnotes should be moved to the end of the document");
    }
}

void run_test(const std::string& name, const std::function<void()>& test,
              int& failures) {
    try {
        test();
        std::cout << "[PASS] " << name << '\n';
    } catch (const std::exception& error) {
        ++failures;
        std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
    }
}

}  // namespace

int main() {
    int failures = 0;

    run_test("reader navigation", test_reader_navigation, failures);
    run_test("utility helpers", test_utils, failures);
    run_test("parser outputs", test_parser_outputs, failures);
    run_test("rendering and pyodide", test_rendering_and_pyodide, failures);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed" << '\n';
        return 1;
    }

    std::cout << "All tests passed" << '\n';
    return 0;
}
