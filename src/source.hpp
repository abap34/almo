#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace almo {

// UTF-8 byte offsets in the original source, [begin, end).
struct SourceRange {
    std::size_t begin = 0;
    std::size_t end = 0;
    bool operator==(const SourceRange&) const = default;
};

struct TextEdit {
    SourceRange range;
    std::string replacement;
};

enum class DiagnosticSeverity { Warning, Error };

struct Diagnostic {
    DiagnosticSeverity severity;
    SourceRange range;
    std::string message;
    bool operator==(const Diagnostic&) const = default;
};

// The legacy SyntaxError and ParseError both derive from this class.
class ParseException : public std::runtime_error {
public:
    ParseException(std::string message, const std::string& prefix)
        : std::runtime_error(prefix + message),
          diagnostic_{DiagnosticSeverity::Error, {}, std::move(message)} {}

    const Diagnostic& diagnostic() const noexcept { return diagnostic_; }
    bool has_source_range() const noexcept { return located_; }
    void locate(SourceRange range) noexcept {
        if (!located_) {
            diagnostic_.range = range;
            located_ = true;
        }
    }

private:
    Diagnostic diagnostic_;
    bool located_ = false;
};

}  // namespace almo
