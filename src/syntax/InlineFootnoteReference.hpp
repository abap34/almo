#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {
struct InlineFootnoteReference : public ASTNode {
   private:
    std::string symbol;

   public:
    InlineFootnoteReference(std::string _symbol) : symbol(_symbol) {
        set_uuid();
    }

    std::string to_html() const override {
        return "<span class=\"footnote-ref\"> <sup id=\"ref_" + symbol +
               "\"><a href=\"#note_" + symbol + "\">[" + symbol +
               "]</a></sup> </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"symbol", symbol}};
    }
    std::string get_classname() const override {
        return "InlineFootnoteReference";
    }
};
struct InlineFootnoteReferenceSyntax : public InlineSyntax {
    static inline const std::regex rex = std::regex(R"((.*?)\[\^(.*?)\](.*))");
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
        std::string symbol = sm.format("$2");
        std::string suffix = sm.format("$3");
        InlineParser::process(prefix, ast);
        InlineFootnoteReference node(symbol);
        ast.pushback_child(std::make_shared<InlineFootnoteReference>(node));
        InlineParser::process(suffix, ast);
    }
};
}  // namespace almo