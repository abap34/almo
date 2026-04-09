#pragma once

#include <vector>

#include "../support/almo_test_support.hpp"

namespace almo_test {

inline void register_reader_and_utils_tests(std::vector<TestCase>& tests) {
    tests.push_back({
        "reader navigation",
        []() {
            almo::Reader reader({"Hello", "World"});

            expect_eq(reader.get_row(), std::string("Hello"),
                      "Reader should start on the first row");
            expect_true(reader.is_line_begin(),
                        "Reader should begin at the start of the line");

            reader.move_next_char(3);
            expect_eq(reader.col, 3, "Reader should move within a line");
            expect_true(!reader.is_line_begin(),
                        "Reader should no longer be at line begin after moving");
            expect_eq(reader.get_rest_row(), std::string("lo"),
                      "Reader should expose the rest of the current row");

            reader.move_next_line();
            expect_eq(reader.get_row(), std::string("World"),
                      "Reader should advance to the next row");
            expect_contains(
                reader.near(), "Hello\nWorld",
                "Reader::near should include the previous and current row");

            reader.move_next_line();
            expect_true(reader.is_eof(),
                        "Reader should report EOF after the last row");
        },
    });

    tests.push_back({
        "utility helpers",
        []() {
            expect_eq(join({"a", "b", "c"}, ","), std::string("a,b,c"),
                      "join should concatenate strings with a separator");

            const auto parts = split("alpha::beta::gamma", "::");
            expect_eq(parts.size(), static_cast<std::size_t>(3),
                      "split should return all segments");
            expect_eq(parts[1], std::string("beta"),
                      "split should preserve middle segments");
            expect_eq(split("single", "::").size(), static_cast<std::size_t>(1),
                      "split should keep strings without separators intact");

            expect_eq(ltrim("  left"), std::string("left"),
                      "ltrim should remove leading whitespace");
            expect_eq(rtrim("right  \n"), std::string("right"),
                      "rtrim should remove trailing whitespace");
            expect_eq(ltrim("   \n\t"), std::string(""),
                      "ltrim should handle whitespace-only strings");
            expect_eq(rtrim("   \n\t"), std::string(""),
                      "rtrim should handle whitespace-only strings");
            expect_eq(remove_listdef("- item"), std::string("item"),
                      "remove_listdef should remove list markers");
            expect_eq(remove_listdef("plain"), std::string("plain"),
                      "remove_listdef should preserve plain text");

            const std::string stripped =
                _remove_comment("visible\n<!-- hidden\nstill hidden -->\nafter");
            expect_not_contains(stripped, "hidden",
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
            const std::string dot_escaped = escape("{a|b}", EscapeFormat::DOT);
            expect_contains(dot_escaped, "\\|",
                            "DOT escaping should escape record separators");
            expect_true(dot_escaped != "{a|b}",
                        "DOT escaping should alter record syntax characters");

            const auto matches = glob(fixtures::kJudgeInputGlob);
            expect_eq(matches.size(), static_cast<std::size_t>(3),
                      "glob should match all judge input fixtures");
            expect_true(!read_file(fixtures::kExampleInputPath).empty(),
                        "read_file should load repository fixtures");
        },
    });
}

}  // namespace almo_test
