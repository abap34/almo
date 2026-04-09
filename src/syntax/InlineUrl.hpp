#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineUrl : public ASTNode {
   private:
    std::string url;
    std::string alt;

   public:
    InlineUrl(std::string_view url, std::string_view alt) : url(url), alt(alt) {
        set_uuid();
    }

    std::string to_html() const override {
        return "<url><a href=\"" + url + "\">" + alt + "</a></url>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"url", url}, {"alt", alt}};
    }
    std::string get_classname() const override { return "InlineUrl"; }
};

struct InlineUrlSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::LinkMatch match;
        if (inline_match::find_link(str, "[", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::LinkMatch match;
        inline_match::find_link(str, "[", match);
        InlineParser::process(match.prefix, ast);
        InlineUrl node(match.target, match.label);
        ast.pushback_child(std::make_shared<InlineUrl>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
