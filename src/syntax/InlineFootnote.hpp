#pragma once

#include"../interfaces/ast.hpp"
#include"../interfaces/parse.hpp"
#include"../interfaces/syntax.hpp"
#include"../utils.hpp"

namespace almo{
struct InlineFootnote : public ASTNode{
  private:
    std::string expr;
    
  public:
    InlineFootnote (std::string _expr) : expr(_expr){
        set_uuid();
    }

    std::string to_html() const override {
        return "<sup id=\"ref_" + expr + "\"><a href=\"#note_" + expr +"\">[" + expr + "]</a></sup>"; 
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
            {"expr", expr}
        };
    }
    std::string get_classname() const override {
        return "InlineFootnote";
    }
};
struct InlineFootnoteSyntax : public InlineSyntax {
    static inline const std::regex rex = std::regex(R"((.*?)\[\^(.*?)\](.*?))");
    int operator()(const std::string &str) const override {
        std::smatch sm;
        if (std::regex_search(str, sm, rex)){
            return sm.position(2) - 1;
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        std::smatch sm;
        std::regex_search(str, sm, rex);
        std::string prefix = sm.format("$1");
        std::string expr = sm.format("$2");
        std::string suffix = sm.format("$3");
        InlineParser::process(prefix, ast);
        InlineFootnote node(expr);
        ast.add_child(std::make_shared<InlineFootnote>(node));
        InlineParser::process(suffix, ast);
    }
};
}