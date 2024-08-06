#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct Markdown : public ASTNode {
   public:
    Markdown() { set_uuid(); }

    std::string to_html() const override { return concatenated_childs_html(); }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "Markdown"; }

    std::string to_dot() const {
        std::string childs_dot = concatenated_childs_dot();
        std::string dot =
            "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + childs_dot + "}";
        return dot;
    }
};

}  // namespace almo