#pragma once

#include <array>
#include <random>

#include "../support/almo_test_support.hpp"
#include "parser_api_tests.hpp"

namespace almo_test {

inline void expect_inline_matches_and_tree(const std::string& source) {
    const almo::InlineMathSyntax math;
    const almo::InlineOverlineSyntax overline;
    const almo::InlineStrongSyntax strong;
    const almo::InlineItalicSyntax italic;
    const almo::InlineCodeBlockSyntax code;
    const almo::InlineImageSyntax image;
    const almo::InlineUrlSyntax url;
    const almo::InlineFootnoteReferenceSyntax footnote;
    const std::array<const almo::InlineSyntax*, 8> syntaxes{
        &math, &overline, &strong, &italic, &code, &image, &url, &footnote};
    const std::array<const std::regex*, 8> patterns{
        &math.rex, &overline.rex, &strong.rex, &italic.rex,
        &code.rex, &image.rex, &url.rex, &footnote.rex};

    for (std::size_t i = 0; i < syntaxes.size(); ++i) {
        const auto actual = syntaxes[i]->find(source);
        const auto expected = almo::find_inline(source, *patterns[i]);
        expect_eq(actual.has_value(), expected.has_value(), "Literal search must match the original regex");
        if (actual && expected) {
            expect_eq(actual->range, expected->range, "Match positions must be unchanged");
            expect_eq(actual->captures, expected->captures, "Captured text must be unchanged");
        }
    }

    // Keep the former uncached regex search as an independent oracle for
    // selection order, overlapping matches and raw-text boundaries.
    almo::Markdown expected;
    std::string_view rest(source);
    while (!rest.empty()) {
        auto selected = syntaxes.size();
        std::optional<almo::InlineMatch> matched;
        for (std::size_t i = 0; i < syntaxes.size(); ++i) {
            auto candidate = almo::find_inline(rest, *patterns[i]);
            if (candidate && (!matched || candidate->range.begin < matched->range.begin)) {
                selected = i;
                matched = std::move(candidate);
            }
        }
        if (!matched) {
            expected.pushback_child(std::make_shared<almo::RawText>(std::string(rest)));
            break;
        }
        if (matched->range.begin)
            expected.pushback_child(std::make_shared<almo::RawText>(std::string(rest.substr(0, matched->range.begin))));
        syntaxes[selected]->parse(*matched, expected);
        rest.remove_prefix(matched->range.end);
    }
    almo::Markdown actual;
    almo::InlineParser::process(source, actual);
    expect_same_tree(actual, expected);
    expect_eq(actual.to_html(), expected.to_html(), "Cached matching must preserve HTML");
}

inline void register_syntax_match_tests(std::vector<TestCase>& tests) {
    tests.push_back({"block matching preserves whitespace and captured text", [] {
        const almo::NewLineSyntax newline;
        const almo::JudgeSyntax judge;
        const almo::ExecutableCodeBlockSyntax executable;
        const almo::LoadLibSyntax loadlib;
        const almo::DivBlockSyntax div;
        const almo::QuoteSyntax quote;
        const almo::ListBlockSyntax list;
        const almo::EnumerateBlockSyntax enumeration;
        const almo::HeaderSyntax header;
        const almo::CodeBlockSyntax code;
        const almo::MathBlockSyntax math;
        const almo::HorizontalLineSyntax rule;
        const almo::TableSyntax table;
        const almo::FootnoteDefinitionSyntax footnote;
        const std::array<const almo::BlockSyntax*, 14> syntaxes{
            &newline, &judge, &executable, &loadlib, &div, &quote, &list,
            &enumeration, &header, &code, &math, &rule, &table, &footnote};
        const std::regex table_pattern(R"((\|[^\|]+).+\|)");
        std::vector<std::string> rows{
            "", " \t\r", "\v", "\f", "plain 日本語😀", ":::judge", ":::code \t",
            ":::loadlib\r", ":::class", "::: ", "> quote", "- item", "1. item",
            "123. ", "1. item\r", "   1. nested", "# heading", "###### heading",
            "####### raw", "```cpp", "$$x$$", "---\t", "___\r", "*** ",
            "| a | b |\r", "|a|", "||", "[^n]: text", "[^]: text", "[^a]:b]:c"};
        const char embedded[] = "[^a\0b]:c\0d";
        rows.emplace_back(embedded, sizeof(embedded) - 1);
        for (const auto& row : rows) {
            const auto trimmed = rtrim(row);
            bool is_header = false;
            for (int level = 1; level <= 6; ++level)
                is_header = is_header || row.starts_with(std::string(level, '#') + " ");
            const std::array<bool, 14> expected{
                trimmed.empty(), trimmed == ":::judge", trimmed == ":::code",
                trimmed == ":::loadlib", row.starts_with(":::") && trimmed != ":::",
                row.starts_with("> "), row.starts_with("- "),
                std::regex_match(row, enumeration.rex), is_header, row.starts_with("```"),
                row.starts_with("$$"), trimmed == "---" || trimmed == "___" || trimmed == "***",
                std::regex_match(trimmed, table_pattern), std::regex_match(row, footnote.rex)};
            auto read = almo::Reader::from_source(row);
            for (std::size_t i = 0; i < syntaxes.size(); ++i)
                expect_eq(syntaxes[i]->match(read).has_value(), expected[i],
                          "Block matching must preserve original predicates: " + escape(row));
            if (expected.back()) {
                std::smatch found;
                std::regex_match(row, found, footnote.rex);
                expect_eq(footnote.match(read)->captures,
                          std::vector<std::string>{found[1].str(), found[2].str()},
                          "Footnote captures must own the same bytes");
            }
            if (!row.empty()) {
                read.move_next_char(1);
                for (const auto* syntax : syntaxes)
                    expect_true(!syntax->match(read), "Blocks require the beginning of a line");
            }
        }
    }});

    tests.push_back({"table cell scanning preserves the original regex matches", [] {
        auto compare = [](const std::string& row) {
            static const std::regex pattern(R"(\|[^\|]+)");
            std::vector<std::string> expected;
            std::smatch found;
            auto rest = row;
            while (std::regex_search(rest, found, pattern)) {
                expected.push_back(found[0].str().substr(1));
                rest = found.suffix();
            }
            std::vector<std::string> actual;
            for (const auto cell : almo::detail::table_cells(row)) actual.emplace_back(cell);
            expect_eq(actual, expected, "Table cells must preserve empty-cell and whitespace behavior");
        };
        for (const std::string row : {"", "|", "||", "|a||b|", "prefix | a | b |  ",
                                     "| a\nline | b\r |", "| 日本語😀 | **bold** |"})
            compare(row);
        const char embedded[] = "|a\0b||c\0d|";
        compare(std::string(embedded, sizeof(embedded) - 1));
        std::mt19937 random(341);
        const std::array<std::string, 8> tokens{"a", "|", "||", " ", "\r", "\n", "日本語", std::string(1, '\0')};
        for (int i = 0; i < 600; ++i) {
            std::string row;
            for (int j = 0; j < 30; ++j) row += tokens[random() % tokens.size()];
            compare(row);
        }
    }});

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

    tests.push_back({"inline searches preserve regex semantics and selection order", [] {
        for (const std::string source : {
            "", "**** *** ** * ~~~~ ~~ $$ `` [^] []( ) ![]()",
            "**outer *inner* tail** plain *after*",
            "`$literal$ [x](y) **strong**` $real$ [link](target)",
            "![a](b) [c](d) [^n](target) [^n] **b** *i* ~~s~~",
            "[a](b [c](d)) ![one](two](three)four)",
            "**broken\n**valid** $bad\r$ok$ [a\n[b](c) ![x](bad\r![y](z)",
            "日本語 **強調😀** と [名前](行き先) と `コード`"})
            expect_inline_matches_and_tree(source);
        const char embedded[] = "a\0**b\0c** [x](y\0z)";
        expect_inline_matches_and_tree(std::string(embedded, sizeof(embedded) - 1));

        std::mt19937 random(340);
        const std::array<std::string, 18> tokens{
            "a", " ", "\n", "\r", "*", "**", "~", "~~", "`", "$", "[", "![",
            "]", "](", ")", "[^", "日本語", std::string(1, '\0')};
        for (int i = 0; i < 1200; ++i) {
            std::string source;
            const auto count = random() % 24;
            for (std::size_t j = 0; j < count; ++j) source += tokens[random() % tokens.size()];
            expect_inline_matches_and_tree(source);
        }
    }});

    tests.push_back({"long unmatched inline delimiters remain raw text", [] {
        const std::string source(32000, '[');
        const auto result = almo::Parser{}.parse(source);
        expect_true(result.ast.nodes_byclass("InlineUrl").empty(), "Unclosed links must remain raw text");
        expect_eq(result.ast.to_html(), source, "All unmatched openers must survive");
    }});
}

}  // namespace almo_test
