#pragma once

#include "../support/almo_test_support.hpp"

namespace almo_test {

inline void expect_same_tree(const almo::ASTNode& actual, const almo::ASTNode& expected) {
    expect_eq(actual.get_classname(), expected.get_classname(), "Node classes must match full parsing");
    expect_eq(actual.get_properties(), expected.get_properties(), "Node properties must match full parsing");
    expect_eq(actual.childs.size(), expected.childs.size(), "Child count must match full parsing");
    for (std::size_t i = 0; i < actual.childs.size(); ++i)
        expect_same_tree(*actual.childs[i], *expected.childs[i]);
}

inline void register_parser_api_tests(std::vector<TestCase>& tests) {
    tests.push_back({"Parser owns results and is reusable", [] {
        const almo::Parser parser;
        std::string source = "first **strong**";
        auto first = parser.parse(source);
        source.assign("changed");
        auto second = parser.parse("second");
        expect_contains(first.ast.to_html(), "first", "AST must own source text");
        expect_eq(second.ast.to_html(), std::string("second"), "Parser must not retain cursor state");
        for (const std::string text : {"", "\n", "text\n", "text\r\n\r\n# title\r\n"}) {
            auto result = parser.parse(text);
            expect_same_tree(result.ast, parse_markdown(text));
        }
        almo::Markdown legacy;
        almo::MarkdownParser one({"one"}), two({"two"});
        one.process(legacy);
        one.process(legacy);
        two.process(legacy);
        expect_eq(legacy.to_html(), std::string("onetwo"), "Legacy API keeps append and exhausted-reader behavior");
    }});

    tests.push_back({"diagnostics retain original UTF-8 source positions", [] {
        const std::string source = "前文\n\n:::outer\n> > [^]: warning\n:::\n";
        auto result = almo::Parser{}.parse(source);
        expect_eq(result.diagnostics.size(), std::size_t{1}, "Nested warning must be returned once");
        const auto start = source.find("[^]");
        expect_eq(result.diagnostics[0].range,
                  almo::SourceRange{start, source.find('\n', start)}, "Quote prefixes must map to outer source");
        expect_eq(result.diagnostics[0].severity, almo::DiagnosticSeverity::Warning, "Warning severity");
        const std::string bad = "先頭\n\n> :::judge\n> broken\n> :::";
        try {
            (void)almo::Parser{}.parse(bad);
            throw std::runtime_error("Invalid judge should fail");
        } catch (const SyntaxError& error) {
            expect_true(error.has_source_range(), "Syntax errors must have a range");
            const auto begin = bad.find("broken");
            expect_eq(error.diagnostic().range, almo::SourceRange{begin, begin + 6}, "Nested error position");
            expect_eq(error.diagnostic().severity, almo::DiagnosticSeverity::Error, "Error severity");
        }
    }});
}

}  // namespace almo_test
