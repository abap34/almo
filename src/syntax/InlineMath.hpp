#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineMath : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineMath>();
    }

   private:
    std::string expr;

   public:
    InlineMath(std::string _expr) : expr(_expr) { set_uuid(); }

    // mathjax の　インライン数式用に \( \) で囲む
    std::string to_html() const override {
        return "<span class=\"math-inline\"> \\( " + expr + " \\) </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"expr", expr}};
    }
    std::string get_classname() const override { return "InlineMath"; }
};

struct InlineMathSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\$(.*?)\$)"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return find_inline(source, rex);
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineMath>(matched.captures[0]));
    }
};

}  // namespace almo
