#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineItalic : public ASTNode {
   public:
    InlineItalic() { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"italic\"><i>" + concatenated_childs_html() +
               "</i></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "InlineItalic"; }
};

struct InlineItalicSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::DelimitedMatch match;
        if (inline_match::find_delimited(str, "*", "*", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::DelimitedMatch match;
        inline_match::find_delimited(str, "*", "*", match);
        InlineParser::process(match.prefix, ast);
        InlineItalic node;
        InlineParser::process(match.content, node);
        ast.pushback_child(std::make_shared<InlineItalic>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
