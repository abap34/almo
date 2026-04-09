#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineStrong : public ASTNode {
   public:
    InlineStrong() { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"strong\"><strong>" + concatenated_childs_html() +
               "</strong></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "InlineStrong"; }
};

struct InlineStrongSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::DelimitedMatch match;
        if (inline_match::find_delimited(str, "**", "**", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::DelimitedMatch match;
        inline_match::find_delimited(str, "**", "**", match);
        InlineParser::process(match.prefix, ast);
        InlineStrong node;
        InlineParser::process(match.content, node);
        ast.pushback_child(std::make_shared<InlineStrong>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
