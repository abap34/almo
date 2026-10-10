#pragma once

#include <string>
#include "../reader.hpp"
#include "ast.hpp"

namespace almo {

struct InlineParser {
    static void process(const std::string& source, ASTNode& ast);
};

namespace detail {
// Used by both full/incremental parsing and nested quote/div contents.
void parse_into(Reader& reader, ASTNode& ast);
}

// Compatibility API. New code should use Parser::parse(source).
struct MarkdownParser {
    Reader reader;
    explicit MarkdownParser(const std::vector<std::string>& lines) : reader(lines) {}
    void process(ASTNode& ast);
};

}  // namespace almo
