#pragma once

#include "../new_ast.hpp"
#include "../new_reader.hpp"
#include "../new_syntax.hpp"
#include "../new_parser.hpp"
#include "RawText.hpp"

namespace almo::feature {

struct InlineMath : public ASTNode {

    std::string expr;
    InlineMath (std::string _expr) : expr(_expr) {
        set_uuid();
    }

    // mathjax の　インライン数式用に \( \) で囲む
    std::string to_html() const override {
        return "<span class=\"math-inline\"> \\( " + expr + " \\) </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
            {"expr", expr}
        };
    }
    std::string get_classname() const override {
        return "InlineMath";
    }
};

struct InlineMath_syntax : public InlineSyntax {
    bool operator()(const std::string &str) const override {
        const std::regex rex(R"((.*)\$(.*)\$(.*))");
        return std::regex_match(str, rex);
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        const std::regex rex(R"((.*)\$(.*)\$(.*))");
        std::smatch sm;
        std::regex_search(str, sm, rex);
        std::string prefix = sm.format("$1");
        std::string expr = sm.format("$2");
        std::string suffix = sm.format("$3");
        InlineParser::process(prefix, ast);
        InlineMath node(expr);
        ast.add_child(std::make_shared<InlineMath>(node));
        InlineParser::process(suffix, ast);
    }
};

} // namespace almo