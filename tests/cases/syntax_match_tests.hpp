#pragma once

#include "../support/almo_test_support.hpp"

namespace almo_test {

inline void register_syntax_match_tests(std::vector<TestCase>& tests) {
    tests.push_back({"inline matches retain their captured text", [] {
        almo::InlineStrongSyntax syntax;
        expect_true(!syntax.find({}), "Empty input has no match");
        std::string source = "a **bold** tail";
        const auto match = syntax.find(source);
        expect_true(match.has_value(), "Strong syntax should match");
        expect_eq(match->range, almo::SourceRange{2, 10}, "Match range includes delimiters");
        source.assign("changed input");
        almo::Markdown ast;
        syntax.parse(*match, ast);
        expect_contains(ast.to_html(), "<strong>bold</strong>", "Emission uses saved captures");
    }});
}

}  // namespace almo_test
