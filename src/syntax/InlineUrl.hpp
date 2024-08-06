#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineUrl : public ASTNode {
   private:
    std::string url;
    std::string alt;

   public:
    InlineUrl(std::string url, std::string alt) : url(url), alt(alt) {
        set_uuid();
    }

    std::string to_html() const override {
        return "<url> <a href=\"" + url + "\">" + alt + "</a> </url>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"url", url}, {"alt", alt}};
    }
    std::string get_classname() const override { return "InlineUrl"; }
};

struct InlineUrlSyntax : public InlineSyntax {
    static inline const std::regex rex =
        std::regex(R"((.*?)\[(.*?)\]\((.*?)\)(.*))");
    int operator()(const std::string &str) const override {
        std::smatch sm;
        if (std::regex_search(str, sm, rex)) {
            return sm.position(2) - 1;
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        std::smatch sm;
        std::regex_search(str, sm, rex);
        std::string prefix = sm.format("$1");
        std::string alt = sm.format("$2");
        std::string url = sm.format("$3");
        std::string suffix = sm.format("$4");
        InlineParser::process(prefix, ast);
        InlineUrl node(url, alt);
        ast.pushback_child(std::make_shared<InlineUrl>(node));
        InlineParser::process(suffix, ast);
    }
};

}  // namespace almo