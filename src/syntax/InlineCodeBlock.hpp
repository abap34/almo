#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineCodeBlock : public ASTNode {
   private:
    std::string code;

   public:
    InlineCodeBlock(std::string_view code) : code(code) { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"inline-code\"><code>" + escape(code, EscapeFormat::HTML) +
               "</code></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"code", code}};
    }
    std::string get_classname() const override { return "InlineCodeBlock"; }
};

struct InlineCodeBlockSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::DelimitedMatch match;
        if (inline_match::find_delimited(str, "`", "`", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::DelimitedMatch match;
        inline_match::find_delimited(str, "`", "`", match);
        InlineParser::process(match.prefix, ast);
        InlineCodeBlock node(match.content);
        ast.pushback_child(std::make_shared<InlineCodeBlock>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
