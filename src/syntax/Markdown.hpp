#pragma once

#include"../interfaces/ast.hpp"
#include"../interfaces/parse.hpp"
#include"../interfaces/syntax.hpp"

namespace almo {

struct Markdown : public ASTNode {
  public:
    Markdown () {
        set_uuid();
    }

    std::string to_html() const override {
        return concatenated_childs_html();
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
        };
    }
    std::string get_classname() const override {
        return "Markdown";
    }
};

} // namespace almo