#pragma once

#include <array>
#include <utility>

#include "interfaces/parse.hpp"
#include "ast.hpp"
#include "syntax_all.hpp"

namespace almo {

inline void InlineParser::process(const std::string& source, ASTNode& ast) {
    static const InlineMathSyntax math;
    static const InlineOverlineSyntax overline;
    static const InlineStrongSyntax strong;
    static const InlineItalicSyntax italic;
    static const InlineCodeBlockSyntax code;
    static const InlineImageSyntax image;
    static const InlineUrlSyntax url;
    static const InlineFootnoteReferenceSyntax footnote;
    static const std::array<const InlineSyntax*, 8> syntaxes{
        &math, &overline, &strong, &italic, &code, &image, &url, &footnote};

    std::string_view rest(source);
    while (!rest.empty()) {
        const InlineSyntax* selected = nullptr;
        std::optional<InlineMatch> matched;
        for (const auto* syntax : syntaxes) {
            auto candidate = syntax->find(rest);
            if (candidate && (!matched || candidate->range.begin < matched->range.begin)) {
                selected = syntax;
                matched = std::move(candidate);
            }
        }
        if (!matched) {
            ast.pushback_child(std::make_shared<RawText>(std::string(rest)));
            break;
        }
        if (matched->range.begin)
            ast.pushback_child(std::make_shared<RawText>(std::string(rest.substr(0, matched->range.begin))));
        selected->parse(*matched, ast);
        rest.remove_prefix(matched->range.end);
    }
}

namespace detail {

struct BlockSelection {
    const BlockSyntax* syntax;
    BlockMatch match;
};

inline std::optional<BlockSelection> match_block(const Reader& reader) {
    static const NewLineSyntax newline;
    static const JudgeSyntax judge;
    static const ExecutableCodeBlockSyntax executable;
    static const LoadLibSyntax loadlib;
    static const DivBlockSyntax div;
    static const QuoteSyntax quote;
    static const ListBlockSyntax list;
    static const EnumerateBlockSyntax enumeration;
    static const HeaderSyntax header;
    static const CodeBlockSyntax code;
    static const MathBlockSyntax math;
    static const HorizontalLineSyntax rule;
    static const TableSyntax table;
    static const FootnoteDefinitionSyntax footnote;
    static const std::array<const BlockSyntax*, 14> syntaxes{
        &newline, &judge, &executable, &loadlib, &div, &quote, &list,
        &enumeration, &header, &code, &math, &rule, &table, &footnote};
    for (const auto* syntax : syntaxes)
        if (auto match = syntax->match(reader))
            return BlockSelection{syntax, std::move(*match)};
    return std::nullopt;
}

inline void parse_into(Reader& reader, ASTNode& ast) {
    while (static_cast<std::size_t>(reader.row) < reader.lines.size()) {
        try {
            auto selected = match_block(reader);
            if (selected) {
                selected->syntax->parse(reader, ast, selected->match);
            } else {
                std::string paragraph;
                do {
                    if (!paragraph.empty()) paragraph += ' ';
                    paragraph += reader.get_row();
                    reader.move_next_line();
                } while (!reader.is_eof() && !match_block(reader));
                InlineParser::process(paragraph, ast);
            }
        } catch (ParseException& error) {
            error.locate(reader.range());
            throw;
        }
    }
    reader.eof_read_flg = true;
}

}  // namespace detail

inline void MarkdownParser::process(ASTNode& ast) {
    if (reader.eof_read_flg) return;
    std::vector<Diagnostic> diagnostics;
    auto* original = reader.diagnostics;
    if (!original) reader.diagnostics = &diagnostics;
    try {
        detail::parse_into(reader, ast);
    } catch (...) {
        reader.diagnostics = original;
        throw;
    }
    reader.diagnostics = original;
    for (const auto& diagnostic : diagnostics)
        std::cerr << "Warning: " << diagnostic.message << " ["
                  << diagnostic.range.begin << ", " << diagnostic.range.end << ")\n";
}

struct ParseResult {
    Markdown ast;
    std::vector<Diagnostic> diagnostics;
};

class Parser {
public:
    // Source is borrowed only for this call. The result owns all parsed text.
    [[nodiscard]] ParseResult parse(std::string_view source) const {
        ParseResult result;
        auto reader = Reader::from_source(source, &result.diagnostics);
        detail::parse_into(reader, result.ast);
        return result;
    }
};

}  // namespace almo
