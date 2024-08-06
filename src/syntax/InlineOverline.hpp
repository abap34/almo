#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct InlineOverline : public ASTNode {
   public:
    InlineOverline() { set_uuid(); }
    std::string to_html() const override {
        return "<span class=\"overline\"> <s>" + concatenated_childs_html() +
               "</s> </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "InlineOverline"; }
};

struct InlineOverlineSyntax : public InlineSyntax {
    static inline const std::regex rex =
        std::regex(R"((.*?)\~\~(.*?)\~\~(.*))");
    int operator()(const std::string &str) const override {
        std::smatch sm;
        if (std::regex_search(str, sm, rex)) {
            return sm.position(2) - 2;
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        std::smatch sm;
        std::regex_search(str, sm, rex);
        std::string prefix = sm.format("$1");
        std::string content = sm.format("$2");
        std::string suffix = sm.format("$3");
        InlineParser::process(prefix, ast);
        InlineOverline node;
        InlineParser::process(content, node);
        ast.pushback_child(std::make_shared<InlineOverline>(node));
        InlineParser::process(suffix, ast);
    }
};

}  // namespace almo