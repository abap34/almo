#pragma once

#include <array>
#include <random>
#include <type_traits>

#include "../support/almo_test_support.hpp"
#include "parser_api_tests.hpp"

namespace almo_test {

inline void expect_fresh_parse(const almo::IncrementalParser& parser) {
    const auto fresh = almo::Parser{}.parse(parser.source());
    const auto copy = parser.clone_ast();
    expect_same_tree(copy, fresh.ast);
    expect_eq(std::vector<almo::Diagnostic>(parser.diagnostics().begin(), parser.diagnostics().end()),
              fresh.diagnostics, "Diagnostics must match full parsing");
    // Renderers embed generated UUIDs in multiple places. Normalize only UUID
    // fields, preserving all actual text (including numbers in source content).
    auto normalize_ids = [](const std::string& html) {
        return std::regex_replace(html, std::regex(R"re((id=|"id":)"[0-9]+")re"), "$1\"UUID\"");
    };
    expect_eq(normalize_ids(copy.to_html()), normalize_ids(fresh.ast.to_html()),
              "HTML including table headers must match full parsing");
}

inline almo::ParseUpdate edit_one(almo::IncrementalParser& parser, std::size_t begin,
                                  std::size_t end, std::string replacement) {
    const std::array edits{almo::TextEdit{{begin, end}, std::move(replacement)}};
    return parser.apply_edits(edits);
}

inline void register_incremental_parser_tests(std::vector<TestCase>& tests) {
    tests.push_back({"incremental parsing preserves unaffected UUIDs", [] {
        const std::string source = "# First\n\nalpha **bold**\n\n# Last\n\nomega";
        almo::IncrementalParser parser(source);
        auto children = parser.ast().get_childs();
        const auto first_id = children.front().get_uuid_str();
        const auto last_id = children.back().get_uuid_str();
        auto update = edit_one(parser, source.find("alpha"), source.find("alpha") + 5, "beta");
        expect_eq(parser.ast().get_childs().front().get_uuid_str(), first_id, "Prefix UUID must survive");
        expect_eq(parser.ast().get_childs().back().get_uuid_str(), last_id, "Suffix UUID must survive");
        expect_eq(update.reparsed_ranges.size(), std::size_t{1}, "One contiguous region is reparsed");
        expect_true(update.reparsed_ranges.front().begin > 0 && update.reparsed_ranges.front().end < parser.source().size(),
                    "Local edit should not parse the whole source");
        expect_fresh_parse(parser);
        const auto json = parser.ast().to_json();
        expect_true(parser.set_source(std::string(parser.source())).reparsed_ranges.empty(), "Unchanged source does no work");
        expect_true(parser.apply_edits({}).reparsed_ranges.empty(), "Empty edits do no work");
        expect_eq(parser.ast().to_json(), json, "No-op keeps all UUIDs");
    }});

    tests.push_back({"incremental boundaries and source replacement", [] {
        const std::vector<std::pair<std::string, std::string>> cases{
            {"", "a"}, {"a", ""}, {"a", "a\n"}, {"a\n", "a"}, {"a\n", "a\nb"},
            {"a\n\nb", "a\nb"}, {"a\n# b", "a\nb"}, {"# a\n# b", "# a\nb"},
            {"- a\n\nend", "- a\nb\nend"}, {"- a", "- a\n- b"},
            {"1. a\n\nend", "1. a\n2. b\nend"},
            {"> a\nend", "> a\n> end"},
            {"```\nx\n```\nend", "```\nx\nend"}, {"```\nx", "```\nx\n```\nend"},
            {"$$ x", "$$ x $$\nend"},
            {":::outer\n> a\n:::\nend", ":::outer\n> a\nend"},
            {"| a |\n| - |\n| b |\n\nend", "| a |\n| - |\n| b |\n| c |\n\nend"},
            {"# a\r\n\r\nb", "# a\r\n\r\nc"},
            {"a\n\n[^]: warning", "longer\n\n[^]: warning"},
            {"- a\n   - b\n\nend", "- a\n  - b\n\nend"},
            {"[^]: warning\n\nx", "x"},
        };
        for (const auto& [before, after] : cases) {
            almo::IncrementalParser parser(before);
            parser.set_source(after);
            expect_eq(std::string(parser.source()), after, "Updated source");
            expect_fresh_parse(parser);
            parser.set_source(before);
            expect_fresh_parse(parser);
        }
    }});

    tests.push_back({"batched edits validate boundaries and roll back on errors", [] {
        almo::IncrementalParser parser("日本語\n\ntext\n\nend");
        const std::array edits{
            almo::TextEdit{{0, 3}, "英"}, almo::TextEdit{{11, 15}, "word"}};
        parser.apply_edits(edits);
        expect_eq(std::string(parser.source()), std::string("英本語\n\nword\n\nend"), "Batch uses old offsets");
        expect_fresh_parse(parser);
        const auto source = std::string(parser.source());
        const auto json = parser.ast().to_json();
        expect_throws<std::invalid_argument>([&] { edit_one(parser, 1, 3, "x"); }, "UTF-8 mid-byte start rejected");
        expect_throws<std::invalid_argument>([&] { edit_one(parser, 0, 2, "x"); }, "UTF-8 mid-byte end rejected");
        expect_throws<std::invalid_argument>([&] { edit_one(parser, 0, 999, "x"); }, "Out of bounds rejected");
        expect_throws<std::invalid_argument>([&] { edit_one(parser, 6, 3, "x"); }, "Reversed range rejected");
        for (const auto edits : {
            std::array{almo::TextEdit{{0, 6}, "x"}, almo::TextEdit{{3, 9}, "y"}},
            std::array{almo::TextEdit{{3, 6}, "x"}, almo::TextEdit{{0, 3}, "y"}},
            std::array{almo::TextEdit{{0, 0}, "x"}, almo::TextEdit{{0, 0}, "y"}}})
            expect_throws<std::invalid_argument>([&] { parser.apply_edits(edits); }, "Overlapping or unordered edits rejected");
        expect_throws<SyntaxError>([&] { parser.set_source(":::judge\nbroken\n:::"); }, "Invalid source rejected");
        expect_throws<SyntaxError>([&] { edit_one(parser, 0, source.size(), "| a | b |"); }, "Invalid edit rejected");
        expect_eq(std::string(parser.source()), source, "Failures preserve source");
        expect_eq(parser.ast().to_json(), json, "Failures preserve the original AST and IDs");
        edit_one(parser, 0, 3, "日");
        expect_fresh_parse(parser);
    }});

    tests.push_back({"cloned AST is independent and parser is movable", [] {
        static_assert(!std::is_copy_constructible_v<almo::IncrementalParser>);
        static_assert(std::is_nothrow_move_constructible_v<almo::IncrementalParser>);
        static_assert(std::is_same_v<decltype(std::declval<almo::ASTView>().get_childs()), std::vector<almo::ASTView>>);
        almo::IncrementalParser parser(":::outer\n[^x]: note\n:::\n\n| **h** |\n| - |\n| cell |");
        const auto html = parser.ast().to_html();
        auto copy = parser.clone_ast();
        almo::move_footnote_to_end(copy);
        copy.childs.front()->childs.clear();
        expect_eq(parser.ast().to_html(), html, "Mutating a deep copy cannot affect stored AST");
        auto table_copy = parser.clone_ast();
        const auto table = table_copy.nodes_byclass("Table").front();
        table->childs.front()->childs.clear();
        expect_eq(parser.ast().to_html(), html, "Table cells must be deep copied");
        auto header = std::make_shared<almo::Markdown>(almo::Parser{}.parse("**original**").ast);
        almo::Table original_table({header}, 0, 1, {"**original**"}, {0});
        const auto cloned_table = original_table.clone();
        header->childs.clear();
        expect_contains(cloned_table->to_html(), "<strong>original</strong>", "Table headers must also be deep copied");
        almo::IncrementalParser moved(std::move(parser));
        moved.set_source("updated");
        expect_fresh_parse(moved);
        expect_contains(table_copy.to_html(), "<strong>h</strong>", "Clone survives parser updates and retains table headers");
    }});

    tests.push_back({"incremental parsing matches full parsing across edit sequences", [] {
        const std::vector<std::string> documents{
            "# title\n\nplain **bold** $x$ [url](target)\n\nlast",
            "- one\n  - nested\n- two\n\n> quote\n> next\n\nend",
            ":::outer\ntext\n:::inner\n> quote\n:::\n:::\n\nend",
            "```cpp\nx\n```\n\n$$x$$\n\n[^]: note\n\nend",
            "| a | b |\n| - | - |\n| x | y |\n\nend"};
        const std::array<std::string, 11> replacements{"", "\n", "x", "\n\n", "# ", "> ", "- ", ":::inner\n", "```\n", "$$", "[^]: "};
        std::mt19937 random(340);
        for (const auto& document : documents) {
            almo::IncrementalParser parser(document);
            for (int i = 0; i < 120; ++i) {
                std::string next(parser.source());
                const auto begin = random() % (next.size() + 1);
                const auto count = std::min<std::size_t>(random() % 5, next.size() - begin);
                const auto replacement = replacements[random() % replacements.size()];
                next.replace(begin, count, replacement);
                const auto original = std::string(parser.source());
                std::optional<almo::Diagnostic> expected_error;
                try { (void)almo::Parser{}.parse(next); }
                catch (const almo::ParseException& error) { expected_error = error.diagnostic(); }
                try {
                    edit_one(parser, begin, begin + count, replacement);
                    expect_true(!expected_error.has_value(), "Incremental parsing must reject the same errors");
                    expect_fresh_parse(parser);
                } catch (const almo::ParseException& error) {
                    expect_true(expected_error.has_value(), "Incremental parsing must accept valid full parses");
                    expect_eq(error.diagnostic(), *expected_error, "Full and incremental errors match");
                    expect_eq(std::string(parser.source()), original, "Failed sequence edit preserves source");
                }
            }
        }
    }});
}

}  // namespace almo_test
