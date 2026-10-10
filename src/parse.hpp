#pragma once

#include <array>
#include <span>
#include <utility>

#include "interfaces/parse.hpp"
#include "ast.hpp"
#include "ast_view.hpp"
#include "syntax_all.hpp"

namespace almo {

inline void InlineParser::process(const std::string& source, ASTNode& ast) {
    if (source.empty()) return;
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

    // Keep each syntax's next match in original-source coordinates. Only
    // matches consumed by the selected syntax need another search; an absent
    // match stays absent in every suffix of these context-free patterns.
    const std::string_view input(source);
    std::array<std::optional<InlineMatch>, syntaxes.size()> candidates;
    for (std::size_t i = 0; i < syntaxes.size(); ++i)
        candidates[i] = syntaxes[i]->find(input);
    std::size_t cursor = 0;
    while (cursor < input.size()) {
        auto selected = syntaxes.size();
        for (std::size_t i = 0; i < syntaxes.size(); ++i) {
            auto& candidate = candidates[i];
            if (candidate && candidate->range.begin < cursor) {
                candidate = syntaxes[i]->find(input.substr(cursor));
                if (candidate) {
                    candidate->range.begin += cursor;
                    candidate->range.end += cursor;
                }
            }
            if (candidate && (selected == syntaxes.size() ||
                              candidate->range.begin < candidates[selected]->range.begin))
                selected = i;
        }
        if (selected == syntaxes.size()) {
            ast.pushback_child(std::make_shared<RawText>(std::string(input.substr(cursor))));
            break;
        }
        const auto& matched = *candidates[selected];
        if (matched.range.begin > cursor)
            ast.pushback_child(std::make_shared<RawText>(
                std::string(input.substr(cursor, matched.range.begin - cursor))));
        syntaxes[selected]->parse(matched, ast);
        cursor = matched.range.end;
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

// Every unit ends with no open block or pending inline text. A paragraph may
// produce several root nodes, so AST child indices alone are not boundaries.
struct ParseUnit {
    std::size_t begin_row, end_row, inspected_end;
    bool inspected_eof;
    SourceRange range;
    std::vector<std::shared_ptr<ASTNode>> nodes;
    std::vector<Diagnostic> diagnostics;
};

inline ParseUnit parse_next(Reader& reader) {
    reader.inspected_end = reader.row;
    reader.inspected_eof = false;
    const auto begin_row = static_cast<std::size_t>(reader.row);
    const auto begin = reader.offset();
    const auto diagnostic_begin = reader.diagnostics ? reader.diagnostics->size() : 0;
    Markdown ast;
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
    ParseUnit unit{begin_row, static_cast<std::size_t>(reader.row),
                   reader.inspected_end, reader.inspected_eof,
                   {begin, reader.offset()}, std::move(ast.childs), {}};
    if (reader.diagnostics)
        unit.diagnostics.assign(reader.diagnostics->begin() + diagnostic_begin,
                                reader.diagnostics->end());
    return unit;
}

inline void parse_into(Reader& reader, ASTNode& ast) {
    while (static_cast<std::size_t>(reader.row) < reader.lines.size()) {
        auto unit = parse_next(reader);
        ast.childs.insert(ast.childs.end(), unit.nodes.begin(), unit.nodes.end());
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

struct ParseUpdate {
    // Consumed ranges in the NEW source, including any reparsed boundary units.
    std::vector<SourceRange> reparsed_ranges;
    // All diagnostics for the new revision, including reused units.
    std::vector<Diagnostic> diagnostics;
};

class IncrementalParser {
public:
    explicit IncrementalParser(std::string source = "");
    ~IncrementalParser() = default;
    IncrementalParser(IncrementalParser&&) noexcept = default;
    IncrementalParser& operator=(IncrementalParser&&) noexcept = default;
    IncrementalParser(const IncrementalParser&) = delete;
    IncrementalParser& operator=(const IncrementalParser&) = delete;

    [[nodiscard]] std::string_view source() const { return state_->source; }
    [[nodiscard]] ASTView ast() const { return ASTView(state_->ast); }
    [[nodiscard]] std::span<const Diagnostic> diagnostics() const { return state_->diagnostics; }
    [[nodiscard]] Markdown clone_ast() const {
        return *std::static_pointer_cast<Markdown>(state_->ast.clone());
    }

    // Edits are sorted, nonoverlapping, and use the source before this call.
    // Equal starting offsets are rejected; combine same-position insertions.
    // Invalid ranges/UTF-8 boundaries throw invalid_argument. Parse errors and
    // invalid edits leave the entire parser state unchanged.
    ParseUpdate apply_edits(std::span<const TextEdit> edits);
    ParseUpdate set_source(std::string source);

private:
    struct State {
        std::string source;
        Markdown ast;
        std::vector<detail::ParseUnit> units;
        std::vector<Diagnostic> diagnostics;
        explicit State(std::string text) : source(std::move(text)) {}
        void append(detail::ParseUnit unit) {
            ast.childs.insert(ast.childs.end(), unit.nodes.begin(), unit.nodes.end());
            diagnostics.insert(diagnostics.end(), unit.diagnostics.begin(), unit.diagnostics.end());
            units.push_back(std::move(unit));
        }
    };
    std::unique_ptr<State> state_;
};

inline IncrementalParser::IncrementalParser(std::string source)
    : state_(std::make_unique<State>(std::move(source))) {
    std::vector<Diagnostic> diagnostics;
    auto reader = Reader::from_source(state_->source, &diagnostics);
    while (static_cast<std::size_t>(reader.row) < reader.lines.size())
        state_->append(detail::parse_next(reader));
}

inline ParseUpdate IncrementalParser::set_source(std::string source) {
    if (source == state_->source) return {{}, state_->diagnostics};

    auto next = std::make_unique<State>(std::move(source));
    std::vector<Diagnostic> diagnostics;
    auto reader = Reader::from_source(next->source, &diagnostics);
    auto previous = Reader::from_source(state_->source);
    const auto& old_lines = previous.lines;
    const auto& new_lines = reader.lines;
    const auto same_line = [](const SourceLine& a, const SourceLine& b) {
        // Include whether a newline follows the line; EOF edits depend on it.
        return a.text == b.text && a.end - a.begin == b.end - b.begin;
    };
    std::size_t prefix = 0;
    while (prefix < old_lines.size() && prefix < new_lines.size() &&
           same_line(old_lines[prefix], new_lines[prefix])) ++prefix;
    std::size_t old_suffix = old_lines.size(), new_suffix = new_lines.size();
    while (old_suffix > prefix && new_suffix > prefix &&
           same_line(old_lines[old_suffix - 1], new_lines[new_suffix - 1])) {
        --old_suffix;
        --new_suffix;
    }

    // Reuse only units whose consumed AND inspected rows are unchanged.
    std::size_t first = 0;
    while (first < state_->units.size()) {
        const auto& unit = state_->units[first];
        if (unit.end_row > prefix || unit.inspected_end > prefix || unit.inspected_eof) break;
        next->append(unit);
        reader.row = static_cast<int>(unit.end_row);
        ++first;
    }
    const auto reparse_begin = reader.offset();
    const auto reparse_begin_row = reader.row;
    std::size_t reuse = first;
    while (static_cast<std::size_t>(reader.row) < new_lines.size()) {
        const auto row = static_cast<std::size_t>(reader.row);
        if (row >= new_suffix) {
            const auto old_row = old_suffix + (row - new_suffix);
            while (reuse < state_->units.size() && state_->units[reuse].begin_row < old_row) ++reuse;
            if (reuse < state_->units.size() && state_->units[reuse].begin_row == old_row) break;
        }
        next->append(detail::parse_next(reader));
    }
    ParseUpdate update;
    if (reader.row != reparse_begin_row)
        update.reparsed_ranges.push_back({reparse_begin, reader.offset()});

    if (static_cast<std::size_t>(reader.row) < new_lines.size()) {
        const auto old_offset = old_lines[old_suffix].begin;
        const auto new_offset = new_lines[new_suffix].begin;
        for (; reuse < state_->units.size(); ++reuse) {
            auto unit = state_->units[reuse];
            unit.begin_row = new_suffix + (unit.begin_row - old_suffix);
            unit.end_row = new_suffix + (unit.end_row - old_suffix);
            unit.inspected_end = new_suffix + (unit.inspected_end - old_suffix);
            unit.range = {new_offset + (unit.range.begin - old_offset),
                          new_offset + (unit.range.end - old_offset)};
            for (auto& diagnostic : unit.diagnostics) {
                diagnostic.range = {new_offset + (diagnostic.range.begin - old_offset),
                                    new_offset + (diagnostic.range.end - old_offset)};
            }
            next->append(std::move(unit));
        }
    }
    update.diagnostics = next->diagnostics;
    // Commit once parsing and all allocations have succeeded.
    state_.swap(next);
    return update;
}

inline ParseUpdate IncrementalParser::apply_edits(std::span<const TextEdit> edits) {
    const auto& old = state_->source;
    auto is_boundary = [&](std::size_t offset) {
        return offset == old.size() || (static_cast<unsigned char>(old[offset]) & 0xc0) != 0x80;
    };
    std::size_t previous_begin = 0, previous_end = 0;
    bool first = true;
    for (const auto& edit : edits) {
        if (edit.range.begin > edit.range.end || edit.range.end > old.size() ||
            (!first && (edit.range.begin < previous_end || edit.range.begin == previous_begin)) ||
            !is_boundary(edit.range.begin) || !is_boundary(edit.range.end))
            throw std::invalid_argument("Edits must have ordered, nonoverlapping ranges at UTF-8 boundaries");
        first = false;
        previous_begin = edit.range.begin;
        previous_end = edit.range.end;
    }
    if (edits.empty()) return {{}, state_->diagnostics};
    std::string next;
    std::size_t cursor = 0;
    for (const auto& edit : edits) {
        next.append(old, cursor, edit.range.begin - cursor);
        next += edit.replacement;
        cursor = edit.range.end;
    }
    next.append(old, cursor, std::string::npos);
    return set_source(std::move(next));
}

}  // namespace almo
