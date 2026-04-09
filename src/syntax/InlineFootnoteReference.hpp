#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "inline_match_utils.hpp"

namespace almo {
struct InlineFootnoteReference : public ASTNode {
   private:
    std::string symbol;

   public:
    InlineFootnoteReference(std::string_view _symbol) : symbol(_symbol) {
        set_uuid();
    }

    std::string to_html() const override {
        // This is correct.
        // `label_` is setted in `FootnoteDefinition` (which we have to jump to)
        std::string label = "ref_" + symbol;
        std::string jump_to = "label_" + symbol;

        return "<span class=\"footnote-ref\"><sup id=\"" + label +
               "\"><a href=\"#" + jump_to + "\">[" + symbol +
               "]</a></sup></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"symbol", symbol}};
    }
    std::string get_classname() const override {
        return "InlineFootnoteReference";
    }
};
struct InlineFootnoteReferenceSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::DelimitedMatch match;
        if (inline_match::find_delimited(str, "[^", "]", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::DelimitedMatch match;
        inline_match::find_delimited(str, "[^", "]", match);
        InlineParser::process(match.prefix, ast);
        InlineFootnoteReference node(match.content);
        ast.pushback_child(std::make_shared<InlineFootnoteReference>(node));
        InlineParser::process(match.suffix, ast);
    }
};
}  // namespace almo
