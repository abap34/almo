#pragma once

#include <vector>

#include "../support/almo_test_support.hpp"
#include "../support/test_documents.hpp"

namespace almo_test {

inline void register_parser_tests(std::vector<TestCase>& tests) {
    tests.push_back({
        "parser outputs",
        []() {
            const auto lines = basic_parser_document();
            const almo::Markdown ast = parse_markdown(lines);

            expect_eq(ast.nodes_byclass("Header").size(),
                      static_cast<std::size_t>(1),
                      "Parser should create one header node");
            expect_eq(ast.nodes_byclass("CodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Parser should create one code block");
            expect_eq(ast.nodes_byclass("InlineStrong").size(),
                      static_cast<std::size_t>(1),
                      "Parser should detect inline strong syntax");
            expect_eq(ast.nodes_byclass("InlineCodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Parser should detect inline code syntax");

            const auto summary = summarize_markdown(lines);
            expect_contains(summary.html, "<h1",
                            "HTML output should contain a heading");
            expect_contains(summary.html, "<strong>",
                            "HTML output should contain strong text");
            expect_contains(summary.html, "language-cpp",
                            "HTML output should preserve code block language");
            expect_contains(summary.json, "\"class\":\"Markdown\"",
                            "JSON output should serialize the root node");
            expect_contains(summary.dot, "digraph G",
                            "DOT output should contain the graph header");
        },
    });

    tests.push_back({
        "table and inline syntax",
        []() {
            const auto summary =
                summarize_markdown(table_with_inline_syntax_document());

            expect_eq(summary.ast.nodes_byclass("Table").size(),
                      static_cast<std::size_t>(1),
                      "Tables should be parsed as a dedicated block");
            expect_eq(summary.ast.nodes_byclass("InlineStrong").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support strong inline syntax");
            expect_eq(summary.ast.nodes_byclass("InlineMath").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support inline math");
            expect_eq(summary.ast.nodes_byclass("InlineUrl").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support links");
            expect_eq(summary.ast.nodes_byclass("InlineImage").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support images");
            expect_eq(summary.ast.nodes_byclass("InlineOverline").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support strikethrough");
            expect_eq(summary.ast.nodes_byclass("InlineCodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Table cells should support inline code");

            expect_contains(summary.html, "<th align=\"left\">",
                            "Table headers should preserve left alignment");
            expect_contains(summary.html, "<th align=\"center\">",
                            "Table headers should preserve center alignment");
            expect_contains(summary.html, "<th align=\"right\">",
                            "Table headers should preserve right alignment");
            expect_contains(
                summary.html,
                "<figure><img src=\"https://example.com/plot.png\" >",
                "Table cells should render inline images");
        },
    });

    tests.push_back({
        "headers horizontal lines and plain code blocks",
        []() {
            const auto marker_summary =
                summarize_markdown(header_horizontal_rule_document());

            expect_eq(marker_summary.ast.nodes_byclass("Header").size(),
                      static_cast<std::size_t>(2),
                      "Multiple headers should be parsed");
            expect_eq(marker_summary.ast.nodes_byclass("HorizontalLine").size(),
                      static_cast<std::size_t>(1),
                      "Horizontal rules should be parsed");
            expect_eq(marker_summary.ast.nodes_byclass("InlineItalic").size(),
                      static_cast<std::size_t>(1),
                      "Inline italic syntax should be parsed");
            expect_contains(marker_summary.html, "<hr>",
                            "Horizontal rules should render as hr");
            expect_contains(marker_summary.html, "<h2",
                            "Level 2 headers should render");
            expect_contains(marker_summary.html, "<h3",
                            "Level 3 headers should render");

            const auto code_summary =
                summarize_markdown(plain_code_escape_document());
            expect_eq(code_summary.ast.nodes_byclass("CodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Plain code blocks should be parsed");
            expect_eq(code_summary.ast.nodes_byclass("InlineCodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Inline code should still be parsed outside code blocks");
            expect_contains(code_summary.html, "language-plaintext",
                            "Code blocks without an explicit language should default to plaintext");
            expect_contains(code_summary.html, "&lt;tag&gt;",
                            "Code blocks should HTML-escape angle brackets");
            expect_contains(code_summary.html, "&amp;&amp;",
                            "Code blocks should HTML-escape ampersands");
            expect_contains(code_summary.html,
                            "<span class=\"inline-code\"><code>x &lt; y</code></span>",
                            "Inline code blocks should HTML-escape their contents");
        },
    });

    tests.push_back({
        "div quote and math blocks",
        []() {
            const auto lines = nested_div_quote_math_document();
            const auto ast = parse_markdown(lines);
            const auto summary = summarize_markdown(lines);

            expect_eq(ast.nodes_byclass("DivBlock").size(),
                      static_cast<std::size_t>(2),
                      "Nested div blocks should be preserved");
            expect_eq(ast.nodes_byclass("Quote").size(),
                      static_cast<std::size_t>(1),
                      "Quotes inside custom div blocks should be parsed");
            expect_eq(ast.nodes_byclass("MathBlock").size(),
                      static_cast<std::size_t>(1),
                      "Math blocks should be parsed");

            expect_contains(summary.html, "<div class=\"callout\">",
                            "Outer custom div should render its class");
            expect_contains(summary.html, "<div class=\"inner\">",
                            "Inner custom div should render its class");
            expect_contains(summary.html, "<blockquote>",
                            "Quote blocks should render blockquote HTML");
            expect_contains(summary.html, "class=\"math-block\"",
                            "Math blocks should render dedicated markup");
        },
    });

    tests.push_back({
        "nested lists and enumerations",
        []() {
            const auto summary = summarize_markdown(nested_lists_document());

            expect_eq(summary.ast.nodes_byclass("ListBlock").size(),
                      static_cast<std::size_t>(2),
                      "Nested unordered lists should create nested list blocks");
            expect_eq(summary.ast.nodes_byclass("EnumerateBlock").size(),
                      static_cast<std::size_t>(2),
                      "Nested ordered lists should create nested enumerate blocks");
            expect_eq(summary.ast.nodes_byclass("Item").size(),
                      static_cast<std::size_t>(6),
                      "List parsing should preserve all list items");

            expect_contains(summary.html, "<ul>",
                            "Unordered lists should render as ul");
            expect_contains(summary.html, "<ol>",
                            "Ordered lists should render as ol");
        },
    });
}

}  // namespace almo_test
