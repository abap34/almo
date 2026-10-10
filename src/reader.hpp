#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "source.hpp"

namespace almo {

// A line can be a slice of a quote or div and still refer to the outer source.
struct SourceLine {
    std::string_view text;
    std::size_t begin;
    std::size_t end;  // includes the original line terminator, when present
};

struct Reader {
    std::vector<SourceLine> lines;
    int row = 0, col = 0;
    bool eof_read_flg = false;
    std::vector<Diagnostic>* diagnostics = nullptr;

    // Compatibility constructor. Copies are owned together so views stay valid.
    explicit Reader(const std::vector<std::string>& input)
        : owned_lines_(std::make_shared<std::vector<std::string>>(input)) {
        std::size_t offset = 0;
        for (std::size_t i = 0; i < input.size(); ++i) {
            const auto& text = (*owned_lines_)[i];
            auto end = offset + text.size() + (i + 1 < input.size() ? 1 : 0);
            lines.push_back({text, offset, end});
            offset = end;
        }
        eof_offset_ = offset;
    }

    static Reader from_source(std::string_view source,
                              std::vector<Diagnostic>* diagnostics = nullptr) {
        Reader read(std::vector<std::string>{});
        std::size_t begin = 0;
        for (;;) {
            const auto newline = source.find('\n', begin);
            const auto end = newline == source.npos ? source.size() : newline;
            read.lines.push_back({source.substr(begin, end - begin), begin,
                                  end + (newline != source.npos ? 1 : 0)});
            if (newline == source.npos) break;
            begin = end + 1;
        }
        read.eof_offset_ = source.size();
        read.diagnostics = diagnostics;
        return read;
    }

    Reader nested(std::vector<SourceLine> input) const {
        Reader read(std::vector<std::string>{});
        read.lines = std::move(input);
        read.owned_lines_ = owned_lines_;
        read.diagnostics = diagnostics;
        read.eof_offset_ = read.lines.empty() ? offset() : read.lines.back().end;
        return read;
    }

    bool is_line_begin() const { return col == 0; }
    bool is_line_end() const { return col == static_cast<int>(line().text.size()); }
    bool is_eof() const { return static_cast<std::size_t>(row) == lines.size(); }
    const SourceLine& line() const {
        assert(static_cast<std::size_t>(row) < lines.size());
        return lines[row];
    }
    std::string get_row() const { return std::string(line().text); }
    std::string get_rest_row() const { return std::string(line().text.substr(col)); }
    std::size_t offset() const {
        return static_cast<std::size_t>(row) < lines.size()
                   ? lines[row].begin + col : eof_offset_;
    }
    SourceRange range() const {
        return static_cast<std::size_t>(row) < lines.size()
                   ? SourceRange{offset(), lines[row].begin + lines[row].text.size()}
                   : SourceRange{eof_offset_, eof_offset_};
    }
    template<class Error>
    Error error(std::string message, SourceRange range) const {
        Error result(std::move(message));
        result.locate(range);
        return result;
    }
    void warn(std::string message) const {
        if (diagnostics)
            diagnostics->push_back({DiagnosticSeverity::Warning, range(), std::move(message)});
    }
    void move_next_line() {
        assert(static_cast<std::size_t>(row) < lines.size());
        ++row;
        col = 0;
    }
    void move_next_char(int n) {
        assert(n >= 0 && n <= static_cast<int>(line().text.size()) - col);
        col += n;
    }
    void read_eof() { assert(!eof_read_flg); eof_read_flg = true; }
    std::string near() const {
        std::string result;
        if (row > 0) result = std::string(lines[row - 1].text) + "\n";
        if (static_cast<std::size_t>(row) < lines.size()) result += lines[row].text;
        return result;
    }

private:
    std::shared_ptr<std::vector<std::string>> owned_lines_;
    std::size_t eof_offset_ = 0;
};

}  // namespace almo
