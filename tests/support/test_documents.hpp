#pragma once

#include <string>
#include <vector>

#include "almo_test_support.hpp"

namespace almo_test {

inline std::vector<std::string> basic_parser_document() {
    return load_fixture_lines("markdown/basic_parser.md");
}

inline std::vector<std::string> table_with_inline_syntax_document() {
    return load_fixture_lines("markdown/table_with_inline_syntax.md");
}

inline std::vector<std::string> nested_div_quote_math_document() {
    return load_fixture_lines("markdown/nested_div_quote_math.md");
}

inline std::vector<std::string> nested_lists_document() {
    return load_fixture_lines("markdown/nested_lists.md");
}

inline std::vector<std::string> loadlib_executable_document() {
    return load_fixture_lines("markdown/loadlib_executable.md");
}

inline std::vector<std::string> footnote_document() {
    return load_fixture_lines("markdown/footnote.md");
}

inline std::vector<std::string> judge_document() {
    return load_fixture_lines("markdown/judge.md");
}

inline std::vector<std::string> judge_document_missing_required_arg() {
    return load_fixture_lines("markdown/judge_missing_required_arg.md");
}

inline std::vector<std::string> judge_document_with_duplicate_title() {
    return load_fixture_lines("markdown/judge_duplicate_title.md");
}

inline std::vector<std::string> judge_document_with_invalid_type() {
    return load_fixture_lines("markdown/judge_invalid_type.md");
}

}  // namespace almo_test
