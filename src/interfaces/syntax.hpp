#pragma once

#include <optional>
#include <regex>
#include <string_view>
#include "../reader.hpp"
#include "../source.hpp"
#include "ast.hpp"

namespace almo {

struct BlockMatch {
    std::vector<std::string> captures;
};
inline std::optional<BlockMatch> block_match(bool matched) {
    return matched ? std::optional<BlockMatch>(BlockMatch{}) : std::nullopt;
}

struct BlockSyntax {
    virtual ~BlockSyntax() = default;
    virtual std::optional<BlockMatch> match(const Reader& read) const = 0;
    virtual void parse(Reader& read, ASTNode& ast, const BlockMatch& match) const = 0;
};

struct InlineMatch {
    SourceRange range;  // offsets relative to the inline input
    std::vector<std::string> captures;
};

struct InlineSyntax {
    virtual ~InlineSyntax() = default;
    virtual std::optional<InlineMatch> find(std::string_view source) const = 0;
    virtual void parse(const InlineMatch& match, ASTNode& ast) const = 0;
};

inline std::optional<InlineMatch> find_inline(std::string_view source,
                                             const std::regex& pattern) {
    if (source.empty()) return std::nullopt;
    std::match_results<std::string_view::const_iterator> found;
    if (!std::regex_search(source.begin(), source.end(), found, pattern))
        return std::nullopt;
    auto begin = static_cast<std::size_t>(found.position());
    InlineMatch result{{begin, begin + static_cast<std::size_t>(found.length())}, {}};
    for (std::size_t i = 1; i < found.size(); ++i)
        result.captures.push_back(found[i].str());
    return result;
}

}  // namespace almo
