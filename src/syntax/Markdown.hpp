#pragma once

#include "../new_ast.hpp"
#include "../new_reader.hpp"
#include "../new_syntax.hpp"

namespace almo::feature {

struct Markdown : public ASTNode {

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