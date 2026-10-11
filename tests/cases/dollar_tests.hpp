#pragma once

#include "../support/almo_test_support.hpp"
#include "incremental_parser_tests.hpp"

namespace almo_test {

inline void register_dollar_tests(std::vector<TestCase>& tests) {
    tests.push_back({"unmatched dollar amounts survive final HTML rendering", [] {
        for (const std::string source : {"$30", "$1500", "price: $30", "price: $1500"}) {
            const auto ast = almo::Parser{}.parse(source).ast;
            expect_eq(ast.to_html(), source, "An unmatched dollar remains plain text");
            expect_contains(almo::render(ast, default_meta()), source, "Template insertion must not interpret dollar amounts");
        }
    }});

    tests.push_back({"CSS JavaScript metadata and body are not substituted again", [] {
        auto meta = default_meta();
        meta["template_file"] = fixture_path("templates/literal.html");
        meta["css_setting"] = fixture_path("styles/literal.css");
        meta["title"] = R"($1 $30 $1500 $& $$ \ {{contents}} {{runner}})";
        meta["script"] = R"(<script>const text = String.raw`$1 $30 $1500 $& $$ \ \\ {{title}} {{contents}}`;</script>)";
        const std::string body = R"(body $30 {{title}} {{runner}} {{contents}})";
        const auto html = almo::render(almo::Parser{}.parse(body).ast, meta);
        expect_contains(html, "<title>" + meta["title"] + "</title>", "Metadata must be inserted once");
        expect_contains(html, "<style>" + load_fixture_text("styles/literal.css") + "</style>",
                        "CSS placeholders and replacement tokens must remain literal");
        expect_contains(html, meta["script"], "JavaScript strings must remain literal");
        expect_contains(html, "<article>" + body + "</article>", "Body placeholders must remain literal");
        expect_contains(html, "<nav>{{navigation}}</nav>", "Unknown placeholders are reserved for downstream processing");
    }});

    tests.push_back({"escaped dollar amounts stay text including inside emphasis", [] {
        for (const std::string source : {R"(\$30 と \$40)", R"(**\$30 と \$40**)",
                                         R"(*\$30 と \$40*)", R"(~~\$30 と \$40~~)"}) {
            const auto summary = summarize_markdown(source);
            expect_true(summary.ast.nodes_byclass("InlineMath").empty(), "Escaped dollars must not start math");
            expect_contains(summary.ast.to_html(), "$30 と $40", "Text escapes must be removed exactly once");
            expect_contains(summary.html, "$30 と $40", "Escaped currency must survive template insertion");
            expect_not_contains(summary.ast.to_html(), R"(\$)", "The escape backslash is not displayed");
        }
        const auto ast = almo::Parser{}.parse(R"(\$30 $x$ \$40)").ast;
        expect_eq(ast.nodes_byclass("InlineMath").size(), std::size_t{1}, "Real math can follow escaped text");
        expect_eq(ast.childs.front()->to_html(), std::string("$30 "), "An escaped dollar is never reparsed");
        expect_eq(ast.childs.back()->to_html(), std::string(" $40"), "Escaped text after math stays literal");
    }});

    tests.push_back({"dollar delimiters respect odd and even backslash counts", [] {
        for (std::size_t count = 0; count <= 8; ++count) {
            const std::string slashes(count, '\\');
            const auto opening = almo::Parser{}.parse("before " + slashes + "$x$ after").ast;
            const auto math = opening.nodes_byclass("InlineMath");
            expect_eq(math.size(), count % 2 ? std::size_t{0} : std::size_t{1},
                      "Only an odd backslash count escapes the opening dollar");
            const auto prefix = "before " + std::string(count / 2, '\\');
            if (count % 2) {
                expect_eq(opening.to_html(), prefix + "$x$ after", "Backslash pairs collapse only in text");
            } else {
                expect_eq(opening.childs.front()->to_html(), prefix, "Text before math retains paired backslashes");
                expect_eq(math.front()->get_properties().at("expr"), std::string("x"), "The math expression is unchanged");
            }

            const auto closing = almo::Parser{}.parse("$a" + slashes + "$b$ tail").ast;
            const auto closed_math = closing.nodes_byclass("InlineMath");
            expect_eq(closed_math.size(), std::size_t{1}, "Exactly one math expression closes");
            expect_eq(closed_math.front()->get_properties().at("expr"),
                      "a" + slashes + (count % 2 ? "$b" : ""),
                      "Escaped closing candidates are skipped and TeX backslashes remain verbatim");
            expect_eq(closing.childs.back()->to_html(), std::string(count % 2 ? " tail" : "b$ tail"),
                      "The expression stops at the first unescaped closing dollar");
        }
        const auto raw = almo::Parser{}.parse(R"($a\$)").ast;
        expect_true(raw.nodes_byclass("InlineMath").empty(), "An escaped dollar cannot close an otherwise unclosed expression");
        expect_eq(raw.to_html(), std::string("$a$"), "Unclosed math falls back to ordinary text escaping");
        expect_eq(almo::Parser{}.parse(R"(plain \\ \q \\q)").ast.to_html(),
                  std::string(R"(plain \ \q \q)"), "Backslash pairs are handled without stripping unrelated escapes");
    }});

    tests.push_back({"code retains dollars backslashes and template placeholders", [] {
        const std::string code = R"($1 $30 $1500 $& $$ \$40 \\ \frac{a}{b} {{title}})";
        const auto summary = summarize_markdown("`" + code + "`\n\n```text\n" + code + "\n```");
        const auto inline_code = summary.ast.nodes_byclass("InlineCodeBlock");
        const auto block_code = summary.ast.nodes_byclass("CodeBlock");
        expect_eq(inline_code.size(), std::size_t{1}, "Inline code is recognized before its contents");
        expect_eq(block_code.size(), std::size_t{1}, "Code blocks are recognized before inline parsing");
        expect_eq(inline_code.front()->get_properties().at("code"), code, "Inline code bytes must be preserved");
        expect_eq(block_code.front()->get_properties().at("code"), code + "\n", "Code block bytes must be preserved");
        expect_true(summary.ast.nodes_byclass("InlineMath").empty(), "Code contents are never parsed as math");
        expect_contains(summary.html, escape(code, EscapeFormat::HTML), "Final HTML preserves code text with HTML escaping");
    }});

    tests.push_back({"ordinary math and TeX backslashes keep their existing syntax", [] {
        const auto summary = summarize_markdown("$x$ $2^n$ $ x $\n\n$$\n\\frac{1}{2}+\\$30\n$$");
        const auto math = summary.ast.nodes_byclass("InlineMath");
        expect_eq(math.size(), std::size_t{3}, "Existing inline math forms remain valid");
        expect_eq(math[0]->get_properties().at("expr"), std::string("x"), "Symbol expression");
        expect_eq(math[1]->get_properties().at("expr"), std::string("2^n"), "Numeric expression");
        expect_eq(math[2]->get_properties().at("expr"), std::string(" x "), "Whitespace inside delimiters is retained");
        const auto blocks = summary.ast.nodes_byclass("MathBlock");
        expect_eq(blocks.size(), std::size_t{1}, "Display math is unchanged");
        expect_eq(blocks[0]->get_properties().at("expression"), std::string("\n\\frac{1}{2}+\\$30\n"),
                  "Display math preserves TeX and escaped dollars");
        const std::string expr = R"(\frac{1}{2}+\$30)";
        const auto inline_math = summarize_markdown("$" + expr + "$");
        expect_eq(inline_math.ast.nodes_byclass("InlineMath").front()->get_properties().at("expr"), expr,
                  "Escaped closing candidates remain inside TeX");
        expect_contains(inline_math.html, expr, "TeX escapes survive final rendering");
        const auto currency = almo::Parser{}.parse("$30 と $40").ast;
        expect_eq(currency.nodes_byclass("InlineMath").size(), std::size_t{1},
                  "Unescaped currency-looking pairs retain the existing delimiter rule");
    }});

    tests.push_back({"incremental edits update dollar escape parity like a full parse", [] {
        almo::IncrementalParser parser("before\n\n$x$\n\nafter");
        const auto prefix_id = parser.ast().get_childs().front().get_uuid_str();
        const auto position = parser.source().find('$');
        edit_one(parser, position, position, "\\");
        expect_fresh_parse(parser);
        expect_true(parser.clone_ast().nodes_byclass("InlineMath").empty(), "One backslash disables the opener");
        edit_one(parser, position, position, "\\");
        expect_fresh_parse(parser);
        expect_eq(parser.clone_ast().nodes_byclass("InlineMath").size(), std::size_t{1}, "Two backslashes enable the opener");
        expect_eq(parser.ast().get_childs().front().get_uuid_str(), prefix_id, "Unchanged paragraphs remain reusable");
    }});
}

}  // namespace almo_test
