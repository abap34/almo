#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineCodeBlock : public ASTNode {
   private:
    std::string code;

   public:
    InlineCodeBlock(std::string code) : code(code) { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"inline-code\"> <code>" + escape_for_html(code) +
               "</code> </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"code", code}};
    }
    std::string get_classname() const override { return "InlineCodeBlock"; }
};

struct InlineCodeBlockSyntax : public InlineSyntax {
    static inline const std::regex rex = std::regex(R"((.*?)\`(.*?)\`(.*))");
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
        std::string code = sm.format("$2");
        std::string suffix = sm.format("$3");
        InlineParser::process(prefix, ast);
        InlineCodeBlock node(code);
        ast.pushback_child(std::make_shared<InlineCodeBlock>(node));
        InlineParser::process(suffix, ast);
    }
};

}  // namespace almo