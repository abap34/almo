#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineMath : public ASTNode {
   private:
    std::string expr;

   public:
    InlineMath(std::string_view _expr) : expr(_expr) { set_uuid(); }

    // mathjax の　インライン数式用に \( \) で囲む
    std::string to_html() const override {
        return "<span class=\"math-inline\"> \\( " + expr + " \\) </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"expr", expr}};
    }
    std::string get_classname() const override { return "InlineMath"; }
};

struct InlineMathSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::DelimitedMatch match;
        if (inline_match::find_delimited(str, "$", "$", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::DelimitedMatch match;
        inline_match::find_delimited(str, "$", "$", match);
        InlineParser::process(match.prefix, ast);
        InlineMath node(match.content);
        ast.pushback_child(std::make_shared<InlineMath>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
