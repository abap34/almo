#pragma once

#include <vector>

#include "../support/almo_test_support.hpp"
#include "../support/test_documents.hpp"

namespace almo_test {

inline void register_error_tests(std::vector<TestCase>& tests) {
    tests.push_back({
        "error paths",
        []() {
            expect_throws<SyntaxError>(
                []() { parse_markdown(std::vector<std::string>{":::loadlib"}); },
                "Empty LoadLib blocks should be rejected", "Empty Library");

            expect_throws<SyntaxError>(
                []() { parse_markdown(judge_document_with_duplicate_title()); },
                "Duplicate judge options should be rejected",
                "Duplicate judge options");

            expect_throws<SyntaxError>(
                []() { parse_markdown(judge_document_missing_required_arg()); },
                "Missing judge arguments should be rejected", "out");

            expect_throws<ParseError>(
                []() { parse_markdown(judge_document_with_invalid_type()); },
                "Invalid judge types should be rejected", "Invalid judge type");

            expect_throws<InvalidCommandLineArgumentsError>(
                []() { almo::load_html_template("__default__", "sepia", false); },
                "Unknown CSS settings should be rejected", "不正なCSS");
        },
    });
}

}  // namespace almo_test
