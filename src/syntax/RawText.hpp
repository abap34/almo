#pragma once

#include <limits>

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct RawText : public ASTNode {
   private:
    std::string content;

   public:
    RawText(std::string _content) : content(_content) { set_uuid(); }

    std::string to_html() const override { return content; }

    std::map<std::string, std::string> get_properties() const override {
        return {{"content", content}};
    }
    std::string get_classname() const override { return "RawText"; }
};

struct RawTextSyntax : InlineSyntax {
    // All string matches Rawtext syntax
    // but matches infinity position
    // because Rawtext syntax is weakest.
    // If the string matches other Inline syntax
    // RawText does not match it.
    int operator()(const std::string &str) const override {
        return std::numeric_limits<int>::max();
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        RawText node(str);
        ast.pushback_child(std::make_shared<RawText>(node));
    }
};

}  // namespace almo